/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "server/race/magic/MagicSelector.hpp"

#include <algorithm>
#include <numeric>
#include <spdlog/spdlog.h>

namespace server::magic
{

MagicSelector::MagicSelector(const registry::MagicRegistry& magicRegistry)
  : _magicRegistry(magicRegistry)
{
}

// ============================================================================
// PUBLIC API
// ============================================================================

registry::Magic::SlotInfo MagicSelector::SelectItem(
  const tracker::RaceTracker::Racer& racer,
  const tracker::RaceTracker::RacerPositionInfo& positionInfo,
  uint32_t totalActiveRacers) const
{
  spdlog::debug("Selecting magic item for racer: rank={}, trackProgress={:.7f}, totalRacers={}",
    positionInfo.rank,
    positionInfo.trackProgress,
    totalActiveRacers);

  // Build context
  RaceContext context = BuildContext(positionInfo, totalActiveRacers);
  spdlog::debug("Context: rank={}, totalPlayers={}, aheadCount={}, isLeading={}, isTeamMode={}",
    context.rank,
    context.totalPlayers,
    context.aheadCount,
    context.isLeading,
    context.isTeamMode);

  // Layer 0: Get the pool based on team mode
  const auto& pool = GetItemPool(racer);

  // Layer 1: Filter pool items by allocation rules (using normalized rank for scaling)
  std::vector<MagicType> validTypes;
  for (uint32_t poolType : pool)
  {
    MagicType type = static_cast<MagicType>(poolType);

    // Skip invalid types
    if (type == MagicType::Invalid || type >= MagicType::MaxTypes)
      continue;

    // Check allocation rules using normalized rank for proper scaling
    const auto& config = MagicConfig::GetInstance();
    const AllocationRule* rule = config.GetAllocationRule(type);

    if (rule && (context.totalPlayers < rule->minPlayerCount ||
                  context.normalizedRank < rule->minRank ||
                  context.normalizedRank > rule->maxRank))
    {
      continue; // Item excluded by allocation rules
    }

    validTypes.push_back(type);
  }

  if (validTypes.empty())
  {
    spdlog::warn("No valid magic types for context: rank={}, totalRacers={}",
      context.rank,
      context.totalPlayers);
    // Fallback: return first item from pool
    if (!pool.empty())
    {
      return _magicRegistry.GetSlotInfo(pool[0]);
    }
    throw std::runtime_error("No magic items available");
  }

  spdlog::debug("Valid types for rank {}: {}", context.rank, [&]()
    {
      std::string result;
      for (MagicType t : validTypes)
      {
        if (!result.empty())
          result += ", ";
        result += std::to_string(ToUnderlying(t));
      }
      return result;
    }());

  // Layer 2: Select group by normalized rank (scales with race size)
  MagicGroup selectedGroup = SelectGroupByRank(context.normalizedRank);
  context.group = selectedGroup;

  spdlog::debug("Rank={}, normalizedRank={}, selectedGroup={}",
    context.rank,
    context.normalizedRank,
    static_cast<int>(selectedGroup));
  spdlog::debug("Selected group for rank {}: {}", context.rank, static_cast<int>(selectedGroup));

  // Layer 3: Filter to only items in the selected group, then apply slot ratios
  const auto& config = MagicConfig::GetInstance();
  std::vector<MagicType> groupTypes;
  for (MagicType type : validTypes)
  {
    MagicGroup typeGroup = config.GetGroupForType(type);
    if (typeGroup == selectedGroup)
    {
      groupTypes.push_back(type);
    }
  }

  spdlog::debug("Group {} types: {}", static_cast<int>(selectedGroup), [&]()
    {
      std::string result;
      for (MagicType t : groupTypes)
      {
        if (!result.empty())
          result += ", ";
        result += std::to_string(ToUnderlying(t));
      }
      return result;
    }());

  // If no types in the selected group, try to select a different group that has items
  if (groupTypes.empty())
  {
    spdlog::debug("No items found for selectedGroup={}, trying alternative groups",
      static_cast<int>(selectedGroup));

    // Try other groups in order of their weight for this normalized rank
    // Get all groups sorted by weight for this normalized rank
    std::vector<std::pair<MagicGroup, uint32_t>> alternativeGroups;
    for (uint32_t g = 1; g < static_cast<uint32_t>(MagicGroup::MaxGroups); ++g)
    {
      MagicGroup group = static_cast<MagicGroup>(g);
      if (group == selectedGroup)
        continue;

      const GroupRatio* ratio = config.GetGroupRatio(group);
      if (ratio && ratio->rankWeights[context.normalizedRank - 1] > 0)
      {
        alternativeGroups.emplace_back(group, ratio->rankWeights[context.normalizedRank - 1]);
      }
    }

    // Sort by weight descending
    std::sort(alternativeGroups.begin(), alternativeGroups.end(), [](const auto& a, const auto& b)
      {
        return a.second > b.second;
      });

    // Try each alternative group until we find one with items
    for (const auto& [altGroup, weight] : alternativeGroups)
    {
      std::vector<MagicType> altGroupTypes;
      for (MagicType type : validTypes)
      {
        if (config.GetGroupForType(type) == altGroup)
        {
          altGroupTypes.push_back(type);
        }
      }

      if (!altGroupTypes.empty())
      {
        groupTypes = altGroupTypes;
        selectedGroup = altGroup;
        context.group = altGroup;
        spdlog::debug("Found items in alternative group={}", static_cast<int>(altGroup));
        break;
      }
    }

    // If still no types found, fall back to all valid types
    if (groupTypes.empty())
    {
      spdlog::warn("No items found in any group for rank={}, falling back to all valid types",
        context.rank);
      groupTypes = validTypes;
      spdlog::debug("Fallback: using all valid types: {}",
        [&]()
        {
          std::string result;
          for (MagicType t : groupTypes)
          {
            if (!result.empty())
              result += ", ";
            result += std::to_string(ToUnderlying(t));
          }
          return result;
        }());
    }
    else
    {
      spdlog::debug("Using alternative group={}, types: {}", static_cast<int>(selectedGroup), [&]()
        {
          std::string result;
          for (MagicType t : groupTypes)
          {
            if (!result.empty())
              result += ", ";
            result += std::to_string(ToUnderlying(t));
          }
          return result;
        }());
    }
  }

  // Apply slot ratios to items in the selected group (using normalized rank)
  std::vector<std::pair<MagicType, uint32_t>> weightedTypes =
    ApplySlotRatios(groupTypes, context.normalizedRank, totalActiveRacers);

  // If all items in the group have 0 weight for this normalized rank, try alternative groups
  if (weightedTypes.empty() && !groupTypes.empty())
  {
    spdlog::debug("All items in group {} have 0 weight for normalized rank {}, trying alternative groups",
      static_cast<int>(selectedGroup),
      context.normalizedRank);

    // Try other groups in order of their weight for this normalized rank
    std::vector<std::pair<MagicGroup, uint32_t>> alternativeGroups;
    for (uint32_t g = 1; g < static_cast<uint32_t>(MagicGroup::MaxGroups); ++g)
    {
      MagicGroup group = static_cast<MagicGroup>(g);
      if (group == selectedGroup)
        continue;

      const GroupRatio* ratio = config.GetGroupRatio(group);
      if (ratio && ratio->rankWeights[context.normalizedRank - 1] > 0)
      {
        alternativeGroups.emplace_back(group, ratio->rankWeights[context.normalizedRank - 1]);
      }
    }

    // Sort by weight descending
    std::sort(alternativeGroups.begin(), alternativeGroups.end(), [](const auto& a, const auto& b)
      {
        return a.second > b.second;
      });

    // Try each alternative group until we find one with non-zero weight items
    for (const auto& [altGroup, weight] : alternativeGroups)
    {
      std::vector<MagicType> altGroupTypes;
      for (MagicType type : validTypes)
      {
        if (config.GetGroupForType(type) == altGroup)
        {
          altGroupTypes.push_back(type);
        }
      }

      std::vector<std::pair<MagicType, uint32_t>> altWeightedTypes =
        ApplySlotRatios(altGroupTypes, context.normalizedRank, totalActiveRacers);

      if (!altWeightedTypes.empty())
      {
        weightedTypes = altWeightedTypes;
        selectedGroup = altGroup;
        context.group = altGroup;
        spdlog::debug("Found items in alternative group={} with non-zero weights", static_cast<int>(altGroup));
        break;
      }
    }

    // If still no types found, fall back to all valid types
    if (weightedTypes.empty())
    {
      spdlog::warn("No items found in any group for normalized rank={} after weight filtering, falling back to all valid types",
        context.normalizedRank);
      weightedTypes = ApplySlotRatios(validTypes, context.normalizedRank, totalActiveRacers);
    }
  }

  // Layer 4: Apply dynamic modifiers
  ApplyDynamicModifiers(weightedTypes, context);

  // Select final type
  MagicType selectedType = SelectFromWeighted(weightedTypes);

  // Get slot info and handle critical chance
  registry::Magic::SlotInfo slotInfo = _magicRegistry.GetSlotInfo(ToUnderlying(selectedType));
  slotInfo = HandleCriticalChance(slotInfo, racer);

  // Get the actual group of the selected item for logging
  MagicGroup actualGroup = config.GetGroupForType(selectedType);

  spdlog::debug("Selected magic item: rank={}/{}, normalized={}, type={}, basicType={}, selectedGroup={}, actualGroup={}",
    context.rank,
    context.totalPlayers,
    context.normalizedRank,
    slotInfo.type,
    slotInfo.basicType,
    static_cast<int>(selectedGroup),
    static_cast<int>(actualGroup));

  return slotInfo;
}

registry::Magic::SlotInfo MagicSelector::SelectItem(
  const tracker::RaceTracker::Racer& racer,
  data::Uid characterUid,
  const std::vector<tracker::RaceTracker::RacerPositionInfo>& racePositions) const
{
  if (racePositions.empty())
  {
    spdlog::warn("No position info available, falling back to random selection");
    // Fallback to simple random
    const auto& pool = GetItemPool(racer);
    if (!pool.empty())
    {
      std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
      uint32_t randomType = pool[dist(_randomDevice)];
      registry::Magic::SlotInfo slotInfo = _magicRegistry.GetSlotInfo(randomType);
      return HandleCriticalChance(slotInfo, racer);
    }
    throw std::runtime_error("No magic items available");
  }

  // Find this racer's position
  auto positionIt = std::find_if(racePositions.begin(), racePositions.end(), [characterUid](const auto& pos)
    {
      return pos.characterUid == characterUid;
    });

  if (positionIt == racePositions.end())
  {
    spdlog::warn("Racer {} not found in position list", characterUid);
    const auto& pool = GetItemPool(racer);
    if (!pool.empty())
    {
      std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
      uint32_t randomType = pool[dist(_randomDevice)];
      registry::Magic::SlotInfo slotInfo = _magicRegistry.GetSlotInfo(randomType);
      return HandleCriticalChance(slotInfo, racer);
    }
    throw std::runtime_error("Racer not found in position list");
  }

  uint32_t totalActiveRacers = static_cast<uint32_t>(racePositions.size());
  return SelectItem(racer, *positionIt, totalActiveRacers);
}

// ============================================================================
// PRIVATE IMPLEMENTATION
// ============================================================================

MagicGroup MagicSelector::SelectGroupByRank(uint32_t rank) const
{
  // Use normalized rank if provided (from RaceContext), otherwise use the raw rank
  // This allows for consistent distribution across different race sizes
  const auto& config = MagicConfig::GetInstance();

  // Use MagicGroupTeamRatio data (Table 355) for weighted group selection
  // This defines the probability percentage for each group at each rank
  // We need to select a group based on these weights

  // Clamp rank to valid range [1, 8]
  if (rank < 1)
    rank = 1;
  if (rank > 8)
    rank = 8;

  // Build weighted group selection based on MagicGroupTeamRatio
  // Only include groups that have non-zero weight for this rank
  std::vector<std::pair<MagicGroup, uint32_t>> groupWeights;

  for (uint32_t g = 1; g < static_cast<uint32_t>(MagicGroup::MaxGroups); ++g)
  {
    MagicGroup group = static_cast<MagicGroup>(g);
    const GroupRatio* ratio = config.GetGroupRatio(group);

    if (ratio && ratio->rankWeights[rank - 1] > 0)
    {
      // Store the weight directly (these are already percentages)
      groupWeights.emplace_back(group, ratio->rankWeights[rank - 1]);
    }
  }

  // If no groups have weights, use fallback
  if (groupWeights.empty())
  {
    // Ranks 1-2: Defensive (Group 2)
    // Ranks 7-8: SpeedUtility (Group 3)
    // Others: Offensive (Group 1)
    if (rank <= 2)
    {
      return MagicGroup::Defensive;
    }
    else if (rank >= 7)
    {
      return MagicGroup::SpeedUtility;
    }
    else
    {
      return MagicGroup::Offensive;
    }
  }

  // Select group using weighted random based on the weights
  return SelectGroupFromWeighted(groupWeights);
}

MagicGroup MagicSelector::SelectGroupFromWeighted(
  const std::vector<std::pair<MagicGroup, uint32_t>>& groupWeights) const
{

  if (groupWeights.empty())
  {
    return MagicGroup::Offensive;
  }

  // Calculate total weight
  uint32_t totalWeight = 0;
  for (const auto& [group, weight] : groupWeights)
  {
    totalWeight += weight;
  }

  if (totalWeight == 0)
  {
    // Uniform selection
    std::uniform_int_distribution<size_t> dist(0, groupWeights.size() - 1);
    return groupWeights[dist(_randomDevice)].first;
  }

  // Build cumulative distribution
  std::vector<uint32_t> cumulativeWeights;
  cumulativeWeights.reserve(groupWeights.size());
  uint32_t runningSum = 0;

  for (const auto& [group, weight] : groupWeights)
  {
    runningSum += weight;
    cumulativeWeights.push_back(runningSum);
  }

  // Select random value
  std::uniform_int_distribution<uint32_t> dist(1, totalWeight);
  uint32_t randomValue = dist(_randomDevice);

  // Find selected group
  for (size_t i = 0; i < cumulativeWeights.size(); ++i)
  {
    if (randomValue <= cumulativeWeights[i])
    {
      return groupWeights[i].first;
    }
  }

  // Fallback to last element
  return groupWeights.back().first;
}

std::vector<std::pair<MagicType, uint32_t>> MagicSelector::ApplySlotRatios(
  const std::vector<MagicType>& types,
  uint32_t rank,
  uint32_t /*totalRacers*/) const
{

  const auto& config = MagicConfig::GetInstance();
  std::vector<std::pair<MagicType, uint32_t>> weightedTypes;

  for (MagicType type : types)
  {
    uint32_t underlyingType = ToUnderlying(type);

    // Get slot ratio for this type
    const SlotRatio* slotRatio = config.GetSlotRatio(type);
    uint32_t rankWeight = 100; // Default weight

    if (slotRatio && rank >= 1 && rank <= 8)
    {
      rankWeight = slotRatio->rankWeights[rank - 1];
    }

    // Handle team assistance items (BufPower=20, BufGauge=22, BufSpeed=24)
    // These have special ratios from MagicGroupTeamAssistanceRatio
    if (underlyingType == 20 || underlyingType == 22 || underlyingType == 24)
    {
      // For team items, check if rank allows them
      // BufPower (20): ranks 7-8 only, 20% each
      // BufSpeed (24): ranks 7-8 only, 80% each
      if (rank >= 7)
      {
        if (underlyingType == 20 || underlyingType == 21)
        {
          rankWeight = 20; // BufPower and critical
        }
        else if (underlyingType == 24 || underlyingType == 25)
        {
          rankWeight = 80; // BufSpeed and critical
        }
        else
        {
          rankWeight = 0; // BufGauge not available
        }
      }
      else
      {
        rankWeight = 0; // Team assistance items only for ranks 7-8
      }
    }

    // Handle offensive items with MagicGroupAttackRatio
    MagicGroup group = config.GetGroupForType(type);
    if (group == MagicGroup::Offensive || group == MagicGroup::RareOffensive)
    {
      // From MagicGroupAttackRatio:
      // Type 2 (FireBall): ranks 2-3 have 15%, rank 4-5 have 10%, rank 6 has 25%
      // Type 14 (DarkFire): ranks 2-3 have 20-17%, rank 4-5 have 10%, rank 6 has 15%
      // Type 16 (Summon): ranks 2-5 have 5-15%, rank 6 has 10%
      // Type 12 (JumpStun): rank 5-6 have 5%

      // Apply additional weighting based on attack type ratios
      // This fine-tunes the offensive items within the group
      // For now, we'll use the slotRatio which already incorporates this
      if (rank >= 1 && rank <= 8)
      {
        // Get attack ratio for this basic type if available
        // But we can add additional modifiers
      }
    }

    // Convert percentage (0-100) to weight
    // If rankWeight is 0, the item is not available for this rank
    if (rankWeight == 0)
    {
      spdlog::debug("Type {} has 0 weight for rank {}, skipping", ToUnderlying(type), rank);
      continue;
    }

    uint32_t finalWeight = rankWeight;

    weightedTypes.emplace_back(type, finalWeight);
  }

  return weightedTypes;
}

void MagicSelector::ApplyDynamicModifiers(
  std::vector<std::pair<MagicType, uint32_t>>& weightedTypes,
  const RaceContext& context) const
{

  const auto& config = MagicConfig::GetInstance();

  for (auto& [type, weight] : weightedTypes)
  {
    if (weight == 0)
      continue;

    MagicGroup group = config.GetGroupForType(type);
    if (group == MagicGroup::Invalid)
      continue;

    // Get team modifier for this group
    const TeamModifier* modifier = config.GetTeamModifier(group, context.isLeading);

    if (modifier && modifier->appliesWhenLeading == context.isLeading)
    {
      // Apply modifier based on how many players are ahead
      int32_t modValue = 0;

      switch (context.aheadCount)
      {
        case 1:
          modValue = modifier->ahead1;
          break;
        case 2:
          modValue = modifier->ahead2;
          break;
        case 3:
          modValue = modifier->ahead3;
          break;
        case 4:
          modValue = modifier->ahead4;
          break;
        default:
          modValue = 0;
          break;
      }

      // Apply modifier as percentage (negative values reduce, positive increase)
      // Convert modValue to a multiplier: -10 = -10%, +10 = +10%
      int32_t adjustedWeight = static_cast<int32_t>(weight) +
                               static_cast<int32_t>((weight * modValue) / 100);

      // Ensure weight doesn't go negative
      weight = static_cast<uint32_t>(std::max(1, adjustedWeight));
    }

    // Special handling for certain groups
    if (context.isLeading && group == MagicGroup::Defensive)
    {
      // Leaders get bonus to defensive items
      weight = static_cast<uint32_t>(weight * 1.2f);
    }
    else if (!context.isLeading && group == MagicGroup::Offensive)
    {
      // Non-leaders get bonus to offensive items
      weight = static_cast<uint32_t>(weight * 1.1f);
    }
  }
}

MagicType MagicSelector::SelectFromWeighted(
  const std::vector<std::pair<MagicType, uint32_t>>& weightedTypes) const
{

  if (weightedTypes.empty())
  {
    throw std::runtime_error("No weighted types to select from");
  }

  // Calculate total weight
  uint32_t totalWeight = std::accumulate(weightedTypes.begin(), weightedTypes.end(), 0u, [](uint32_t sum, const auto& pair)
    {
      return sum + pair.second;
    });

  if (totalWeight == 0)
  {
    // All weights are zero, select uniformly
    std::uniform_int_distribution<size_t> dist(0, weightedTypes.size() - 1);
    return weightedTypes[dist(_randomDevice)].first;
  }

  // Build cumulative distribution
  std::vector<uint32_t> cumulativeWeights;
  cumulativeWeights.reserve(weightedTypes.size());
  uint32_t runningSum = 0;

  for (const auto& [type, weight] : weightedTypes)
  {
    runningSum += weight;
    cumulativeWeights.push_back(runningSum);
  }

  // Select random value
  std::uniform_int_distribution<uint32_t> dist(1, totalWeight);
  uint32_t randomValue = dist(_randomDevice);

  // Find selected type
  for (size_t i = 0; i < cumulativeWeights.size(); ++i)
  {
    if (randomValue <= cumulativeWeights[i])
    {
      return weightedTypes[i].first;
    }
  }

  // Fallback to last element (shouldn't happen)
  return weightedTypes.back().first;
}

uint32_t MagicSelector::NormalizeRank(uint32_t rank, uint32_t totalPlayers) const
{
  // If only 1 player, always use rank 1
  if (totalPlayers <= 1)
  {
    return 1;
  }

  rank = std::clamp(rank, 1u, totalPlayers);

  // The XML tables are 8-position tables, but they should not be stretched
  // linearly for very small races. In a 1v1, the trailing racer is last, but
  // treating it as normalized rank 8 makes it behave like 8th of 8 and gives
  // rank-8 catch-up items such as Booster/HotRodding far too often. Clamp the
  // normalized scale so 2-player races use the rank-2 part of the tables.
  if (totalPlayers == 2)
  {
    return std::clamp(rank, 1u, 2u);
  }

  // Normalize rank from [1, totalPlayers] to [1, 8] for larger races.
  // This keeps 3-player races closer to the official table shape while still
  // preserving first/last scaling for normal 4-8 player races.
  float ratio = static_cast<float>(rank - 1) / static_cast<float>(totalPlayers - 1);
  return std::clamp(1u + static_cast<uint32_t>(ratio * 7.0f), 1u, 8u);
}

RaceContext MagicSelector::BuildContext(
  const tracker::RaceTracker::RacerPositionInfo& positionInfo,
  uint32_t totalActiveRacers) const
{

  RaceContext context;
  context.rank = positionInfo.rank;
  context.totalPlayers = totalActiveRacers;
  context.aheadCount = context.rank - 1; // Number of players ahead
  context.isLeading = (context.rank == 1);
  context.isTeamMode = false; // Will be set based on racer
  context.group = MagicGroup::Invalid;

  // Calculate normalized rank for consistent distribution across race sizes
  context.normalizedRank = NormalizeRank(context.rank, totalActiveRacers);

  return context;
}

const std::vector<uint32_t>& MagicSelector::GetItemPool(
  const tracker::RaceTracker::Racer& racer) const
{

  using Team = tracker::RaceTracker::Racer::Team;

  if (racer.team == Team::Solo)
  {
    return _magicRegistry.GetSoloPool();
  }
  else
  {
    return _magicRegistry.GetTeamPool();
  }
}

registry::Magic::SlotInfo MagicSelector::HandleCriticalChance(
  registry::Magic::SlotInfo slotInfo,
  const tracker::RaceTracker::Racer& racer) const
{

  // Only apply critical chance if there's a critical version available
  if (slotInfo.criticalType == 0)
  {
    return slotInfo;
  }

  uint32_t critChanceBp = _magicRegistry.GetBaseCritChanceBp();

  // Apply stat scaling if available
  if (const auto* scaling = _magicRegistry.GetStatScaling(slotInfo.basicType))
  {
    // Get mount stat value
    uint32_t statValue = 0;
    switch (scaling->stat)
    {
      case registry::Magic::MountStat::Agility:
        statValue = racer.mountStats.agility;
        break;
      case registry::Magic::MountStat::Ambition:
        statValue = racer.mountStats.ambition;
        break;
      case registry::Magic::MountStat::Rush:
        statValue = racer.mountStats.rush;
        break;
      case registry::Magic::MountStat::Endurance:
        statValue = racer.mountStats.endurance;
        break;
      case registry::Magic::MountStat::Courage:
        statValue = racer.mountStats.courage;
        break;
    }

    critChanceBp += scaling->critStepBp * (statValue / 10u);
  }

  // Roll for critical
  if ((rand() % 10000) < static_cast<int>(critChanceBp))
  {
    slotInfo = _magicRegistry.GetSlotInfo(slotInfo.criticalType);
  }

  return slotInfo;
}

} // namespace server::magic
