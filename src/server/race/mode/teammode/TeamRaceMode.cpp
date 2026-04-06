/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
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

#include "server/race/mode/teammode/TeamRaceMode.hpp"

#include "server/system/RoomSystem.hpp"

namespace server::race::mode
{

bool TeamRaceMode::AreTeamsBalanced(server::Room& room) const
{
  uint32_t redTeamCount = 0;
  uint32_t blueTeamCount = 0;

  for (const auto& [characterUid, player] : room.GetPlayers())
  {
    switch (player.GetTeam())
    {
      case Room::Player::Team::Red:
        redTeamCount++;
        break;
      case Room::Player::Team::Blue:
        blueTeamCount++;
        break;
      default:
        break;
    }
  }

  return redTeamCount == blueTeamCount;
}

bool TeamRaceMode::IsEnemy(
  const tracker::RaceTracker::Racer& a,
  const tracker::RaceTracker::Racer& b) const
{
  return a.oid != b.oid && a.team != b.team;
}

bool TeamRaceMode::IsAlly(
  const tracker::RaceTracker::Racer& a,
  const tracker::RaceTracker::Racer& b) const
{
  return a.oid != b.oid && a.team == b.team;
}

void TeamRaceMode::OnTeamGauge(
  [[maybe_unused]] ClientId clientId,
  [[maybe_unused]] RaceDirector::RaceInstance& raceInstance,
  [[maybe_unused]] tracker::RaceTracker::Racer& racer)
{
  // TODO: copy implementation from RaceDirector::HandleTeamGauge
}

} // namespace server::race::mode
