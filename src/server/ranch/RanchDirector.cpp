//
// Created by alborrajo on 16/11/2024.
//

#include "server/ranch/RanchDirector.hpp"

#include "spdlog/spdlog.h"

namespace alicia
{

RanchDirector::RanchDirector(soa::DataDirector& dataDirector, Settings::RanchSettings settings)
    : _settings(std::move(settings))
    , _dataDirector(dataDirector)
    , _server()
{
  _ranches[100] = RanchInstance{};

  // Handlers

  // EnterRanch handler
  _server.RegisterCommandHandler<RanchCommandEnterRanch>(
    CommandId::RanchEnterRanch,
    [this](ClientId clientId, const auto& message) { HandleEnterRanch(clientId, message); });

  // Snapshot handler
  _server.RegisterCommandHandler<RanchCommandRanchSnapshot>(
    CommandId::RanchSnapshot,
    [this](ClientId clientId, const auto& message) { HandleSnapshot(clientId, message); });

  // RanchCmdAction handler
  _server.RegisterCommandHandler<RanchCommandRanchCmdAction>(
    CommandId::RanchCmdAction,
    [this](ClientId clientId, const auto& message) { HandleCmdAction(clientId, message); });

  // RanchStuff handler
  _server.RegisterCommandHandler<RanchCommandRanchStuff>(
    CommandId::RanchStuff,
    [this](ClientId clientId, const auto& message) { HandleRanchStuff(clientId, message); });

  _server.RegisterCommandHandler<RanchCommandUpdateBusyState>(
    CommandId::RanchUpdateBusyState,
    [this](ClientId clientId, auto& command) { HandleUpdateBusyState(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandSearchStallion>(
    CommandId::RanchSearchStallion,
    [this](ClientId clientId, auto& command) { HandleSearchStallion(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandEnterBreedingMarket>(
    CommandId::RanchEnterBreedingMarket,
    [this](ClientId clientId, auto& command) { HandleEnterBreedingMarket(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandTryBreeding>(
    CommandId::RanchTryBreeding,
    [this](ClientId clientId, auto& command) { HandleTryBreeding(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandBreedingWishlist>(
    CommandId::RanchBreedingWishlist,
    [this](ClientId clientId, auto& command) { HandleBreedingWishlist(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandUpdateMountNickname>(
    CommandId::RanchUpdateMountNickname,
    [this](ClientId clientId, auto& command) { HandleUpdateMountNickname(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandRequestStorage>(
    CommandId::RanchRequestStorage,
    [this](ClientId clientId, auto& command) { HandleRequestStorage(clientId, command); });
}

void RanchDirector::Initialize()
{
  spdlog::debug(
    "Ranch server listening on {}:{}",
    _settings.address.to_string(),
    _settings.port);

  _server.RegisterCommandHandler<RanchCommandRequestNpcDressList>(
    CommandId::RanchRequestNpcDressList,
    [this](ClientId clientId, const auto& message) {
      HandleRequestNpcDressList(clientId, message);
    });

  // Host the server.
  _server.Host(_settings.address, _settings.port);
}
void RanchDirector::Terminate()
{

}

void RanchDirector::Tick()
{

}

void RanchDirector::HandleEnterRanch(
  ClientId clientId,
  const RanchCommandEnterRanch& enterRanch)
{
  // Todo: Validate the received data and the code.
  // ( so you cant pretend to be someone else :) )

  const auto characterUid = enterRanch.characterUid;
  const auto ranchUid = enterRanch.ranchUid;

  auto ranch = _dataDirector.GetRanches().Get(ranchUid);
  auto& ranchInstance = _ranches[ranchUid];

  // Add character to the ranch.
  ranchInstance._worldTracker.AddCharacter(characterUid);

  RanchPlayer enteringRanchPlayer;

  RanchCommandEnterRanchOK response{
    .ranchId = enterRanch.ranchUid,
    .unk0 = "unk0",
    .ranchName = "default",
    .unk11 = {
      .unk0 = 1,
      .unk1 = 1}
  };

  ranchInstance._worldTracker.AddMount(3);
  ranchInstance._worldTracker.AddMount(2);

  // Add the ranch mounts.
  for (auto [mountUid, mountEntityId] : ranchInstance._worldTracker.GetMountEntities())
  {
    //const auto& mount = _dataDirector.GetHorse(mountUid);
    response.horses.push_back({
      .ranchIndex = mountEntityId,
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
        .stats = {.agility = 9, .spirit = 9, .speed = 9, .strength = 9, .ambition = 0x13},
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
         {.val0 = 0x00,
          .val1 = 0x00,
          .val2 = 0xb8a167e4,
          .val3 = 0x02,
          .val4 = 0x00,
          .classProgression = 0x32e7d,
          .val5 = 0x00,
          .potentialLevel = 0x40,
          .hasPotential = 0x1,
          .potentialValue = 0x64,
          .val9 = 0x00,
          .luck = 0x05,
          .hasLuck = 0x00,
          .val12 = 0x00,
          .fatigue = 0x00,
          .val14 = 0x00,
          .emblem = 0xA},
        .mastery =
         {
           .magic = 0x1fe,
           .jumping = 0x421,
           .sliding = 0x5f8,
           .gliding = 0xcfa4,
         },
        .val16 = 0xb8a167e4,
        .val17 = 0}});
  }

  // Add the ranch players.
  for (auto [characterUid, characterEntityId] : ranchInstance._worldTracker.GetCharacterEntities())
  {
    const RanchPlayer ranchPlayer{
      .userUid = characterUid,
      .name = "ranch player",
      .gender = Gender::Unspecified ,
      .unk0 = 1,
      .unk1 = 1,
      .description = "this is a ranch player",
      .character = {},
      .horse = {
        .uid = 2,
        .tid = 0x4e21,
        .name = "ranch horse",
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
            .potentialLevel = 0x00,
            .hasPotential = 0x00,
            .potentialValue = 0x00,
            .val9 = 0x00,
            .luck = 0x04,
            .hasLuck = 0x00,
            .val12 = 0x00,
            .fatigue = 0x00,
            .val14 = 0x00,
            .emblem = 0x01
          },
        .mastery =
          {
            .magic = 0x1fe,
            .jumping = 0x421,
            .sliding = 0x5f8,
            .gliding = 0xcfa4,
          },
        .val16 = 0xb8a167e4,
        .val17 = 0x186A0
      },
      .characterEquipment = {},
      .playerRelatedThing = {
        .val1 = 1
      },
      .ranchIndex = characterEntityId,
      .anotherPlayerRelatedThing = {.mountUid = 2, .val1 = 0x12}
    };

    if (enterRanch.characterUid == characterUid)
    {
      enteringRanchPlayer = ranchPlayer;
    }

    response.users.push_back(ranchPlayer);
  }

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _server.SetCode(clientId, {});
  _server.QueueCommand<RanchCommandEnterRanchOK>(
    clientId,
    CommandId::RanchEnterRanchOK,
    [response]()
    {
      return response;
    });

  // // Notify to all other players of the entering player.
  // const RanchCommandEnterRanchNotify notification {
  //   .player = enteringRanchPlayer
  // };
  //
  // // Iterate over all the clients and broadcast join notification.
  // for (auto [clientId, clientCharacterUid] : _clientUsers)
  // {
  //   // Todo: Too many uncecessary lookups
  //   // Do not broadcast to the client that sent the snapshot.
  //   if (clientCharacterUid == characterUid)
  //   {
  //     continue;
  //   }
  //
  //   // Do not broadcast to clients that are not on the ranch.
  //   const EntityId characterEntityId = ranchInstance._worldTracker.GetCharacterEntityId(
  //     clientCharacterUid);
  //   if (characterEntityId == InvalidEntityId)
  //   {
  //     continue;
  //   }
  //
  //   _server.QueueCommand(
  //     clientId,
  //     CommandId::RanchEnterRanchNotify,
  //     [&](auto& sink){
  //       RanchCommandEnterRanchNotify::Write(notification, sink);
  //     });
  // }
}

void RanchDirector::HandleSnapshot(
  ClientId clientId,
  const RanchCommandRanchSnapshot& snapshot)
{
  // const auto characterUid = _clientUsers[clientId];
  //
  // auto character = _dataDirector.GetCharacter(characterUid);
  // auto& ranchInstance = _ranches[character->ranchUid];
  //
  // const RanchCommandRanchSnapshotNotify response {
  //   .ranchIndex = ranchInstance._worldTracker.GetCharacterEntityId(characterUid),
  //   .unk0 = snapshot.unk0,
  //   .snapshot = snapshot.snapshot
  // };
  //
  // // Iterate over all the clients and broadcast the snapshot.
  // for (auto [clientId, clientCharacterUid] : _clientUsers)
  // {
  //   // Todo: Too many uncecessary lookups
  //   // Do not broadcast to the client that sent the snapshot.
  //   if (clientCharacterUid == characterUid)
  //   {
  //     continue;
  //   }
  //
  //   // Do not broadcast to clients that are not on the ranch.
  //   const EntityId characterEntityId = ranchInstance._worldTracker.GetCharacterEntityId(
  //     clientCharacterUid);
  //   if (characterEntityId == InvalidEntityId)
  //   {
  //     continue;
  //   }
  //
  //   _server.QueueCommand(
  //     clientId,
  //     CommandId::RanchSnapshotNotify,
  //     [&](auto& sink)
  //     {
  //       RanchCommandRanchSnapshotNotify::Write(response, sink);
  //     });
  // }
}

void RanchDirector::HandleCmdAction(ClientId clientId, const RanchCommandRanchCmdAction& action)
{
  // TODO: Actual implementation of it
  _server.QueueCommand(
    clientId,
    CommandId::RanchCmdActionNotify,
    [action](auto& sink)
    {
      RanchCommandRanchCmdActionNotify response{
        .unk0 = 2,
        .unk1 = 3,
        .unk2 = 1,
      };
      RanchCommandRanchCmdActionNotify::Write(response, sink);
    });
}

void RanchDirector::HandleRanchStuff(ClientId clientId, const RanchCommandRanchStuff& command)
{
  // const DatumUid characterUid = _clientUsers[clientId];
  // auto character = _dataDirector.GetCharacter(characterUid);
  //
  // // todo: needs validation
  // character->carrots += command.value;
  // const auto totalCarrots = character->carrots;
  //
  // _server.QueueCommand(
  //   clientId,
  //   CommandId::RanchStuffOK,
  //   [command, totalCarrots](auto& sink)
  //   {
  //     RanchCommandRanchStuffOK response{
  //       command.eventId,
  //       command.value,
  //       totalCarrots};
  //
  //     RanchCommandRanchStuffOK::Write(response, sink);
  //   });
}

void RanchDirector::HandleUpdateBusyState(
  ClientId clientId,
  const RanchCommandUpdateBusyState& command)
{
  // const DatumUid characterUid = _clientUsers[clientId];
  // auto character = _dataDirector.GetCharacter(characterUid);
  // auto& ranchInstance = _ranches[character->ranchUid];
  //
  // // TODO: Store the busy state in the character instance
  //
  // RanchCommandUpdateBusyStateNotify response {.characterId = characterUid, .busyState = command.busyState};
  //
  // for (auto [clientId, clientCharacterUid] : _clientUsers)
  // {
  //   if (ranchInstance._worldTracker.GetCharacterEntityId(clientCharacterUid) == InvalidEntityId)
  //   {
  //     continue;
  //   }
  //
  //   _server.QueueCommand(
  //     clientId,
  //     CommandId::RanchSnapshotNotify,
  //     [&](auto& sink) { RanchCommandUpdateBusyStateNotify::Write(response, sink); });
  // }
}

void RanchDirector::HandleSearchStallion(
  ClientId clientId,
  const RanchCommandSearchStallion& command)
{
  _server.QueueCommand(
    clientId,
    CommandId::RanchSearchStallionOK,
    [command](auto& sink)
    {
      // TODO: Fetch data from DB according to the filters in the request
      RanchCommandSearchStallionOK response
      {
        .unk0 = 0,
        .unk1 = 0,
        .stallions = {
          RanchCommandSearchStallionOK::Stallion {
            .unk0 = "test",
            .unk1 = 0x3004e21,
            .unk2 = 0x4e21,
            .name = "Juan",
            .grade = 4,
            .chance = 0,
            .price = 1,
            .unk7 = 0xFFFFFFFF,
            .unk8 = 0xFFFFFFFF,
            .stats = {
              .agility = 9,
              .spirit = 9,
              .speed = 9,
              .strength = 9,
              .ambition = 9
            },
            .parts = {
              .skinId = 1,
              .maneId = 4,
              .tailId = 4,
              .faceId = 5,
            },
            .appearance = {
              .scale = 4,
              .legLength = 4,
              .legVolume = 5,
              .bodyLength = 3,
              .bodyVolume = 4
            },
            .unk11 = 5,
            .coatBonus = 0
          }
        }
      };

      RanchCommandSearchStallionOK::Write(response, sink);
    });
}

void RanchDirector::HandleEnterBreedingMarket(ClientId clientId, const RanchCommandEnterBreedingMarket& command)
{
  // const DatumUid characterUid = _clientUsers[clientId];
  // auto character = _dataDirector.GetCharacter(characterUid);
  // _server.QueueCommand(
  //   clientId,
  //   CommandId::RanchEnterBreedingMarketOK,
  //   [&](auto& sink)
  //   {
  //     RanchCommandEnterBreedingMarketOK response;
  //     for(DatumUid horseId : character->horses)
  //     {
  //       auto horse = _dataDirector.GetHorse(horseId);
  //       RanchCommandEnterBreedingMarketOK::AvailableHorse availableHorse
  //       {
  //         .uid = horseId,
  //         .tid = horse->tid,
  //         .unk0 = 0,
  //         .unk1 = 0,
  //         .unk2 = 0,
  //         .unk3 = 0
  //       };
  //       response.availableHorses.push_back(availableHorse);
  //     }
  //     RanchCommandEnterBreedingMarketOK::Write(response, sink);
  //   });
}

void RanchDirector::HandleTryBreeding(
  ClientId clientId,
  const RanchCommandTryBreeding& command)
{
  // TODO: Actually do something
  _server.QueueCommand(
    clientId,
    CommandId::RanchTryBreedingOK,
    [&](auto& sink)
    {
      RanchCommandTryBreedingOK response
      {
        .uid = command.unk0, // wild guess
        .tid = command.unk1, // lmao
        .val = 0,
        .count = 0,
        .unk0 = 0,
        .parts = {
          .skinId = 1,
          .maneId = 4,
          .tailId = 4,
          .faceId = 5
        },
        .appearance = {
          .scale = 4,
          .legLength = 4,
          .legVolume = 5,
          .bodyLength = 3,
          .bodyVolume = 4
        },
        .stats = {
          .agility = 9,
          .spirit = 9,
          .speed = 9,
          .strength = 9,
          .ambition = 9
        },
        .unk1 = 0,
        .unk2 = 0,
        .unk3 = 0,
        .unk4 = 0,
        .unk5 = 0,
        .unk6 = 0,
        .unk7 = 0,
        .unk8 = 0,
        .unk9 = 0,
        .unk10 = 0,
      };
      RanchCommandTryBreedingOK::Write(response, sink);
    });
}

void RanchDirector::HandleBreedingWishlist(
  ClientId clientId,
  const RanchCommandBreedingWishlist& command)
{
  // TODO: Actually do something
  _server.QueueCommand(
    clientId,
    CommandId::RanchBreedingWishlistOK,
    [&](auto& sink)
    {
      RanchCommandBreedingWishlistOK response{};
      RanchCommandBreedingWishlistOK::Write(response, sink);
    });
}

void RanchDirector::HandleUpdateMountNickname(
  ClientId clientId,
  const RanchCommandUpdateMountNickname& command)
{
  // TODO: Actually do something
  RanchCommandUpdateMountNicknameOK response
  {
    .unk0 = command.unk0,
    .nickname = command.nickname,
    .unk1 = command.unk1,
    .unk2 = 0
  };

  _server.QueueCommand(
    clientId,
    CommandId::RanchUpdateMountNicknameOK,
    [response](auto& sink)
    {
      RanchCommandUpdateMountNicknameOK::Write(response, sink);
    });
  
}

void RanchDirector::HandleRequestStorage(
  ClientId clientId,
  const RanchCommandRequestStorage& command)
{
  // TODO: Actually do something
  const RanchCommandRequestStorageOK response{
    .val0 = command.val0,
    .val1 = command.val1};

  _server.QueueCommand(
    clientId,
    CommandId::RanchRequestStorageOK,
    [response](auto& sink)
    {
      RanchCommandRequestStorageOK::Write(response, sink);
    });
}

void RanchDirector::HandleRequestNpcDressList(ClientId clientId, const RanchCommandRequestNpcDressList& requestNpcDressList)
{
  RanchCommandRequestNpcDressListOK response {
    .unk0 = requestNpcDressList.unk0,
    .dressList = {} // TODO: Fetch dress list from somewhere
  };
  
  _server.QueueCommand(
    clientId,
    CommandId::RanchRequestNpcDressListOK,
    [response](auto& sink)
    {
      RanchCommandRequestNpcDressListOK::Write(response, sink);
    });
}

} // namespace alicia
