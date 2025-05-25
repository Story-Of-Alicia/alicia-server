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
#include "spdlog/spdlog.h"

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
}

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    _settings.address.to_string(),
    _settings.port);

  // Host the server
  _server.Host(_settings.address, _settings.port);
}

void RaceDirector::Terminate()
{
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
  /*assert(enterRoom.otp == 0x44332211);

  _clientCharacters[clientId] = enterRoom.characterUid;
  auto character = _dataDirector.GetCharacter(enterRoom.characterUid);
  auto mount = _dataDirector.GetMount(character->mountUid);
  auto room = _dataDirector.GetRoom(character->roomUid.value());

  // TODO: Send RaceEnterRoomNotify to all clients in the room

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _server.SetCode(clientId, {});
  _server.QueueCommand(
    clientId,
    CommandId::RaceEnterRoomOK,
    [&](auto& sink)
    {
      RaceCommandEnterRoomOK response
      {
        .racers =
        {
          Racer {
            .unk0 = 1,
            .unk1 = 1,
            .level = character->level,
            .exp = 1,
            .uid = enterRoom.characterUid,
            .name = character->nickName,
            .unk5 = 1,
            .unk6 = 1,
            .bitset = 0,
            .isNPC = false,
            .playerRacer = PlayerRacer {
              .characterEquipment = character->characterEquipment,
              .character = character->looks.value(),
              .horse = {.uid = character->mountUid,
                .tid = mount->tid,
                .name = mount->name,
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
                      .spirit = 9,
                      .speed = 9,
                      .strength = 9,
                      .ambition = 0x13
                    },
                  .rating = 0,
                  .clazz = 0x15,
                  .val0 = 1,
                  .grade = 5,
                  .growthPoints = 2,
                  .vals0 =
                    {
                      .stamina = 0x7d0,
                      .attractiveness = 0x3c,
                      .hunger = 0x21c,
                      .val0 = 0x00,
                      .val1 = 0x03E8,
                      .val2 = 0x00,
                      .val3 = 0x00,
                      .val4 = 0x00,
                      .val5 = 0x03E8,
                      .val6 = 0x1E,
                      .val7 = 0x0A,
                      .val8 = 0x0A,
                      .val9 = 0x0A,
                      .val10 = 0x00,
                    },
                  .vals1 =
                    {
                      .val0 = 0x00,
                      .val1 = 0x00,
                      .val2 = 0xb8a167e4,
                      .val3 = 0x02,
                      .val4 = 0x00,
                      .classProgression = 0x32e7d,
                      .val5 = 0x00,
                      .val6 = 0x00,
                      .val7 = 0x00,
                      .val8 = 0x00,
                      .val9 = 0x00,
                      .val10 = 0x04,
                      .val11 = 0x00,
                      .val12 = 0x00,
                      .val13 = 0x00,
                      .val14 = 0x00,
                      .val15 = 0x01
                    },
                  .mastery =
                    {
                      .magic = 0x1fe,
                      .jumping = 0x421,
                      .sliding = 0x5f8,
                      .gliding = 0xcfa4,
                    },
                  .val16 = 0xb8a167e4,
                  .val17 = 0},
              .unk0 = 0
              // Horse equipment?
            },
            .unk8 = {
              .unk0 = 0,
              .anotherPlayerRelatedThing = {.mountUid = character->mountUid, .val1 = 0x12}
            },
            .yetAnotherPlayerRelatedThing = {},
            .playerRelatedThing = {.val1 = 1},
            .unk9 = {.unk0 = 1, .unk1 = 1}
          }
        },
        .unk0 = 1,
        .unk1 = 1,
        .roomDescription = {
          .name = room->name,
          .val_between_name_and_desc = (uint8_t) character->roomUid.value(), // ?
          .description = room->description,
          .unk1 = room->unk0,
          .unk2 = room->unk1,
          .unk3 = 20004,
          .unk4 = room->unk2,
          .missionId = room->missionId,
          .unk6 = room->unk4,
          .unk7 = room->unk6
        }
      };
      RaceCommandEnterRoomOK::Write(response, sink);
    });*/
}

void RaceDirector::HandleChangeRoomOptions(ClientId clientId, const RaceCommandChangeRoomOptions& changeRoomOptions)
{
  // TODO: Actually do something

  // TODO: Send to all clients in the room
  _server.QueueCommand(
    clientId,
    CommandId::RaceChangeRoomOptionsNotify,
    [&](auto& sink)
    {
      RaceCommandChangeRoomOptionsNotify response{
        .optionsBitfield = changeRoomOptions.optionsBitfield,
        .option0 = changeRoomOptions.name,
        .option1 = changeRoomOptions.val_between_name_and_desc,
        .option2 = changeRoomOptions.description,
        .option3 = changeRoomOptions.option3,
        .option4 = changeRoomOptions.map,
        .option5 = changeRoomOptions.raceStarted};
      RaceCommandChangeRoomOptionsNotify::Write(response, sink);
    });
}

void RaceDirector::HandleStartRace(ClientId clientId, const RaceCommandStartRace& startRace)
{
  /*auto characterUid = _clientCharacters[clientId];
  auto character = _dataDirector.GetCharacter(characterUid);
  auto mount = _dataDirector.GetMount(character->mountUid);
  auto room = _dataDirector.GetRoom(character->roomUid.value());

  const RaceCommandStartRaceNotify response {
    .gamemode = 1,
    .unk3 = character->roomUid.value(),
    .map = 20004,
    .racers = {
      {
        .oid = 1,
        .name = character->nickName,
        .unk2 = 1,
        .unk3 = 1,
        .unk4 = 1,
        .p2dId = 1,
        .unk6 = 1,
        .unk7 = 1,
      }
    },
    .ip = htonl(_settings._lobbySettings.raceAdvAddress.to_uint()),
    .port = _settings._lobbySettings.raceAdvPort
  };

  // TODO: Send to all clients in the room
  _server.QueueCommand(
    clientId,
    CommandId::RaceStartRaceNotify,
    [response](auto& sink)
    {
      RaceCommandStartRaceNotify::Write(response, sink);
    });*/
}

void RaceDirector::HandleRaceTimer(ClientId clientId, const UserRaceTimer& raceTimer)
{
  _server.QueueCommand(
    clientId,
    CommandId::UserRaceTimer,
    [&](auto& sink)
    {
      UserRaceTimerOK response{
        //.unk0 = raceTimer.timestamp + 1000 * 60,
        .unk1 = raceTimer.timestamp + 1000 * 60};
      UserRaceTimerOK::Write(response, sink);
    });
}

} // namespace alicia
