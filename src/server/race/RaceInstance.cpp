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

#include "server/race/RaceInstance.hpp"
#include "server/race/RaceNetworkHandler.hpp"

#include <libserver/util/Util.hpp>

#include <tuple>
#include <format>

namespace server
{

namespace
{

constexpr registry::MapBlockId AllMapsCourseId = 10000;
constexpr registry::MapBlockId NewMapsCourseId = 10001;
constexpr registry::MapBlockId HotMapsCourseId = 10002;

std::random_device _randomDevice;

} // anon namespace

RaceInstance::RaceInstance(
  RaceNetworkHandler& raceDirector,
  const uint32_t roomUid)
  : _roomUid(roomUid)
  , _raceNetworkHandler(raceDirector)
{

}

void RaceInstance::GetRoom(const std::function<void(Room&)>& consumer)
{
  _raceNetworkHandler.GetServerInstance().GetRoomSystem().GetRoom(
    _roomUid,
    consumer);
}

void RaceInstance::GetRoom(const std::function<void(const Room&)>& consumer) const
{
  _raceNetworkHandler.GetServerInstance().GetRoomSystem().GetRoom(
    _roomUid,
    consumer);
}

bool RaceInstance::Start(
  const Parameters& parameters)
{
  _parameters = parameters;

  try
  {
    PrepareGameMode();
    PrepareMap();
  }
  catch (const std::runtime_error& e)
  {
    spdlog::error("Failed to start race instance: {}", e.what());
    return false;
  }

  _stage = Stage::Loading;
  _loadingStartTimePoint = Clock::now();
  // todo: configurable loading timeout
  _stageTimeoutTimePoint = _loadingStartTimePoint+ std::chrono::seconds(60);

  return true;
}

void RaceInstance::Stop()
{
  protocol::AcCmdRCRaceResultNotify raceResult{};

  using Team = tracker::RaceTracker::Racer::Team;
  using State = tracker::RaceTracker::Racer::State;

  // Determine winning team (team of the first finisher).
  // Solo/FFA leaves `winningTeam` as Solo.
  Team winningTeam = Team::Solo;
  if (_parameters.teamMode == protocol::TeamMode::Team)
  {
    uint32_t best = tracker::InvalidCourseTime;
    for (const auto& racer : _tracker.GetRacers() | std::views::values)
    {
      if (racer.state != State::Disconnected
        && racer.courseTime != tracker::InvalidCourseTime
        && racer.courseTime < best)
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
      score.bitset = protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset::Connected;
    }

    // If the player has disconnected
    score.courseTime = racer.state != State::Disconnected
      ? racer.courseTime
      : tracker::InvalidCourseTime;

    score.experience = 420;
    score.carrots = 2500;
    score.teamColor = racer.team;
    const auto characterRecord = _raceNetworkHandler.GetServerInstance().GetDataDirector().GetCharacter(
      characterUid);

    characterRecord.Mutable([this, &score](data::Character& character)
    {
      character.carrots() += score.carrots;
      character.experience() += score.experience;

      const uint32_t newLevel = _raceNetworkHandler.GetServerInstance().GetCharacterRegistry().GetLevelForExp(character.experience());
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

      _raceNetworkHandler.GetServerInstance().GetDataDirector().GetHorse(character.mountUid()).Immutable(
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
      const bool hasValidTime = score.courseTime < tracker::InvalidCourseTime;

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
  _raceNetworkHandler.Broadcast(*this, raceResult);

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
      const auto userName = _raceNetworkHandler.GetServerInstance().GetLobbyDirector().GetUserByCharacterUid(
        newMasterUid).userName;
      spdlog::info("Player {} ({}) has won the match and is now master of [Room {}]",
        userName,
        newMasterName,
        this->GetRoomUid());

      const protocol::AcCmdCRChangeMasterNotify masterNotify{
        .masterUid = newMasterUid};
      _raceNetworkHandler.Broadcast(*this, masterNotify);
    }
  }

  // Clear the ready state of all of the players and update their balances.
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
        _raceNetworkHandler.GetServerInstance().GetDataDirector().GetCharacter(uid).Immutable(
          [&updateGameMoney](const data::Character& character)
          {
            updateGameMoney.carrotBalance = character.carrots();
          });

        _raceNetworkHandler.GetCommandServer().QueueCommand<protocol::AcCmdRCUpdateGameMoney>(
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
  try
  {
    switch (_stage)
    {
      case Stage::Waiting:
        // Do nothing on waiting stage
        break;
      case Stage::Loading:
        // Process rooms which are loading
        this->TickLoading();
        break;
      case Stage::Racing:
        // Process rooms which are racing
        this->TickRacing();
        break;
      case Stage::Finishing:
        // Process rooms which are finishing
        this->TickFinishing();
        break;
    }
  }
  catch (const std::exception& x)
  {
    spdlog::error("Exception ticking race instance {}: {}", GetRoomUid(), x.what());
  }
}

uint32_t RaceInstance::GetRoomUid()
{
  return _roomUid;
}

const RaceInstance::Parameters& RaceInstance::GetParameters() const
{
  return _parameters;
}

registry::GameModeId RaceInstance::GetGameModeId() const
{
  return _gameModeId;
}

registry::MapBlockId RaceInstance::GetMapBlockId() const
{
  return _mapBlockId;
}

std::chrono::steady_clock::time_point RaceInstance::GetLoadingStartTimePoint() const noexcept
{
  return _loadingStartTimePoint;
}

std::chrono::steady_clock::time_point RaceInstance::GetRaceStartTimePoint() const noexcept
{
  return _raceStartTimePoint;
}

RaceInstance::Stage RaceInstance::GetStage() const noexcept
{
  return _stage;
}

std::chrono::steady_clock::time_point RaceInstance::GetStageTimeoutTimePoint() const noexcept
{
  return _stageTimeoutTimePoint;
}

tracker::RaceTracker& RaceInstance::GetTracker()
{
  return _tracker;
}

const tracker::RaceTracker& RaceInstance::GetTracker() const
{
  return _tracker;
}

void RaceInstance::TickLoading()
{
  // Determine whether all racers have started racing.
  const bool allRacersLoaded = std::ranges::all_of(
    std::views::values(_tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Racing
        || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
    });

  const bool loadTimeoutReached = std::chrono::steady_clock::now() >= _stageTimeoutTimePoint;

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

  const auto& mapBlockTemplate = _raceNetworkHandler
    .GetServerInstance()
    .GetCourseRegistry()
    .GetMapBlockInfo(GetMapBlockId());

  // Switch to the racing stage and set the timeout time point.
  _stage = Stage::Racing;
  _stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(
    mapBlockTemplate.timeLimit);

  // Set up the race start time point.
  const auto now = std::chrono::steady_clock::now();
  _raceStartTimePoint = now + std::chrono::seconds(
    mapBlockTemplate.waitTime);

  const protocol::AcCmdUserRaceCountdown raceCountdown{
    .raceStartTimestamp = util::TimePointToRaceTimePoint(
      _raceStartTimePoint)};

  // Broadcast the race countdown.
  _raceNetworkHandler.Broadcast(*this, raceCountdown);
}

void RaceInstance::TickRacing()
{
  const bool raceTimeoutReached = std::chrono::steady_clock::now() >= _stageTimeoutTimePoint;

  const bool isFinishing = std::ranges::any_of(
    std::views::values(_tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Finishing;
    });

  // If the race is not finishing and the timeout was not reached
  // do not finish the race.
  if (not isFinishing && not raceTimeoutReached)
  {
    // Tick main race functions
    this->TickItemSpawners();
    if (this->GetParameters().gameMode == protocol::GameMode::Magic)
      // Tick magic gauge
      this->TickMagicGauge();
    return;
  }

  _stage = Stage::Finishing;
  _stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(15);

  // If the race timeout was reached notify the clients about the finale.
  if (not raceTimeoutReached)
    return;

  // Broadcast the race final (only to participants).
  this->GetRoom([this](const Room& room)
  {
    for (const auto& [characterUid, player] : room.GetPlayers())
    {
      const bool isParticipant = _tracker.IsRacer(characterUid);
      if (not isParticipant)
        continue;

      const protocol::AcCmdUserRaceFinalNotify notify{};
      _raceNetworkHandler.GetCommandServer().QueueCommand<decltype(notify)>(
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
  // Determine whether all racers have finished.
  const bool allRacersFinished = std::ranges::all_of(
    std::views::values(_tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Finishing
        || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
    });

  const bool finishTimeoutReached = std::chrono::steady_clock::now() >= _stageTimeoutTimePoint;

  // If not all the racers have finished yet and the timeout has not been reached yet
  // do not finish the race.
  if (not allRacersFinished && not finishTimeoutReached)
    return;

  if (finishTimeoutReached)
  {
    spdlog::warn("Room {} has reached the race timeout threshold",
      this->GetRoomUid());
  }

  Stop();
 _stage = Stage::Waiting;
}

void RaceInstance::TickItemSpawners()
{
  constexpr double ItemSpawnDistanceThreshold = 90.0;

  const auto processItemSpawn = [&](
    ClientId clientId,
    tracker::RaceTracker::Racer& racer,
    const tracker::Oid oid,
    const uint32_t itemType,
    const protocol::Vector3& position,
    const uint32_t spawnStyle = 0)
  {
    const auto distance = (racer.position - position).Length();

    const bool isInProximity = distance < ItemSpawnDistanceThreshold;
    const bool isAlreadyTracked = racer.trackedDecks.contains(oid);

    if (isAlreadyTracked)
    {
      if (not isInProximity)
        racer.trackedDecks.erase(oid);

      return;
    }

    // If item is not in proximity or this is not the first pass
    // then do not trigger item spawn
    if (not isInProximity and not this->GetTracker().firstPassItemSpawn)
      return;

    const protocol::AcCmdRCCreateItem spawn{
      .itemId = oid,
      .itemType = itemType,
      .position = position,
      .spawnStyle = spawnStyle,
      .spawnerId = 0,
      .sizeLevel = 0};

    racer.trackedDecks.insert(oid);
    _raceNetworkHandler.GetCommandServer().QueueCommand<decltype(spawn)>(
      clientId,
      [spawn]()
      {
        return spawn;
      });
  };

  // Loop through each player in the room
  this->GetRoom([this, &processItemSpawn](const Room& room)
  {
    for (const auto& [characterUid, player] : room.GetPlayers())
    {
      // Check if this player is an active racer
      if (not this->GetTracker().IsRacer(characterUid))
        continue;

      auto& racer = this->GetTracker().GetRacer(characterUid);
      for (const auto& item : this->GetTracker().GetItemDecks() | std::views::values)
      {
        if (std::chrono::steady_clock::now() < item.respawnTimePoint)
          continue;

        processItemSpawn(
          player.GetClientId(),
          racer,
          item.oid,
          item.currentItem,
          item.position);
      }

      for (const auto& eventItem : racer.eventItems)
        processItemSpawn(
          player.GetClientId(),
          racer,
          eventItem.oid,
          eventItem.itemType,
          eventItem.position,
          3);
    }
  });

  // Flip first pass item spawn logic
  if (this->GetTracker().firstPassItemSpawn)
    this->GetTracker().firstPassItemSpawn = false;
}

void RaceInstance::TickMagicGauge()
{
  // Only regenerate magic during an active race (after the countdown finishes)
  const auto now = std::chrono::steady_clock::now();
  if (now <= this->GetRaceStartTimePoint())
    return;

  this->GetRoom([this, &now](const Room& room)
  {
    const auto& regenerationInfo = _raceNetworkHandler.GetServerInstance().GetMagicRegistry().GetRegenInfo();
    const auto tickInterval = std::chrono::milliseconds(regenerationInfo.intervalMs);

    for (const auto& [characterUid, player] : room.GetPlayers())
    {
      // Check if this player is an active racer
      if (not this->GetTracker().IsRacer(characterUid))
        continue;

      auto& racer = this->GetTracker().GetRacer(characterUid);
      const bool isRacerHoldingItem = racer.magicItem.has_value();

      // Anchor at race start so fill time is consistent regardless of when the first pos-update arrives.
      if (racer.lastGaugeUpdateTimePoint == std::chrono::steady_clock::time_point::max())
        racer.lastGaugeUpdateTimePoint = this->GetRaceStartTimePoint();

      // Elapsed time since the last gauge update.
      const auto elapsed = now - racer.lastGaugeUpdateTimePoint;
      const auto elapsedTickCount = elapsed / tickInterval;

      if (elapsedTickCount > 0)
      {
        racer.lastGaugeUpdateTimePoint = now;

        if (not isRacerHoldingItem and racer.starPointValue < _gameModeInfo.starPointsMax)
        {
          uint32_t gainedPerTick = regenerationInfo.pointPerTick
            * (1000u + regenerationInfo.courageScaleBp * racer.mountStats.courage) / 1000u;

          // BufGauge buff doubles regen while active.
          if (racer.effects[20] or racer.effects[21])
            gainedPerTick *= 2;

          const uint32_t totalGain = gainedPerTick * static_cast<uint32_t>(elapsedTickCount);
          racer.starPointValue = std::min(
            _gameModeInfo.starPointsMax,
            racer.starPointValue + totalGain);
        }
      }

      const bool shouldGiveItem =
        not isRacerHoldingItem and
        racer.starPointValue >= _gameModeInfo.starPointsMax;

      const protocol::AcCmdCRStarPointGetOK starPointResponse{
        .characterOid = racer.oid,
        .starPointValue = racer.starPointValue,
        .giveMagicItem = shouldGiveItem};

      _raceNetworkHandler.GetCommandServer().QueueCommand<decltype(starPointResponse)>(
        player.GetClientId(),
        [starPointResponse]
        {
          return starPointResponse;
        });
    }
  });
}

void RaceInstance::PrepareGameMode()
{
  _gameModeId = static_cast<registry::GameModeId>(_parameters.gameMode);
  _gameModeInfo = _raceNetworkHandler
    .GetServerInstance()
    .GetCourseRegistry()
    .GetCourseGameModeInfo(_gameModeId);
}

void RaceInstance::PickRandomMapFromCourse()
{
  uint32_t masterLevel{};
  // Use the room master's level to filter the maps
  _raceNetworkHandler.GetServerInstance()
    .GetDataDirector()
    .GetCharacter(_parameters.masterUid)
    .Immutable(
      [&masterLevel](const data::Character& character)
      {
        masterLevel = character.level();
      });

  // Filter out the maps that are above the master's level.
  std::vector<registry::MapBlockId> filtered;
  std::ranges::copy_if(
    std::as_const(_gameModeInfo.mapBlockPool),
    std::back_inserter(filtered),
    [this, masterLevel](registry::MapBlockId mapBlockId)
    {
      try
      {
        const auto& mapBlockInfo = _raceNetworkHandler.GetServerInstance()
          .GetCourseRegistry()
          .GetMapBlockInfo(
            mapBlockId);
        return mapBlockInfo.requiredLevel <= masterLevel;
      }
      catch (const std::exception& e)
      {
        spdlog::warn("Failed to get map block info for mapBlockId {}: {}", mapBlockId, e.what());
        return false;
      }
    });

  // Select a random map from the pool.
  std::uniform_int_distribution<registry::MapBlockId> distribution(
    0,
    static_cast<int>(filtered.size() - 1));

  _mapBlockId = filtered[distribution(_randomDevice)];
}

void RaceInstance::PrepareMap()
{
  if (_gameModeInfo.mapBlockPool.empty())
  {
    throw std::runtime_error(
      std::format(
        "Game mode {} does not have any maps",
        _gameModeId));
  }

  // If the map is set to a course pick a random map.
  if (_parameters.mapBlockId == AllMapsCourseId
    || _parameters.mapBlockId == NewMapsCourseId
    || _parameters.mapBlockId == HotMapsCourseId)
  {
    PickRandomMapFromCourse();
  }
  else
  {
    _mapBlockId = _parameters.mapBlockId;
  }

  _mapBlockInfo = _raceNetworkHandler
    .GetServerInstance()
    .GetCourseRegistry()
    .GetMapBlockInfo(_mapBlockId);

  try
  {
    // Prepare the item decks on the map.
    PrepareItemDecks();
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error(
      std::format(
        "Exception while preparing items for game mode {} and map id {}: {}",
        _gameModeId,
        _mapBlockId,
        e.what()));
  }
}

void RaceInstance::PickRandomItemFromDeck(tracker::RaceTracker::ItemDeck& deck)
{
  if (deck.items.empty())
    return;

  std::uniform_int_distribution<size_t> distribution(0, deck.items.size() - 1);
  deck.currentItem = deck.items[distribution(_randomDevice)];
}

void RaceInstance::PrepareItemDecks()
{
  // Get the map position offset
  const auto& offset = _mapBlockInfo.offset;

  // Create item decks based on the game mode.
  for (const registry::DeckId usedDeckId : _gameModeInfo.usedDeckIds)
  {
    const auto& deckInfo = _raceNetworkHandler.GetServerInstance().GetCourseRegistry().GetDeckInfo(
      usedDeckId);

    for (const auto& deckInstance : _mapBlockInfo.itemDecks)
    {
      if (deckInstance.deckId != usedDeckId)
        continue;

      auto& deck = _tracker.AddItemDeck();
      deck.items = deckInfo.items;
      deck.respawnTime = deckInfo.respawnTime;

      deck.position = deckInstance.position + offset;

      PickRandomItemFromDeck(deck);
    }
  }
}

} // namespace server
