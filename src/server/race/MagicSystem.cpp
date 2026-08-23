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

#include "server/race/MagicSystem.hpp"
#include "server/system/PotentialSystem.hpp"
#include "server/tracker/RaceTracker.hpp"

#include <libserver/util/Util.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <ranges>

namespace server::race
{

uint32_t MagicSystem::GetMountStatValue(
  const tracker::RaceTracker::Racer::MountStatsSnapshot& stats,
  registry::Magic::MountStat which)
{
  switch (which)
  {
    case registry::Magic::MountStat::Agility:   return stats.agility;
    case registry::Magic::MountStat::Ambition:  return stats.ambition;
    case registry::Magic::MountStat::Rush:      return stats.rush;
    case registry::Magic::MountStat::Endurance: return stats.endurance;
    case registry::Magic::MountStat::Courage:   return stats.courage;
  }
  return 0;
}

tracker::RaceTracker::RacerObjectMap::iterator MagicSystem::FindRacerByOid(
  tracker::RaceTracker::RacerObjectMap& racers,
  const tracker::Oid oid)
{
  return std::ranges::find_if(
    racers,
    [oid](const auto& pair)
    {
      return pair.second.oid == oid;
    });
}

bool MagicSystem::IsSelfCast(const uint32_t magicType)
{
  switch (magicType)
  {
    case MagicType::WaterShield:
    case MagicType::WaterShieldCritical:
    case MagicType::Booster:
    case MagicType::BoosterCritical:
    case MagicType::HotRodding:
    case MagicType::HotRoddingCritical:
      return true;
    default:
      return false;
  }
}

bool MagicSystem::IsTeamBuff(const uint32_t magicType)
{
  switch (magicType)
  {
    case MagicType::BufPower:
    case MagicType::BufPowerCritical:
    case MagicType::BufGauge:
    case MagicType::BufGaugeCritical:
    case MagicType::BufSpeed:
    case MagicType::BufSpeedCritical:
      return true;
    default:
      return false;
  }
}

bool MagicSystem::IsIceWall(const uint32_t magicType)
{
  return magicType == MagicType::IceWall || magicType == MagicType::IceWallCritical;
}

bool MagicSystem::IsValidSkillEffectId(const uint32_t skillEffectId)
{
  constexpr uint32_t ClientCrashingEffectId = 4;

  return skillEffectId < tracker::RaceTracker::Racer::EffectCount
    && skillEffectId != ClientCrashingEffectId;
}

bool MagicSystem::IsDowned(const tracker::RaceTracker::Racer& racer)
{
  return racer.attackRank >= HeavyAttackRank;
}

bool MagicSystem::IsStrippedByAttack(
  const registry::Magic::SlotInfo& attackSlotInfo,
  const registry::Magic::SlotInfo& activeSlotInfo)
{
  switch (activeSlotInfo.skillEffectId)
  {
    case SkillEffect::HotRodding:
    case SkillEffect::HotRoddingCritical:
    case SkillEffect::BufPower:
    case SkillEffect::BufPowerCritical:
    case SkillEffect::BufGauge:
    case SkillEffect::BufGaugeCritical:
      return false;

    case SkillEffect::JumpStun:
    case SkillEffect::JumpStunCritical:
    case SkillEffect::DarkFire:
    case SkillEffect::DarkFireCritical:
      return attackSlotInfo.removeMagic && attackSlotInfo.attackRank >= HeavyAttackRank;

    case SkillEffect::Booster:
    case SkillEffect::BufSpeed:
    case SkillEffect::BufSpeedCritical:
      return attackSlotInfo.removeMagic
        || attackSlotInfo.basicType == MagicType::JumpStun;

    case SkillEffect::WaterShield:
      return attackSlotInfo.removeMagic
        || attackSlotInfo.type == MagicType::JumpStunCritical
        || attackSlotInfo.type == MagicType::DarkFireCritical;

    default:
      return attackSlotInfo.removeMagic
        && activeSlotInfo.adjustMotionSpeed && activeSlotInfo.attackValue == 0;
  }
}

bool MagicSystem::DrainsGaugeOnHit(const registry::Magic::SlotInfo& attackSlotInfo)
{
  switch (attackSlotInfo.type)
  {
    case MagicType::FireBallCritical:
    case MagicType::SummonCritical:
    case MagicType::Lightning:
    case MagicType::LightningCritical:
      return true;
    default:
      return false;
  }
}

MagicSystem::EffectResolution MagicSystem::ResolveEffect(
  const registry::MagicRegistry& magicRegistry,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const tracker::RaceTracker::Racer& targetRacer)
{
  const bool isAttack = magicSlotInfo.attackValue > 0;

  const bool hasCriticalShield = targetRacer.effects[SkillEffect::WaterShieldCritical];
  const bool hasShield = hasCriticalShield || targetRacer.effects[SkillEffect::WaterShield];

  const uint32_t shieldThreshold = hasCriticalShield ? 200u : hasShield ? 100u : 0u;

  EffectResolution resolution{};
  resolution.shieldBlocks = isAttack && magicSlotInfo.attackValue < shieldThreshold;

  resolution.effectId = resolution.shieldBlocks
    ? (hasCriticalShield ? SkillEffect::WaterShieldCritical : SkillEffect::WaterShield)
    : magicSlotInfo.skillEffectId;

  const bool isLightning = isAttack && magicSlotInfo.removeHotRodding;
  const bool isCriticalLightning = isLightning && magicSlotInfo.criticalType == 0;

  const bool hotRoddingBlocks = isAttack
    && ((targetRacer.effects[SkillEffect::HotRodding] && not isLightning)
      || (targetRacer.effects[SkillEffect::HotRoddingCritical] && not isCriticalLightning));

  const uint32_t occupiedEffectId = magicSlotInfo.replaceEffect
    ? magicSlotInfo.skillEffectId
    : magicRegistry.GetSlotInfo(magicSlotInfo.basicType).skillEffectId;

  bool criticalEffectActive = false;
  if (isAttack)
  {
    const uint32_t criticalType = magicRegistry.GetSlotInfo(magicSlotInfo.basicType).criticalType;
    if (criticalType != 0)
    {
      const uint32_t criticalEffectId = magicRegistry.GetSlotInfo(criticalType).skillEffectId;
      criticalEffectActive = IsValidSkillEffectId(criticalEffectId)
        && targetRacer.effects[criticalEffectId];
    }
  }

  resolution.isDuplicated = hotRoddingBlocks
    || criticalEffectActive
    || (isAttack && magicSlotInfo.attackRank < 2 && targetRacer.attackRank >= 2)
    || (magicSlotInfo.attackRank > 0
      ? targetRacer.attackRank >= magicSlotInfo.attackRank
      : targetRacer.effects[occupiedEffectId] && (isAttack || not magicSlotInfo.replaceEffect));

  return resolution;
}

uint32_t MagicSystem::ComputeEffectDurationMs(
  const registry::MagicRegistry& magicRegistry,
  const registry::HorseRegistry& horseRegistry,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const tracker::RaceTracker::Racer* attackerRacer,
  const tracker::RaceTracker::Racer& targetRacer)
{
  auto effectDurationMs = static_cast<uint32_t>(magicSlotInfo.effectDelay * 1000.0f);

  if (const auto* scaling = magicRegistry.GetStatScaling(magicSlotInfo.basicType))
  {
    if (scaling->durationScaleBp > 0 && attackerRacer != nullptr)
    {
      constexpr uint32_t MaxDurationBonusBp = 1150;
      const uint32_t statValue = GetMountStatValue(attackerRacer->mountStats, scaling->stat);
      const uint32_t bonusBp = std::min(scaling->durationScaleBp * statValue, MaxDurationBonusBp);

      effectDurationMs = effectDurationMs * (1000u + bonusBp) / 1000u;
    }

    if (scaling->targetDurationReductionBp > 0)
    {
      const uint32_t statValue = GetMountStatValue(targetRacer.mountStats, scaling->stat);
      const uint32_t reductionBp = std::min<uint32_t>(
        scaling->targetDurationReductionBp * statValue, 1000u);

      effectDurationMs = effectDurationMs * (1000u - reductionBp) / 1000u;
    }
  }

  if (attackerRacer != nullptr)
  {
    if (magicSlotInfo.basicType == MagicType::WaterShield)
    {
      effectDurationMs += PotentialSystem::GetShieldDurationBonusMs(
        horseRegistry, attackerRacer->potential);
    }
    else if (magicSlotInfo.basicType == MagicType::JumpStun)
    {
      effectDurationMs += PotentialSystem::GetShackleDurationBonusMs(
        horseRegistry, attackerRacer->potential);
    }
  }

  return effectDurationMs;
}

// Function to select a random item based on position weights
const registry::Magic::SlotInfo& MagicSystem::SelectMagicTypeByPosition(
  const registry::MagicRegistry& magicRegistry,
  uint32_t position,
  bool isTeam)
{
  // Validate position (0..7)
  if (position > 7)
    throw std::out_of_range("Position must be between 0 and 7");

  const uint32_t canonicalRank = position + 1;
  const auto& groupWeights = isTeam
    ? magicRegistry.GetTeamGroupWeights(canonicalRank)
    : magicRegistry.GetSoloGroupWeights(canonicalRank);

  if (groupWeights.empty())
  {
    // Fallback to legacy position weights if group weights not configured
    const auto& legacyWeights = isTeam
      ? magicRegistry.GetTeamPositionWeights(position)
      : magicRegistry.GetSoloPositionWeights(position);

    const auto& weights = legacyWeights | std::views::keys;
    std::discrete_distribution<uint32_t> dist(weights.cbegin(), weights.cend());
    return legacyWeights[dist(server::util::GetRandomEngine())].second;
  }

  std::vector<uint32_t> groupIds;
  std::vector<uint32_t> weights;
  groupIds.reserve(groupWeights.size());
  weights.reserve(groupWeights.size());

  for (const auto& [groupId, weight] : groupWeights)
  {
    groupIds.push_back(groupId);
    weights.push_back(weight);
  }

  std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
  const uint32_t selectedGroup = groupIds[dist(server::util::GetRandomEngine())];

  uint32_t basicType = MagicType::WaterShield;
  switch (selectedGroup)
  {
    case 1: // Attack Group
    {
      const auto& attackWeights = magicRegistry.GetAttackGroupWeights(canonicalRank);
      if (not attackWeights.empty())
      {
        std::vector<uint32_t> attackTypes;
        std::vector<uint32_t> subWeights;
        for (const auto& [type, w] : attackWeights)
        {
          attackTypes.push_back(type);
          subWeights.push_back(w);
        }
        const uint32_t attackSum = std::accumulate(subWeights.begin(), subWeights.end(), 0u);
        if (attackSum > 0)
        {
          std::discrete_distribution<size_t> attackDist(subWeights.begin(), subWeights.end());
          basicType = attackTypes[attackDist(server::util::GetRandomEngine())];
        }
        else
        {
          basicType = MagicType::FireBall;
        }
      }
      else
      {
        basicType = MagicType::FireBall;
      }
      break;
    }
    case 2: // Defense
      basicType = MagicType::WaterShield;
      break;
    case 3: // Booster
      basicType = MagicType::Booster;
      break;
    case 4: // Rear hazard
      basicType = MagicType::IceWall;
      break;
    case 5: // Team Assistance
    {
      if (isTeam)
      {
        const auto& assistWeights = magicRegistry.GetTeamAssistanceWeights(canonicalRank);
        if (not assistWeights.empty())
        {
          std::vector<uint32_t> assistTypes;
          std::vector<uint32_t> subWeights;
          for (const auto& [type, w] : assistWeights)
          {
            assistTypes.push_back(type);
            subWeights.push_back(w);
          }
          const uint32_t assistSum = std::accumulate(subWeights.begin(), subWeights.end(), 0u);
          if (assistSum > 0)
          {
            std::discrete_distribution<size_t> assistDist(subWeights.begin(), subWeights.end());
            basicType = assistTypes[assistDist(server::util::GetRandomEngine())];
          }
          else
          {
            basicType = MagicType::BufSpeed;
          }
        }
        else
        {
          basicType = MagicType::BufSpeed;
        }
      }
      else
      {
        basicType = MagicType::WaterShield;
      }
      break;
    }
    case 6: // HotRodding
      basicType = MagicType::HotRodding;
      break;
    case 7: // Team Acceleration
      basicType = isTeam ? MagicType::BufSpeed : MagicType::Booster;
      break;
    default:
      basicType = MagicType::WaterShield;
      break;
  }

  return magicRegistry.GetSlotInfo(basicType);
}

const registry::Magic::SlotInfo& MagicSystem::RandomMagicItem(
  const registry::MagicRegistry& magicRegistry,
  tracker::RaceTracker& tracker,
  data::Uid racerUid)
{
  const auto& racer = tracker.GetRacer(racerUid);
  const bool isTeam =
    racer.team == protocol::TeamColor::Red or
    racer.team == protocol::TeamColor::Blue;

  // Determine the racer's actual rank (1-indexed: 1 = 1st place)
  // and count how many opponents are ahead for team modifiers.
  uint32_t racerRank = 1;
  uint32_t opponentsAhead = 0;
  bool teamIsLeading = true;
  float bestProgress = racer.raceProgress;
  protocol::TeamColor leaderTeam = racer.team;

  const auto& allRacers = tracker.GetRacers();
  const size_t totalRacers = allRacers.size();

  for (const auto& [uid, instanceRacer] : allRacers)
  {
    if (instanceRacer.raceProgress > bestProgress)
    {
      bestProgress = instanceRacer.raceProgress;
      leaderTeam = instanceRacer.team;
    }

    if (uid == racerUid)
      continue;

    if (instanceRacer.raceProgress > racer.raceProgress)
    {
      racerRank++;
      if (isTeam && instanceRacer.team != racer.team)
      {
        opponentsAhead++;
      }
    }
  }

  if (isTeam)
  {
    teamIsLeading = (leaderTeam == racer.team);
  }

  // Canonical rank (1..8) from RankingConversionInfo
  const uint32_t canonicalRank = magicRegistry.GetCanonicalRank(
    totalRacers,
    racerRank);

  // Fetch base group distribution
  const auto& baseGroupWeights = isTeam
    ? magicRegistry.GetTeamGroupWeights(canonicalRank)
    : magicRegistry.GetSoloGroupWeights(canonicalRank);

  uint32_t chosenBasicType = MagicType::WaterShield;

  if (not baseGroupWeights.empty())
  {
    std::vector<uint32_t> groupIds;
    std::vector<uint32_t> adjustedWeights;
    groupIds.reserve(baseGroupWeights.size());
    adjustedWeights.reserve(baseGroupWeights.size());

    for (const auto& [groupId, baseWeight] : baseGroupWeights)
    {
      int32_t finalWeight = static_cast<int32_t>(baseWeight);
      if (isTeam && opponentsAhead > 0)
      {
        finalWeight += magicRegistry.GetGroupTeamModifier(groupId, teamIsLeading, opponentsAhead);
      }
      groupIds.push_back(groupId);
      adjustedWeights.push_back(finalWeight > 0 ? static_cast<uint32_t>(finalWeight) : 0u);
    }

    // If all adjusted weights sum to 0, fallback to raw base weights
    const uint32_t weightSum = std::accumulate(adjustedWeights.begin(), adjustedWeights.end(), 0u);
    if (weightSum == 0)
    {
      adjustedWeights.clear();
      for (const auto& [groupId, baseWeight] : baseGroupWeights)
        adjustedWeights.push_back(baseWeight);
    }

    std::discrete_distribution<size_t> groupDist(adjustedWeights.begin(), adjustedWeights.end());
    const uint32_t selectedGroup = groupIds[groupDist(server::util::GetRandomEngine())];

    switch (selectedGroup)
    {
      case 1: // Attacks
      {
        const auto& attackWeights = magicRegistry.GetAttackGroupWeights(canonicalRank);
        if (not attackWeights.empty())
        {
          std::vector<uint32_t> attackTypes;
          std::vector<uint32_t> subWeights;
          for (const auto& [type, w] : attackWeights)
          {
            attackTypes.push_back(type);
            subWeights.push_back(w);
          }
          const uint32_t attackSum = std::accumulate(subWeights.begin(), subWeights.end(), 0u);
          if (attackSum > 0)
          {
            std::discrete_distribution<size_t> attackDist(subWeights.begin(), subWeights.end());
            chosenBasicType = attackTypes[attackDist(server::util::GetRandomEngine())];
          }
          else
          {
            chosenBasicType = MagicType::FireBall;
          }
        }
        else
        {
          chosenBasicType = MagicType::FireBall;
        }
        break;
      }
      case 2: // Defense
        chosenBasicType = MagicType::WaterShield;
        break;
      case 3: // Speed / Booster
        chosenBasicType = MagicType::Booster;
        break;
      case 4: // Rear Hazard
        chosenBasicType = MagicType::IceWall;
        break;
      case 5: // Team Assistance / Buffs
      {
        if (isTeam)
        {
          const auto& assistWeights = magicRegistry.GetTeamAssistanceWeights(canonicalRank);
          if (not assistWeights.empty())
          {
            std::vector<uint32_t> assistTypes;
            std::vector<uint32_t> subWeights;
            for (const auto& [type, baseW] : assistWeights)
            {
              int32_t finalW = static_cast<int32_t>(baseW);
              if (opponentsAhead > 0)
                finalW += magicRegistry.GetSlotTeamModifier(type, teamIsLeading, opponentsAhead);
              assistTypes.push_back(type);
              subWeights.push_back(finalW > 0 ? static_cast<uint32_t>(finalW) : 0u);
            }

            const uint32_t subSum = std::accumulate(subWeights.begin(), subWeights.end(), 0u);
            if (subSum == 0)
            {
              subWeights.clear();
              for (const auto& [type, baseW] : assistWeights)
                subWeights.push_back(baseW);
            }

            const uint32_t finalSum = std::accumulate(subWeights.begin(), subWeights.end(), 0u);
            if (finalSum > 0)
            {
              std::discrete_distribution<size_t> assistDist(subWeights.begin(), subWeights.end());
              chosenBasicType = assistTypes[assistDist(server::util::GetRandomEngine())];
            }
            else
            {
              chosenBasicType = MagicType::BufSpeed;
            }
          }
          else
          {
            chosenBasicType = MagicType::BufSpeed;
          }
        }
        else
        {
          chosenBasicType = MagicType::WaterShield;
        }
        break;
      }
      case 6: // HotRodding
        chosenBasicType = MagicType::HotRodding;
        break;
      case 7: // Team Acceleration / BufSpeed
        chosenBasicType = isTeam ? MagicType::BufSpeed : MagicType::Booster;
        break;
      default:
        chosenBasicType = MagicType::WaterShield;
        break;
    }
  }
  else
  {
    // Fallback if group tables were empty
    const registry::Magic::SlotInfo& fallbackSlot = SelectMagicTypeByPosition(
      magicRegistry,
      std::clamp<uint32_t>(canonicalRank - 1, 0u, 7u),
      isTeam);
    chosenBasicType = fallbackSlot.basicType;
  }

  const registry::Magic::SlotInfo& magicSlotInfo = magicRegistry.GetSlotInfo(chosenBasicType);

  // Roll critical upgrade
  uint32_t critChanceBp = magicRegistry.GetBaseCritChanceBp();
  if (magicSlotInfo.criticalType != 0)
  {
    if (const auto* scaling = magicRegistry.GetStatScaling(magicSlotInfo.basicType))
    {
      const uint32_t statValue = GetMountStatValue(racer.mountStats, scaling->stat);
      critChanceBp += scaling->critStepBp * (statValue / 10u);
    }

    // Mount-equipment set bonus: increased critical spell chance.
    if (racer.activeSetEffect == registry::SetEquipEffect::CriticalSpellChance)
      critChanceBp += magicRegistry.GetSetBonusInfo().critChanceBonusBp;

    // LastSpurt bonus (progress threshold from config)
    if (racer.raceProgress >= magicRegistry.GetConfig().lastSpurtProgressThreshold)
      critChanceBp += magicRegistry.GetConfig().lastSpurtCritBonusBp;
  }

  if (std::uniform_int_distribution<int>(0, 9999)(server::util::GetRandomEngine()) < static_cast<int>(critChanceBp))
    return magicRegistry.GetSlotInfo(magicSlotInfo.criticalType);

  return magicSlotInfo;
}

} // namespace server::race

