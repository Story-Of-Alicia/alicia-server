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

#ifndef GAMEMODE_HANDLER_HPP
#define GAMEMODE_HANDLER_HPP

#include "libserver/registry/CourseRegistry.hpp"
#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"

#include "server/race/RaceDirector.hpp"

namespace server::race::mode
{

class GameModeHandler
{
public:
  explicit GameModeHandler(RaceDirector& director, const protocol::GameMode gameMode);
  ~GameModeHandler();

  virtual void OnHurdleClear(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRHurdleClearResult& command) = 0;

  virtual void OnRaceUserPos(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdUserRaceUpdatePos& command) = 0;

  virtual void OnItemGet(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdUserRaceItemGet& command,
    tracker::RaceTracker::Item& item) = 0;

  virtual void OnRequestSpur(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRRequestSpur& command) = 0;

  virtual void OnStartingRate(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRStartingRate& command) = 0;

  virtual void OnUseMagicItem(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    const protocol::AcCmdCRUseMagicItem& command) = 0;

protected:
  RaceDirector& _director;
  const protocol::GameMode _gameMode;
};

} // namespace server::race::mode

#endif // GAMEMODE_HANDLER_HPP
