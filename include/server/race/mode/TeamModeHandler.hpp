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

#ifndef TEAMMODE_HANDLER_HPP
#define TEAMMODE_HANDLER_HPP

#include "server/race/RaceDirector.hpp"

namespace server
{
class Room;
} // namespace server

namespace server::race::mode
{

class TeamModeHandler
{
public:
  explicit TeamModeHandler(RaceDirector& director)
    : _director(director)
  {}

  virtual ~TeamModeHandler() = default;

  virtual bool AreTeamsBalanced(server::Room& room) const = 0;

  virtual bool IsEnemy(
    const tracker::RaceTracker::Racer& a,
    const tracker::RaceTracker::Racer& b) const = 0;

  virtual bool IsAlly(
    const tracker::RaceTracker::Racer& a,
    const tracker::RaceTracker::Racer& b) const = 0;

  virtual void OnTeamGauge(
    ClientId clientId,
    RaceDirector::RaceInstance& raceInstance) = 0;

protected:
  RaceDirector& _director;
};

} // namespace server::race::mode

#endif // TEAMMODE_HANDLER_HPP
