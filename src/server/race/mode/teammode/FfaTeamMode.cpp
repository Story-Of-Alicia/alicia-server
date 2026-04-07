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

#include "server/race/mode/teammode/FfaTeamMode.hpp"

namespace server::race::mode
{

bool FfaTeamMode::AreTeamsBalanced(server::Room&) const
{
  return true; // Always balanced in FFA
}

bool FfaTeamMode::IsEnemy(
  const tracker::RaceTracker::Racer& a,
  const tracker::RaceTracker::Racer& b) const
{
  return a.oid != b.oid;
}

bool FfaTeamMode::IsAlly(
  const tracker::RaceTracker::Racer&,
  const tracker::RaceTracker::Racer&) const
{
  return false;
}

void FfaTeamMode::OnTeamGauge(
  ClientId,
  RaceDirector::RaceInstance&)
{
  // No team gauge in FFA
}

} // namespace server::race::mode
