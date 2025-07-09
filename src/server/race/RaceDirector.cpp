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
  _commandServer.RegisterCommandHandler<protocol::RaceCommandEnterRoom>(
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRoom(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RaceCommandChangeRoomOptions>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeRoomOptions(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RaceCommandStartRace>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStartRace(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RaceCommandUserRaceTimer>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceTimer(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RaceCommandLoadingComplete>(
    [this](ClientId clientId, const auto& message)
    {
      _commandServer.QueueCommand<protocol::RaceCommandLoadingCompleteNotify>(
        clientId,
        []()
        {
          return protocol::RaceCommandLoadingCompleteNotify{
            .oid = 1};
        });
    });

  _commandServer.RegisterCommandHandler<protocol::RaceCommandReadyRace>(
    [this](ClientId clientId, const auto& message)
    {
      HandleReadyRace(clientId, message);
    });
}

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    GetSettings().address.to_string(),
    GetSettings().port);

  // Host the server
  _commandServer.BeginHost(GetSettings().address, GetSettings().port);
}

void RaceDirector::Terminate()
{
  _commandServer.EndHost();
}

void RaceDirector::Tick() {}

void RaceDirector::HandleClientConnected(ClientId clientId)
{
  spdlog::info("Client {} connected to the race", clientId);
}

void RaceDirector::HandleClientDisconnected(ClientId clientId)
{
  spdlog::info("Client {} disconnected from the race", clientId);
}

ServerInstance& RaceDirector::GetServerInstance()
{
  return _serverInstance;
}

Settings::RaceSettings& RaceDirector::GetSettings()
{
  return GetServerInstance().GetSettings()._raceSettings;
}

void RaceDirector::HandleEnterRoom(
  ClientId clientId,
  const protocol::RaceCommandEnterRoom& command)
{
  assert(command.otp == 0xBAAD);

  auto& clientContext = _clientContexts[clientId];
  clientContext.characterUid = command.characterUid;
  clientContext.roomUid = command.roomUid;

  auto& roomRegistry = RoomRegistry::Get();
  auto& room = roomRegistry.GetRoom(command.roomUid);

  auto& roomInstance = _roomInstances[command.roomUid];
  roomInstance.worldTracker.AddCharacter(
    clientContext.characterUid);

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});

  protocol::RaceCommandEnterRoomOK response{
    .nowPlaying = 1,
    .unk1 = 1, // Room code
    .roomDescription = {
      .name = room.name,
      .val_between_name_and_desc = static_cast<uint8_t>(room.uid), // ?
      .description = room.description,
      .unk1 = room.unk0,
      .gameMode = room.gameMode,
      .unk3 = 8, // Currently selected map, can be both ADV or normal map
      .teamMode = room.teamMode,
      .missionId = room.missionId,
      .unk6 = room.unk3,
      .unk7 = room.unk4}}; // 0 = Shows room above your skill bracket warning, 1 and above = hides the warning

  protocol::Racer joiningRacer;

  for (const auto& [characterUid, characterOid] : roomInstance.worldTracker.GetCharacterEntities())
  {
    auto& protocolRacer = response.racers.emplace_back();

    const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
      characterUid);
    characterRecord.Immutable(
      [this, characterOid, &protocolRacer](const data::Character& character)
      {
        protocolRacer.level = character.level();
        protocolRacer.oid = character.uid();
        protocolRacer.uid = character.uid();
        protocolRacer.name = character.name();
        protocolRacer.isHidden = false;
        protocolRacer.isNPC = false;

        protocolRacer.avatar = protocol::Avatar{};

        protocol::BuildProtocolCharacter(protocolRacer.avatar->character, character);

        const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
          character.mountUid());
        mountRecord->Immutable([&protocolRacer](const data::Horse& mount)
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

  protocol::RaceCommandEnterRoomNotify notify{
    .racer = joiningRacer,
    .averageTimeRecord = clientContext.characterUid};

  for (const ClientId& roomClientId : roomInstance.clients)
  {
    _commandServer.QueueCommand<decltype(response)>(
      roomClientId,
      [response]()
      {
        return response;
      });
  }

  roomInstance.clients.emplace_back(clientId);
}

void RaceDirector::HandleChangeRoomOptions(
  ClientId clientId,
  const protocol::RaceCommandChangeRoomOptions& command)
{
  // TODO: Actually do something

  protocol::RaceCommandChangeRoomOptionsNotify response{
    .optionsBitfield = command.optionsBitfield,
    .option0 = command.name,
    .option1 = command.val_between_name_and_desc,
    .option2 = command.description,
    .option3 = command.option3,
    .option4 = command.map,
    .option5 = command.raceStarted};

  // TODO: Send to all clients in the room
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleStartRace(
  ClientId clientId,
  const protocol::RaceCommandStartRace& command)
{
  // Start the race or AcCmdRCRoomCountdown
  const protocol::RaceCommandStartRaceNotify response{
    .gamemode = 6,
    .unk3 = 1,
    .map = 8,
    .racers = {
      {
        .oid = 1,
        .name = "regent",
        .unk2 = 1,
        .unk3 = 1,
        .unk4 = 1,
        .p2dId = 3,
        .unk6 = 1,
        .unk7 = 3,
      }},
    .ip = GetSettings().address.to_uint(),
    .port = htons(GetSettings().port),
  };

  // TODO: Send to all clients in the room
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleRaceTimer(
  ClientId clientId,
  const protocol::RaceCommandUserRaceTimer& command)
{
  protocol::RaceCommandUserRaceTimerOK response{
    .unk0 = command.timestamp + 10000,
    .unk1 = command.timestamp + 20000};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleReadyRace(
  ClientId clientId,
  const protocol::RaceCommandReadyRace& command)
{
  auto& clientContext = _clientContexts[clientId];
  clientContext.ready = !_clientContexts[clientId].ready;

  protocol::RaceCommandReadyRaceNotify response{
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

} // namespace server
