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

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "server/ServerInstance.hpp"

#include "server/system/RoomSystem.hpp"

#include <spdlog/spdlog.h>
#include <bitset>
#include <limits>

namespace server
{

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
}

void RaceDirector::HandleClientConnected(ClientId clientId)
{
  _clients.try_emplace(clientId);

  spdlog::info("Client {} connected to the race", clientId);
}

void RaceDirector::HandleClientDisconnected(ClientId clientId)
{
  const auto& clientContext = _clients[clientId];

  const auto roomIter = _roomInstances.find(
    clientContext.roomUid);
  if (roomIter != _roomInstances.cend())
  {
    HandleLeaveRoom(clientId);
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

void RaceDirector::HandleEnterRoom(
  ClientId clientId,
  const protocol::AcCmdCREnterRoom& command)
{
  auto& clientContext = _clients[clientId];
  clientContext.characterUid = command.characterUid;
  clientContext.roomUid = command.roomUid;

  const auto& room = _serverInstance.GetRoomSystem().GetRoom(
    command.roomUid);

  // todo: verify otp

  const auto& [roomInstanceIter, inserted] = _roomInstances.try_emplace(
    command.roomUid);
  auto& roomInstance = roomInstanceIter->second;

  // If the room instance was just created, set it up.
  if (inserted)
  {
    roomInstance.masterUid = command.characterUid;
  }

  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [roomUid = clientContext.roomUid, inserted](
      const data::Character& character)
    {
      if (inserted)
        spdlog::info("Character '{}' has created the room {}", character.name(), roomUid);
      else
        spdlog::info("Character '{}' has joined the room {}", character.name(), roomUid);
    });

  auto& joinedRacer = roomInstance.tracker.AddRacer(
    command.characterUid);

  joinedRacer.state = tracker::RaceTracker::Racer::State::NotReady;

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});

  protocol::AcCmdCREnterRoomOK response{
    .nowPlaying = 1,
    .uid = room.uid,
    .roomDescription = {
      .name = room.name,
      .playerCount = room.playerCount,
      .password = room.password,
      .gameModeMaps = room.gameMode,
      .gameMode = room.gameMode,
      .mapBlockId = room.mapBlockId,
      .teamMode = room.teamMode,
      .missionId = room.missionId,
      .unk6 = room.unk3,
      .skillBracket = room.unk4}};

  protocol::Racer joiningRacer;

  for (const auto& [characterUid, racer] : roomInstance.tracker.GetRacers())
  {
    auto& protocolRacer = response.racers.emplace_back();

    protocolRacer.isReady = racer.state == tracker::RaceTracker::Racer::State::Ready;

    const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
      characterUid);
    characterRecord.Immutable(
      [this, racer, &protocolRacer, leaderUid = roomInstance.masterUid](
        const data::Character& character)
      {
        if (character.uid() == leaderUid)
          protocolRacer.isMaster = true;

        protocolRacer.level = character.level();
        protocolRacer.oid = racer.oid;
        protocolRacer.uid = character.uid();
        protocolRacer.name = character.name();
        protocolRacer.isHidden = false;
        protocolRacer.isNPC = false;

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

  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]()
      {
        return notify;
      });
  }

  roomInstance.clients.insert(clientId);
}

void RaceDirector::HandleChangeRoomOptions(
  ClientId clientId,
  const protocol::AcCmdCRChangeRoomOptions& command)
{
  // todo: validate command fields

  const auto& clientContext = _clients[clientId];
  auto& room = _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid);

  const std::bitset<6> options(
    static_cast<uint16_t>(command.optionsBitfield));

  if (options.test(0))
    room.name = command.name;
  if (options.test(1))
    room.playerCount = command.playerCount;
  if (options.test(2))
    room.password = command.password;
  if (options.test(3))
    room.gameMode = command.gameMode;
  if (options.test(4))
    room.mapBlockId = command.mapBlockId;
  if (options.test(5))
    room.unk3 = command.npcRace;
  
  protocol::AcCmdCRChangeRoomOptionsNotify notify{
    .optionsBitfield = command.optionsBitfield,
    .name = command.name,
    .playerCount = command.playerCount,
    .password = command.password,
    .gameMode = command.gameMode,
    .mapBlockId = command.mapBlockId,
    .npcRace = command.npcRace};

  const auto& roomInstance = _roomInstances[clientContext.roomUid];

  for (const auto roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
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
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);

  // todo: team balancing

  switch (command.teamColor)
  {
    case protocol::TeamColor::Red:
      racer.team = tracker::RaceTracker::Racer::Team::Red;
      break;
    case protocol::TeamColor::Blue:
      racer.team = tracker::RaceTracker::Racer::Team::Blue;
      break;
    default: {}
  }

  protocol::AcCmdCRChangeTeamOK response{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};

  protocol::AcCmdCRChangeTeamNotify notify{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Notify all other clients in the room
  for (const ClientId& roomClientId : roomInstance.clients)
  {
    if (roomClientId == clientId)
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

  auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [roomUid = clientContext.roomUid](
      const data::Character& character)
    {
      spdlog::info("Character '{}' has left the room {}", character.name(), roomUid);
    });

  roomInstance.tracker.RemoveRacer(
    clientContext.characterUid);
  roomInstance.clients.erase(clientId);

  // Check if the leaving player was the leader
  const bool wasLeader = roomInstance.masterUid == clientContext.characterUid;

  {
    // Notify other clients in the room about the character leaving.
    protocol::AcCmdCRLeaveRoomNotify notify{
      .characterId = clientContext.characterUid,
      .unk0 = 1};

    for (const ClientId& roomClientId : roomInstance.clients)
    {
      if (roomClientId == clientId)
        continue;

      _commandServer.QueueCommand<decltype(notify)>(
        roomClientId,
        [notify]()
        {
          return notify;
        });
    }
  }

  if (not roomInstance.tracker.GetRacers().empty())
  {
    if (wasLeader)
    {
      // Find the next leader.
      // todo: assign mastership to the best player

      roomInstance.masterUid = roomInstance.tracker.GetRacers().begin()->first;

      spdlog::info("Character {} became the master of room {} after the previous master left",
        roomInstance.masterUid,
        clientContext.roomUid);

      {
        // Notify other clients in the room about the new master.
        protocol::AcCmdCRChangeMasterNotify notify{
          .masterUid = roomInstance.masterUid};

        for (const ClientId& roomClientId : roomInstance.clients)
        {
          _commandServer.QueueCommand<decltype(notify)>(
            roomClientId,
            [notify]()
            {
              return notify;
            });
        }
      }
    }
  }
  else
  {
    _serverInstance.GetRoomSystem().DeleteRoom(
      clientContext.roomUid);
    _roomInstances.erase(clientContext.roomUid);
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
  auto& clientContext = _clients[clientId];

  auto& roomInstance = _roomInstances[clientContext.roomUid];

  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);

  // Toggle the ready state.
  if (racer.state == tracker::RaceTracker::Racer::State::NotReady)
    racer.state = tracker::RaceTracker::Racer::State::Ready;
  else if (racer.state == tracker::RaceTracker::Racer::State::Ready)
    racer.state = tracker::RaceTracker::Racer::State::NotReady;

  protocol::AcCmdCRReadyRaceNotify response{
    .characterUid = clientContext.characterUid,
    .isReady = racer.state == tracker::RaceTracker::Racer::State::Ready};

  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(response)>(
      roomClientId,
      [response]()
      {
        return response;
      });
  }
}

void RaceDirector::HandleStartRace(
  ClientId clientId,
  const protocol::AcCmdCRStartRace& command)
{
  _scheduler.Queue(
    [this, clientId]()
    {
      const auto& clientContext = _clients[clientId];

      const auto& room = _serverInstance.GetRoomSystem().GetRoom(
        clientContext.roomUid);
      auto& roomInstance = _roomInstances[clientContext.roomUid];

      // todo: verify master

      constexpr uint32_t AllMapsCourseId = 10000;
      constexpr uint32_t NewMapsCourseId = 10001;
      constexpr uint32_t HotMapsCourseId = 10002;

      protocol::AcCmdCRStartRaceNotify notify{
        .gameMode = room.gameMode,
        .teamMode = room.teamMode,
        .p2pRelayAddress = asio::ip::address_v4::loopback().to_uint(),
        .p2pRelayPort = static_cast<uint16_t>(10500)};

      if (room.mapBlockId == AllMapsCourseId || room.mapBlockId == NewMapsCourseId || room.mapBlockId == HotMapsCourseId)
      {
        // TODO: Select a random mapBlockId from a predefined list
        // For now its a map that at least loads in
        notify.mapBlockId = 1;
      }
      else
      {
        notify.mapBlockId = room.mapBlockId;
      }
      notify.missionId = room.missionId;

      for (const auto& [characterUid, racer] : roomInstance.tracker.GetRacers())
      {
        std::string characterName;
        GetServerInstance().GetDataDirector().GetCharacter(characterUid).Immutable([&characterName](const data::Character& character)
          {
            characterName = character.name();
          });

        auto& protocolRacer = notify.racers.emplace_back(protocol::AcCmdCRStartRaceNotify::Player{
          .oid = racer.oid,
          .name = characterName,
          .p2dId = racer.oid,
        });

        switch (racer.team)
        {
          case tracker::RaceTracker::Racer::Team::Solo:
            protocolRacer.teamColor = protocol::TeamColor::Solo;
            break;
          case tracker::RaceTracker::Racer::Team::Red:
            protocolRacer.teamColor = protocol::TeamColor::Red;
            break;
          case tracker::RaceTracker::Racer::Team::Blue:
            protocolRacer.teamColor = protocol::TeamColor::Blue;
            break;
        }
      }

      // Reset jump combo/star point (boost)
      for (auto& racer : roomInstance.tracker.GetRacers() | std::views::values)
      {
        racer.jumpComboValue = 0;
        racer.starPointValue = 0;
      }

      // todo: start loading timeout timer
      // Send to all clients in the room.
      for (const ClientId& roomClientId : roomInstance.clients)
      {
        const auto& roomClientContext = _clients[roomClientId];

        auto& racer = roomInstance.tracker.GetRacer(
          roomClientContext.characterUid);
        racer.state = tracker::RaceTracker::Racer::State::Loading;

        _serverInstance.GetDataDirector().GetCharacter(roomClientContext.characterUid).Immutable([](const data::Character& character)
          {
            spdlog::info("Race start sent to '{}'", character.name());
          });

        notify.hostOid = racer.oid;

        _commandServer.QueueCommand<decltype(notify)>(
          roomClientId,
          [notify]()
          {
            return notify;
          });
      }
    },
    Scheduler::Clock::now() + std::chrono::seconds(3));
}

void RaceDirector::HandleRaceTimer(
  ClientId clientId,
  const protocol::AcCmdUserRaceTimer& command)
{
  protocol::AcCmdUserRaceTimerOK response{
    .clientTimestamp = command.timestamp,
    .serverTimestamp = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
      ).count() / 100)
  };

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
  auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);

  racer.state = tracker::RaceTracker::Racer::State::Racing;

  // Notify all clients in the room that this player's loading is complete
  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<protocol::AcCmdCRLoadingCompleteNotify>(
      roomClientId,
      [oid = racer.oid]()
      {
        return protocol::AcCmdCRLoadingCompleteNotify{
          .oid = oid};
      });
  }

  const bool allRacersLoaded = std::ranges::all_of(
    std::views::values(roomInstance.tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Racing;
    });

  if (not allRacersLoaded)
  {
    return;
  }

  // TODO: better way of doing this? Reinstantiating the room?
  // Clear room items before populating
  roomInstance.tracker.GetItems().clear();

  // map id 1, right in front of start line [20.631426, -25.969913, -8004.5986]
  // 101 - Gold horseshoe
  // 102 - Silver horseshoe
  // 402 - magic horseshoe (tutorial?)
  for (uint32_t i = 0; i < 5; ++i)
  {
    auto& item = roomInstance.tracker.AddItem();
    item.itemType = 102;
    // FIXME: do not use hardcoded positions, store them in files instead
    item.position = {30.0f, -25.0f, -8012.0f + (i * 3)};

    protocol::AcCmdGameRaceItemSpawn spawn{
      .itemId = item.itemId,
      .itemType = item.itemType,
      .position = item.position,
      .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
      .member5 = false,
      .removeDelay = -1.0f};

    for (const ClientId& roomClientId : roomInstance.clients)
      _commandServer.QueueCommand<decltype(spawn)>(roomClientId, [spawn](){return spawn;});
  }

  spdlog::info(
    "All players in room {} have finished loading,"
    " starting the countdown.",
    clientContext.roomUid);

  // Record countdown start time
  roomInstance.countdownStartTime = std::chrono::steady_clock::now();

  // todo: start race timeout timer

  const auto countdownTimestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::steady_clock::now().time_since_epoch())
    .count() / 100 + 10 * 10'000'000;
    
  // Store when the race will actually start (countdown timestamp is when race begins)
  roomInstance.raceStartTimestamp = countdownTimestamp;

  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<protocol::AcCmdUserRaceCountdown>(
      roomClientId,
      [countdownTimestamp]()
      {
        return protocol::AcCmdUserRaceCountdown{
          .timestamp = countdownTimestamp};
      });
  }
}

void RaceDirector::HandleUserRaceFinal(
  ClientId clientId,
  const protocol::AcCmdUserRaceFinal& command)
{
  auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  // todo: sanity check for course time
  // todo: address npc racers and update their states
  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);
  racer.state = tracker::RaceTracker::Racer::State::Finished;
  racer.courseTime = command.courseTime;

  protocol::AcCmdUserRaceFinalNotify notify{
    .oid = racer.oid,
    .courseTime = command.courseTime};

  // todo: start finish timeout timer

  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
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

  auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  protocol::AcCmdCRRaceResultOK response{
    .member1 = 1,
    .member2 = 1,
    .member3 = 1,
    .member4 = 1,
    .member5 = 1,
    .member6 = 1};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  protocol::AcCmdRCRaceResultNotify notify{};

  const bool allRacersFinished = std::ranges::all_of(
    std::views::values(roomInstance.tracker.GetRacers()),
    [](const tracker::RaceTracker::Racer& racer)
    {
      return racer.state == tracker::RaceTracker::Racer::State::Finished;
    });

  if (not allRacersFinished)
    return;

  // Build the score board.
  for (const auto& [characterUid, racer] : roomInstance.tracker.GetRacers())
  {
    auto& score = notify.scores.emplace_back();

    // todo: figure out the other bit set values

    if (racer.state != tracker::RaceTracker::Racer::State::Disconnected)
      score.bitset = protocol::AcCmdRCRaceResultNotify::ScoreInfo::Connected;

    score.courseTime = racer.courseTime;

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

  for (const ClientId roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RaceDirector::HandleP2PRaceResult(
  ClientId clientId,
  const protocol::AcCmdCRP2PResult& command)
{
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
  const auto& clientContext = _clients[clientId];
  const auto& roomInstance = _roomInstances[clientContext.roomUid];

  protocol::AcCmdRCAwardNotify notify{
    .member1 = command.member1};

  for (const auto roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
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
  const auto& clientContext = _clients[clientId];
  const auto& roomInstance = _roomInstances[clientContext.roomUid];

  protocol::AcCmdCRAwardEndNotify notify{};

  for (const auto roomClientId : roomInstance.clients)
  {
    if (roomClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RaceDirector::HandleStarPointGet(
  ClientId clientId,
  const protocol::AcCmdCRStarPointGet& command)
{
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }
  
  // Get pointer and if inserted with characterOid as key
  const auto& room = _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid);
  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    room.gameMode);

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
    [clientId, response]()
    {
      return response;
    });
}

void RaceDirector::HandleRequestSpur(
  ClientId clientId,
  const protocol::AcCmdCRRequestSpur& command)
{
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& room = _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid);
  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    room.gameMode);

  if (racer.starPointValue < gameModeTemplate.spurConsumeStarPoints)
    throw std::runtime_error("Client is dead ass cheating (or is really desynced)");

  racer.starPointValue -= gameModeTemplate.spurConsumeStarPoints;

  protocol::AcCmdCRRequestSpurOK response{
    .characterOid = command.characterOid,
    .activeBoosters = command.activeBoosters,
    .startPointValue = racer.starPointValue,
    .comboBreak = command.comboBreak};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [clientId, response]()
    {
      return response;
    });
}

void RaceDirector::HandleHurdleClearResult(
  ClientId clientId,
  const protocol::AcCmdCRHurdleClearResult& command)
{
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  auto& racer = roomInstance.tracker.GetRacer(
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

  const auto& room = _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid);
  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    room.gameMode);

  switch (command.hurdleClearType)
  {
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect:
    {
      // Perfect jump over the hurdle.
      racer.jumpComboValue = std::min(
        static_cast<uint32_t>(99),
        racer.jumpComboValue + 1);

      if (room.gameMode == 1)
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
    room.gameMode == 2 &&
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

  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& room = _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid);
  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    room.gameMode);

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
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);
  if (command.oid != racer.oid)
  {
    // TODO: command character oid does not match calling character oid
    return;
  }
  
  // Note: Position tracking removed for Docker compatibility
  // racer.position = command.member2;  // Position (x, y, z)
  // racer.rotation = command.member3;  // Rotation (x, y, z)

  const auto& room = _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid);
  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    room.gameMode);

  // Only regenerate magic during active race (after countdown finishes)
  // Check if gamemode is magic, race is active, countdown finished, and not holding an item
  bool raceActuallyStarted = false;
  if (roomInstance.raceStartTimestamp.has_value())
  {
    // Get current timestamp in same format as countdown timestamp
    auto currentTimestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count() / 100;
      
    raceActuallyStarted = currentTimestamp >= roomInstance.raceStartTimestamp.value();
  }
  
  if (room.gameMode == 2 && racer.state == tracker::RaceTracker::Racer::State::Racing && raceActuallyStarted && not racer.magicItem.has_value())
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

  for (const auto& roomClientId : roomInstance.clients)
  {
    // Prevent broadcast to self.
    if (clientId == roomClientId)
      continue;
  }
}

void RaceDirector::HandleChat(ClientId clientId, const protocol::AcCmdCRChat& command)
{
  const auto& clientContext = _clients[clientId];

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

  const auto& roomInstance = _roomInstances[clientContext.roomUid];
  for (const ClientId roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]{return notify;});
  }
}

void RaceDirector::HandleRelayCommand(
  ClientId clientId,
  const protocol::AcCmdCRRelayCommand& command)
{
  const auto& clientContext = _clients[clientId];
  
  // Create relay notify message
  protocol::AcCmdCRRelayCommandNotify notify{
    .senderOid = command.senderOid,
    .relayData = command.relayData
  };

  // Get the room instance for this client
  const auto& roomInstance = _roomInstances[clientContext.roomUid];
  
  // Relay the command to all other clients in the room
  for (const ClientId roomClientId : roomInstance.clients)
  {
    if (roomClientId != clientId) // Don't send back to sender
    {
      _commandServer.QueueCommand<decltype(notify)>(
        roomClientId,
        [notify]{return notify;});
    }
  }
}

void RaceDirector::HandleRelay(
  ClientId clientId,
  const protocol::AcCmdCRRelay& command)
{
  const auto& clientContext = _clients[clientId];
  
  // Create relay notify message
  protocol::AcCmdCRRelayNotify notify{
    .senderOid = command.senderOid,
    .relayData = command.relayData
  };

  // Get the room instance for this client
  const auto& roomInstance = _roomInstances[clientContext.roomUid];
  
  // Relay the command to all other clients in the room
  for (const ClientId roomClientId : roomInstance.clients)
  {
    if (roomClientId != clientId) // Don't send back to sender
    {
      _commandServer.QueueCommand<decltype(notify)>(
        roomClientId,
        [notify]{return notify;});
    }
  }
}

void RaceDirector::HandleUserRaceActivateInteractiveEvent
(
  ClientId clientId,
  const protocol::AcCmdUserRaceActivateInteractiveEvent& command)
{
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  // Get the sender's OID from the room tracker
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);

  protocol::AcCmdUserRaceActivateInteractiveEvent notify{
    .member1 = command.member1,
    .characterOid = racer.oid, // sender oid
    .member3 = command.member3
  };

  // Broadcast to all clients in the room
  for (const ClientId roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]{return notify;});
  }
}

void RaceDirector::HandleUserRaceActivateEvent
(
  ClientId clientId,
  const protocol::AcCmdUserRaceActivateEvent& command)
{
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];

  // Get the sender's OID from the room tracker
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);

  spdlog::info("HandleUserRaceActivateEvent: clientId={}, eventId={}, characterOid={}", 
    clientId, command.eventId, racer.oid);

  protocol::AcCmdUserRaceActivateEvent notify{
    .eventId = command.eventId,
    .characterOid = racer.oid, // sender oid
  };

  // Broadcast to all clients in the room
  for (const ClientId roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]{return notify;});
  }
}

void RaceDirector::HandleRequestMagicItem(
  ClientId clientId,
  const protocol::AcCmdCRRequestMagicItem& command)
{
  spdlog::info("Player {} requested magic item (OID: {}, type: {})", 
    clientId, command.member1, command.member2);

  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);

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

  // 2 - Bolt
  // 4 - Shield
  // 10 - Ice wall
  protocol::AcCmdCRRequestMagicItemOK response{
    .member1 = command.member1,
    .member2 = racer.magicItem.emplace(10),
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

  for (const auto& roomClientId : roomInstance.clients)
  {
    // Prevent broadcast to self
    if (roomClientId == clientId)
      continue;
    
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
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
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);

  if (command.characterOid != racer.oid)
  {
    // TODO: throw? return?
    return;
  }

  protocol::AcCmdCRUseMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = command.magicItemId,
    .unk3 = command.characterOid,
    .unk4 = command.optional3.has_value() ? command.optional3.value() : 0
  };

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
    for (const auto& roomClientId : roomInstance.clients)
    {
      if (roomClientId == clientId)
        continue;
      
      _commandServer.QueueCommand<decltype(usageNotify)>(
        roomClientId,
        [usageNotify]() { return usageNotify; });
    }
  }

  // Special handling for bolt (magic item ID 2) - Auto-targeting system
  if (command.magicItemId == 2)
  {
    spdlog::info("Bolt used! Implementing auto-targeting system for player {}", clientId);
    
    // Find a target automatically (first other player in the room)
    tracker::Oid targetOid = tracker::InvalidEntityOid;
    for (const auto& [targetUid, targetRacer] : roomInstance.tracker.GetRacers())
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
      for (auto& [targetUid, targetRacer] : roomInstance.tracker.GetRacers())
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
          
          for (const ClientId& roomClientId : roomInstance.clients)
          {
            spdlog::info("Sending bolt hit notification to client {}", roomClientId);
            _commandServer.QueueCommand<decltype(boltHitNotify)>(
              roomClientId, 
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
    
    // Spawn ice wall at a reasonable position (near start line like other items)
    auto& iceWall = roomInstance.tracker.AddItem();
    iceWall.itemType = 102;  // Use same type as working items (temporarily)
    iceWall.position = {25.0f, -25.0f, -8010.0f};  // Near other track items
    
    spdlog::info("Spawned ice wall with ID {} at position ({}, {}, {})", 
      iceWall.itemId, iceWall.position[0], iceWall.position[1], iceWall.position[2]);
    
    // Notify all clients about the ice wall spawn using proper race item spawn command
    protocol::AcCmdGameRaceItemSpawn iceWallSpawn{
      .itemId = iceWall.itemId,
      .itemType = iceWall.itemType,
      .position = iceWall.position,
      .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
      .member5 = false,
      .removeDelay = -1.0f  // Use same as working items (no removal)
    };
    
    spdlog::info("Sending ice wall spawn using AcCmdGameRaceItemSpawn: itemId={}, position=({}, {}, {})", 
      iceWallSpawn.itemId, iceWallSpawn.position[0], iceWallSpawn.position[1], iceWallSpawn.position[2]);
    
    spdlog::info("Broadcasting to {} clients in room", roomInstance.clients.size());
    for (const ClientId& roomClientId : roomInstance.clients)
    {
      spdlog::info("Sending ice wall spawn to client {}", roomClientId);
      _commandServer.QueueCommand<decltype(iceWallSpawn)>(
        roomClientId, 
        [iceWallSpawn]() { return iceWallSpawn; });
    }
  }

  racer.magicItem.reset();
}

void RaceDirector::HandleUserRaceItemGet(
  ClientId clientId,
  const protocol::AcCmdUserRaceItemGet& command)
{
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto const& item = roomInstance.tracker.GetItems().at(command.itemId);
  protocol::AcCmdGameRaceItemGet get{
    .characterOid = command.characterOid,
    .itemId = command.itemId,
    .itemType = item.itemType,
  };

  // Notify all clients in the room that this item has been picked up
  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(get)>(
      roomClientId,
      [get]()
      {
        return get;
      });
  }
  // Wait for ItemDeck registry, to give the correct amount of SP for item pick up

  _scheduler.Queue(
    [this, clientId, item, &roomInstance]()
    {
      // Respawn the item after a delay
      protocol::AcCmdGameRaceItemSpawn spawn{
        .itemId = item.itemId,
        .itemType = item.itemType,
        .position = item.position,
        .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
        .member5 = false,
        .removeDelay = -1.0f
      };

      for (const ClientId& roomClientId : roomInstance.clients)
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
  
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);
  
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
  
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);
  
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
  for (const ClientId& roomClientId : roomInstance.clients)
  {
    const auto& targetClientContext = _clients[roomClientId];
    if (roomInstance.tracker.GetRacer(targetClientContext.characterUid).oid == command.targetOid)
    {
      _commandServer.QueueCommand<decltype(targetNotify)>(
        roomClientId, 
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
  
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);
  
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleChangeMagicTargetOK");
    return;
  }
  
  // This is where the Bolt should be fired!
  spdlog::info("BOLT FIRED! {} -> {}", command.characterOid, command.targetOid);
  
  // Find the target racer and apply bolt effects
  for (auto& [targetUid, targetRacer] : roomInstance.tracker.GetRacers())
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
      
      for (const ClientId& roomClientId : roomInstance.clients)
      {
        spdlog::info("Sending bolt hit notification to client {}", roomClientId);
        _commandServer.QueueCommand<decltype(boltHitNotify)>(
          roomClientId, 
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
  
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  auto& racer = roomInstance.tracker.GetRacer(clientContext.characterUid);
  
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
    for (const ClientId& roomClientId : roomInstance.clients)
    {
      const auto& targetClientContext = _clients[roomClientId];
      if (roomInstance.tracker.GetRacer(targetClientContext.characterUid).oid == racer.currentTarget)
      {
        _commandServer.QueueCommand<decltype(removeNotify)>(
          roomClientId, 
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
  const auto& clientContext = _clients[clientId];
  auto& roomInstance = _roomInstances[clientContext.roomUid];
  
  // Convert unk2 back to float (it's 1.0f = 1065353216 as uint32)
  float intensity = *reinterpret_cast<const float*>(&command.unk2);
  
  spdlog::info("HandleActivateSkillEffect: clientId={}, characterOid={}, skillId={}, unk1={}, intensity={}", 
    clientId, command.characterOid, command.skillId, command.unk1, intensity);
  
  // Process the skill effect activation - give target extra gauge (Attack Compensation skill)
  auto& targetRacer = roomInstance.tracker.GetRacer(clientContext.characterUid);
  
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


} // namespace server
