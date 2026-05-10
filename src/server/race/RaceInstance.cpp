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

#include "server/ServerInstance.hpp"

#include "server/race/RaceDirector.hpp"
#include "server/race/RaceInstance.hpp"

#include <libserver/util/Util.hpp>

#include <limits>
#include <tuple>

namespace server
{

RaceInstance::RaceInstance(
  RaceDirector& raceDirector,
  uint32_t roomUid) :
    _raceDirector(raceDirector),
    _roomUid(roomUid)
{

}

uint32_t RaceInstance::GetRoomUid()
{
  return _roomUid;
}

void RaceInstance::GetRoom(const std::function<void(Room&)>& consumer)
{
  _raceDirector._serverInstance.GetRoomSystem().GetRoom(
    _roomUid,
    consumer);
}

void RaceInstance::GetRoom(const std::function<void(const Room&)>& consumer) const
{
  _raceDirector._serverInstance.GetRoomSystem().GetRoom(
    _roomUid,
    consumer);
}

void RaceInstance::TickLoading()
{
  auto& parameters = this->GetParameters();

  // Determine whether all racers have started racing.
  const bool allRacersLoaded = std::ranges::all_of(
    std::views::values(_tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Racing
        || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
    });

  const bool loadTimeoutReached = std::chrono::steady_clock::now() >= parameters.stageTimeoutTimePoint;

  // If not all the racers have loaded yet and the timeout has not been reached yet
  // do not start the race.
  if (not allRacersLoaded && not loadTimeoutReached)
    return;

  if (loadTimeoutReached)
  {
    spdlog::warn("Room {} has reached the loading timeout threshold",
      this->GetRoomUid());
  }

  for (auto& racer : _tracker.GetRacers() | std::views::values)
  {
    // todo: handle the players that did not load in to the race.
    // for now just consider them disconnected
    if (racer.state != tracker::RaceTracker::Racer::State::Racing)
      racer.state = tracker::RaceTracker::Racer::State::Disconnected;
  }

  const auto& mapBlockTemplate = _raceDirector._serverInstance.GetCourseRegistry().GetMapBlockInfo(
    parameters.raceMapBlockId);

  // Switch to the racing stage and set the timeout time point.
  parameters.stage = Parameters::Stage::Racing;
  parameters.stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(
    mapBlockTemplate.timeLimit);

  // Set up the race start time point.
  const auto now = std::chrono::steady_clock::now();
  parameters.raceStartTimePoint = now + std::chrono::seconds(
    mapBlockTemplate.waitTime);

  const protocol::AcCmdUserRaceCountdown raceCountdown{
    .raceStartTimestamp = util::TimePointToRaceTimePoint(
      parameters.raceStartTimePoint)};

  // Broadcast the race countdown.
  _raceDirector.Broadcast(*this, raceCountdown);
}

void RaceInstance::TickRacing()
{
  auto& parameters = this->GetParameters(); 

  const bool raceTimeoutReached = std::chrono::steady_clock::now() >= parameters.stageTimeoutTimePoint;

  const bool isFinishing = std::ranges::any_of(
    std::views::values(_tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Finishing;
    });

  // If the race is not finishing and the timeout was not reached
  // do not finish the race.
  if (not isFinishing && not raceTimeoutReached)
    return;

  parameters.stage = Parameters::Stage::Finishing;
  parameters.stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(15);

  // If the race timeout was reached notify the clients about the finale.
  if (not raceTimeoutReached)
    return;
  
  // Broadcast the race final (only to participants).
  this->GetRoom([this](const server::Room& room)
  {
    const protocol::AcCmdUserRaceFinalNotify notify{};
    for (const auto& [characterUid, player] : room.GetPlayers())
    {
      const bool isParticipant = _tracker.IsRacer(characterUid);
      if (not isParticipant)
        continue;

      _raceDirector.GetCommandServer().QueueCommand<decltype(notify)>(
        player.GetClientId(),
        [notify]()
        {
          return notify;
        });
    }
  });
}

void RaceInstance::TickFinishing()
{
  auto& parameters = this->GetParameters();

  // Determine whether all racers have finished.
  const bool allRacersFinished = std::ranges::all_of(
    std::views::values(_tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Finishing
        || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
    });

  const bool finishTimeoutReached = std::chrono::steady_clock::now() >= parameters.stageTimeoutTimePoint;

  // If not all of the racer have finished yet and the timeout has not been reached yet
  // do not finish the race.
  if (not allRacersFinished && not finishTimeoutReached)
    return;

  if (finishTimeoutReached)
  {
    spdlog::warn("Room {} has reached the race timeout threshold",
      this->GetRoomUid());
  }

  protocol::AcCmdRCRaceResultNotify raceResult{};

  using Team = tracker::RaceTracker::Racer::Team;
  using State = tracker::RaceTracker::Racer::State;

  // Determine winning team (team of the first finisher). Solo/FFA leaves winningTeam as Solo.
  Team winningTeam = Team::Solo;
  if (parameters.raceTeamMode == protocol::TeamMode::Team)
  {
    int32_t best = std::numeric_limits<int32_t>::max();
    for (const auto& [uid, racer] : _tracker.GetRacers())
    {
      if (racer.state != State::Disconnected && racer.courseTime != -1 && racer.courseTime < best)
      {
        best = racer.courseTime;
        winningTeam = racer.team;
      }
    }
  }

  // Build the score board.
  for (const auto& [characterUid, racer] : _tracker.GetRacers())
  {
    auto& score = raceResult.scores.emplace_back();

    // todo: figure out the other bit set values

    if (racer.state != State::Disconnected)
    {
      score.bitset = static_cast<protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset>(
          protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset::Connected);
    }
    score.courseTime = racer.state != State::Disconnected ? racer.courseTime : std::numeric_limits<int32_t>::max();
    score.experience = 420;
    score.carrots = 420;
    score.teamColor = racer.team;
    const auto characterRecord = _raceDirector._serverInstance.GetDataDirector().GetCharacter(
      characterUid);

    characterRecord.Mutable([this, &score](data::Character& character)
    {
      character.carrots() += score.carrots;
      character.experience() += score.experience;

      const uint32_t newLevel = _raceDirector._serverInstance.GetCharacterRegistry().GetLevelForExp(character.experience());
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

      _raceDirector._serverInstance.GetDataDirector().GetHorse(character.mountUid()).Immutable(
        [&score](const data::Horse& horse)
        {
          score.mountName = horse.name();
          score.horseClass = static_cast<uint8_t>(horse.clazz());
          score.growthPoints = static_cast<uint16_t>(horse.growthPoints());
        });
    });
  }

  // Sort: winning team first, then by result state, then by courseTime ascending.
  std::ranges::sort(raceResult.scores, [winningTeam](const auto& a, const auto& b)
  {
    auto priority = [winningTeam](const auto& score)
    {
      using ScoreInfo = protocol::AcCmdRCRaceResultNotify::ScoreInfo;

      const uint32_t bitset = static_cast<uint32_t>(score.bitset);
      const bool isConnected = (bitset & static_cast<uint32_t>(
        ScoreInfo::Bitset::Connected)) != 0;
      const bool hasValidTime = score.courseTime < static_cast<uint32_t>(
        std::numeric_limits<int32_t>::max());

      // Connected racers with no valid finish time are time-over/DNF and should rank
      // below timed finishers but above disconnected racers.
      const auto resultRank = not isConnected ? 2 : hasValidTime ? 0 : 1;

      return std::make_tuple(
        score.teamColor != winningTeam ? 1 : 0,
        resultRank,
        score.courseTime);
    };
    return priority(a) < priority(b);
  });

  // Broadcast the race result
  _raceDirector.Broadcast(*this, raceResult);

  // Assign room master to the first-place finisher.
  if (not raceResult.scores.empty())
  {
    data::Uid newMasterUid = raceResult.scores[0].uid;
    std::string newMasterName = raceResult.scores[0].name;
    this->GetRoom(
      [&newMasterUid, &newMasterName, scores = raceResult.scores](Room& room)
      {
        // Check if room even has players
        if (room.GetPlayerCount() < 1)
        {
          // TODO: mark room for delete
          newMasterUid = data::InvalidUid;
          return;
        }

        // Get room details to update
        auto& details = room.GetRoomDetails();

        // Check if room has this player
        if (room.HasPlayer(newMasterUid))
        {
          // New master exists in room
          details.masterUid = newMasterUid;
          return;
        }

        // New master left, proceed with the scores list
        // and assign until none found
        for (const auto& score : scores)
        {
          // Check that this next best player is in room
          if (not room.HasPlayer(score.uid))
            continue;
          
          // Character is in room, set room master to new uid
          newMasterUid = details.masterUid = score.uid;
          newMasterName = score.name;
          return;
        }

        // No characters available for room master (how?)
        newMasterUid = data::InvalidUid;
      });

    if (newMasterUid != data::InvalidUid)
    {
      const auto& winnerClientContext = _raceDirector.GetClientContextByCharacterUid(newMasterUid);
      spdlog::info("Player {} ({}) has won the match and is now master of [Room {}]",
        winnerClientContext.userName,
        newMasterName,
        this->GetRoomUid());
      
      const protocol::AcCmdCRChangeMasterNotify masterNotify{
        .masterUid = newMasterUid};
      _raceDirector.Broadcast(*this, masterNotify);
    }
  }

  // Clear the ready state of all of the players.
  // todo: this should have been reset with the room instance data
  parameters.stage = Parameters::Stage::Waiting;
  this->GetRoom(
    [this](Room& room)
    {
      room.SetRoomPlaying(false);
      for (auto& [uid, player] : room.GetPlayers())
      {
        // Set racer's ready state to false
        room.GetPlayer(uid).SetReady(false);

        // Update this racer's carrot balance
        protocol::AcCmdRCUpdateGameMoney updateGameMoney{};
        _raceDirector.GetServerInstance().GetDataDirector().GetCharacter(uid).Immutable(
          [&updateGameMoney](const data::Character& character)
          {
            updateGameMoney.carrotBalance = character.carrots();
          });

        _raceDirector.GetCommandServer().QueueCommand<protocol::AcCmdRCUpdateGameMoney>(
          player.GetClientId(),
          [updateGameMoney]()
          {
            return updateGameMoney;
          });
      }
    });
}

void RaceInstance::Tick()
{
  const auto& parameters = this->GetParameters();
  switch (parameters.stage)
  {
    case Parameters::Stage::Waiting:
      // Do nothing on waiting stage
      break;
    case Parameters::Stage::Loading:
      // Process rooms which are loading
      this->TickLoading();
      break;
    case Parameters::Stage::Racing:
      // Process rooms which are racing
      this->TickRacing();
      break;
    case Parameters::Stage::Finishing:
      // Process rooms which are finishing
      this->TickFinishing();
      break;
  }
}

RaceInstance::Parameters& RaceInstance::GetParameters()
{
  return _parameters;
}

const RaceInstance::Parameters& RaceInstance::GetParameters() const
{
  return _parameters;
}

tracker::RaceTracker& RaceInstance::GetTracker()
{
  return _tracker;
}

const tracker::RaceTracker& RaceInstance::GetTracker() const
{
  return _tracker;
}

} // namespace server
