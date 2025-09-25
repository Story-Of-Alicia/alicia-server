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

#include "libserver/registry/RoomRegistry.hpp"

#include <spdlog/spdlog.h>

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
}

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  test = std::thread([this]()
  {
    asio::io_context ioCtx;
    asio::ip::udp::socket skt(
      ioCtx,
      asio::ip::udp::endpoint(
        asio::ip::address_v4::loopback(),
        10500));

    asio::streambuf buf;
    while (run_test)
    {
      const size_t b = skt.receive(buf.prepare(1024));
      buf.consume(b);
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
  spdlog::info("Client {} connected to the race", clientId);
}

void RaceDirector::HandleClientDisconnected(ClientId clientId)
{
  const auto& clientContext = _clients[clientId];

  const auto roomIter = _roomInstances.find(
    clientContext.roomUid);
  if (roomIter == _roomInstances.cend())
  {
    spdlog::warn(
      "Client {} left a room {} which does not exist.",
      clientId,
      clientContext.roomUid);
  }
  else
  {
    if (clientContext.roomUid != data::InvalidUid)
    {
      auto& roomInstance = _roomInstances[clientContext.roomUid];

      roomInstance.clients.erase(clientId);
      roomInstance.worldTracker.RemoveCharacter(
        clientContext.characterUid);

      // Check if the leaving player was the leader
      bool wasLeader = (roomInstance.leaderCharacterUid == clientContext.characterUid);

      // Notify other clients in the room.
      protocol::AcCmdCRLeaveRoomNotify notify{
        .characterId = clientContext.characterUid,
        .unk0 = 1};

      for (const ClientId& roomClientId : roomInstance.clients)
      {
        _commandServer.QueueCommand<decltype(notify)>(
          roomClientId,
          [notify]()
          {
            return notify;
          });
      }

      // If the leader left and there are still players, assign new leader
      if (wasLeader && !roomInstance.clients.empty())
      {
        // Find the first remaining client and make them leader
        ClientId newLeaderClientId = *roomInstance.clients.begin();
        const auto& newLeaderContext = _clients[newLeaderClientId];
        roomInstance.leaderCharacterUid = newLeaderContext.characterUid;
        
        spdlog::info("Character {} became the new leader of room {} after leader left", 
                     newLeaderContext.characterUid, clientContext.roomUid);
      }

      if (roomInstance.clients.empty())
      {
        RoomRegistry::Get().DeleteRoom(clientContext.roomUid);
        _roomInstances.erase(clientContext.roomUid);
        spdlog::info("Room {} deleted as it is now empty", clientContext.roomUid);
      }
    }
    else
    {
      spdlog::warn("Client {} is not in a room", clientId);
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

void RaceDirector::HandleEnterRoom(
  ClientId clientId,
  const protocol::AcCmdCREnterRoom& command)
{
  assert(command.otp == 0xBAAD);

  auto& clientContext = _clients[clientId];
  clientContext.characterUid = command.characterUid;
  clientContext.roomUid = command.roomUid;

  auto& roomRegistry = RoomRegistry::Get();
  auto& room = roomRegistry.GetRoom(command.roomUid);

  auto& roomInstance = _roomInstances[command.roomUid];
  roomInstance.worldTracker.AddCharacter(command.characterUid);

  // Set the first player joining as room leader
  if (roomInstance.clients.empty())
  {
    roomInstance.leaderCharacterUid = command.characterUid;
    spdlog::info("Character {} became the leader of room {}", command.characterUid, command.roomUid);
  }

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});

  protocol::AcCmdCREnterRoomOK response{
    .nowPlaying = 1,
    .uid = room.uid,
    .roomDescription = {
      .name = room.name,
      .playerCount = room.playerCount,
      .description = room.description,
      .unk1 = 0,
      .gameMode = room.gameMode,
      .mapBlockId = room.mapBlockId,
      .teamMode = room.teamMode,
      .missionId = room.missionId,
      .unk6 = room.unk3,
      .skillBracket = room.unk4}};

  protocol::Racer joiningRacer;

  for (const auto& [characterUid, characterOid] : roomInstance.worldTracker.GetCharacters())
  {
    auto& protocolRacer = response.racers.emplace_back();

    const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
      characterUid);
    characterRecord.Immutable(
      [this, characterOid, &protocolRacer, leaderUid = roomInstance.leaderCharacterUid](const data::Character& character)
      {
        if (character.uid() == leaderUid)
          protocolRacer.member1 = 1; 
        protocolRacer.level = character.level();
        protocolRacer.oid = characterOid;
        protocolRacer.uid = character.uid();
        protocolRacer.name = character.name();
        protocolRacer.isHidden = false;
        protocolRacer.isNPC = false;

        protocolRacer.avatar = protocol::Avatar{};

        protocol::BuildProtocolCharacter(protocolRacer.avatar->character, character);

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
  auto& room = RoomRegistry::Get().GetRoom(clientContext.roomUid);

  room.mapBlockId = command.mapBlockId;

  protocol::AcCmdCRChangeRoomOptionsNotify response{
    .optionsBitfield = command.optionsBitfield,
    .name = command.name,
    .playerCount = command.playerCount,
    .description = command.description,
    .option3 = command.option3,
    .mapBlockId = command.mapBlockId,
    .hasRaceStarted = command.hasRaceStarted};

  const auto& roomInstance = _roomInstances[clientContext.roomUid];

  for (const auto roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(response)>(
      roomClientId,
      [response]()
      {
        return response;
      });
  }
}

void RaceDirector::HandleLeaveRoom(ClientId clientId)
{
  protocol::AcCmdCRLeaveRoomOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleStartRace(
  ClientId clientId,
  const protocol::AcCmdCRStartRace& command)
{
  const auto& clientContext = _clients[clientId];

  const auto& room = RoomRegistry::Get().GetRoom(clientContext.roomUid);
  const auto& roomInstance = _roomInstances[clientContext.roomUid];

  protocol::AcCmdCRStartRaceNotify response{
    .gameMode = room.gameMode,
    .mapBlockId = room.mapBlockId,
    .ip = asio::ip::address_v4::loopback().to_uint(),
    .port = static_cast<uint16_t>(10500),
  };

  for (const auto& [characterUid, characterOid] : roomInstance.worldTracker.GetCharacters())
  {
    std::string characterName;
    GetServerInstance().GetDataDirector().GetCharacter(characterUid).Immutable(
      [&characterName](const data::Character& character)
      {
        characterName = character.name();
    });

    response.racers.emplace_back(protocol::AcCmdCRStartRaceNotify::Racer{
      .oid = characterOid,
      .name = characterName,
      .unk2 = 2,
      .unk3 = 2,
      .unk4 = 2,
      .p2dId = 0,
      .unk6 = 2,
      .unk7 = 2});
  }

  // Send to all clients in the room.
  for (const ClientId& roomClientId : roomInstance.clients)
  {
    const auto& roomClientContext = _clients[roomClientId];
    response.racerOid = roomInstance.worldTracker.GetCharacterOid(
      roomClientContext.characterUid);

    _commandServer.QueueCommand<decltype(response)>(
      roomClientId,
      [response]()
      {
        return response;
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
  
  // Add this client to the loaded clients set
  roomInstance.loadedRaceClients.insert(clientId);

  const auto characterOid = roomInstance.worldTracker.GetCharacterOid(clientContext.characterUid);

  // Notify all clients in the room that this player's loading is complete
  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<protocol::AcCmdCRLoadingCompleteNotify>(
      roomClientId,
      [characterOid]()
      {
        return protocol::AcCmdCRLoadingCompleteNotify{
          .oid = characterOid};
      });
  }

  // Check if all players in the room have completed loading
  bool allPlayersLoaded = (roomInstance.loadedRaceClients.size() == roomInstance.clients.size());

  // If all players have loaded, start countdown for everyone
  if (allPlayersLoaded)
  {
    spdlog::info("All players in room {} have finished loading, starting countdown", clientContext.roomUid);
    
    auto countdownTimestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
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
    
    // Reset loading state for next race
    roomInstance.loadedRaceClients.clear();
  }
}

void RaceDirector::HandleReadyRace(
  ClientId clientId,
  const protocol::AcCmdCRReadyRace& command)
{
  auto& clientContext = _clients[clientId];
  clientContext.ready = !_clients[clientId].ready;

  protocol::AcCmdCRReadyRaceNotify response{
    .characterUid = clientContext.characterUid,
    .ready = clientContext.ready};

  const auto& roomInstance = _roomInstances[clientContext.roomUid];

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

void RaceDirector::HandleUserRaceFinal(
  ClientId clientId,
  const protocol::AcCmdUserRaceFinal& command)
{
  auto& clientContext = _clients[clientId];

  const auto& roomInstance = _roomInstances[clientContext.roomUid];

  protocol::AcCmdUserRaceFinalNotify notify{
    .oid = command.oid,
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
  const auto& roomInstance = _roomInstances[clientContext.roomUid];

  protocol::AcCmdCRRaceResultOK notify{
    .member1 = 1,
    .member2 = 1,
    .member3 = 1,
    .member4 = 1,
    .member5 = 1,
    .member6 = 1};

  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      roomClientId,
      [notify]()
      {
        return notify;
      });
  }

  protocol::AcCmdRCRaceResultNotify result{
    .scores = {{}}};

  for (const auto roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(result)>(
      roomClientId,
      [result]()
      {
        return result;
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

} // namespace server
