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
#include "server/tracker/RaceTracker.hpp"

#include <algorithm>
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

// Function to select a random item based on position weights
const registry::Magic::SlotInfo& MagicSystem::SelectMagicTypeByPosition(
  const registry::MagicRegistry& magicRegistry,
  uint32_t position,
  bool isTeam)
{
  // Validate position
  if (position > 7)
    throw std::out_of_range("Position must be between 0 and 7");

  // Get the weights for the specified position
  const auto& positionSlotInfoWeights = isTeam ? magicRegistry.GetTeamPositionWeights(position) : magicRegistry.GetSoloPositionWeights(position);

  // Keep reference to weights only for the discrete distribution
  const auto& positionWeights = positionSlotInfoWeights | std::views::keys;

  // Create a discrete distribution based on the weights
  std::discrete_distribution<uint32_t> dist(
    positionWeights.cbegin(),
    positionWeights.cend());

  // Random number generator
  static std::random_device rd;
  static std::mt19937 gen(rd());

  // Select a random index based on the distribution
  // and map the discrete index to the corresponding magic item
  const uint32_t selectedIndex = dist(gen);
  return positionSlotInfoWeights[selectedIndex].second;
}

const registry::Magic::SlotInfo& MagicSystem::RandomMagicItem(
  const registry::MagicRegistry& magicRegistry,
  tracker::RaceTracker& tracker,
  data::Uid racerUid)
{
  const auto& racer = tracker.GetRacer(racerUid);

  // Get this racer's track progress
  const float racerTrackProgress = racer.progress;

  // Determine the racer's position (0 = 1st place)
  uint32_t racerPosition = 0;
  const auto& allRacers = tracker.GetRacers();
  size_t totalRacers = allRacers.size();

  for (const auto& [uid, instanceRacer] : allRacers)
  {
    if (uid == racerUid)
      continue;

    // TODO: do we ignore disconnected racers too?

    if (instanceRacer.progress > racerTrackProgress)
      racerPosition++;
  }

  // Map the actual position to one of the 8 weight slots [0, 7] via linear interpolation.
  // This ensures last place in a small race gets "last-place" item weights.
  uint32_t effectivePosition = racerPosition;
  if (totalRacers > 1)
  {
    effectivePosition = static_cast<uint32_t>(
      static_cast<float>(racerPosition) * 7.0f /
      static_cast<float>(totalRacers - 1));
  }

  // Clamp effective position to [0, 7] for safety
  effectivePosition = std::clamp(effectivePosition, 0u, 7u);

  const registry::Magic::SlotInfo& magicSlotInfo = SelectMagicTypeByPosition(
    magicRegistry,
    effectivePosition,
    racer.team == protocol::TeamColor::Red or racer.team == protocol::TeamColor::Blue);

  uint32_t critChanceBp = magicRegistry.GetBaseCritChanceBp();
  if (magicSlotInfo.criticalType != 0)
  {
    if (const auto* scaling = magicRegistry.GetStatScaling(magicSlotInfo.basicType))
    {
      const uint32_t statValue = GetMountStatValue(racer.mountStats, scaling->stat);
      critChanceBp += scaling->critStepBp * (statValue / 10u);
    }
  }

  if ((rand() % 10000) < static_cast<int>(critChanceBp))
    return magicRegistry.GetSlotInfo(magicSlotInfo.criticalType);

  return magicSlotInfo;
}

} // namespace server::race
