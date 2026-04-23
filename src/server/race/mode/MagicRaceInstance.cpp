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

#include "server/ServerInstance.hpp"

#include "server/race/mode/MagicRaceInstance.hpp"

#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>

namespace server
{

MagicRaceInstance::MagicRaceInstance(
  ServerInstance& serverInstance,
  CommandServer& commandServer) : RaceInstance(serverInstance, commandServer)
{}

MagicRaceInstance::~MagicRaceInstance() = default;

void MagicRaceInstance::TickRacing()
{
  RaceInstance::TickRacing();

  TickGauge();
}

void MagicRaceInstance::TickGauge()
{
  const auto& now = std::chrono::steady_clock::now();

  // Only regenerate magic during active race (after countdown finishes)
  if (now < raceStartTimePoint)
    return;

  // Check if it is time to tick the magic gauge
  if (now < _nextMagicGaugeTickTimePoint)
    return;

  // TODO: extract this into the magic instance
  const auto& gameModeTemplate = _serverInstance.GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(raceGameMode));

  for (auto& [racerCharacterUid, racer] : tracker.GetRacers())
  {
    const bool isRacing = racer.state == tracker::RaceTracker::Racer::State::Racing;
    const bool isHoldingMagicItem = racer.magicItem.has_value();
    // Check if racer is actively racing and not holding an item
    if (isRacing and not isHoldingMagicItem)
    {
      if (racer.starPointValue < gameModeTemplate.starPointsMax)
      {
        uint32_t multiplier = 1;
        const bool isNormalTeamMagicBoostActive = racer.effects[20];
        const bool isCritTeamMagicBoostActive = racer.effects[21];
        if (isNormalTeamMagicBoostActive or isCritTeamMagicBoostActive)
        {
          // TODO: Something sensible, idk what the bonus does
          multiplier = 2;
        }

        // Calculate gained star points (gauge) depending on whether the racer
        // is holding an item or not, and apply magic boost multiplier if it is active
        const uint32_t gainedStarPoints = (isHoldingMagicItem ?
          ItemHeldWithEquipmentBoostAmount :
          NoItemHeldBoostAmount) * multiplier;
        
        racer.starPointValue = std::min(gameModeTemplate.starPointsMax, racer.starPointValue + gainedStarPoints);
      }

      // Conditional already checks if there is no magic item and gamemode is magic,
      // only check if racer has max magic gauge to give magic item
      protocol::AcCmdCRStarPointGetOK starPointResponse{
        .characterOid = racer.oid,
        .starPointValue = racer.starPointValue,
        .giveMagicItem = racer.starPointValue >= gameModeTemplate.starPointsMax
      };

      const ClientId racerClientId = clients.at(racerCharacterUid);
      _commandServer.QueueCommand<decltype(starPointResponse)>(
        racerClientId,
        [starPointResponse]
        {
          return starPointResponse;
        });
    }
  }

  // Set the next magic gauge tick time point
  _nextMagicGaugeTickTimePoint = now + MagicGaugeTickInterval;
}

} // namespace server
