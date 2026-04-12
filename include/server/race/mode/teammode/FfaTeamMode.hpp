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

#ifndef FFA_TEAMMODE_HANDLER_HPP
#define FFA_TEAMMODE_HANDLER_HPP

#include "server/race/mode/TeamModeHandler.hpp"

namespace server::race::mode
{

class FfaTeamMode final : public TeamModeHandler
{
public:
  explicit FfaTeamMode(RaceDirector& director, RaceDirector::RaceInstance& raceInstance)
    : TeamModeHandler(director, raceInstance)
  {}

  bool AreTeamsBalanced(server::Room& room) const override;

  bool IsEnemy(
    const tracker::RaceTracker::Racer& a,
    const tracker::RaceTracker::Racer& b) const override;

  bool IsAlly(
    const tracker::RaceTracker::Racer& a,
    const tracker::RaceTracker::Racer& b) const override;

  void OnTeamGauge(
    ClientId clientId) override;
};

} // namespace server::race::mode

#endif // FFA_TEAMMODE_HANDLER_HPP
