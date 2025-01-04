//
// Created by alborrajo on 30/12/2024.
//

#include "server/race/RaceDirector.hpp"
#include "spdlog/spdlog.h"

namespace alicia
{

RaceDirector::RaceDirector(DataDirector& dataDirector, Settings settings)
    : _settings(settings)
    , _dataDirector(dataDirector)
    , _server("Race")
{
  _server.RegisterCommandHandler<RaceCommandEnterRoom>(
    CommandId::RaceEnterRoom,
    [this](ClientId clientId, const auto& message) {
      HandleEnterRoom(clientId, message);
    });

  _server.RegisterCommandHandler<RaceCommandChangeRoomOptions>(
    CommandId::RaceChangeRoomOptions,
    [this](ClientId clientId, const auto& message) {
      HandleChangeRoomOptions(clientId, message);
    });

  // Host the server
  _server.Host(_settings._raceSettings.address, _settings._raceSettings.port);
}

void RaceDirector::HandleEnterRoom(ClientId clientId, const RaceCommandEnterRoom& enterRoom)
{
  assert(enterRoom.otp == 0x44332211);

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
    });
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
      RaceCommandChangeRoomOptionsNotify response {
        .optionsBitfield = changeRoomOptions.optionsBitfield,
        .option0 = changeRoomOptions.name,
        .option1 = changeRoomOptions.val_between_name_and_desc,
        .option2 = changeRoomOptions.description,
        .option3 = changeRoomOptions.option3,
        .option4 = changeRoomOptions.map,
        .option5 = changeRoomOptions.raceStarted
      };
      RaceCommandChangeRoomOptionsNotify::Write(response, sink);
    });
}

} // namespace alicia
