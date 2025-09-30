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

#include "../../../include/server/system/RoomSystem.hpp"

#include <spdlog/spdlog.h>
#include <bitset>

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

void RaceDirector::Tick() {}

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

  roomInstance.tracker.AddRacer(
    command.characterUid);

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

    const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
      characterUid);
    characterRecord.Immutable(
      [this, racer, &protocolRacer, leaderUid = roomInstance.masterUid](
        const data::Character& character)
      {
        if (character.uid() == leaderUid)
          protocolRacer.isRoomLeader = true;

        protocolRacer.level = character.level();
        protocolRacer.oid = racer.oid;
        protocolRacer.uid = character.uid();
        protocolRacer.name = character.name();
        protocolRacer.isHidden = false;
        protocolRacer.isNPC = false;

        protocolRacer.avatar = protocol::Avatar{};

        protocol::BuildProtocolCharacter(
          protocolRacer.avatar->character, character);

        protocol::BuildProtocolItems(
          protocolRacer.avatar->characterEquipment,
          *_serverInstance.GetDataDirector().GetItemCache().Get(
            character.characterEquipment()));
        // todo: horse equipment

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

  roomInstance.tracker.RemoveRacer(
    clientContext.characterUid);

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

  if (room.mapBlockId == AllMapsCourseId
    || room.mapBlockId == NewMapsCourseId
    || room.mapBlockId == HotMapsCourseId)
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
    GetServerInstance().GetDataDirector().GetCharacter(characterUid).Immutable(
      [&characterName](const data::Character& character)
      {
        characterName = character.name();
    });

    auto& protocolRacer = notify.racers.emplace_back(protocol::AcCmdCRStartRaceNotify::Player{
      .oid = racer.oid,
      .name = characterName,
      .p2dId = racer.oid,});

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

  // Send to all clients in the room.
  for (const ClientId& roomClientId : roomInstance.clients)
  {
    const auto& roomClientContext = _clients[roomClientId];

    auto& racer = roomInstance.tracker.GetRacer(
      roomClientContext.characterUid);
    racer.state = tracker::RaceTracker::Racer::State::Loading;

    notify.hostOid = racer.oid;

    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]()
      {
        return notify;
      });
  }
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

  spdlog::info(
    "All players in room {} have finished loading,"
    " starting the countdown.",
    clientContext.roomUid);

  const auto countdownTimestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::steady_clock::now().time_since_epoch())
    .count() / 100 + 10 * 10'000'000;

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

  // todo: address npc racers and update their states
  auto& racer = roomInstance.tracker.GetRacer(
    clientContext.characterUid);
  racer.state = tracker::RaceTracker::Racer::State::Finished;

  protocol::AcCmdUserRaceFinalNotify notify{
    .oid = racer.oid,
    .member2 = command.member2};

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
  for (const auto& characterUid : roomInstance.tracker.GetRacers() | std::views::keys)
  {
    auto& score = notify.scores.emplace_back();

    const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
      characterUid);
    if (characterRecord)
    {
      characterRecord.Immutable([&score](const data::Character& character)
      {
        score.uid = character.uid();
        score.name = character.name();
        score.level = character.level();
      });
    }
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
  spdlog::info("abc");
}

void RaceDirector::HandleP2PUserRaceResult(
  ClientId clientId,
  const protocol::AcCmdUserRaceP2PResult& command)
{
  spdlog::info("abc");
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

  protocol::AcCmdCRStarPointGetOK response{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .unk2 = false
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
  
  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .unk2 = false,
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
    .unk2 = false};

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
  const auto& roomInstance = _roomInstances[clientContext.roomUid];

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
    .unknown = 1};

  characterRecord.Immutable([&notify](const data::Character& character)
  {
    notify.author = character.name();
  });

  spdlog::info("[Room {}] {}: {}", clientContext.roomUid, notify.author, notify.message);

  const auto& roomInstance = _roomInstances[clientContext.characterUid];
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

} // namespace server
