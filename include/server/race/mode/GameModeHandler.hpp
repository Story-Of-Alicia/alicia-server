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

#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"

#include "server/race/RaceDirector.hpp"

namespace server::race::mode
{

class GameModeHandler
{
public:
  virtual ~GameModeHandler() = default;

  virtual void OnHurdleClear(
    RaceDirector& director,
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    tracker::RaceTracker::Racer& racer,
    const protocol::AcCmdCRHurdleClearResult& command) = 0;

  virtual void OnRaceUserPos(
    RaceDirector& director,
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    tracker::RaceTracker::Racer& racer,
    const protocol::AcCmdUserRaceUpdatePos& command) = 0;

  virtual void OnItemGet(
    RaceDirector& director,
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    tracker::RaceTracker::Racer& racer,
    const protocol::AcCmdUserRaceItemGet&
    command, tracker::RaceTracker::Item& item) = 0;

  virtual void OnRequestSpur(
    RaceDirector& director,
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    tracker::RaceTracker::Racer& racer,
    const protocol::AcCmdCRRequestSpur& command) = 0;

  virtual void OnStartingRate(
    RaceDirector& director,
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    tracker::RaceTracker::Racer& racer,
    const protocol::AcCmdCRStartingRate& command) = 0;

  virtual void OnUseMagicItem(
    RaceDirector& director,
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance,
    tracker::RaceTracker::Racer& racer,
    const protocol::AcCmdCRUseMagicItem& command) = 0;
};

} // namespace server::race::mode

#endif // GAMEMODE_HANDLER_HPP
