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

#ifndef MAGIC_GAMEMODE_HANDLER_HPP
#define MAGIC_GAMEMODE_HANDLER_HPP

#include "server/race/mode/GameModeHandler.hpp"

namespace server::race::mode
{

class MagicGameMode : public GameModeHandler
{
public:
  explicit MagicGameMode(RaceDirector& director)
    : GameModeHandler(director)
  {}

  void OnHurdleClear(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRHurdleClearResult& command) override;

  void OnRaceUserPos(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdUserRaceUpdatePos& command) override;

  void OnItemGet(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdUserRaceItemGet& command,
    tracker::RaceTracker::Item& item) override;

  void OnRequestSpur(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRRequestSpur& command) override;

  void OnStartingRate(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRStartingRate& command) override;

  void OnUseMagicItem(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRUseMagicItem& command) override;
};

} // namespace server::race::mode

#endif // MAGIC_GAMEMODE_HANDLER_HPP
