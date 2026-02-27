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

#include "server/race/RaceDirector.hpp"

#include "server/ServerInstance.hpp"
#include "server/system/RoomSystem.hpp"

#include <libserver/data/helper/ProtocolHelper.hpp>

#include <boost/container_hash/hash.hpp>
#include <spdlog/spdlog.h>

#include <bitset>

namespace server
{

namespace
{

//! Converts a steady clock's time point to a race clock's time point.
//! @param timePoint Time point.
//! @return Race clock time point.
uint64_t TimePointToRaceTimePoint(const std::chrono::steady_clock::time_point& timePoint)
{
  // Amount of 100ns
  constexpr uint64_t IntervalConstant = 100;
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
    timePoint.time_since_epoch()).count() / IntervalConstant;
}

const std::array<uint32_t, 12> magicItems = {
  2, // FireBall     Team mode 0
  4, // WaterShield  Team mode 0
  6, // Booster      Team mode 0
  8, // HotRodding   Team mode 0
  10, // IceWall     Team mode 0
  12, // JumpStun    Team mode 0
  14, // DarkFire    Team mode 0
  16, // Summon      Team mode 0
  18, // Lightning   Team mode 0
  20, // BufPower    Team mode 1
  22, // BufGauge    Team mode 1
  24, // BufSpeed    Team mode 1
};

bool RollCritical(tracker::RaceTracker::Racer& racer)
{
  if (racer.critChance) {
    return true;
  }
  return (rand() % 100) < 5;
}

uint32_t RandomMagicItem(tracker::RaceTracker::Racer& racer)
{
  static std::random_device rd;
  std::uniform_int_distribution distribution(0, racer.team == tracker::RaceTracker::Racer::Team::Solo ? 8 : 11);
  uint32_t magicItemId = magicItems[distribution(rd)];
  if (RollCritical(racer))
  {
    magicItemId += 1;
  }
  return magicItemId;
}

bool SkillIsCritical(uint32_t skillOrEffectId)
{
  return skillOrEffectId % 2 != 0;
}

} // anon namespace

RaceDirector::RaceDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
{
  _commandServer.RegisterCommandHandler<protocol::AcCmdCREnterRoom>(
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRoom(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeRoomOptions>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeRoomOptions(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeTeam>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeTeam(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRLeaveRoom>(
    [this](ClientId clientId, const auto&)
    {
      HandleLeaveRoom(clientId);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStartRace>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStartRace(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceTimer>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceTimer(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRLoadingComplete>(
    [this](ClientId clientId, const auto& message)
    {
      HandleLoadingComplete(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRReadyRace>(
    [this](ClientId clientId, const auto& message)
    {
      HandleReadyRace(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceFinal>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUserRaceFinal(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRaceResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRP2PResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleP2PRaceResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceP2PResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleP2PUserRaceResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRAwardStart>(
    [this](ClientId clientId, const auto& message)
    {
      // TODO: is this the correct place to process stamp rewards before scoreboard/stamp progress is shown?
      HandleStampReward(clientId);
      HandleAwardStart(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRAwardEnd>(
    [this](ClientId clientId, const auto& message)
    {
      HandleAwardEnd(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStarPointGet>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStarPointGet(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRequestSpur>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestSpur(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRHurdleClearResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleHurdleClearResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStartingRate>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStartingRate(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceUpdatePos>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceUserPos(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChat>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChat(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRelayCommand>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRelayCommand(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRelay>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRelay(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceActivateInteractiveEvent>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUserRaceActivateInteractiveEvent(clientId, message);
    });

  // _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceActivateEvent>(
  //   [this](ClientId clientId, const auto& message)
  //    {
  //      HandleUserRaceActivateEvent(clientId, message);
  //    });

  // _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceDeactivateEvent>(
  //   [this](ClientId clientId, const auto& message)
  //   {
  //     HandleUserRaceDeactivateEvent(clientId, message);
  //   });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRequestMagicItem>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestMagicItem(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRUseMagicItem>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUseMagicItem(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceItemGet>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUserRaceItemGet(clientId, message);
    });

  // Magic Targeting Commands for Bolt System
  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStartMagicTarget>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStartMagicTarget(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeMagicTarget>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeMagicTarget(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRActivateSkillEffect>(
    [this](ClientId clientId, const auto& message)
    {
      HandleActivateSkillEffect(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeSkillCardPresetID>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeSkillCardPresetId(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCROpCmd>(
    [this](ClientId clientId, const auto& message)
    {
      HandleOpCmd(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRInviteUser>(
    [this](ClientId clientId, const auto& message)
    {
      HandleInviteUser(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRKick>(
    [this](ClientId clientId, const auto& message)
    {
      HandleKickUser(clientId, message);
    });
}

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  test = std::thread([this]()
  {
    std::unordered_set<asio::ip::udp::endpoint> _clients;

    asio::io_context ioCtx;
    asio::ip::udp::socket skt(
      ioCtx,
      asio::ip::udp::endpoint(
        asio::ip::address_v4::loopback(),
        10500));

    asio::streambuf readBuffer;
    asio::streambuf writeBuffer;

    while (run_test)
    {
      const auto request = readBuffer.prepare(1024);
      asio::ip::udp::endpoint sender;

      try
      {
        const size_t bytesRead = skt.receive_from(request, sender);

        struct RelayHeader
        {
          uint16_t member0{};
          uint16_t member1{};
          uint16_t member2{};
        };

        const auto response = writeBuffer.prepare(1024);

        RelayHeader* header = static_cast<RelayHeader*>(response.data());
        header->member2 = 1;

        for (const auto idx : std::views::iota(0ull, bytesRead))
        {
          static_cast<std::byte*>(response.data())[idx + sizeof(RelayHeader)] = static_cast<const std::byte*>(
            request.data())[idx];
        }

        writeBuffer.commit(bytesRead + sizeof(RelayHeader));
        readBuffer.consume(bytesRead + sizeof(RelayHeader));

        for (auto& client : _clients)
        {
          if (client == sender)
            continue;

          writeBuffer.consume(
            skt.send_to(writeBuffer.data(), client));
        }

        if (not _clients.contains(sender))
          _clients.insert(sender);

      } catch (const std::exception&) {
      }
    }
  });
  test.detach();

  _commandServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void RaceDirector::Terminate()
{
  run_test = false;
  _commandServer.EndHost();
}

void RaceDirector::Tick()
{
  try
  {
    _scheduler.Tick();
  }
  catch (const std::exception& x)
  {
    spdlog::error("Exception ticking a race scheduler: {}", x.what());
  }

  // Process rooms which are loading
  for (auto& [raceUid, raceInstance] : _raceInstances)
  {
    if (raceInstance.stage != RaceInstance::Stage::Loading)
      continue;

    // Determine whether all racers have started racing.
    const bool allRacersLoaded = std::ranges::all_of(
      std::views::values(raceInstance.tracker.GetRacers()),
      [](const tracker::RaceTracker::Racer& racer)
      {
        return racer.state == tracker::RaceTracker::Racer::State::Racing
          || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
      });

    const bool loadTimeoutReached = std::chrono::steady_clock::now() >= raceInstance.stageTimeoutTimePoint;

    // If not all the racers have loaded yet and the timeout has not been reached yet
    // do not start the race.
    if (not allRacersLoaded && not loadTimeoutReached)
      continue;

    if (loadTimeoutReached)
    {
      spdlog::warn("Room {} has reached the loading timeout threshold", raceUid);
    }

    for (auto& racer : raceInstance.tracker.GetRacers() | std::views::values)
    {
      // todo: handle the players that did not load in to the race.
      // for now just consider them disconnected
      if (racer.state != tracker::RaceTracker::Racer::State::Racing)
        racer.state = tracker::RaceTracker::Racer::State::Disconnected;
    }

    const auto mapBlockTemplate = _serverInstance.GetCourseRegistry().GetMapBlockInfo(
      raceInstance.raceMapBlockId);

    // Switch to the racing stage and set the timeout time point.
    raceInstance.stage = RaceInstance::Stage::Racing;
    raceInstance.stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(
      mapBlockTemplate.timeLimit);

    // Set up the race start time point.
    const auto now = std::chrono::steady_clock::now();
    raceInstance.raceStartTimePoint = now + std::chrono::seconds(
      mapBlockTemplate.waitTime);

    const protocol::AcCmdUserRaceCountdown raceCountdown{
      .raceStartTimestamp = TimePointToRaceTimePoint(
        raceInstance.raceStartTimePoint)};

    // Broadcast the race countdown.
    std::scoped_lock lock(raceInstance.clientsMutex);
    for (const ClientId& raceClientId : raceInstance.clients)
    {
      _commandServer.QueueCommand<protocol::AcCmdUserRaceCountdown>(
        raceClientId,
        [&raceCountdown]()
        {
          return raceCountdown;
        });
    }
  }

  // Process rooms which are racing
  for (auto& [raceUid, raceInstance] : _raceInstances)
  {
    if (raceInstance.stage != RaceInstance::Stage::Racing)
      continue;

    const bool raceTimeoutReached = std::chrono::steady_clock::now() >= raceInstance.stageTimeoutTimePoint;

    const bool isFinishing = std::ranges::any_of(
      std::views::values(raceInstance.tracker.GetRacers()),
      [](const tracker::RaceTracker::Racer& racer)
      {
        return racer.state == tracker::RaceTracker::Racer::State::Finishing;
      });

    // If the race is not finishing and the timeout was not reached
    // do not finish the race.
    if (not isFinishing && not raceTimeoutReached)
      continue;

    raceInstance.stage = RaceInstance::Stage::Finishing;
    raceInstance.stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(15);

    // If the race timeout was reached notify the clients about the finale.
    if (raceTimeoutReached)
    {
      protocol::AcCmdUserRaceFinalNotify notify{};

      // Broadcast the race final.
      std::scoped_lock lock(raceInstance.clientsMutex);
      for (const ClientId& raceClientId : raceInstance.clients)
      {
        bool isParticipant = false;
        try
        {
          const auto& raceClientContext = GetClientContext(raceClientId);
          isParticipant = raceInstance.tracker.IsRacer(
            raceClientContext.characterUid);
        }
        catch ([[maybe_unused]] const std::exception& x)
        {
          // the client has disconnected
          // this is a data race
        }

        if (not isParticipant)
          continue;

        _commandServer.QueueCommand<decltype(notify)>(
          raceClientId,
          [notify]()
          {
            return notify;
          });
      }
    }
  }

  // Process rooms which are finishing
  for (auto& [raceUid, raceInstance] : _raceInstances)
  {
    if (raceInstance.stage != RaceInstance::Stage::Finishing)
      continue;

    // Determine whether all racers have finished.
    const bool allRacersFinished = std::ranges::all_of(
      std::views::values(raceInstance.tracker.GetRacers()),
      [](const tracker::RaceTracker::Racer& racer)
      {
        return racer.state == tracker::RaceTracker::Racer::State::Finishing
          || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
      });

    const bool finishTimeoutReached = std::chrono::steady_clock::now() >= raceInstance.stageTimeoutTimePoint;

    // If not all of the racer have finished yet and the timeout has not been reached yet
    // do not finish the race.
    if (not allRacersFinished && not finishTimeoutReached)
      continue;

    if (finishTimeoutReached)
    {
      spdlog::warn("Room {} has reached the race timeout threshold", raceUid);
    }

    protocol::AcCmdRCRaceResultNotify raceResult{};

    std::map<uint32_t, data::Uid> scoreboard;
    for (const auto& [characterUid, racer] : raceInstance.tracker.GetRacers())
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
      auto& racer = raceInstance.tracker.GetRacer(characterUid);
      auto& score = raceResult.scores.emplace_back();

      // todo: figure out the other bit set values

      if (racer.state != tracker::RaceTracker::Racer::State::Disconnected)
      {
        score.bitset = static_cast<protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset>(
            protocol::AcCmdRCRaceResultNotify::ScoreInfo::Bitset::Connected);
      }

      score.courseTime = courseTime;
      //score.experience = 420;
      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        characterUid);

      characterRecord.Immutable([this, &score](const data::Character& character)
      {
        score.uid = character.uid();
        score.name = character.name();
        score.level = character.level();

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
    for (const ClientId raceClientId : raceInstance.clients)
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
    raceInstance.stage = RaceInstance::Stage::Waiting;
    _serverInstance.GetRoomSystem().GetRoom(
      raceUid,
      [](Room& room)
      {
        room.SetRoomPlaying(false);
        for (auto& [uid, player] : room.GetPlayers())
        {
          player.SetReady(false);
        }
      });
  }
}

void RaceDirector::BroadcastChangeRoomOptions(
  const data::Uid& roomUid,
  const protocol::AcCmdCRChangeRoomOptionsNotify notify)
{
  auto& raceInstance = _raceInstances[roomUid];
  std::scoped_lock lock(raceInstance.clientsMutex);
  for (const auto raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RaceDirector::HandleClientConnected(ClientId clientId)
{
  _clients.try_emplace(clientId);

  spdlog::debug(
    "Client {} connected to the race server from {}",
    clientId,
    _commandServer.GetClientAddress(clientId).to_string());
}

void RaceDirector::HandleClientDisconnected(ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId, false);
  if (clientContext.isAuthenticated)
  {
    const auto raceIter = _raceInstances.find(clientContext.roomUid);
    if (raceIter != _raceInstances.cend())
    {
      HandleLeaveRoom(clientId);
    }
  }

  spdlog::info("Client {} disconnected from the race server", clientId);
  _clients.erase(clientId);
}

void RaceDirector::DisconnectCharacter(data::Uid characterUid)
{
  try
  {
    _commandServer.DisconnectClient(GetClientIdByCharacterUid(characterUid));
  }
  catch (const std::exception&)
  {
    // We really don't care.
  }
}

ServerInstance& RaceDirector::GetServerInstance()
{
  return _serverInstance;
}

Config::Race& RaceDirector::GetConfig()
{
  return GetServerInstance().GetSettings().race;
}

RaceDirector::ClientContext& RaceDirector::GetClientContext(ClientId clientId, bool requireAuthorized)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Race client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthorized && not clientContext.isAuthenticated)
    throw std::runtime_error("Race client is not authenticated");

  return clientContext;
}

ClientId RaceDirector::GetClientIdByCharacterUid(data::Uid characterUid)
{
  for (auto& [clientId, clientContext] : _clients)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientId;
  }

  throw std::runtime_error("Character not associated with any client");
}

RaceDirector::ClientContext& RaceDirector::GetClientContextByCharacterUid(
  data::Uid characterUid)
{
  for (auto& clientContext : _clients | std::views::values)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientContext;
  }

  throw std::runtime_error("Character not associated with any client");
}

void RaceDirector::HandleEnterRoom(
  ClientId clientId,
  const protocol::AcCmdCREnterRoom& command)
{
  auto& clientContext = _clients[clientId];

  size_t identityHash = std::hash<uint32_t>()(command.characterUid);
  boost::hash_combine(identityHash, command.roomUid);

  clientContext.isAuthenticated = _serverInstance.GetOtpSystem().AuthorizeCode(
    identityHash,
    command.oneTimePassword);

  const bool doesRoomExist = _serverInstance.GetRoomSystem().RoomExists(
    command.roomUid);

  // Determine the racer count and whether the room is full.
  bool isOvercrowded = false;
  if (clientContext.isAuthenticated)
  {
    _serverInstance.GetRoomSystem().GetRoom(
      command.roomUid,
      [&isOvercrowded, characterUid = command.characterUid](Room& room)
      {
        // If the player is not able to be added, the room is full.
        isOvercrowded = not room.AddPlayer(characterUid);
      });
  }

  // Cancel the enter room if the client is not authenticated,
  // the room does not exist or the room is full.
  if (not clientContext.isAuthenticated
    || not doesRoomExist
    || isOvercrowded)
  {
    const protocol::AcCmdCREnterRoomCancel response{};
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  // The client is authorized so we can trust the identifiers
  // that were provided.
  clientContext.characterUid = command.characterUid;
  clientContext.roomUid = command.roomUid;

  // Try to emplace the room instance.
  const auto& [raceInstanceIter, inserted] = _raceInstances.try_emplace(
    command.roomUid);
  auto& raceInstance = raceInstanceIter->second;

  // If the room instance was just created, set it up.
  if (inserted)
  {
    raceInstance.masterUid = command.characterUid;
  }

  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [roomUid = clientContext.roomUid, inserted](
      const data::Character& character)
    {
      if (inserted)
        spdlog::info("Player '{}' has created a room {}", character.name(), roomUid);
      else
        spdlog::info("Player '{}' has joined the room {}", character.name(), roomUid);
    });

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});

  protocol::AcCmdCREnterRoomOK response{
    .isRoomWaiting = raceInstance.stage == RaceInstance::Stage::Waiting,
    .uid = command.roomUid};

  try
  {
    _serverInstance.GetRoomSystem().GetRoom(
      command.roomUid,
      [&response](Room& room)
      {
        const auto& roomDetails = room.GetRoomDetails();
        response.roomDescription = {
          .name = roomDetails.name,
          .maxPlayerCount = static_cast<uint8_t>(roomDetails.maxPlayerCount),
          .password = roomDetails.password,
          .gameModeMaps = static_cast<uint8_t>(roomDetails.gameMode),
          .gameMode = static_cast<protocol::GameMode>(roomDetails.gameMode),
          .mapBlockId = roomDetails.courseId,
          .teamMode = static_cast<protocol::TeamMode>(roomDetails.teamMode),
          .missionId = roomDetails.missionId,
          .unk6 = roomDetails.npcDifficulty,
          .skillBracket = roomDetails.skillBracket};
      });
  }
  catch (const std::exception&)
  {
    throw std::runtime_error("Client tried entering a deleted room");
  }

  protocol::Racer joiningRacer;

  // Collect the room players.
  std::vector<data::Uid> characterUids;
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&characterUids](Room& room)
    {
      for (const auto& characterUid : room.GetPlayers() | std::views::keys)
      {
        characterUids.emplace_back(characterUid);
      }
    });

  // Build the room players.
  for (const auto& characterUid : characterUids)
  {
    auto& protocolRacer = response.racers.emplace_back();

    bool isPlayerReady = false;
    Room::Player::Team team;

    _serverInstance.GetRoomSystem().GetRoom(
      clientContext.roomUid,
      [&isPlayerReady, &team, characterUid](Room& room)
      {
        const auto& player = room.GetPlayer(characterUid);
        isPlayerReady = player.IsReady();
        team = player.GetTeam();
      });

    // Fill data from the character record.
    const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
      characterUid);
    characterRecord.Immutable(
      [this, isPlayerReady, team, &protocolRacer, leaderUid = raceInstance.masterUid](
        const data::Character& character)
      {
        if (character.uid() == leaderUid)
          protocolRacer.isMaster = true;

        const auto& settingsRecord = GetServerInstance().GetDataDirector().GetSettings(character.settingsUid());
        if (settingsRecord.IsAvailable())
        {
          settingsRecord.Immutable(
            [&protocolRacer, modelId = character.parts.modelId()](const data::Settings& settings)
            {
              if (not settings.hideAge())
              {
                // TODO: Add age here (find if it is even possible)
                // todo: model constants
                protocolRacer.gender =
                  modelId == 10 ? protocol::Gender::Boy :
                  modelId == 20 ? protocol::Gender::Girl :
                  throw std::runtime_error("Character gender not recognised by model ID");
              }
            });
        }
        else
        {
          spdlog::warn("Settings record for character {} was not found, skipping role/gender assignment...",
            character.uid());
        }

        protocolRacer.level = character.level();
        protocolRacer.uid = character.uid();
        protocolRacer.name = character.name();
        protocolRacer.role = static_cast<protocol::Racer::Role>(character.role());
        protocolRacer.isHidden = false;
        protocolRacer.isNPC = false;
        protocolRacer.isReady = isPlayerReady;

        switch (team)
        {
          case Room::Player::Team::Red:
            protocolRacer.teamColor = protocol::TeamColor::Red;
            break;
          case Room::Player::Team::Blue:
            protocolRacer.teamColor = protocol::TeamColor::Blue;
            break;
          default:
            protocolRacer.teamColor = protocol::TeamColor::None;
            break;
        }

        protocolRacer.avatar = protocol::Avatar{};

        protocol::BuildProtocolCharacter(
          protocolRacer.avatar->character, character);

        // Build the character equipment.
        protocol::BuildProtocolItems(
          protocolRacer.avatar->equipment,
          *_serverInstance.GetDataDirector().GetItemCache().Get(
            character.characterEquipment()));

        // Build the mount equipment.
        protocol::BuildProtocolItems(
          protocolRacer.avatar->equipment,
          *_serverInstance.GetDataDirector().GetItemCache().Get(
            character.expiredEquipment()));

        const auto mountRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(
          character.mountUid());
        mountRecord->Immutable(
          [&protocolRacer](const data::Horse& mount)
          {
            protocol::BuildProtocolHorse(protocolRacer.avatar->mount, mount);
          });

        if (character.guildUid() != data::InvalidUid)
        {
          GetServerInstance().GetDataDirector().GetGuild(character.guildUid()).Immutable(
            [&protocolRacer, characterUid = character.uid()](const data::Guild& guild)
            {
              protocol::BuildProtocolGuild(protocolRacer.guild, guild);

              if (guild.owner() == characterUid)
              {
                protocolRacer.guild.guildRole = protocol::GuildRole::Owner;
              }
              else if (std::ranges::contains(guild.officers(), characterUid))
              {
                protocolRacer.guild.guildRole = protocol::GuildRole::Officer;
              }
              else
              {
                protocolRacer.guild.guildRole = protocol::GuildRole::Member;
              }
            });
        }

        if (character.petUid() != data::InvalidUid)
        {
          const auto& petRecord = GetServerInstance().GetDataDirector().GetPet(character.petUid());
          if (petRecord.IsAvailable())
          {
            petRecord.Immutable(
              [&protocolRacer](const data::Pet& pet)
              {
                protocol::BuildProtocolPet(protocolRacer.pet, pet);
              });
          }
          else
          {
            spdlog::warn("Character {} tried to load pet {} but it is not available.",
              character.uid(),
              character.petUid());
          }
        }
      });

    if (characterUid == clientContext.characterUid)
    {
      joiningRacer = protocolRacer;
    }
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  protocol::AcCmdCREnterRoomNotify notify{
    .racer = joiningRacer,
    .averageTimeRecord = clientContext.characterUid};

  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]()
      {
        return notify;
      });
  }

  {
    std::scoped_lock lock(raceInstance.clientsMutex);
    raceInstance.clients.insert(clientId);
  }
}

void RaceDirector::HandleChangeRoomOptions(
  ClientId clientId,
  const protocol::AcCmdCRChangeRoomOptions& command)
{
  // todo: validate command fields
  const auto& clientContext = GetClientContext(clientId);

  const std::bitset<6> options(
    static_cast<uint16_t>(command.optionsBitfield));

  if (options.test(0))
  {
    _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
      [this, roomUid = clientContext.roomUid, &command](const data::Character& character)
      {
        const auto userName = _serverInstance.GetLobbyDirector().GetUserByCharacterUid(
          character.uid()).userName;
        spdlog::info("Room {}'s name changed by '{}' to '{}'", roomUid, userName, command.name);
      });
  }

  // Change the room options.
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&options, &command](Room& room)
    {
      auto& roomDetails = room.GetRoomDetails();

      if (options.test(0))
      {
        roomDetails.name = command.name;
      }
      if (options.test(1))
      {
        roomDetails.maxPlayerCount = command.playerCount;
      }
      if (options.test(2))
      {
        roomDetails.password = command.password;
      }
      if (options.test(3))
      {
        switch (command.gameMode)
        {
          case protocol::GameMode::Speed:
            roomDetails.gameMode = Room::GameMode::Speed;
            break;
          case protocol::GameMode::Magic:
            roomDetails.gameMode = Room::GameMode::Magic;
            break;
          case protocol::GameMode::Tutorial:
            roomDetails.gameMode = Room::GameMode::Tutorial;
            break;
          default:
            spdlog::error("Unknown game mode '{}'", static_cast<uint32_t>(command.gameMode));
        }
      }
      if (options.test(4))
      {
        roomDetails.courseId = command.mapBlockId;
      }
      if (options.test(5))
      {
        roomDetails.npcDifficulty = command.npcDifficulty;
      }
    });

  protocol::AcCmdCRChangeRoomOptionsNotify notify{
    .optionsBitfield = command.optionsBitfield,
    .name = command.name,
    .playerCount = command.playerCount,
    .password = command.password,
    .gameMode = command.gameMode,
    .mapBlockId = command.mapBlockId,
    .npcDifficulty = command.npcDifficulty};

  BroadcastChangeRoomOptions(clientContext.roomUid, notify);
}

void RaceDirector::HandleChangeTeam(
  ClientId clientId,
  const protocol::AcCmdCRChangeTeam& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // todo: team balancing
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&command](Room& room)
    {
      auto& player = room.GetPlayer(command.characterOid);
      switch (command.teamColor)
      {
        case protocol::TeamColor::Red:
          player.SetTeam(Room::Player::Team::Red);
          break;
        case protocol::TeamColor::Blue:
          player.SetTeam(Room::Player::Team::Blue);
          break;
        default: {}
      }
    });

  const auto& raceInstance = _raceInstances[clientContext.roomUid];

  const protocol::AcCmdCRChangeTeamOK response{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  const protocol::AcCmdCRChangeTeamNotify notify{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};

  // Notify all other clients in the room
  for (const auto& roomClientId : raceInstance.clients)
  {
    // Prevent broadcast to self.
    if (clientId == roomClientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RaceDirector::HandleLeaveRoom(ClientId clientId)
{
  protocol::AcCmdCRLeaveRoomOK response{};

  auto& clientContext = GetClientContext(clientId);
  if (clientContext.roomUid == 0)
    return;

  auto& raceInstance = _raceInstances[clientContext.roomUid];

  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [roomUid = clientContext.roomUid](
      const data::Character& character)
    {
      spdlog::info("Character '{}' has left the room {}", character.name(), roomUid);
    });

  if (raceInstance.tracker.IsRacer(clientContext.characterUid))
  {
    auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);
    racer.state = tracker::RaceTracker::Racer::State::Disconnected;

    // Notify all the other racers that the client has disconnected
    protocol::AcCmdUserRaceDeleteNotify deleteNotify{
      .racerOid = racer.oid};
    for (const auto& raceClientId : raceInstance.clients)
    {
      // Prevent self broadcast
      if (raceClientId == clientId)
        continue;

      _commandServer.QueueCommand<decltype(deleteNotify)>(
        raceClientId,
        [deleteNotify]()
        {
          return deleteNotify;
        });
    }
  }

  {
    std::scoped_lock lock(raceInstance.clientsMutex);
    raceInstance.clients.erase(clientId);
  }

  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [characterUid = clientContext.characterUid](Room& room)
    {
      room.RemovePlayer(characterUid);
    });

  // Check if the leaving player was the leader
  const bool wasMaster = raceInstance.masterUid == clientContext.characterUid;

  {
    // Notify other clients in the room about the character leaving.
    protocol::AcCmdCRLeaveRoomNotify notify{
      .characterId = clientContext.characterUid,
      .unk0 = 1};

    for (const ClientId& raceClientId : raceInstance.clients)
    {
      if (raceClientId == clientId)
        continue;

      _commandServer.QueueCommand<decltype(notify)>(
        raceClientId,
        [notify]()
        {
          return notify;
        });
    }
  }

  if (wasMaster)
  {
    // Try to find the next master.
    auto nextMasterUid{data::InvalidUid};

    // todo: improve this
    // If the room is waiting, pick from room users.
    if (raceInstance.stage == RaceInstance::Stage::Waiting)
    {
      _serverInstance.GetRoomSystem().GetRoom(
        clientContext.roomUid,
        [&nextMasterUid](Room& room)
        {
          for (const auto characterUid : room.GetPlayers() | std::views::keys)
          {
            // todo: assign mastership to the best player
            nextMasterUid = characterUid;
            break;
          }
        });
    }
    else
    {
      for (const auto& characterUid : raceInstance.tracker.GetRacers() | std::views::keys)
      {
        nextMasterUid = characterUid;
        break;
      }
    }

    if (nextMasterUid != data::InvalidUid)
    {
      raceInstance.masterUid = nextMasterUid;

      spdlog::info("Player {} became the master of room {} after the previous master left",
        raceInstance.masterUid,
        clientContext.roomUid);

      // Notify other clients in the room about the new master.
      protocol::AcCmdCRChangeMasterNotify notify{
        .masterUid = raceInstance.masterUid};

      for (const ClientId& raceClientId : raceInstance.clients)
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

  if (raceInstance.clients.empty())
  {
    _serverInstance.GetRoomSystem().DeleteRoom(
      clientContext.roomUid);
    _raceInstances.erase(clientContext.roomUid);
  }

  clientContext.roomUid = data::InvalidUid;

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleReadyRace(
  ClientId clientId,
  const protocol::AcCmdCRReadyRace&)
{
  const auto& clientContext = GetClientContext(clientId);

  bool isPlayerReady = false;
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&isPlayerReady, characterUid = clientContext.characterUid](Room& room)
    {
      isPlayerReady = room.GetPlayer(characterUid).ToggleReady();
    });

  protocol::AcCmdCRReadyRaceNotify response{
    .characterUid = clientContext.characterUid,
    .isReady = isPlayerReady};

  const auto& raceInstance = _raceInstances[clientContext.roomUid];
  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(response)>(
      raceClientId,
      [response]()
      {
        return response;
      });
  }
}

void RaceDirector::PrepareItemSpawners(data::Uid roomUid)
{
  auto& raceInstance = _raceInstances[roomUid];

  try {
    const auto& gameModeInfo = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
      static_cast<uint32_t>(raceInstance.raceGameMode));
    const auto& mapBlockInfo = GetServerInstance().GetCourseRegistry().GetMapBlockInfo(
      raceInstance.raceMapBlockId);

    // Get the map position offset
    const auto& offset = mapBlockInfo.offset;

    // Spawn items based on map positions and game mode allowed deck IDs
    for (const uint32_t usedDeckItemId : gameModeInfo.usedDeckItemIds)
    {
      for (const auto& mapDeckItemInstance : mapBlockInfo.deckItems)
      {
        if (mapDeckItemInstance.deckId != usedDeckItemId)
          continue;

        auto& item = raceInstance.tracker.AddItem();
        item.deckId = mapDeckItemInstance.deckId;
        item.position[0] = mapDeckItemInstance.position[0] + offset[0];
        item.position[1] = mapDeckItemInstance.position[1] + offset[1];
        item.position[2] = mapDeckItemInstance.position[2] + offset[2];
      }
    }

  }
  catch (const std::exception& e) {
    spdlog::warn("Failed to prepare item spawners for room {}: {}", roomUid, e.what());
  }
}

void RaceDirector::HandleStartRace(
  const ClientId clientId,
  [[maybe_unused]] const protocol::AcCmdCRStartRace& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  if (clientContext.characterUid != raceInstance.masterUid)
    throw std::runtime_error("Client tried to start the race even though they're not the master");

  // Check if all race requirements are met to start the race

  bool allPlayersReady = true;
  bool isTeamMode = false;
  bool areTeamsBalanced = false;

  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&allPlayersReady, &isTeamMode, &areTeamsBalanced, master = raceInstance.masterUid](Room& room)
    {
      isTeamMode = room.GetRoomDetails().teamMode == Room::TeamMode::Team;

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

        // todo: observer
        const bool isRoomMaster = characterUid == master;

        // Only count the ready state of player which are not masters.
        if (!isRoomMaster && !player.IsReady())
        {
          allPlayersReady = false;
          break;
        }
      }

      areTeamsBalanced = redTeamCount == blueTeamCount;
  });

  if (not allPlayersReady)
  {
    SendStartRaceCancel(clientId, protocol::AcCmdCRStartRaceCancel::Reason::NotReady);
    return;
  }

  if (isTeamMode && not areTeamsBalanced)
  {
    SendStartRaceCancel(clientId, protocol::AcCmdCRStartRaceCancel::Reason::NotTeamBalance);
    return;
  }

  const auto roomUid = clientContext.roomUid;
  uint16_t roomSelectedCourses;
  uint8_t roomGameMode;

  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [&roomSelectedCourses, &roomGameMode, &raceInstance](Room& room)
    {
      auto& details = room.GetRoomDetails();

      raceInstance.raceGameMode = static_cast<protocol::GameMode>(details.gameMode);
      raceInstance.raceTeamMode = static_cast<protocol::TeamMode>(details.teamMode);
      raceInstance.raceMissionId = details.missionId;

      roomGameMode = static_cast<uint8_t>(details.gameMode);
      roomSelectedCourses = details.courseId;
    });

  constexpr uint32_t AllMapsCourseId = 10000;
  constexpr uint32_t NewMapsCourseId = 10001;
  constexpr uint32_t HotMapsCourseId = 10002;

  if (roomSelectedCourses == AllMapsCourseId
    || roomSelectedCourses == NewMapsCourseId
    || roomSelectedCourses == HotMapsCourseId)
  {
    const auto& gameMode = _serverInstance.GetCourseRegistry().GetCourseGameModeInfo(
      roomGameMode);
    if (not gameMode.mapPool.empty())
    {
      uint32_t masterLevel{};
      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        raceInstance.masterUid);
      characterRecord.Immutable(
        [&masterLevel](const data::Character& character)
        {
          masterLevel = character.level();
        });

      // Filter out the maps that are above the master's level.
      std::vector<uint16_t> filteredMaps;
      std::copy_if(
        gameMode.mapPool.cbegin(),
        gameMode.mapPool.cend(),
        std::back_inserter(filteredMaps),
        [this, masterLevel](uint32_t mapBlockId)
        {
          try
          {
            const auto& mapBlockInfo = _serverInstance.GetCourseRegistry().GetMapBlockInfo(
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
      static std::random_device rd;
      std::uniform_int_distribution distribution(0, static_cast<int>(filteredMaps.size() - 1));
      raceInstance.raceMapBlockId = filteredMaps[distribution(rd)];
    }
    else
    {
      raceInstance.raceMapBlockId = 1;
    }
  }
  else
  {
    raceInstance.raceMapBlockId = roomSelectedCourses;
  }

  const protocol::AcCmdRCRoomCountdown roomCountdown{
    .countdown = 3000,
    .mapBlockId = raceInstance.raceMapBlockId};

  // Broadcast room countdown.
  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(roomCountdown)>(
      raceClientId,
      [roomCountdown]()
      {
        return roomCountdown;
      });
  }

  // Clear the tracker before the race.
  raceInstance.tracker.Clear();

  // Add the items.
  PrepareItemSpawners(roomUid);

  // Add the racers.
  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [&raceInstance](Room& room)
    {
      // todo: observers
      for (const auto& [characterUid, roomPlayer] : room.GetPlayers())
      {
        auto& racer = raceInstance.tracker.AddRacer(characterUid);
        racer.state = tracker::RaceTracker::Racer::State::Loading;
        switch (roomPlayer.GetTeam())
        {
          case Room::Player::Team::Solo:
            racer.team = tracker::RaceTracker::Racer::Team::Solo;
            break;
          case Room::Player::Team::Red:
            racer.team = tracker::RaceTracker::Racer::Team::Red;
            break;
          case Room::Player::Team::Blue:
            racer.team = tracker::RaceTracker::Racer::Team::Blue;
            break;
        }
      }
    });

  raceInstance.stage = RaceInstance::Stage::Loading;
  raceInstance.stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(30);

  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [](Room& room)
    {
      room.SetRoomPlaying(true);
    });

  // Queue race start after room countdown.
  _scheduler.Queue(
    [this, roomUid]()
    {
      // todo: this could fail
      auto& raceInstance = _raceInstances[roomUid];

      protocol::AcCmdCRStartRaceNotify notify{
        .raceGameMode = raceInstance.raceGameMode,
        .raceTeamMode = raceInstance.raceTeamMode,
        .raceMapBlockId = raceInstance.raceMapBlockId,
        .p2pRelayAddress = asio::ip::address_v4::loopback().to_uint(),
        .p2pRelayPort = static_cast<uint16_t>(10500),
        .raceMissionId = raceInstance.raceMissionId,};

      // Build the racers.
      for (const auto& [characterUid, racer] : raceInstance.tracker.GetRacers())
      {
        std::string characterName;
        GetServerInstance().GetDataDirector().GetCharacter(characterUid).Immutable(
          [&characterName](const data::Character& character)
          {
            characterName = character.name();
          });

        auto& protocolRacer = notify.racers.emplace_back(
          protocol::AcCmdCRStartRaceNotify::Player{
            .oid = racer.oid,
            .name = characterName,
            .p2dId = racer.oid,});

        switch (racer.team)
        {
          case tracker::RaceTracker::Racer::Team::Solo:
            protocolRacer.teamColor = protocol::TeamColor::None;
            break;
          case tracker::RaceTracker::Racer::Team::Red:
            protocolRacer.teamColor = protocol::TeamColor::Red;
            break;
          case tracker::RaceTracker::Racer::Team::Blue:
            protocolRacer.teamColor = protocol::TeamColor::Blue;
            break;
        }
      }

      const bool isEligibleForSkills = (notify.raceGameMode == protocol::GameMode::Speed
        || notify.raceGameMode == protocol::GameMode::Magic)
        && notify.raceTeamMode == protocol::TeamMode::FFA;

      std::scoped_lock lock(raceInstance.clientsMutex);
      // Send to all clients participating in the race.
      for (const ClientId& raceClientId : raceInstance.clients)
      {
        const auto& raceClientContext = GetClientContext(raceClientId);

        if (not raceInstance.tracker.IsRacer(raceClientContext.characterUid))
          continue;

        auto& racer = raceInstance.tracker.GetRacer(raceClientContext.characterUid);
        notify.hostOid = racer.oid;

        // Skills only apply for speed single or magic single
        if (isEligibleForSkills)
        {
          // Notify racer of confirmed selection of skills
          GetServerInstance().GetDataDirector().GetCharacter(raceClientContext.characterUid).Immutable(
            [&notify](const data::Character& character)
            {
              // Get skill set by gamemode
              const auto& skillSets =
                notify.raceGameMode == protocol::GameMode::Speed ? character.skills.speed() :
                notify.raceGameMode == protocol::GameMode::Magic ? character.skills.magic() :
                  throw std::runtime_error("Unknown game mode");

              // Get racer's active skill set ID and set it in notify
              notify.racerActiveSkillSet.setId = static_cast<uint8_t>(skillSets.activeSetId);

              const auto& skillSet =
                skillSets.activeSetId == 0 ? skillSets.set1 :
                skillSets.activeSetId == 1 ? skillSets.set2 :
                throw std::runtime_error("Invalid skill set ID");

              // Slot 1, slot 2, bonus (calculated after)
              notify.racerActiveSkillSet.skills[0] = skillSet.slot1;
              notify.racerActiveSkillSet.skills[1] = skillSet.slot2;
            });

          // Bonus skills are unique for each racer in the racer
          // TODO: put these in a skill registry table
          std::vector<uint32_t> speedOnlyBonusSkills = {59, 32, 31};
          std::vector<uint32_t> magicOnlyBonusSkills = {34, 35, 36, 57, 58};
          std::vector<uint32_t> bonusSkillIds = {43, 29, 30}; // Speed + magic

          // Append to list depending on gamemode
          if (notify.raceGameMode == protocol::GameMode::Speed)
          {
            bonusSkillIds.insert(
              bonusSkillIds.end(),
              speedOnlyBonusSkills.begin(),
              speedOnlyBonusSkills.end());
          }
          else if (notify.raceGameMode == protocol::GameMode::Magic)
          {
            bonusSkillIds.insert(
              bonusSkillIds.end(),
              magicOnlyBonusSkills.begin(),
              magicOnlyBonusSkills.end());
          }

          std::uniform_int_distribution<uint32_t> bonusSkillDist(
            0,
            static_cast<uint32_t>(bonusSkillIds.size()) - 1);

          const auto bonusSkillIdx = bonusSkillDist(_randomDevice);
          notify.racerActiveSkillSet.skills[2] = bonusSkillIds[bonusSkillIdx];
        }

        _commandServer.QueueCommand<decltype(notify)>(
          raceClientId,
          [notify]()
          {
            return notify;
          });
      }
    },
    Scheduler::Clock::now() + std::chrono::milliseconds(roomCountdown.countdown));
}

void RaceDirector::SendStartRaceCancel(
  ClientId clientId,
  protocol::AcCmdCRStartRaceCancel::Reason reason)
{
  _commandServer.QueueCommand<protocol::AcCmdCRStartRaceCancel>(
    clientId,
    [reason]()
    {
      return protocol::AcCmdCRStartRaceCancel{
        .reason = reason};
    });
}

void RaceDirector::HandleRaceTimer(
  ClientId clientId,
  const protocol::AcCmdUserRaceTimer& command)
{
  protocol::AcCmdUserRaceTimerOK response{
    .clientRaceClock = command.clientClock,
    .serverRaceClock = TimePointToRaceTimePoint(
      std::chrono::steady_clock::now()),};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleLoadingComplete(
  ClientId clientId,
  const protocol::AcCmdCRLoadingComplete&)
{
  auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // Switch the racer to the racing state.
  racer.state = tracker::RaceTracker::Racer::State::Racing;

  // Notify all clients in the room that this player's loading is complete
  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<protocol::AcCmdCRLoadingCompleteNotify>(
      raceClientId,
      [oid = racer.oid]()
      {
        return protocol::AcCmdCRLoadingCompleteNotify{
          .oid = oid};
      });
  }
}

void RaceDirector::HandleUserRaceFinal(
  ClientId clientId,
  const protocol::AcCmdUserRaceFinal& command)
{
  bool isDnf = command.member3 > 0;
  std::chrono::hh_mm_ss raceTime{command.courseTime};
  spdlog::debug("[{}] AcCmdUserRaceFinal: {} {} {}",
    clientId,
    command.oid,
    isDnf ?
      "DNF" :
      std::format("{}:{}.{}",
        raceTime.minutes().count(),
        raceTime.seconds().count(),
        raceTime.subseconds().count()),
    command.member3);

  auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  // todo: sanity check for course time
  // todo: address npc racers and update their states
  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  racer.state = tracker::RaceTracker::Racer::State::Finishing;
  racer.courseTime = isDnf ? -1 :static_cast<int32_t>(command.courseTime.count());

  protocol::AcCmdUserRaceFinalNotify notify{
    .oid = racer.oid,
    .courseTime = command.member3 < 0 ? 
      command.courseTime :
      std::chrono::milliseconds{-1}};

  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RaceDirector::HandleRaceResult(
  ClientId clientId,
  [[maybe_unused]] const protocol::AcCmdCRRaceResult& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // todo:
  //  - award carrots and experience,
  //  - record replays,
  //  - mount emblem unlocked
  //  - implement mount fatigue
  protocol::AcCmdCRRaceResultOK response{};

  characterRecord.Immutable(
    [this, &response](const data::Character& character)
    {
      response.currentCarrots = character.carrots();

      GetServerInstance().GetDataDirector().GetHorse(character.mountUid()).Immutable(
        [&response](const data::Horse& horse)
        {
          response.horseFatigue = static_cast<uint16_t>(
            horse.fatigue());
        });
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleP2PRaceResult(
  ClientId clientId,
  const protocol::AcCmdCRP2PResult&)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  protocol::AcCmdGameRaceP2PResult result{};
  for (const auto & [uid, racer] : raceInstance.tracker.GetRacers())
  {
    auto& protocolRacer = result.member1.emplace_back();
    protocolRacer.oid = racer.oid;
  }

  _commandServer.QueueCommand<decltype(result)>(clientId, [result](){return result;});
}

void RaceDirector::HandleP2PUserRaceResult(
  ClientId,
  const protocol::AcCmdUserRaceP2PResult&)
{
}

void RaceDirector::HandleAwardStart(
  ClientId clientId,
  const protocol::AcCmdCRAwardStart& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  protocol::AcCmdRCAwardNotify notify{
    .member1 = command.member1};

  // Send to clients not participating in races.
  for (const auto raceClientId : raceInstance.clients)
  {
    const auto& raceClientContext = _clients[raceClientId];

    // Whether the client is a participating racer that did not disconnect.
    bool isParticipatingRacer = false;
    if (raceInstance.tracker.IsRacer(raceClientContext.characterUid))
    {
      auto& racer = raceInstance.tracker.GetRacer(
        raceClientContext.characterUid);
      // todo: handle player reconnect instead of ignoring them here
      isParticipatingRacer = racer.state != tracker::RaceTracker::Racer::State::Disconnected;
    }

    if (isParticipatingRacer)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RaceDirector::HandleAwardEnd(
  ClientId,
  const protocol::AcCmdCRAwardEnd&)
{
  // todo: this always crashes everyone

  // const auto& clientContext = GetClientContext(clientId);
  // auto& raceInstance = _raceInstances[clientContext.roomUid];
  //
  // protocol::AcCmdCRAwardEndNotify notify{};
  //
  // // Send to clients not participating in races.
  // for (const auto raceClientId : raceInstance.clients)
  // {
  //   const auto& roomClientContext = _clients[raceClientId];
  //
  //   // Whether the client is a participating racer that did not disconnect.
  //   bool isParticipatingRacer = false;
  //   if (raceInstance.tracker.IsRacer(roomClientContext.characterUid))
  //   {
  //     auto& racer = raceInstance.tracker.GetRacer(
  //       roomClientContext.characterUid);
  //     isParticipatingRacer = racer.state != tracker::RaceTracker::Racer::State::Disconnected;
  //   }
  //
  //   if (isParticipatingRacer)
  //     continue;
  //
  //   _commandServer.QueueCommand<decltype(notify)>(
  //     raceClientId,
  //     [notify]()
  //     {
  //       return notify;
  //     });
  // }
}

void RaceDirector::HandleStarPointGet(
  ClientId clientId,
  const protocol::AcCmdCRStarPointGet& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(raceInstance.raceGameMode));

  uint32_t gainedStarPoints = command.gainedStarPoints;
  if (racer.gaugeBuff) {
    // TODO: Something sensible, idk what the bonus does
    gainedStarPoints *= 2;
  }

  racer.starPointValue = std::min(
    racer.starPointValue + gainedStarPoints,
    gameModeTemplate.starPointsMax);

  // Star point get (boost get) is only called in speed, should never give magic item
  protocol::AcCmdCRStarPointGetOK response{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleRequestSpur(
  ClientId clientId,
  const protocol::AcCmdCRRequestSpur& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(raceInstance.raceGameMode));

  if (racer.starPointValue < gameModeTemplate.spurConsumeStarPoints)
    throw std::runtime_error("Client is dead ass cheating (or is really desynced)");

  racer.starPointValue -= gameModeTemplate.spurConsumeStarPoints;

  protocol::AcCmdCRRequestSpurOK response{
    .characterOid = command.characterOid,
    .activeBoosters = command.activeBoosters,
    .startPointValue = racer.starPointValue,
    .comboBreak = command.comboBreak};

  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  _commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]()
    {
      return starPointResponse;
    });
}

void RaceDirector::HandleHurdleClearResult(
  ClientId clientId,
  const protocol::AcCmdCRHurdleClearResult& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  protocol::AcCmdCRHurdleClearResultOK response{
    .characterOid = command.characterOid,
    .hurdleClearType = command.hurdleClearType,
    .jumpCombo = 0,
    .unk3 = 0
  };

  // Give magic item is calculated later
  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
      static_cast<uint8_t>(raceInstance.raceGameMode));

  switch (command.hurdleClearType)
  {
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect:
    {
      // Perfect jump over the hurdle.
      racer.jumpComboValue = std::min(
        static_cast<uint32_t>(99),
        racer.jumpComboValue + 1);

      if (raceInstance.raceGameMode == protocol::GameMode::Speed)
      {
        // Only send jump combo if it is a speed race
        response.jumpCombo = racer.jumpComboValue;
      }

      // Calculate max applicable combo
      const auto& applicableComboCount = std::min(
        gameModeTemplate.perfectJumpMaxBonusCombo,
        racer.jumpComboValue);
      // Calculate max combo count * perfect jump boost unit points
      const auto& gainedStarPointsFromCombo = applicableComboCount * gameModeTemplate.perfectJumpUnitStarPoints;
      // Add boost points to character boost tracker
      racer.starPointValue = std::min(
        racer.starPointValue + gameModeTemplate.perfectJumpStarPoints + gainedStarPointsFromCombo,
        gameModeTemplate.starPointsMax);

      // Update boost gauge
      starPointResponse.starPointValue = racer.starPointValue;
      break;
    }
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Good:
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::DoubleJumpOrGlide:
    {
      // Not a perfect jump over the hurdle, reset the jump combo.
      racer.jumpComboValue = 0;
      response.jumpCombo = racer.jumpComboValue;

      uint32_t gainedStarPoints = gameModeTemplate.goodJumpStarPoints;
      if (racer.gaugeBuff) {
        // TODO: Something sensible, idk what the bonus does
        gainedStarPoints *= 2;
      }

      // Increment boost gauge by a good jump
      racer.starPointValue = std::min(
        racer.starPointValue + gainedStarPoints,
        gameModeTemplate.starPointsMax);

      // Update boost gauge
      starPointResponse.starPointValue = racer.starPointValue;
      break;
    }
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Collision:
    {
      // A collision with hurdle, reset the jump combo.
      racer.jumpComboValue = 0;
      response.jumpCombo = racer.jumpComboValue;
      break;
    }
    default:
    {
      spdlog::warn("Unhandled hurdle clear type {}",
        static_cast<uint8_t>(command.hurdleClearType));
      return;
    }
  }

  // Needs to be assigned after hurdle clear result calculations
  // Triggers magic item request when set to true (if gamemode is magic and magic gauge is max)
  // TODO: is there only perfect clears in magic race?
  starPointResponse.giveMagicItem =
    raceInstance.raceGameMode == protocol::GameMode::Magic &&
    racer.starPointValue >= gameModeTemplate.starPointsMax &&
    command.hurdleClearType == protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect;

  // Update the star point value if the jump was not a collision.
  if (command.hurdleClearType != protocol::AcCmdCRHurdleClearResult::HurdleClearType::Collision)
  {
    _commandServer.QueueCommand<decltype(starPointResponse)>(
      clientId,
      [clientId, starPointResponse]()
      {
        return starPointResponse;
      });
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [clientId, response]()
    {
      return response;
    });
}

void RaceDirector::HandleStartingRate(
  ClientId clientId,
  const protocol::AcCmdCRStartingRate& command)
{
  // TODO: check for sensible values
  if (command.unk1 < 1 && command.boostGained < 1)
  {
    // Velocity and boost gained is not valid
    // TODO: throw?
    return;
  }

  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(raceInstance.raceGameMode));

  // TODO: validate boost gained against a table and determine good/perfect start
  racer.starPointValue = std::min(
    racer.starPointValue + command.boostGained,
    gameModeTemplate.starPointsMax);

  // Only send this on good/perfect starts
  protocol::AcCmdCRStarPointGetOK response{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false // TODO: this would never give a magic item on race start, right?
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [clientId, response]()
    {
      return response;
    });
}

void RaceDirector::HandleRaceUserPos(
  ClientId clientId,
  const protocol::AcCmdUserRaceUpdatePos& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.oid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(raceInstance.raceGameMode));

  for (const auto& [itemOid, item] : raceInstance.tracker.GetItems())
  {
    const bool canItemRespawn = std::chrono::steady_clock::now() >= item.respawnTimePoint;
    if (not canItemRespawn)
      continue;

    // The distance between the player and the item.
    const auto distanceBetweenPlayerAndItem = std::sqrt(
      std::pow(command.member2[0] - item.position[0], 2) +
      std::pow(command.member2[1] - item.position[1], 2) +
      std::pow(command.member2[2] - item.position[2], 2));

    // A distance of the player from the item before it can be spawned.
    constexpr double ItemSpawnDistanceThreshold = 90.0;

    const bool isItemInPlayerProximity = distanceBetweenPlayerAndItem < ItemSpawnDistanceThreshold;
    const bool isItemAlreadyTracked = racer.trackedItems.contains(itemOid);

    if (isItemAlreadyTracked)
    {
      // If the item is not in the player's proximity anymore
      // then remove it from the tracked items.
      if (not isItemInPlayerProximity)
        racer.trackedItems.erase(itemOid);

      continue;
    }

    // If the item is not in player's proximity do not spawn it.
    if (not isItemInPlayerProximity)
      continue;

    protocol::AcCmdGameRaceItemSpawn spawn{
      .itemId = item.oid,
      .itemType = item.deckId,
      .position = item.position,
      .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
      .removeDelay = -1};

    racer.trackedItems.insert(item.oid);

    _commandServer.QueueCommand<decltype(spawn)>(clientId, [spawn]()
    {
      return spawn;
    });
  }

  // Only regenerate magic during active race (after countdown finishes)
  // Check if game mode is magic, race is active, countdown finished, and not holding an item
  const bool raceActuallyStarted = std::chrono::steady_clock::now() >= raceInstance.raceStartTimePoint;

  if (raceInstance.raceGameMode == protocol::GameMode::Magic
    && racer.state == tracker::RaceTracker::Racer::State::Racing
    && raceActuallyStarted
    && not racer.magicItem.has_value())
  {
    if (racer.starPointValue < gameModeTemplate.starPointsMax)
    {
      // TODO: add these to configuration somewhere
      // Eyeballed these values from watching videos
      constexpr uint32_t NoItemHeldBoostAmount = 2000;
      // TODO: does holding an item and with certain equipment give you magic? At a reduced rate?
      constexpr uint32_t ItemHeldWithEquipmentBoostAmount = 1000;
      uint32_t gainedStarPoints;
      if (racer.magicItem.has_value()) {
        gainedStarPoints = ItemHeldWithEquipmentBoostAmount;
      } else {
        gainedStarPoints = NoItemHeldBoostAmount;
      }
      if (racer.gaugeBuff) {
        // TODO: Something sensible, idk what the bonus does
        gainedStarPoints *= 2;
      }
      racer.starPointValue = std::min(gameModeTemplate.starPointsMax, racer.starPointValue + gainedStarPoints);
    }

    // Conditional already checks if there is no magic item and gamemode is magic,
    // only check if racer has max magic gauge to give magic item
    protocol::AcCmdCRStarPointGetOK starPointResponse{
      .characterOid = command.oid,
      .starPointValue = racer.starPointValue,
      .giveMagicItem = racer.starPointValue >= gameModeTemplate.starPointsMax
    };

    _commandServer.QueueCommand<decltype(starPointResponse)>(
      clientId,
      [starPointResponse]
      {
        return starPointResponse;
      });
  }

  for (const auto& raceClientId : raceInstance.clients)
  {
    // Prevent broadcast to self.
    if (clientId == raceClientId)
      continue;
  }
}

void RaceDirector::HandleChat(ClientId clientId, const protocol::AcCmdCRChat& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Perform moderation before proceeding with chat processing
  const auto verdict = _serverInstance.GetChatSystem().ProcessChatMessage(
    clientContext.characterUid, command.message);

  if (verdict.isMuted)
  {
    // Invoking character is muted. Notify the invoker of their infraction
    spdlog::warn("Character '{}' tried to chat in race chat but has an active mute infraction.",
      clientContext.characterUid);
    protocol::AcCmdCRChatNotify notify{
      .message = verdict.message,
      .isSystem = true};
    _commandServer.QueueCommand<decltype(notify)>(clientId, [notify](){ return notify; });
    return;
  }

  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  std::string characterName;
  characterRecord.Immutable([&characterName](const data::Character& character)
  {
    characterName = character.name();
  });

  const auto userName = _serverInstance.GetLobbyDirector().GetUserByCharacterUid(
    clientContext.characterUid).userName;

  spdlog::info("[Room {}] {} ({}): {}",
    clientContext.roomUid,
    userName,
    characterName,
    command.message);

  std::vector<protocol::AcCmdCRChatNotify> response;
  const bool isCommand = verdict.commandVerdict.has_value();

  if (isCommand)
  {
    for (const auto& line : verdict.commandVerdict->result)
    {
      response.emplace_back(protocol::AcCmdCRChatNotify{
        .message = line,
        .author = "",
        .isSystem = true});
    }
  }
  else
  {
    response.emplace_back(protocol::AcCmdCRChatNotify{
      .message = verdict.message,
      .author = characterName,
      .isSystem = false,});
  }

  if (isCommand)
  {
    for (const auto& notify : response)
    {
      _commandServer.QueueCommand<protocol::AcCmdCRChatNotify>(
        clientId,
        [notify]{ return notify; });
    }
  }
  else
  {
    const auto& raceInstance = _raceInstances[clientContext.roomUid];

    for (const auto& notify : response)
    {
      for (const ClientId raceClientId : raceInstance.clients)
      {
        _commandServer.QueueCommand<protocol::AcCmdCRChatNotify>(
          raceClientId,
          [notify]{ return notify; });
      }
    }
  }
}

void RaceDirector::HandleRelayCommand(
  ClientId clientId,
  const protocol::AcCmdCRRelayCommand& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Create relay notify message
  protocol::AcCmdCRRelayCommandNotify notify{
    .member1 = command.member1,
    .member2 = command.member2};

  // Get the room instance for this client
  const auto& raceInstance = _raceInstances[clientContext.roomUid];

  // Relay the command to all other clients in the room
  for (const ClientId raceClientId : raceInstance.clients)
  {
    if (raceClientId != clientId) // Don't send back to sender
    {
      _commandServer.QueueCommand<decltype(notify)>(
        raceClientId,
        [notify]{return notify;});
    }
  }
}

void RaceDirector::HandleRelay(
  ClientId clientId,
  const protocol::AcCmdCRRelay& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Create relay notify message
  protocol::AcCmdCRRelayNotify notify{
    .oid = command.oid,
    .member2 = command.member2,
    .member3 = command.member3,
    .data = std::move(command.data),};

  // Get the room instance for this client
  const auto& raceInstance = _raceInstances[clientContext.roomUid];

  // Relay the command to all other clients in the room
  for (const ClientId raceClientId : raceInstance.clients)
  {
    if (raceClientId != clientId) // Don't send back to sender
    {
      _commandServer.QueueCommand<decltype(notify)>(
        raceClientId,
        [notify]{return notify;});
    }
  }
}

void RaceDirector::HandleUserRaceActivateInteractiveEvent
(
  ClientId clientId,
  const protocol::AcCmdUserRaceActivateInteractiveEvent& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  // Get the sender's OID from the room tracker
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  protocol::AcCmdUserRaceActivateInteractiveEvent notify{
    .member1 = command.member1,
    .characterOid = racer.oid, // sender oid
    .member3 = command.member3
  };

  // Broadcast to all clients in the room
  for (const ClientId raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]{return notify;});
  }
}

void RaceDirector::HandleUserRaceActivateEvent
(
  ClientId clientId,
  const protocol::AcCmdUserRaceActivateEvent& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  // Get the sender's OID from the room tracker
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  protocol::AcCmdUserRaceActivateEventNotify notify{
    .eventId = command.eventId,
    .characterOid = racer.oid, // sender oid
  };

  // Broadcast to all clients in the room
  for (const ClientId raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]{return notify;});
  }
}

void RaceDirector::HandleUserRaceDeactivateEvent
(
  ClientId clientId,
  const protocol::AcCmdUserRaceDeactivateEvent& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  // Get the sender's OID from the room tracker
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  protocol::AcCmdUserRaceDeactivateEventNotify notify{
    .eventId = command.eventId,
    .characterOid = racer.oid, // sender oid
  };

  // Broadcast to all clients in the room
  for (const ClientId raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]{return notify;});
  }
}

void RaceDirector::HandleRequestMagicItem(
  ClientId clientId,
  const protocol::AcCmdCRRequestMagicItem& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this on NPC races
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Client tried to perform action on behalf of different racer");
    return;
  }

  // Check if racer is already holding a magic item
  if (racer.magicItem.has_value())
  {
    // todo: this seems to happen a lot, figure it out
    return;
  }

  // TODO: reset magic gauge to 0?
  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue = 0,
    .giveMagicItem = false
  };

  _commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]
    {
      return starPointResponse;
    });

  protocol::AcCmdCRRequestMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = racer.magicItem.emplace(RandomMagicItem(racer)),
    .member3 = 0
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });

  protocol::AcCmdCRRequestMagicItemNotify notify{
    .magicItemId = response.magicItemId,
    .characterOid = response.characterOid
  };

  for (const auto& raceClientId : raceInstance.clients)
  {
    // Prevent broadcast to self
    if (raceClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]
      {
        return notify;
      });
  }
}

void RaceDirector::HandleUseMagicItem(
  ClientId clientId,
  const protocol::AcCmdCRUseMagicItem& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Client tried to perform action on behalf of different racer");
    return;
  }

  protocol::AcCmdCRUseMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = command.magicItemId,
    .optional1 = command.optional1,
    .optional2 = command.optional2,
    .unk3 = command.characterOid,
    .unk4 = command.optional3.has_value() ? command.optional3.value().member1 : 0.0f};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });

  // Notify other players that this player used their magic item
  protocol::AcCmdCRUseMagicItemNotify usageNotify{
    .characterOid = command.characterOid,
    .magicItemId = command.magicItemId,
    .optional1 = command.optional1,
    .optional2 = command.optional2,
    .unk3 = command.characterOid,
    .unk4 = command.unk3 // idk man
  };

  // Send usage notification to other players
  for (const auto& raceClientId : raceInstance.clients)
  {
    if (raceClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(usageNotify)>(
      raceClientId,
      [usageNotify]() { return usageNotify; });
  }

  // Send effect for items that have instant effects
  switch (command.magicItemId)
  {
    // Shield
    // TODO: Maybe not change it if they already have a stronger shield?
    case 4:
      racer.shield = tracker::RaceTracker::Racer::Shield::Normal;
      this->ScheduleSkillEffect(raceInstance, command.characterOid, command.characterOid, 2, [&racer]()
      {
        racer.shield = tracker::RaceTracker::Racer::Shield::None;
      });
      break;
    case 5:
      racer.shield = tracker::RaceTracker::Racer::Shield::Critical;
      this->ScheduleSkillEffect(raceInstance, command.characterOid, command.characterOid, 3, [&racer]()
      {
        racer.shield = tracker::RaceTracker::Racer::Shield::None;
      });
      break;
    // Booster
    case 6:
    case 7:
      this->ScheduleSkillEffect(raceInstance, command.characterOid, command.characterOid, 5);
      break;
    // Phoenix
    case 8:
      racer.hotRodded = true;
      this->ScheduleSkillEffect(raceInstance, command.characterOid, command.characterOid, 6, [&racer]()
      {
        racer.hotRodded = false;
      });
      break;
    case 9:
      racer.hotRodded = true;
      this->ScheduleSkillEffect(raceInstance, command.characterOid, command.characterOid, 6, [&racer]()
      {
        racer.hotRodded = false;
      });
      break;
    // Shackles
    // TODO: Apply only to opponents ahead of the racer
    case 12:
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid != otherRacer.oid
        && (racer.team == tracker::RaceTracker::Racer::Team::Solo || racer.team != otherRacer.team))
        {
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 10);
        }
      }
    case 13:
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid != otherRacer.oid
        && (racer.team == tracker::RaceTracker::Racer::Team::Solo || racer.team != otherRacer.team))
        {
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 11);
        }
      }
      break;
    // BufPower
    case 20:
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          otherRacer.critChance = true;
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 18, [&otherRacer]()
          {
            otherRacer.critChance = false;
          });
        }
      }
      break;
    case 21:
      // TODO: Is the critical "crit chance" buff different from the normal gauge buff?
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          otherRacer.critChance = true;
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 19, [&otherRacer]()
          {
            otherRacer.critChance = false;
          });
        }
      }
      break;
    // BufGauge
    case 22:
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          otherRacer.gaugeBuff = true;
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 20, [&otherRacer]()
          {
            otherRacer.gaugeBuff = false;
          });
        }
      }
      break;
    case 23:
      // TODO: Is the crit gauge buff different from the normal gauge buff?
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          otherRacer.gaugeBuff = true;
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 21, [&otherRacer]()
          {
            otherRacer.gaugeBuff = false;
          });
        }
      }
      break;
    // BufSpeed
    case 24:
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 22);
        }
      }
      break;
    case 25:
      for (auto& otherRacer : raceInstance.tracker.GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, 23);
        }
      }
      break;
  }

  racer.magicItem.reset();
}

void RaceDirector::HandleUserRaceItemGet(
  ClientId clientId,
  const protocol::AcCmdUserRaceItemGet& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& item = raceInstance.tracker.GetItems().at(command.itemId);

  constexpr auto ItemRespawnDuration = std::chrono::milliseconds(500);
  item.respawnTimePoint = std::chrono::steady_clock::now() + ItemRespawnDuration;

  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  Room::GameMode gameMode;
  registry::Course::GameModeInfo gameModeInfo;
  _serverInstance.GetRoomSystem().GetRoom(clientContext.roomUid, [this, &gameMode, &gameModeInfo](const Room& room)
  {
    gameMode = room.GetRoomSnapshot().details.gameMode;
    gameModeInfo = this->GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(static_cast<uint8_t>(gameMode));
  });

  switch(gameMode)
  {
    // TODO: Deduplicate from StarPointGet
    case Room::GameMode::Speed:
      {
        switch (item.deckId)
        {
          case 101: // Gold horseshoe. Get star points until the next boost
            racer.starPointValue = std::min(((racer.starPointValue/40000)+1) * 40000, gameModeInfo.starPointsMax);
            break;
          case 102: // Silver horseshoe. Get 10k star points
            racer.starPointValue = std::min(racer.starPointValue+10000, gameModeInfo.starPointsMax);
            break;
          default:
            // TODO: Disconnect?
            spdlog::warn("Player {} picked up unknown item type {}",
              clientId, item.deckId);
            break;
        }

        // Only send this on good/perfect starts
        protocol::AcCmdCRStarPointGetOK starPointResponse{
          .characterOid = command.characterOid,
          .starPointValue = racer.starPointValue,
          .giveMagicItem = false
        };

        _commandServer.QueueCommand<decltype(starPointResponse)>(
          clientId,
          [clientId, starPointResponse]()
          {
            return starPointResponse;
          });
      }
      break;

    // TODO: Deduplicate from RequestMagicItem
    case Room::GameMode::Magic:
    {
        if (racer.magicItem.has_value())
        {
          return;
        }

        // TODO: Replace with the item's deckId?
        const uint32_t gainedMagicItem = RandomMagicItem(racer);
        protocol::AcCmdCRRequestMagicItemOK magicItemOk{
          .characterOid = command.characterOid,
          .magicItemId = racer.magicItem.emplace(gainedMagicItem),
          .member3 = 0
        };
        _commandServer.QueueCommand<decltype(magicItemOk)>(
          clientId,
          [clientId, magicItemOk]()
          {
            return magicItemOk;
          });
        protocol::AcCmdCRRequestMagicItemNotify notify{
          .magicItemId = racer.magicItem.emplace(gainedMagicItem),
          .characterOid = command.characterOid,
        };
        for (const ClientId& roomClientId : raceInstance.clients)
        {
          _commandServer.QueueCommand<decltype(notify)>(
            roomClientId,
            [notify]()
            {
              return notify;
            });
        }

        // TODO: reset magic gauge to 0?
      }
      break;
  }

  // Notify all clients in the room that this item has been picked up
  protocol::AcCmdGameRaceItemGet get{
    .characterOid = command.characterOid,
    .itemId = command.itemId,
    .itemType = item.deckId,
  };

  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(get)>(
      raceClientId,
      [get]()
      {
        return get;
      });
  }

  // Erase the item from item instances of each client.
  for (auto& raceRacer : raceInstance.tracker.GetRacers() | std::views::values)
  {
    raceRacer.trackedItems.erase(item.oid);
  }

  // Respawn the item after a delay
  _scheduler.Queue(
    [this, item, &raceInstance]()
    {
      protocol::AcCmdGameRaceItemSpawn spawn{
        .itemId = item.oid,
        .itemType = item.deckId,
        .position = item.position,
        .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
        .sizeLevel = false,
        .removeDelay = -1
      };

      std::scoped_lock lock(raceInstance.clientsMutex);
      for (const ClientId& roomClientId : raceInstance.clients)
      {
        _commandServer.QueueCommand<decltype(spawn)>(
          roomClientId,
          [spawn]()
          {
            return spawn;
          });
      }
    },
    //only for speed for now, change to itemDeck registry later for magic
    Scheduler::Clock::now() + std::chrono::milliseconds(500));
}

// Magic Targeting System Implementation for Bolt
void RaceDirector::HandleStartMagicTarget(
  ClientId clientId,
  const protocol::AcCmdCRStartMagicTarget& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleStartMagicTarget");
    return;
  }
}

void RaceDirector::HandleChangeMagicTarget(
  ClientId clientId,
  const protocol::AcCmdCRChangeMagicTarget& command)
{
  // TODO: Throttle how often the same dragon can change between the same two targets
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.oldTargetOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleChangeMagicTargetNotify");
    return;
  }

  // Send OK response
  protocol::AcCmdCRChangeMagicTargetOK response{
    .unk0 = command.unk0,
    .unk1 = command.unk1,
    .oldTargetOid = command.oldTargetOid,
    .newTargetOid = command.newTargetOid
  };
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]() { return response; });

  // Send targeting notification to the target
  protocol::AcCmdCRChangeMagicTargetNotify targetNotify{
    .unk0 = command.unk0,
    .unk1 = command.unk1,
    .oldTargetOid = command.oldTargetOid,
    .newTargetOid = command.newTargetOid
  };

  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(targetNotify)>(
      raceClientId,
      [targetNotify]() { return targetNotify; });
  }
}

void RaceDirector::HandleActivateSkillEffect(
  ClientId clientId,
  const protocol::AcCmdCRActivateSkillEffect& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  auto& targetRacer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  uint16_t effectiveEffectId = static_cast<uint16_t>(command.effectId);
  if (targetRacer.darkness && !SkillIsCritical(effectiveEffectId))
  {
    effectiveEffectId += 1; // Use the "critical" version of the skill effect
  }

  if ((!SkillIsCritical(effectiveEffectId) && targetRacer.shield == tracker::RaceTracker::Racer::Shield::Normal)
  || targetRacer.shield == tracker::RaceTracker::Racer::Shield::Critical
  || targetRacer.hotRodded)
  {
    // TODO: Send some kind of notification that the attack was blocked? That picture on the side (PIPEvent?)
    // Maybe i actually need to send the effect but with a different parameter to mean "blocked"
    // Else it doesnt show in the kill list in the corner
    return;
  }

  std::optional<std::function<void()>> afterEffectRemoved = std::nullopt;
  switch (effectiveEffectId)
  {
      // Darkness
      case 12:
      case 13:
        // TODO: Is crit darkness different from normal darkness?
        targetRacer.darkness = true;
        afterEffectRemoved = [&targetRacer]()
        {
          targetRacer.darkness = false;
        };
        break;
  }

  // TODO: Remove held item
  this->ScheduleSkillEffect(raceInstance, command.attackerOid, command.targetOid, effectiveEffectId, afterEffectRemoved);
}

void RaceDirector::HandleOpCmd(
  ClientId clientId,
  const protocol::AcCmdCROpCmd& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::vector<std::string> feedback;

  const auto result = GetServerInstance().GetChatSystem().ProcessChatMessage(
    clientContext.characterUid, "//" + command.command);

  if (not result.commandVerdict)
  {
    return;
  }

  for (const auto response : result.commandVerdict->result)
  {
    _commandServer.QueueCommand<protocol::RanchCommandOpCmdOK>(
      clientId,
      [response = std::move(response)]()
      {
        return protocol::RanchCommandOpCmdOK{
          .feedback = response};
      });
  }
}

void RaceDirector::HandleChangeSkillCardPresetId(
  ClientId clientId,
  const protocol::AcCmdCRChangeSkillCardPresetID& command)
{
  if (command.setId < 0 || command.setId > 2)
  {
    // TODO: throw? return?
    // Calling client requested to change skill preset to something out of range
    // 0 < setId < 3
    return;
  }

  if (command.gamemode != protocol::GameMode::Speed && command.gamemode != protocol::GameMode::Magic)
  {
    // TODO: throw? return?
    // Gamemode can either be speed (1) or magic (2)
    return;
  }

  const auto& clientContext = _clients[clientId];
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&racer, &command](data::Character& character)
    {
      // Get skill sets by gamemode
      auto& skillSets =
        command.gamemode == protocol::GameMode::Speed ? character.skills.speed() :
        command.gamemode == protocol::GameMode::Magic ? character.skills.magic() :
        throw std::runtime_error("Invalid gamemode");
      // Set character's active skill set in the record
      skillSets.activeSetId = command.setId;
    }
  );

  // No response command
}

void RaceDirector::ScheduleSkillEffect(
  RaceInstance& raceInstance,
  tracker::Oid attackerOid,
  tracker::Oid targetOid,
  uint16_t effectId,
  std::optional<std::function<void()>> afterEffectRemoved){
  // Broadcast skill effect activation to all clients in the room
  protocol::AcCmdRCAddSkillEffect addSkillEffect{
    .characterOid = targetOid,
    .effectId = effectId,
    .targetOid = targetOid,
    .attackerOid = attackerOid,
    .unk2 = 0,
    .unk3 = 0,
    .unk4 = 0,
    .defenseMagicEffect = protocol::AcCmdRCAddSkillEffect::DefenseMagicEffect{
      .unk0 = 0,
      .unk1 = 0,
    },
    .attackMagicEffect = 0
  };

  // Broadcast
  for (const ClientId& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(addSkillEffect)>(
      raceClientId,
      [addSkillEffect]() { return addSkillEffect; });
  }

  // Remove the effect after a delay
  // TODO: Handle overlapping effects of the same type
  _scheduler.Queue(
    [this, attackerOid, targetOid, effectId, &raceInstance, afterEffectRemoved]()
    {
      // Broadcast skill effect deactivation to all clients in the room
      protocol::AcCmdRCRemoveSkillEffect removeSkillEffect{
        .characterOid = targetOid,
        .effectId = effectId,
        .targetOid = targetOid,
        .unk1 = 0,
      };

      std::scoped_lock lock(raceInstance.clientsMutex);
      // Broadcast
      for (const ClientId& raceClientId : raceInstance.clients)
      {
        _commandServer.QueueCommand<decltype(removeSkillEffect)>(
          raceClientId,
          [removeSkillEffect]() { return removeSkillEffect; });
      }

      if (afterEffectRemoved.has_value())
      {
        afterEffectRemoved.value()();
      }
    },
    Scheduler::Clock::now() + std::chrono::seconds(3)); // TODO: Different time per effect
}

void RaceDirector::HandleInviteUser(
  ClientId clientId,
  const protocol::AcCmdCRInviteUser& command)
{
  const auto& clientContext = GetClientContext(clientId);

  protocol::AcCmdCRInviteUserCancel cancel{};
  cancel.recipientCharacterUid = command.recipientCharacterUid;
  cancel.recipientCharacterName = command.recipientCharacterName;

  // Check if character by that uid is online
  const auto clientOpt = GetServerInstance().GetMessengerDirector().GetClientByCharacterUid(
    command.recipientCharacterUid);
  if (not clientOpt.has_value())
  {
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Check if there's a name mismatch
  // TODO: this could benefit from caching the character name within the messenger client context
  bool isNameMatch{false};
  GetServerInstance().GetDataDirector().GetCharacter(command.recipientCharacterUid).Immutable(
    [&isNameMatch, recipientCharacterName = command.recipientCharacterName](const data::Character& character)
    {
      isNameMatch = character.name() == recipientCharacterName;
    });

  if (not isNameMatch)
  {
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Race director invites are generally more relaxed, you can invite characters that are in
  // either a ranch or race waiting room
  
  // Sanity check if character can be invited (is away, online or in waiting room)
  const auto& recipientStatus = clientOpt.value().clientContext.presence.status;
  bool canInvite = recipientStatus == protocol::Status::Away or
    recipientStatus == protocol::Status::Online or
    recipientStatus == protocol::Status::WaitingRoom;

  if (not canInvite)
  {
    // Cannot invite character
    spdlog::warn("Character '{}', which is in a race waiting room, tried to invite character '{}' who is not in an invitable state",
      clientContext.characterUid,
      command.recipientCharacterUid);
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::AcCmdCRInviteUserOK response{};
  response.recipientCharacterUid = command.recipientCharacterUid;
  response.recipientCharacterName = command.recipientCharacterName;

  _commandServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void RaceDirector::HandleKickUser(
  ClientId clientId,
  const protocol::AcCmdCRKick& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  // Only the room master may kick players.
  if (clientContext.characterUid != raceInstance.masterUid)
  {
    spdlog::warn(
      "Character '{}' tried to kick character '{}' but is not the room master.",
      clientContext.characterUid,
      command.characterUid);
    return;
  }

  // Prevent self-kick.
  if (command.characterUid == clientContext.characterUid)
  {
    spdlog::warn(
      "Character '{}' tried to kick themselves.",
      clientContext.characterUid);
    return;
  }

  // Verify the target character is actually in this room.
  const bool targetInRoom = std::ranges::any_of(
    raceInstance.clients,
    [this, &command](const ClientId& raceClientId)
    {
      return _clients[raceClientId].characterUid == command.characterUid;
    });

  if (!targetInRoom)
  {
    spdlog::warn(
      "Character '{}' tried to kick character '{}' who is not in the room.",
      clientContext.characterUid,
      command.characterUid);
    return;
  }

  // GameMasters (role 2) cannot be kicked.
  bool targetIsGameMaster = false;
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Immutable([&targetIsGameMaster](const data::Character& character)
    {
      targetIsGameMaster = character.role() == data::Character::Role::GameMaster;
    });

  const auto kickerUserName = _serverInstance.GetLobbyDirector().GetUserByCharacterUid(clientContext.characterUid).userName;
  const auto targetUserName = _serverInstance.GetLobbyDirector().GetUserByCharacterUid(command.characterUid).userName;

  if (targetIsGameMaster)
  {
    spdlog::info(
      "User '{}' tried to kick user '{}' who is a GameMaster.",
      kickerUserName,
      targetUserName);
    return;
  }

  spdlog::info(
    "User '{}' kicked user '{}' from room {}.",
    kickerUserName,
    targetUserName,
    clientContext.roomUid);

  // Broadcast the kick notification to all clients in the room.
  protocol::AcCmdCRKickNotify notify{};
  notify.characterUid = command.characterUid;

  for (const auto& raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RaceDirector::HandleStampReward(ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId);

  // TODO: extract this to somewhere suitable, perhaps a StampEventSystem?
  enum StampRewardType
  {
    None,
    Package,
    Item,
    Carrots
  } rewardType{StampRewardType::None};

  // TODO: track this with every character
  // Supposedly one stamp per every completed match (check for DNF, completed race etc)
  constexpr uint32_t CompletedStamps = 3; 

  protocol::AcCmdRCExchangeItem exchange{
    .packageId = 0,
    .carrotsRewarded = 0,
    .carrotBalance = 0,
    .completedStamps = CompletedStamps,
    .newItems = {}};

  // Get mutable character record
  GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [this, &rewardType, &exchange](data::Character& character)
    {
      // TODO: get completed stamps (after giving stamp) from character's stamp progression
      // In its current state, only stamps 3 and 5 unlock rewards
      bool stampHasReward = 
        exchange.completedStamps == 3 or
        exchange.completedStamps == 5;
      if (stampHasReward)
      {
        // TODO: get reward type from stamp event (StampEventSystem?)
        rewardType = StampRewardType::Package;
      }

      // Distribute reward based on collected stamp, if any at all
      switch (rewardType)
      {
        case StampRewardType::Package:
        {
          // TODO: giftboxes (package ID 101 -> 111) do not appear to be in the package config 
          // TODO: get package ID from stamp event (create new StampEventSystem?)
          // 잭 오 랜턴 미니 햇 7일 (잭 오 랜턴 미니 햇/Jack-o'-lantern mini hat)
          constexpr uint32_t SamplePackageId = 1033;
          exchange.packageId = SamplePackageId;

          // Get package entry from item registry
          const auto& packageEntry = GetServerInstance().GetItemRegistry().GetPackage(SamplePackageId);
          if (not packageEntry.has_value())
            // Package somehow does not exist in the server item registry config
            throw std::runtime_error(
              std::format(
                "HandleStampReward: Package '{}' not found",
                SamplePackageId));

          // Get package item reward metadata
          const registry::Package& package = packageEntry.value();
          const auto& registryItemEntry = GetServerInstance().GetItemRegistry().GetItem(package.tid);
          if (not registryItemEntry.has_value())
            // Package reward item somehow does not exist in the server item registry config
            throw std::runtime_error(
              std::format(
                "HandleStampReward: Package reward item '{}' not found",
                package.tid));

          // Check if package reward item is temporary or not (has item duration)
          const registry::Item& registryItem = registryItemEntry.value();
          std::vector<data::Uid> newPackageRewardItems{};
          if (registryItem.type == registry::Item::Type::Temporary)
          {
            // Item has duration
            const auto& duration = std::chrono::days(package.count); // TODO: is this always in days?
            newPackageRewardItems.emplace_back(
              GetServerInstance().GetItemSystem().AddItem(
                character,
                package.tid,
                duration)); 
          }
          else
          {
            // Item has count
            newPackageRewardItems.emplace_back(
              GetServerInstance().GetItemSystem().AddItem(
                character,
                package.tid,
                package.count));
          }

          // Build protocol items for response with package reward items
          const auto packageRewardItemRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(newPackageRewardItems);
          protocol::BuildProtocolItems(
            exchange.newItems,
            *packageRewardItemRecords);
          break;
        }
        case StampRewardType::Item:
        {
          // Create and add item(s) to character's inventory
          // Multiple items can be rewarded, only the last rewarded item will be shown in the popup

          // TODO: get item TID from stamp event (create new StampEventSystem?)
          // TODO: get item config (duration/count) from stamp event
          // Sample item rewards (horse armour)
          const std::unordered_map<data::Tid, std::chrono::days> itemRewardDictionary{
            {20126, std::chrono::days(90)},
            {20005, std::chrono::days(120)}
          };

          // Add items to character's inventory and build a list of reward item UIDs
          std::vector<data::Uid> rewardItemUids{};
          for (const auto& [itemTid, duration] : itemRewardDictionary)
          {
            const data::Uid newItemUid = GetServerInstance().GetItemSystem().AddItem(
              character,
              itemTid,
              duration);
            rewardItemUids.emplace_back(newItemUid);
          }

          // Build protocol items for response with reward items
          const auto rewardItemRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(rewardItemUids);
          protocol::BuildProtocolItems(
            exchange.newItems,
            *rewardItemRecords);
          break;
        }
        case StampRewardType::Carrots:
        {
          // TODO: get carrot reward from stamp event (StampEventSystem?)
          constexpr uint32_t CarrotReward = 10000;

          // Add carrots to character's balance and indicate carrot reward
          character.carrots() += CarrotReward;
          exchange.carrotsRewarded = CarrotReward;
          exchange.carrotBalance = character.carrots();
          break;
        }
        case StampRewardType::None:
        {
          // No reward to distribute, we are done here.
          break;
        }
        default:
        {
          throw std::runtime_error("Invalid StampRewardType value");
        }
      }
    });

  // Notify client of stamp progression (even if there is no reward)
  _commandServer.QueueCommand<decltype(exchange)>(clientId, [exchange]{ return exchange; });
}

} // namespace server
