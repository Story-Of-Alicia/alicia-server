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

#include "server/ServerInstance.hpp"

#include "server/race/RaceInstance.hpp"

#include <spdlog/spdlog.h>

#include <ranges>

namespace server
{

RaceInstance::RaceInstance(
  ServerInstance& serverInstance,
  CommandServer& commandServer) : _serverInstance(serverInstance), _commandServer(commandServer)
{}

RaceInstance::~RaceInstance() = default;

//! Converts a steady clock's time point to a race clock's time point.
//! @param timePoint Time point.
//! @return Race clock time point.
uint64_t RaceInstance::TimePointToRaceTimePoint(const std::chrono::steady_clock::time_point& timePoint)
{
  // Amount of 100ns
  constexpr uint64_t IntervalConstant = 100;
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
    timePoint.time_since_epoch()).count() / IntervalConstant;
}

void RaceInstance::TickLoading()
{
  // Determine whether all racers have started racing.
  const bool allRacersLoaded = std::ranges::all_of(
    std::views::values(tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Racing
        || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
    });

  const bool loadTimeoutReached = std::chrono::steady_clock::now() >= stageTimeoutTimePoint;

  // If not all the racers have loaded yet and the timeout has not been reached yet
  // do not start the race.
  if (not allRacersLoaded && not loadTimeoutReached)
    return;

  if (loadTimeoutReached)
  {
    spdlog::warn("Room {} has reached the loading timeout threshold", roomUid);
  }

  for (auto& racer : tracker.GetRacers() | std::views::values)
  {
    // todo: handle the players that did not load in to the race.
    // for now just consider them disconnected
    if (racer.state != tracker::RaceTracker::Racer::State::Racing)
      racer.state = tracker::RaceTracker::Racer::State::Disconnected;
  }

  const auto mapBlockTemplate = _serverInstance.GetCourseRegistry().GetMapBlockInfo(
    raceMapBlockId);

  // Switch to the racing stage and set the timeout time point.
  stage = RaceInstance::Stage::Racing;
  stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(
    mapBlockTemplate.timeLimit);

  // Set up the race start time point.
  const auto now = std::chrono::steady_clock::now();
  raceStartTimePoint = now + std::chrono::seconds(
    mapBlockTemplate.waitTime);

  // Broadcast the race countdown.
  const protocol::AcCmdUserRaceCountdown raceCountdown{
    .raceStartTimestamp = TimePointToRaceTimePoint(raceStartTimePoint)};

  for (const network::ClientId clientId : clients)
  {
    _commandServer.QueueCommand<protocol::AcCmdUserRaceCountdown>(
      clientId,
      [&raceCountdown]()
      {
        return raceCountdown;
      });
  }
}

void RaceInstance::TickRacing()
{
  const bool raceTimeoutReached = std::chrono::steady_clock::now() >= stageTimeoutTimePoint;

  const bool isFinishing = std::ranges::any_of(
    std::views::values(tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Finishing;
    });

  // If the race is not finishing and the timeout was not reached
  // do not finish the race.
  if (not isFinishing && not raceTimeoutReached)
    return;

  stage = RaceInstance::Stage::Finishing;
  stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(15);

  // If the race timeout was reached notify the clients about the finale.
  if (raceTimeoutReached)
  {
    const protocol::AcCmdUserRaceFinalNotify notify{};

    // Broadcast the race final to client (does not break for waiting room racers)
    for (const ClientId& raceClientId : clients)
    {
      _commandServer.QueueCommand<decltype(notify)>(
        raceClientId,
        [notify]()
        {
          return notify;
        });
    }
  }
}

void RaceInstance::TickFinishing()
{
  // Determine whether all racers have finished.
  const bool allRacersFinished = std::ranges::all_of(
    std::views::values(tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Finishing
        || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
    });

  const bool finishTimeoutReached = std::chrono::steady_clock::now() >= stageTimeoutTimePoint;

  // If not all of the racer have finished yet and the timeout has not been reached yet
  // do not finish the race.
  if (not allRacersFinished && not finishTimeoutReached)
    return;

  if (finishTimeoutReached)
  {
    spdlog::warn("Room {} has reached the race timeout threshold", roomUid);
  }

  protocol::AcCmdRCRaceResultNotify raceResult{};

  std::map<uint32_t, data::Uid> scoreboard;
  for (const auto& [characterUid, racer] : tracker.GetRacers())
  {
    // todo: do not do this here i guess
    uint32_t courseTime = std::numeric_limits<uint32_t>::max();
    if (racer.state != tracker::RaceTracker::Racer::State::Disconnected)
      courseTime = racer.courseTime;

    scoreboard.try_emplace(courseTime, characterUid);
  }

  // Build the score board.
  for (auto& [courseTime, characterUid] : scoreboard)
  {
    auto& racer = tracker.GetRacer(characterUid);
    auto& score = raceResult.scores.emplace_back();

    // todo: figure out the other bit set values

    if (racer.state != tracker::RaceTracker::Racer::State::Disconnected)
    {
      score.bitset = static_cast<protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset>(
          protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset::Connected);
    }
    score.courseTime = courseTime;
    score.experience = 420;
    score.carrots = 420;
    const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
      characterUid);

    characterRecord.Mutable([this, &score](data::Character& character)
    {
      character.carrots() += score.carrots;
      character.experience() += score.experience;

      const uint32_t newLevel = _serverInstance.GetCharacterRegistry().GetLevelForExp(character.experience());
      if (newLevel > character.level())
      {
        character.level() = newLevel;
        score.bitset = static_cast<protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset>(
          score.bitset | protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset::LevelUp);
      }

      //populate the score info with the character data
      score.uid = character.uid();
      score.name = character.name();
      score.level = character.level();
      score.levelProgress = character.experience();

      _serverInstance.GetDataDirector().GetHorse(character.mountUid()).Immutable(
        [&score](const data::Horse& horse)
        {
          score.mountName = horse.name();
          score.horseClass = static_cast<uint8_t>(horse.clazz());
          score.growthPoints = static_cast<uint16_t>(horse.growthPoints());
        });
    });
  }

  // Broadcast the race result
  for (const ClientId raceClientId : clients)
  {
    _commandServer.QueueCommand<decltype(raceResult)>(
      raceClientId,
      [raceResult]()
      {
        return raceResult;
      });
  }

  // Clear the ready state of oll of the players.
  // todo: this should have been reset with the room instance data
  stage = RaceInstance::Stage::Waiting;
  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [this](Room& room)
    {
      room.SetRoomPlaying(false);
      for (auto& [uid, player] : room.GetPlayers())
      {
        player.SetReady(false);

        // Refresh this character's carrot balance
        // TODO: should this do it players in the Room or the current RaceInstance?
        _serverInstance.GetRaceDirector().RefreshCharacterCarrotBalance(uid);
      }
    });
}

void RaceInstance::Tick()
{
  // Tick instance based on which stage it is on
  switch(this->stage)
  {
    case RaceInstance::Stage::Waiting:
      // Do nothing, room is not racing
      break;
    case RaceInstance::Stage::Loading:
      TickLoading();
      break;
    case RaceInstance::Stage::Racing:
      TickRacing();
      break;
    case RaceInstance::Stage::Finishing:
      TickFinishing();
      break;
    default:
      throw std::runtime_error("Race instance stage is not recognised");
  }
}

} // namespace server
