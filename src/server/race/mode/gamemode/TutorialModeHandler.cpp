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

#include "server/race/mode/gamemode/TutorialModeHandler.hpp"
#include "server/race/mode/gamemode/SpeedModeHandler.hpp"
#include "server/race/mode/gamemode/MagicModeHandler.hpp"

namespace server::race::mode
{

TutorialGameMode::TutorialGameMode(RaceDirector& director, uint32_t missionId)
  : GameModeHandler(director), _missionId(missionId)
{
  // Mission 32 is speed mode (hardcoded for now as per user request)
  // TODO: add MissionRegistry to get the mode from missionId
  switch (_missionId)
  {
    case 31:
    case 32:
      _gamemodeHandler = std::make_unique<SpeedGameMode>(_director);
      break;
    default:
      throw std::runtime_error(
        std::format("Tutorial mission {} is not implemented!", _missionId));
  }
}

void TutorialGameMode::OnHurdleClear(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdCRHurdleClearResult& command)
{
  _gamemodeHandler->OnHurdleClear(clientId, raceInstance, command);
}

void TutorialGameMode::OnRaceUserPos(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdUserRaceUpdatePos& command)
{
  _gamemodeHandler->OnRaceUserPos(clientId, raceInstance, command);
}

void TutorialGameMode::OnItemGet(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdUserRaceItemGet& command,
  tracker::RaceTracker::Item& item)
{
  _gamemodeHandler->OnItemGet(clientId, raceInstance, command, item);
}

void TutorialGameMode::OnRequestSpur(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdCRRequestSpur& command)
{
  _gamemodeHandler->OnRequestSpur(clientId, raceInstance, command);
}

void TutorialGameMode::OnStartingRate(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdCRStartingRate& command)
{
  _gamemodeHandler->OnStartingRate(clientId, raceInstance, command);
}

void TutorialGameMode::OnUseMagicItem(
  ClientId clientId,
  RaceDirector::RaceInstance& raceInstance,
  const protocol::AcCmdCRUseMagicItem& command)
{
  _gamemodeHandler->OnUseMagicItem(clientId, raceInstance, command);
}

} // namespace server::race::mode