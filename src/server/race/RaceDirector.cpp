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

// 2 - Bolt
// 4 - Shield
// 10 - Ice wall
const std::array<uint32_t, 3> magicItems = {2, 4, 10};

uint32_t RandomMagicItem()
{
  static std::random_device rd;
  std::uniform_int_distribution distribution(0, static_cast<int>(magicItems.size() - 1));
  return magicItems[distribution(rd)];
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
    [this](ClientId clientId, const auto& message)
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

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceActivateEvent>(
    [this](ClientId clientId, const auto& message)
     {
       HandleUserRaceActivateEvent(clientId, message);
     });

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

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeMagicTargetNotify>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeMagicTargetNotify(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeMagicTargetOK>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeMagicTargetOK(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeMagicTargetCancel>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeMagicTargetCancel(clientId, message);
    });
  
  // Note: AcCmdCRActivateSkillEffect handler commented out due to build issues
  // _commandServer.RegisterCommandHandler<protocol::AcCmdCRActivateSkillEffect>(
  //   [this](ClientId clientId, const auto& message)
  //   {
  //     HandleActivateSkillEffect(clientId, message);
  //   });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeSkillCardPresetID>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeSkillCardPresetId(clientId, message);
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

      } catch (const std::exception& x) {
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

void RaceDirector::Tick() {
  _scheduler.Tick();

  // Process rooms which are loading
  for (auto& [raceUid, raceInstance] : _raceInstances)
  {
    if (raceInstance.stage != RoomInstance::Stage::Loading)
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

    // If not all of the racer have loaded yet and the timeout has not been reached yet
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
    raceInstance.stage = RoomInstance::Stage::Racing;
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
    if (raceInstance.stage != RoomInstance::Stage::Racing)
      continue;

    // Determine whether all racers have finished.
    const bool allRacersFinished = std::ranges::all_of(
      std::views::values(raceInstance.tracker.GetRacers()),
      [](const tracker::RaceTracker::Racer& racer)
      {
        return racer.state == tracker::RaceTracker::Racer::State::Finishing
          || racer.state == tracker::RaceTracker::Racer::State::Disconnected;
      });

    const bool raceTimeoutReached = std::chrono::steady_clock::now() >= raceInstance.stageTimeoutTimePoint;

    // If not all of the racer have finished yet and the timeout has not been reached yet
    // do not finish the race.
    if (not allRacersFinished && not raceTimeoutReached)
      return;

    if (raceTimeoutReached)
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

    // Set the room state.
    raceInstance.stage = RoomInstance::Stage::Waiting;
    _serverInstance.GetRoomSystem().GetRoom(
      raceUid,
      [](Room& room)
      {
        room.SetRoomPlaying(false);
      });
  }
}

void RaceDirector::HandleClientConnected(ClientId clientId)
{
  _clients.try_emplace(clientId);

  spdlog::info("Client {} connected to the race", clientId);
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

  spdlog::info("Client {} disconnected from the race", clientId);
  _clients.erase(clientId);
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
    .isRoomWaiting = raceInstance.stage == RoomInstance::Stage::Waiting,
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
          .unk6 = roomDetails.member11,
          .skillBracket = roomDetails.skillBracket};
      });
  }
  catch (const std::exception& x)
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

    // Determine whether the player is ready.
    bool isPlayerReady = false;
    server::Room::Player::Team team;
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
            character.mountEquipment()));

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

  raceInstance.clients.insert(clientId);
}

void RaceDirector::HandleChangeRoomOptions(
  ClientId clientId,
  const protocol::AcCmdCRChangeRoomOptions& command)
{
  // todo: validate command fields
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  const std::bitset<6> options(
    static_cast<uint16_t>(command.optionsBitfield));

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
        roomDetails.member11 = command.npcRace;
      }
    });

  protocol::AcCmdCRChangeRoomOptionsNotify notify{
    .optionsBitfield = command.optionsBitfield,
    .name = command.name,
    .playerCount = command.playerCount,
    .password = command.password,
    .gameMode = command.gameMode,
    .mapBlockId = command.mapBlockId,
    .npcRace = command.npcRace};

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

void RaceDirector::HandleChangeTeam(
  ClientId clientId,
  const protocol::AcCmdCRChangeTeam& command)
{
  const auto& clientContext = GetClientContext(clientId);

  protocol::AcCmdCRChangeTeamOK response{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};

  protocol::AcCmdCRChangeTeamNotify notify{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};

  // todo: team balancing
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [this, &clientId, &command, &response, &notify](Room& room)
    {
      auto& player = room.GetPlayer(command.characterOid);
      switch (command.teamColor)
      {
        case protocol::TeamColor::Red:
          player.SetTeam(server::Room::Player::Team::Red);
          break;
        case protocol::TeamColor::Blue:
          player.SetTeam(server::Room::Player::Team::Blue);
          break;
        default: {}
      }

      this->_commandServer.QueueCommand<decltype(response)>(
        clientId,
        [response]()
        {
          return response;
        });

      // Notify all other clients in the room
      for (const auto& characterUid : room.GetPlayers() | std::views::keys)
      {
        if (characterUid == command.characterOid)
          continue;

        auto roomPlayerClientId = this->GetClientIdByCharacterUid(characterUid);

        this->_commandServer.QueueCommand<decltype(notify)>(
          roomPlayerClientId,
          [notify]()
          {
            return notify;
          });
      }
    });
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
  }

  raceInstance.clients.erase(clientId);

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
  const protocol::AcCmdCRReadyRace& command)
{
  auto& clientContext = GetClientContext(clientId);

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

        spdlog::debug("Prepared item spawner: deckId={}, position=[{}, {}, {}]",
          mapDeckItemInstance.deckId,
          item.position[0],
          item.position[1],
          item.position[2]);
      }
    }

    spdlog::info("Prepared {} item spawners for room {} (gameMode={}, mapBlock={})",
      raceInstance.tracker.GetItems().size(),
      roomUid,
      static_cast<uint32_t>(raceInstance.raceGameMode),
      raceInstance.raceMapBlockId);
  }
  catch (const std::exception& e) {
    spdlog::warn("Failed to prepare item spawners for room {}: {}", roomUid, e.what());
  }
}

void RaceDirector::HandleStartRace(
  ClientId clientId,
  const protocol::AcCmdCRStartRace& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  if (clientContext.characterUid != raceInstance.masterUid)
    throw std::runtime_error("Client tried to start the race even though they're not the master");

  uint32_t roomSelectedCourses;
  uint8_t roomGameMode;

  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&roomSelectedCourses, &roomGameMode, &raceInstance](Room& room)
    {
      auto& details = room.GetRoomDetails();

      raceInstance.raceGameMode = static_cast<protocol::GameMode>(details.gameMode);
      raceInstance.raceTeamMode = static_cast<protocol::TeamMode>(details.gameMode);
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
      std::vector<uint32_t> filteredMaps;
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
  PrepareItemSpawners(clientContext.roomUid);

  // Add the racers.
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
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

  raceInstance.stage = RoomInstance::Stage::Loading;
  raceInstance.stageTimeoutTimePoint = std::chrono::steady_clock::now() + std::chrono::seconds(30);

  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [](Room& room)
    {
      room.SetRoomPlaying(true);
    });

  // Queue race start after room countdown.
  _scheduler.Queue(
    [this, clientId]()
    {
      const auto& clientContext = GetClientContext(clientId);

      auto& raceInstance = _raceInstances[clientContext.roomUid];
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
          [&characterName](
            const data::Character& character)
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

      // Send to all clients participating in the race.
      for (const ClientId& raceClientId : raceInstance.clients)
      {
        const auto& raceClientContext = _clients[raceClientId];

        if (not raceInstance.tracker.IsRacer(raceClientContext.characterUid))
          continue;
        auto& racer = raceInstance.tracker.GetRacer(raceClientContext.characterUid);
        notify.hostOid = racer.oid;

        const bool isSpeedOrMagic =
          notify.raceGameMode == protocol::GameMode::Speed ||
          notify.raceGameMode == protocol::GameMode::Magic;
        // Skills only apply for speed single or magic single
        if (isSpeedOrMagic && notify.raceTeamMode == protocol::TeamMode::FFA)
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
              notify.racerActiveSkillSet.setId = skillSets.activeSetId;

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

          std::uniform_int_distribution<uint32_t> bonusSkillDist(0, bonusSkillIds.size() - 1);
          auto bonusSkillIdx = bonusSkillDist(_randomDevice);
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
  const protocol::AcCmdCRLoadingComplete& command)
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
  auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  // todo: sanity check for course time
  // todo: address npc racers and update their states
  auto& racer = raceInstance.tracker.GetRacer(
    clientContext.characterUid);

  racer.state = tracker::RaceTracker::Racer::State::Finishing;
  racer.courseTime = command.courseTime;

  protocol::AcCmdUserRaceFinalNotify notify{
    .oid = racer.oid,
    .courseTime = command.courseTime};

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
  const protocol::AcCmdCRRaceResult& command)
{
  // todo: only requested by the room master

  auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];

  // TODO: veryfy the character ?
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCRRaceResultOK response{
    .member1 = 1,
    .member2 = 1,
    .member3 = 1,
    .member4 = 1,
    .member5 = 1};

  characterRecord.Immutable(
    [&raceInstance, &response](const data::Character& character)
    {
      response.currentCarrots = character.carrots();
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
  const protocol::AcCmdCRP2PResult& command)
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
  ClientId clientId,
  const protocol::AcCmdUserRaceP2PResult& command)
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
  ClientId clientId,
  const protocol::AcCmdCRAwardEnd& command)
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
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(raceInstance.raceGameMode));

  racer.starPointValue = std::min(
    racer.starPointValue + command.gainedStarPoints,
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

      // Increment boost gauge by a good jump
      racer.starPointValue = std::min(
        racer.starPointValue + gameModeTemplate.goodJumpStarPoints,
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
      racer.starPointValue = std::min(gameModeTemplate.starPointsMax, racer.starPointValue + NoItemHeldBoostAmount);
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

  const auto messageVerdict = _serverInstance.GetChatSystem().ProcessChatMessage(
    clientContext.characterUid, command.message);

  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCRChatNotify notify{
    .message = messageVerdict.message,
    .isSystem = false};

  characterRecord.Immutable([&notify](const data::Character& character)
  {
    notify.author = character.name();
  });

  spdlog::info("[Room {}] {}: {}", clientContext.roomUid, notify.author, notify.message);

  const auto& raceInstance = _raceInstances[clientContext.roomUid];
  for (const ClientId raceClientId : raceInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      raceClientId,
      [notify]{return notify;});
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

  spdlog::info("HandleUserRaceActivateEvent: clientId={}, eventId={}, characterOid={}", 
    clientId, command.eventId, racer.oid);

  protocol::AcCmdUserRaceActivateEvent notify{
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
  spdlog::info("Player {} requested magic item (OID: {}, type: {})", 
    clientId, command.member1, command.member2);

  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: command.member1 is character oid?
  if (command.member1 != racer.oid)
  {
    // TODO: throw? return?
    return;
  }

  // Check if racer is already holding a magic item
  if (racer.magicItem.has_value())
  {
    // Has no corresponding cancel, log and return
    spdlog::warn("Character {} tried to request a magic item in race {} but they already have one, skipping...",
      clientContext.characterUid,
      clientContext.roomUid);
    return;
  }

  // TODO: reset magic gauge to 0?
  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.member1,
    .starPointValue = racer.starPointValue = 0,
    .giveMagicItem = false
  };

  _commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]
    {
      return starPointResponse;
    });

  uint32_t gainedMagicItem = RandomMagicItem();

  protocol::AcCmdCRRequestMagicItemOK response{
    .member1 = command.member1,
    .member2 = racer.magicItem.emplace(gainedMagicItem),
    .member3 = 0
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });

  protocol::AcCmdCRRequestMagicItemNotify notify{
    .member1 = response.member2,
    .member2 = response.member1
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
  spdlog::info("Player {} used magic item {} (OID: {})", clientId, command.magicItemId, command.characterOid);
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);

  if (command.characterOid != racer.oid)
  {
    // TODO: throw? return?
    return;
  }

  protocol::AcCmdCRUseMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = command.magicItemId,
    .unk3 = command.characterOid,
    .unk4 = command.optional3.has_value() ? command.optional3.value() : 0};

  if (command.optional1.has_value())
    response.optional1 = command.optional1.value();
  if (command.optional2.has_value())
    response.optional2 = command.optional2.value();

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
    .unk3 = command.characterOid
  };

  // Copy optional fields from the original command
  if (command.optional1.has_value())
    usageNotify.optional1 = command.optional1.value();
  if (command.optional2.has_value())
    usageNotify.optional2 = command.optional2.value();
  if (command.optional3.has_value())
    usageNotify.optional3 = command.optional3.value();
  if (command.optional4.has_value())
    usageNotify.optional4 = command.optional4.value();

  // Special handling for magic items that require optional fields
  if (command.magicItemId == 2)  // Bolt only (ice wall handled separately)
  {
    if (!usageNotify.optional2.has_value()) {
      auto& opt2 = usageNotify.optional2.emplace();
      opt2.size = 0;
      opt2.list.clear();
    }
    
    // These items require optional3 and optional4
    if (!usageNotify.optional3.has_value())
      usageNotify.optional3 = 0.0f;
    if (!usageNotify.optional4.has_value())
      usageNotify.optional4 = 0.0f;
  }

  // Send general usage notification to other players (except for ice wall which has its own notification)
  if (command.magicItemId != 10) 
  {
    for (const auto& raceClientId : raceInstance.clients)
    {
      if (raceClientId == clientId)
        continue;
      
      _commandServer.QueueCommand<decltype(usageNotify)>(
        raceClientId,
        [usageNotify]() { return usageNotify; });
    }
  }

  // Special handling for bolt (magic item ID 2) - Auto-targeting system
  if (command.magicItemId == 2)
  {
    spdlog::info("Bolt used! Implementing auto-targeting system for player {}", clientId);
    
    // Find a target automatically (first other player in the room)
    tracker::Oid targetOid = tracker::InvalidEntityOid;
    for (const auto& [targetUid, targetRacer] : raceInstance.tracker.GetRacers())
    {
      // Skip the attacker, find first valid target
      if (targetRacer.oid != command.characterOid && 
          targetRacer.state == tracker::RaceTracker::Racer::State::Racing)
      {
        targetOid = targetRacer.oid;
        spdlog::info("Auto-selected target: OID {}", targetOid);
        break;
      }
    }
    
    if (targetOid != tracker::InvalidEntityOid)
    {
      // Apply bolt hit effects to the target
      for (auto& [targetUid, targetRacer] : raceInstance.tracker.GetRacers())
      {
        if (targetRacer.oid == targetOid)
        {
          spdlog::info("Applying bolt effects to target racer {} (OID: {})", targetUid, targetRacer.oid);
          
          // Send magic item notify for bolt hit effects (safe approach)
          protocol::AcCmdCRUseMagicItemNotify boltHitNotify{
            .characterOid = targetRacer.oid,  // Target gets hit
            .magicItemId = 2,  // Bolt magic item ID  
            .unk3 = targetRacer.oid
          };
          
          // Populate required optional fields for bolt
          if (!boltHitNotify.optional2.has_value()) {
            auto& opt2 = boltHitNotify.optional2.emplace();
            opt2.size = 0;
            opt2.list.clear();
          }
          
          // Set timing values for bolt animation
          boltHitNotify.optional3 = 1.0f;  // Cast time: 1 second for bolt to hit
          boltHitNotify.optional4 = 3.0f;  // Effect duration: 3 seconds target stays down
          
          spdlog::info("Sending bolt hit notification: characterOid={}, magicItemId={}, timing: {}s/{}s", 
            boltHitNotify.characterOid, boltHitNotify.magicItemId, 
            boltHitNotify.optional3.value(), boltHitNotify.optional4.value());
          
          for (const ClientId& raceClientId : raceInstance.clients)
          {
            spdlog::info("Sending bolt hit notification to client {}", raceClientId);
            _commandServer.QueueCommand<decltype(boltHitNotify)>(
              raceClientId,
              [boltHitNotify]() { return boltHitNotify; });
          }
          
          // Effect 1: Target loses their current magic item
          if (targetRacer.magicItem.has_value())
          {
            uint32_t lostItemId = targetRacer.magicItem.value();
            spdlog::info("Target racer {} lost magic item {}", targetRacer.oid, lostItemId);
            targetRacer.magicItem.reset();
            
            // TODO: Add proper magic expire notification once we confirm bolt hit works
            spdlog::info("Target lost magic item {} (server-side only for now)", lostItemId);
            
            // TODO: Add client notifications once bolt hit animation is working
          }
          else
          {
            spdlog::info("Target racer {} has no magic item to lose", targetRacer.oid);
          }
          
          break;
        }
      }
    }
    else
    {
      spdlog::info("No valid target found for bolt");
    }
  }
  
  // Special handling for ice wall
  else if (command.magicItemId == 10)
  {
    spdlog::info("Ice wall used! Spawning ice wall at player {} location", clientId);

    protocol::AcCmdCRUseMagicItemNotify notify{
      .characterOid = command.characterOid,
      .magicItemId = command.magicItemId};
    // Spawn ice wall at a reasonable position (near start line like other items)
    auto& iceWall = raceInstance.tracker.AddItem();
    iceWall.deckId = 102;  // Use same type as working items (temporarily)
    iceWall.position = {25.0f, -25.0f, -8010.0f};  // Near other track items

    spdlog::info("Spawned ice wall with ID {} at position ({}, {}, {})",
      iceWall.oid, iceWall.position[0], iceWall.position[1], iceWall.position[2]);

    // Notify all clients about the ice wall spawn using proper race item spawn command
    protocol::AcCmdGameRaceItemSpawn iceWallSpawn{
      .itemId = iceWall.oid,
      .itemType = iceWall.deckId,
      .position = iceWall.position,
      .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
      .sizeLevel = false,
      .removeDelay = -1  // Use same as working items (no removal)
    };

    spdlog::info("Sending ice wall spawn using AcCmdGameRaceItemSpawn: itemId={}, position=({}, {}, {})",
      iceWallSpawn.itemId, iceWallSpawn.position[0], iceWallSpawn.position[1], iceWallSpawn.position[2]);

    spdlog::info("Broadcasting to {} clients in room", raceInstance.clients.size());
    for (const ClientId& raceClientId : raceInstance.clients)
    {
      spdlog::info("Sending ice wall spawn to client {}", raceClientId);
      _commandServer.QueueCommand<decltype(iceWallSpawn)>(
        raceClientId,
        [iceWallSpawn]() { return iceWallSpawn; });
    }
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

  server::Room::GameMode gameMode;
  server::registry::Course::GameModeInfo gameModeInfo;
  _serverInstance.GetRoomSystem().GetRoom(clientContext.roomUid, [this, &gameMode, &gameModeInfo](const server::Room& room)
  {
    gameMode = room.GetRoomSnapshot().details.gameMode;
    gameModeInfo = this->GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(static_cast<uint8_t>(gameMode));
  });

  switch(gameMode)
  {
    // TODO: Deduplicate from StarPointGet
    case server::Room::GameMode::Speed:
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
    case server::Room::GameMode::Magic:
      {
        if (racer.magicItem.has_value())
        {
          spdlog::warn("Character {} tried to request a magic item in race {} but they already have one, skipping...",
            clientContext.characterUid,
            clientContext.roomUid);
          return;
        }

        const uint32_t gainedMagicItem = RandomMagicItem();
        protocol::AcCmdCRRequestMagicItemOK magicItemOk{
          .member1 = command.characterOid,
          .member2 = racer.magicItem.emplace(gainedMagicItem),
          .member3 = 0
        };
        _commandServer.QueueCommand<decltype(magicItemOk)>(
          clientId,
          [clientId, magicItemOk]()
          {
            return magicItemOk;
          });
        protocol::AcCmdCRRequestMagicItemNotify notify{
          .member1 = racer.magicItem.emplace(gainedMagicItem),
          .member2 = command.characterOid,
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
  for (auto& racer : raceInstance.tracker.GetRacers() | std::views::values)
  {
    racer.trackedItems.erase(item.oid);
  }

  // Respawn the item after a delay
  _scheduler.Queue(
    [this, clientId, item, &raceInstance]()
    {
      protocol::AcCmdGameRaceItemSpawn spawn{
        .itemId = item.oid,
        .itemType = item.deckId,
        .position = item.position,
        .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
        .sizeLevel = false,
        .removeDelay = -1
      };

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
  spdlog::info("Player {} started magic targeting with character OID {}", 
    clientId, command.characterOid);
  
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);
  
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleStartMagicTarget");
    return;
  }
  
  // Set targeting state
  racer.isTargeting = true;
  racer.currentTarget = tracker::InvalidEntityOid;
  
  spdlog::info("Character {} entered targeting mode", command.characterOid);
}

void RaceDirector::HandleChangeMagicTargetNotify(
  ClientId clientId,
  const protocol::AcCmdCRChangeMagicTargetNotify& command)
{
  spdlog::info("Player {} changed magic target: character OID {} -> target OID {}", 
    clientId, command.characterOid, command.targetOid);
  
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);
  
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleChangeMagicTargetNotify");
    return;
  }
  
  // Update current target
  racer.currentTarget = command.targetOid;
  
  // Send targeting notification to the target
  protocol::AcCmdCRChangeMagicTargetNotify targetNotify{
    .characterOid = command.characterOid,
    .targetOid = command.targetOid
  };
  
  // Find the client ID for this target and send notification
  for (const ClientId& raceClientId : raceInstance.clients)
  {
    const auto& targetClientContext = _clients[raceClientId];
    if (raceInstance.tracker.GetRacer(targetClientContext.characterUid).oid == command.targetOid)
    {
      _commandServer.QueueCommand<decltype(targetNotify)>(
        raceClientId,
        [targetNotify]() { return targetNotify; });
      break;
    }
  }
}

void RaceDirector::HandleChangeMagicTargetOK(
  ClientId clientId,
  const protocol::AcCmdCRChangeMagicTargetOK& command)
{
  spdlog::info("Player {} confirmed magic target: character OID {} -> target OID {}", 
    clientId, command.characterOid, command.targetOid);
  
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);
  
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleChangeMagicTargetOK");
    return;
  }
  
  // This is where the Bolt should be fired!
  spdlog::info("BOLT FIRED! {} -> {}", command.characterOid, command.targetOid);
  
  // Find the target racer and apply bolt effects
  for (auto& [targetUid, targetRacer] : raceInstance.tracker.GetRacers())
  {
    if (targetRacer.oid == command.targetOid)
    {
      spdlog::info("Bolt hit target {}! Applying effects...", command.targetOid);
      
      // Apply bolt effects: fall down, lose speed, lose item
      // Reset their magic item (they lose it when hit)
      targetRacer.magicItem.reset();
      
      // Send bolt hit notification to all clients so they can see the hit effects
      spdlog::info("Sending bolt hit notification to all clients for target {}", command.targetOid);
      
      // Send bolt hit as magic item usage notification
      protocol::AcCmdCRUseMagicItemNotify boltHitNotify{
        .characterOid = command.targetOid,  // The target who gets hit
        .magicItemId = 2,  // Bolt magic item ID  
        .unk3 = command.targetOid
      };
      
      // For bolt (ID 2), we might need to populate optional fields
      // Based on the Read method, bolt (case 0x2) expects optional2 and optional3/4
      if (!boltHitNotify.optional2.has_value()) {
        auto& opt2 = boltHitNotify.optional2.emplace();
        opt2.size = 0;  // Empty list for now
        opt2.list.clear();
      }
      
      for (const ClientId& raceClientId : raceInstance.clients)
      {
        spdlog::info("Sending bolt hit notification to client {}", raceClientId);
        _commandServer.QueueCommand<decltype(boltHitNotify)>(
          raceClientId,
          [boltHitNotify]() { return boltHitNotify; });
      }
      
      break;
    }
  }
  
  // Reset attacker's targeting state
  racer.isTargeting = false;
  racer.currentTarget = tracker::InvalidEntityOid;
  
  // Consume the Bolt magic item
  racer.magicItem.reset();
}

void RaceDirector::HandleChangeMagicTargetCancel(
  ClientId clientId,
  const protocol::AcCmdCRChangeMagicTargetCancel& command)
{
  spdlog::info("Player {} cancelled magic targeting: character OID {}", 
    clientId, command.characterOid);
  
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  auto& racer = raceInstance.tracker.GetRacer(clientContext.characterUid);
  
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleChangeMagicTargetCancel");
    return;
  }
  
  // Send remove target notification to the current target (if any)
  if (racer.currentTarget != tracker::InvalidEntityOid)
  {
    protocol::AcCmdRCRemoveMagicTarget removeNotify{
      .characterOid = command.characterOid
    };
    
    // Find the client ID for the current target
    for (const ClientId& raceClientId : raceInstance.clients)
    {
      const auto& targetClientContext = _clients[raceClientId];
      if (raceInstance.tracker.GetRacer(targetClientContext.characterUid).oid == racer.currentTarget)
      {
        _commandServer.QueueCommand<decltype(removeNotify)>(
          raceClientId,
          [removeNotify]() { return removeNotify; });
        break;
      }
    }
  }
  
  // Reset targeting state
  racer.isTargeting = false;
  racer.currentTarget = tracker::InvalidEntityOid;
  
  spdlog::info("Character {} exited targeting mode", command.characterOid);
}

/*
void RaceDirector::HandleActivateSkillEffect(
  ClientId clientId,
  const protocol::AcCmdCRActivateSkillEffect& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = _raceInstances[clientContext.roomUid];
  
  // Convert unk2 back to float (it's 1.0f = 1065353216 as uint32)
  float intensity = *reinterpret_cast<const float*>(&command.unk2);
  
  spdlog::info("HandleActivateSkillEffect: clientId={}, characterOid={}, skillId={}, unk1={}, intensity={}", 
    clientId, command.characterOid, command.skillId, command.unk1, intensity);
  
  // Process the skill effect activation - give target extra gauge (Attack Compensation skill)
  auto& targetRacer = raceInstance.tracker.GetRacer(clientContext.characterUid);
  
  // Apply "Attack Compensation" skill - target gets extra gauge when attacked
  targetRacer.starPointValue += 50;  // Give 50 star points for being attacked
  spdlog::info("Applied Attack Compensation skill: character {} gained 50 star points", command.characterOid);
  
  // Send skill effect response back to the requesting client
  protocol::AcCmdUserRaceActivateEvent activateResponse{
    .eventId = command.skillId,  // Echo back the skill ID
    .characterOid = command.characterOid
  };
  
  spdlog::info("Responding to skill activation for character {} with skillId {}", command.characterOid, command.skillId);
  
  // Send response back to the client who requested the skill activation
  _commandServer.QueueCommand<decltype(activateResponse)>(
    clientId,
    [activateResponse]() { return activateResponse; });
  
  // TODO: Implement knockdown animation once we figure out the correct structure
  // The AcCmdRCAddSkillEffect command causes disconnections
  spdlog::info("Knockdown effect disabled to prevent disconnections");
}
*/

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

} // namespace server
