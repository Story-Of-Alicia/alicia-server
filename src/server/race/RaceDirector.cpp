//
// Created by alborrajo on 30/12/2024.
//

#include "server/race/RaceDirector.hpp"
#include "spdlog/spdlog.h"

namespace alicia
{

RaceDirector::RaceDirector(DataDirector& dataDirector, Settings::RaceSettings settings)
    : _settings(std::move(settings))
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
  _server.Host(_settings.address, _settings.port);
}

void RaceDirector::HandleEnterRoom(ClientId clientId, const RaceCommandEnterRoom& enterRoom)
{
  assert(enterRoom.otp == 0x44332211);

  _clientCharacters[clientId] = enterRoom.characterUid;
  auto character = _dataDirector.GetCharacter(enterRoom.characterUid);
  auto mount = _dataDirector.GetMount(character->mountUid);

  // TODO: Send RaceEnterRoomNotify to all clients in the room

  // TODO: Actually do something

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
            .unk0 = 0,
            .unk1 = 0,
            .level = character->level,
            .exp = 0,
            .uid = enterRoom.characterUid,
            .name = character->nickName,
            .unk5 = 0,
            .unk6 = 0,
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
        .unk0 = 0,
        .unk1 = 0,
        .roomDescription = {
          .name = "Room",
          .unk0 = 0,
          .description = "Room description",
          .unk1 = 0,
          .unk2 = 0,
          .unk3 = 0,
          .unk4 = 0,
          .missionId = 24,
          .unk6 = 0,
          .unk7 = 0
        }
      };
      RaceCommandEnterRoomOK::Write(response, sink);
    });
}

void RaceDirector::HandleChangeRoomOptions(ClientId clientId, const RaceCommandChangeRoomOptions& changeRoomOptions)
{
  // TODO: Send to all clients in the room
  _server.QueueCommand(
    clientId,
    CommandId::RaceChangeRoomOptionsNotify,
    [&](auto& sink)
    {
      RaceCommandChangeRoomOptionsNotify response {
        .optionsBitfield = changeRoomOptions.optionsBitfield,
        .option0 = changeRoomOptions.option0,
        .option1 = changeRoomOptions.option1,
        .option2 = changeRoomOptions.option2,
        .option3 = changeRoomOptions.option3,
        .option4 = changeRoomOptions.option4,
        .option5 = changeRoomOptions.option5
      };
      RaceCommandChangeRoomOptionsNotify::Write(response, sink);
    });
}

} // namespace alicia
