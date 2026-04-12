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
  ClientId clientId)
{
  const auto& clientContext = _director.GetClientContext(clientId);

  // If race teammode is not team then we are done here.
  // This is necessary to ensure no team-related logic is handled when spur logic is handled.
  // Sanity check for speed gamemode
  bool isTeamMode = _raceInstance.raceTeamMode == protocol::TeamMode::Team;
  bool isSpeedGameMode = _raceInstance.raceGameMode == protocol::GameMode::Speed;
  if (not isTeamMode or not isSpeedGameMode)
    return;

  auto& racer = _raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  auto& blueTeam = _raceInstance.tracker.blueTeam;
  auto& redTeam = _raceInstance.tracker.redTeam;
  auto& team = 
    racer.team == tracker::RaceTracker::Racer::Team::Red ? redTeam :
    racer.team == tracker::RaceTracker::Racer::Team::Blue ? blueTeam :
    throw std::runtime_error(
      std::format(
        "Racer character uid {} is on unrecognised team {}",
        clientContext.characterUid,
        static_cast<uint32_t>(racer.team)));

  // If the invoker's team gauge is locked (beaten by opposing team's spur), reject gauge fill.
  if (team.gaugeLocked)
    return;

  // Track team boost count for gauge fill rate calculation.
  team.boostCount += 1;

  //! Boost fill rates, scaled with team count, iterated with boost count.
  //! Reference: `TeamSpurGaugeInfo` in libconfig
  // TODO: put this in the config somewhere
  const std::vector<float> baseFillRates{
    1.25f,
    2.50f,
    3.00f,
    3.75f,
    5.50f,
    6.50f};

  // Get team size from the racer tracker (immutable for the race duration).
  // Use the max of the two team sizes to handle potentially unbalanced teams.
  uint32_t redTeamCount = 0;
  uint32_t blueTeamCount = 0;
  for (const auto& _ : _raceInstance.tracker.GetRacers() | std::views::values)
  {
    if (_.team == tracker::RaceTracker::Racer::Team::Red)
      ++redTeamCount;
    else if (_.team == tracker::RaceTracker::Racer::Team::Blue)
      ++blueTeamCount;
  }
  const auto teamSize = std::max(redTeamCount, blueTeamCount);

  const auto fillRateIndex = std::min(
    team.boostCount,
    static_cast<uint32_t>(baseFillRates.size() - 1));
  protocol::AcCmdRCTeamSpurGauge spur{
    .team = racer.team,
    .markerSpeed = baseFillRates[fillRateIndex] * teamSize, // Base fill rate * boost count * team size
    .unk5 = 0 // TODO: identify use
  };

  const std::vector<float> basePoints{3.50f, 3.75f, 4.50f, 5.00f, 6.00f, 7.00f};

  // Final points per boost = Table reference * team size
  const uint32_t pointsPerBoost = 
    static_cast<uint32_t>(basePoints[fillRateIndex] * teamSize * 10.0f);
  
  // Final max points for team size.
  // Reference: `TeamSpurGaugeInfo` `TeamSpurMax`
  const uint32_t maxPoints = 
    teamSize == 1 ? 250 :
    teamSize == 2 ? 400 :
    teamSize == 3 ? 650 :
    1000; // 4v4

  auto& blueTeamPoints = blueTeam.points;
  auto& redTeamPoints = redTeam.points;
  auto& teamPoints = 
    racer.team == tracker::RaceTracker::Racer::Team::Red ? redTeamPoints :
    racer.team == tracker::RaceTracker::Racer::Team::Blue ? blueTeamPoints :
    throw std::runtime_error(
      std::format(
        "Racer character uid {} is on unrecognised team {}",
        clientContext.characterUid,
        static_cast<uint32_t>(racer.team)));
  
  spur.currentPoints = teamPoints / 10.0f;
  teamPoints = std::min(
    maxPoints,
    teamPoints + pointsPerBoost);
  spur.newPoints = teamPoints / 10.0f;

  // If any of the teams got max points to spur, reset points and broadcast team spur
  bool isTeamRed = racer.team == tracker::RaceTracker::Racer::Team::Red;
  bool isTeamBlue = racer.team == tracker::RaceTracker::Racer::Team::Blue;

  // Can invoker's team spur
  bool isTeamSpur = false;
  // Check if either red or blue team points have hit max
  if (redTeamPoints >= maxPoints or blueTeamPoints >= maxPoints)
  {
    // If any (red or blue) team can spur.
    // Team check is added for additional validation.
    isTeamSpur = (isTeamRed and redTeamPoints >= maxPoints) or
      (isTeamBlue and blueTeamPoints >= maxPoints);

    // Reset points
    redTeamPoints = 0;
    blueTeamPoints = 0;
  }

  // If any of the teams can spur, schedule a spur/reset event.
  if (isTeamSpur)
  {
    // Reset team boost counters
    redTeam.boostCount = 0;
    blueTeam.boostCount = 0;

    // Lock the spurring team's gauge so it cannot fill during the spur.
    auto& spurringTeamInfo =
      racer.team == tracker::RaceTracker::Racer::Team::Red ? redTeam :
      racer.team == tracker::RaceTracker::Racer::Team::Blue ? blueTeam :
      throw std::runtime_error(
        std::format(
          "Unrecognised racer team '{}'",
          static_cast<uint32_t>(racer.team)));
    spurringTeamInfo.gaugeLocked = true;

    // TODO: put this into the config somewhere
    // When to begin the spur/reset event.
    // Reference: `TeamSpurGaugeInfo`/`ReduceWaitTime` in libconfig
    constexpr auto SpurStartDelay = std::chrono::milliseconds(1500);

    _director.GetScheduler().Queue(
      [this, roomUid = _raceInstance.roomUid, &racer, &spurringTeamInfo, maxPoints, teamSize]()
      {
        std::scoped_lock lock(_director._raceInstancesMutex);
        const auto raceInstanceIter = _director._raceInstances.find(roomUid);;
        if (raceInstanceIter == _director._raceInstances.cend())
          return;

        const auto& raceInstance = raceInstanceIter->second;

        const float BaseLoseTeamSpurConsumeRate = -10.0f;
        const float BaseWinTeamSpurConsumeRate = -2.5f;

        // Reset boost gauge for the team that lost it.
        protocol::AcCmdRCTeamSpurGauge beatenSpur{
          .team = 
            // This red/blue swap is intentional, if team A wins, team B is punished and reset.
            racer.team == tracker::RaceTracker::Racer::Team::Red ? tracker::RaceTracker::Racer::Team::Blue :
            racer.team == tracker::RaceTracker::Racer::Team::Blue ? tracker::RaceTracker::Racer::Team::Red :
            throw std::runtime_error(
              std::format(
                "Unrecognised racer team '{}'",
                static_cast<uint32_t>(racer.team))),
          .currentPoints = 0.0f,
          .newPoints = 0.0f,
          .markerSpeed = BaseLoseTeamSpurConsumeRate * teamSize, // Scales with `LoseTeamSpurConsumeRate`
          .unk5 = 3 // Reset gauge and markers.
        };

        // Trigger spur for the team that has won it.
        protocol::AcCmdRCTeamSpurGauge successfulSpur{
          .team = racer.team,
          .currentPoints = maxPoints / 10.0f,
          .newPoints = 0.0f,
          .markerSpeed = BaseWinTeamSpurConsumeRate * teamSize, // Scales with `WinTeamSpurConsumeRate`
          .unk5 = 0
        };

        // Spur duration = (maxPoints / 10.0f) / (abs(consumeRate) * teamSize)
        // For example: 25.0f / (2.5f * 1) = 10s for a team of 1.
        const float spurDurationSeconds =
          (maxPoints / 10.0f) / (std::abs(BaseWinTeamSpurConsumeRate) * teamSize);

        // Schedule unlock of the spurring team's gauge after the spur completes.
        _director.GetScheduler().Queue(
          [&spurringTeamInfo]()
          {
            spurringTeamInfo.gaugeLocked = false;
          },
          Scheduler::Clock::now() + std::chrono::milliseconds(
            static_cast<int64_t>(spurDurationSeconds * 1000)));

        for (const ClientId& raceClientId : raceInstance.clients)
        {
          // Broadcast losing team's gauge status
          _director.GetCommandServer().QueueCommand<decltype(beatenSpur)>(
            raceClientId,
            [beatenSpur]()
            {
              return beatenSpur;
            });

          // Broadcast winning team's gauge status
          _director.GetCommandServer().QueueCommand<decltype(successfulSpur)>(
            raceClientId,
            [successfulSpur]()
            {
              return successfulSpur;
            });
        }
      },
      Scheduler::Clock::now() + SpurStartDelay); 
  }

  // Broadcast invoker's team gauge status
  for (const auto& raceClientId : _raceInstance.clients)
  {
    _director.GetCommandServer().QueueCommand<decltype(spur)>(raceClientId, [spur](){ return spur; });
  }
}

} // namespace server::race::mode
