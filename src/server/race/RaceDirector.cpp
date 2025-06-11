//
// Created by alborrajo on 30/12/2024.
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

#include "libserver/registry/RoomRegistry.hpp"

#include <spdlog/spdlog.h>

namespace alicia
{

RaceDirector::RaceDirector(
  soa::DataDirector& dataDirector,
  Settings::RaceSettings settings)
  : _settings(std::move(settings))
  , _dataDirector(dataDirector)
  , _server()
{
  _server.RegisterCommandHandler<RaceCommandEnterRoom>(
    CommandId::RaceEnterRoom,
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRoom(clientId, message);
    });

  _server.RegisterCommandHandler<RaceCommandChangeRoomOptions>(
    CommandId::RaceChangeRoomOptions,
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeRoomOptions(clientId, message);
    });

  _server.RegisterCommandHandler<RaceCommandStartRace>(
    CommandId::RaceStartRace,
    [this](ClientId clientId, const auto& message)
    {
      HandleStartRace(clientId, message);
    });

  _server.RegisterCommandHandler<UserRaceTimer>(
    CommandId::UserRaceTimer,
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceTimer(clientId, message);
    });

  _server.RegisterCommandHandler<RaceCommandLoadingComplete>(
  CommandId::RaceLoadingComplete,
  [this](ClientId clientId, const auto& message)
  {
    _server.QueueCommand<RaceCommandLoadingCompleteNotify>(clientId, CommandId::RaceLoadingCompleteNotify, [](){
      return RaceCommandLoadingCompleteNotify{
      .member0 = 1};
    });
  });
}

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    _settings.address.to_string(),
    _settings.port);

  // Host the server
  _server.BeginHost(_settings.address, _settings.port);
}

void RaceDirector::Terminate()
{
  _server.EndHost();
}

void RaceDirector::Tick() {}

/*

Rooms will not be in data director.

  struct Room
  {
    std::string name;
    std::string description;
    uint8_t unk0;
    uint8_t unk1;
    uint8_t unk2;
    uint16_t missionId;
    uint8_t unk4;
    uint16_t bitset;
    uint8_t unk6;
  };

*/

void RaceDirector::HandleEnterRoom(ClientId clientId, const RaceCommandEnterRoom& enterRoom)
{
  assert(enterRoom.otp == 0xBAAD);

  // TODO: Send RaceEnterRoomNotify to all clients in the room

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _server.SetCode(clientId, {});

  auto& roomRegistry = soa::RoomRegistry::Get();
  auto& room = roomRegistry.GetRoom(enterRoom.roomUid);

  RaceCommandEnterRoomOK response{
    .racers = {
      Racer {
        .unk0 = 1,
        .unk1 = 1,
        .level = 0,
        .exp = 1,
        .uid = enterRoom.characterUid,
        .name = "racer",
        .unk5 = 1,
        .unk6 = 1,
        .bitset = 0,
        .isNPC = false,
        .playerRacer = PlayerRacer {
          .characterEquipment = {},
          .character = {},
          .horse = {
            .uid = 2,
            .tid = 0x4e21,
            .name = "default",
            .parts = {.skinId = 0x2, .maneId = 0x3, .tailId = 0x3, .faceId = 0x3},
              .appearance =
                {.scale = 0x4,
                  .legLength = 0x4,
                  .legVolume = 0x5,
                  .bodyLength = 0x3,
                  .bodyVolume = 0x4},
              .stats =
                {
                  .agility = 9,
                  .control = 9,
                  .speed = 9,
                  .strength = 9,
                  .spirit = 0x13
                },
              .rating = 0,
              .clazz = 0x15,
              .val0 = 1,
              .grade = 5,
              .growthPoints = 2,
              .mastery =
                {
                  .spurMagicCount = 0x1fe,
                  .jumpCount = 0x421,
                  .slidingTime = 0x5f8,
                  .glidingDistance = 0xcfa4,
                },
              .val16 = 0xb8a167e4,
              .val17 = 0},
          .unk0 = 0
          // Horse equipment?
        },
        .unk8 = {
          .unk0 = 0,
          //.anotherPlayerRelatedThing = {.mountUid = character->mountUid, .val1 = 0x12}
        },
        .yetAnotherPlayerRelatedThing = {},
        .playerRelatedThing = {.val1 = 1},
        .unk9 = {.unk0 = 1, .unk1 = 1}
      }
    },
    .nowPlaying = 1,
    .unk1 = 1,
    .roomDescription = {
      .name = room.name,
      .val_between_name_and_desc = static_cast<uint8_t>(room.uid), // ?
      .description = room.description,
      .unk1 = room.unk0,
      .unk2 = room.unk1,
      .unk3 = 20004,
      .unk4 = room.unk2,
      .missionId = room.missionId,
      .unk6 = room.unk3,
      .unk7 = room.unk4
    }
  };

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RaceEnterRoomOK,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleChangeRoomOptions(ClientId clientId, const RaceCommandChangeRoomOptions& changeRoomOptions)
{
  // TODO: Actually do something

  RaceCommandChangeRoomOptionsNotify response{
    .optionsBitfield = changeRoomOptions.optionsBitfield,
    .option0 = changeRoomOptions.name,
    .option1 = changeRoomOptions.val_between_name_and_desc,
    .option2 = changeRoomOptions.description,
    .option3 = changeRoomOptions.option3,
    .option4 = changeRoomOptions.map,
    .option5 = changeRoomOptions.raceStarted};

  // TODO: Send to all clients in the room
  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RaceChangeRoomOptionsNotify,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleStartRace(ClientId clientId, const RaceCommandStartRace& startRace)
{
  // Start the race or AcCmdRCRoomCountdown
  const RaceCommandStartRaceNotify response {
    .gamemode = 6,
    .unk3 = 1,
    .map = 20004,
    .racers = {
      {
        .oid = 1,
        .name = "default",
        .unk2 = 1,
        .unk3 = 1,
        .unk4 = 1,
        .p2dId = 1,
        .unk6 = 1,
        .unk7 = 1,
      }
    },
    .ip = _settings.address.to_uint(),
    .port = htons(_settings.port),
  };

  // TODO: Send to all clients in the room
  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RaceStartRaceNotify,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleRaceTimer(ClientId clientId, const UserRaceTimer& raceTimer)
{
  UserRaceTimerOK response{
    .unk0 = raceTimer.timestamp + 10000,
    .unk1 = raceTimer.timestamp + 20000};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::UserRaceTimerOK,
    [response]()
    {
      return response;
    });
}

} // namespace alicia
