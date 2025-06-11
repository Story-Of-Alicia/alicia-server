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

#include "server/ranch/RanchDirector.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"

#include "spdlog/spdlog.h"

namespace alicia
{

RanchDirector::RanchDirector(soa::DataDirector& dataDirector, Settings::RanchSettings settings)
  : _settings(std::move(settings))
  , _dataDirector(dataDirector)
  , _server()
{
  // Handlers

  // EnterRanch handler
  _server.RegisterCommandHandler<RanchCommandEnterRanch>(
    CommandId::RanchEnterRanch,
    [this](ClientId clientId, const auto& message)
    { HandleEnterRanch(clientId, message); });

  // Snapshot handler
  _server.RegisterCommandHandler<RanchCommandRanchSnapshot>(
    CommandId::RanchSnapshot,
    [this](ClientId clientId, const auto& message)
    { HandleSnapshot(clientId, message); });

  // RanchCmdAction handler
  _server.RegisterCommandHandler<RanchCommandRanchCmdAction>(
    CommandId::RanchCmdAction,
    [this](ClientId clientId, const auto& message)
    { HandleCmdAction(clientId, message); });

  // RanchStuff handler
  _server.RegisterCommandHandler<RanchCommandRanchStuff>(
    CommandId::RanchStuff,
    [this](ClientId clientId, const auto& message)
    { HandleRanchStuff(clientId, message); });

  _server.RegisterCommandHandler<RanchCommandUpdateBusyState>(
    CommandId::RanchUpdateBusyState,
    [this](ClientId clientId, auto& command)
    { HandleUpdateBusyState(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandSearchStallion>(
    CommandId::RanchSearchStallion,
    [this](ClientId clientId, auto& command)
    { HandleSearchStallion(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandEnterBreedingMarket>(
    CommandId::RanchEnterBreedingMarket,
    [this](ClientId clientId, auto& command)
    { HandleEnterBreedingMarket(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandTryBreeding>(
    CommandId::RanchTryBreeding,
    [this](ClientId clientId, auto& command)
    { HandleTryBreeding(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandBreedingWishlist>(
    CommandId::RanchBreedingWishlist,
    [this](ClientId clientId, auto& command)
    { HandleBreedingWishlist(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandUpdateMountNickname>(
    CommandId::RanchUpdateMountNickname,
    [this](ClientId clientId, auto& command)
    { HandleUpdateMountNickname(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandRequestStorage>(
    CommandId::RanchRequestStorage,
    [this](ClientId clientId, auto& command)
    { HandleRequestStorage(clientId, command); });

  _server.RegisterCommandHandler<RanchCommandChat>(
    CommandId::RanchChat,
    [this](ClientId clientId, auto& command)
    { HandleChat(clientId, command); });
}

void RanchDirector::Initialize()
{
  spdlog::debug(
    "Ranch server listening on {}:{}",
    _settings.address.to_string(),
    _settings.port);

  _server.RegisterCommandHandler<RanchCommandRequestNpcDressList>(
    CommandId::RanchRequestNpcDressList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestNpcDressList(clientId, message);
    });

  // Host the server.
  _server.BeginHost(_settings.address, _settings.port);
}

void RanchDirector::Terminate()
{
  _server.EndHost();
}

void RanchDirector::Tick()
{
}

void RanchDirector::HandleEnterRanch(
  ClientId clientId,
  const RanchCommandEnterRanch& enterRanch)
{
  // todo verify the OTP against the character UID

  auto& clientContext = _clientContext[clientId];

  clientContext.characterUid = enterRanch.characterUid;
  clientContext.ranchUid = enterRanch.ranchUid;

  RanchCommandEnterRanchOK response{
    .ranchId = enterRanch.ranchUid,
    .unk0 = "unk0",
    .unk11 = {
      .unk0 = 1,
      .unk1 = 1}};

  // Get the ranch the user is connecting to.
  auto ranchRecord = _dataDirector.GetRanches().Get(enterRanch.ranchUid);
  if (not ranchRecord)
  {
    throw std::runtime_error(
      std::format("Ranch [{}] not available",
        enterRanch.characterUid));
  }

  ranchRecord->Immutable([&response](const soa::data::Ranch& ranch)
  {
    response.ranchName = ranch.name();
  });

  auto& ranchInstance = _ranches[enterRanch.ranchUid];

  // Add the character to the ranch.
  ranchInstance._worldTracker.AddCharacter(
    enterRanch.characterUid);

  RanchCharacter enteringRanchPlayer;

  // Add the ranch horses.
  for (auto [horseUid, horseEntityId] : ranchInstance._worldTracker.GetHorseEntities())
  {
    auto& ranchHorse = response.horses.emplace_back();
    ranchHorse.ranchIndex = horseEntityId;

    auto horseRecord = _dataDirector.GetHorses().Get(horseUid);
    if (not horseRecord)
    {
      throw std::runtime_error(
        std::format("Horse [{}] not available", horseUid));
    }

    horseRecord->Immutable([&ranchHorse](const soa::data::Horse& horse)
    {
      protocol::BuildProtocolHorse(ranchHorse.horse, horse);
    });
  }

  // Add the ranch characters.
  for (auto [characterUid, characterEntityId] : ranchInstance._worldTracker.GetCharacterEntities())
  {
    auto& ranchCharacter = response.characters.emplace_back();
    ranchCharacter.ranchIndex = characterEntityId;
    ranchCharacter.playerRelatedThing = {
      .val1 = 1};

    auto characterRecord = _dataDirector.GetCharacters().Get(characterUid);
    if (not characterRecord)
    {
      throw std::runtime_error(
        std::format("Character [{}] not available", characterUid));
    }

    characterRecord->Immutable([this, &ranchCharacter](const soa::data::Character& character)
    {
      ranchCharacter.uid = character.uid();
      ranchCharacter.name = character.name();
      ranchCharacter.gender = Gender::Unspecified;
      ranchCharacter.unk0 = 1;
      ranchCharacter.unk1 = 1;
      ranchCharacter.description = "this is a ranch player";

      protocol::BuildProtocolCharacter(ranchCharacter.character, character);

      // Character's equipment.
      auto equipment = _dataDirector.GetItems().Get(character.characterEquipment());
      if (not equipment)
      {
        throw std::runtime_error(
          std::format(
            "Character's [{}] equipment is not available",
            character.uid()));
      }

      protocol::BuildProtocolItems(ranchCharacter.characterEquipment, *equipment);

      // Character's mount.
      auto mountRecord = _dataDirector.GetHorses().Get(character.mountUid());
      if (not mountRecord)
      {
        throw std::runtime_error(
          std::format(
            "Character's [{}] mount [{}] is not available",
            character.uid(),
            character.mountUid()));
      }

      mountRecord->Immutable([&ranchCharacter](const soa::data::Horse& horse)
      {
        protocol::BuildProtocolHorse(ranchCharacter.mount, horse);
        ranchCharacter.anotherPlayerRelatedThing = {
          .mountUid = horse.uid(),
          .val1 = 0x12};
      });
      spdlog::info("aa");
    });

    if (enterRanch.characterUid == characterUid)
    {
      enteringRanchPlayer = ranchCharacter;
    }
  }

  spdlog::debug("{} is entering ranch with:", enterRanch.characterUid);
  for (const auto& ranchCharacter : response.characters)
  {
    spdlog::debug(
      "Character '{}' ({}), index {}",
      ranchCharacter.name,
      ranchCharacter.uid,
      ranchCharacter.ranchIndex);
  }

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _server.SetCode(clientId, {});
  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchEnterRanchOK,
    [response]()
    {
      return response;
    });

  // Notify to all other players of the entering player.
  const RanchCommandEnterRanchNotify ranchJoinNotification {
    .character = enteringRanchPlayer
  };

  // Iterate over all the clients connected
  // to the ranch and broadcast join notification.
  for (ClientId ranchClient : ranchInstance._clients)
  {
    spdlog::debug(
      "Sending notification to {}, player {} ('{}') index {} is entering the ranch.",
      _clientContext[ranchClient].characterUid,
      ranchJoinNotification.character.name,
      ranchJoinNotification.character.uid,
      ranchJoinNotification.character.ranchIndex);

    _server.QueueCommand<decltype(ranchJoinNotification)>(
      ranchClient,
      CommandId::RanchEnterRanchNotify,
      [ranchJoinNotification](){
        return ranchJoinNotification;
      });
  }

  ranchInstance._clients.emplace(clientId);
}

void RanchDirector::HandleSnapshot(
  ClientId clientId,
  const RanchCommandRanchSnapshot& snapshot)
{
  const auto& clientContext = _clientContext[clientId];
  auto& ranchInstance = _ranches[clientContext.ranchUid];

  RanchCommandRanchSnapshotNotify response {
    .ranchIndex = ranchInstance._worldTracker.GetCharacterEntityId(
      clientContext.characterUid),
    .type = snapshot.type,
  };

  switch (snapshot.type)
  {
    case RanchCommandRanchSnapshot::Full:
    {
      response.full = snapshot.full;
      break;
    }
    case RanchCommandRanchSnapshot::Partial:
    {
      response.partial = snapshot.partial;
      break;
    }
  }

  for (const auto& ranchClient : ranchInstance._clients)
  {
    // Do not broadcast to the client that sent the snapshot.
    if (ranchClient == clientId)
      continue;

    if (snapshot.type == RanchCommandRanchSnapshot::Full)
    {
      spdlog::debug(
        "Full update from {} sent to {}. [ranchIndex: {}, time: {}, {}, {}, velX: {}, velY: {}, velZ: {}]",
        clientContext.characterUid,
        _clientContext[ranchClient].characterUid,
        snapshot.full.ranchIndex,
        snapshot.full.time,
        snapshot.full.action,
        snapshot.full.timer,
        snapshot.full.velocityX,
        snapshot.full.velocityY,
        snapshot.full.velocityZ);
    }
    else
    {
      spdlog::debug(
        "Partial update from {} sent to {}.",
        clientContext.characterUid,
        _clientContext[ranchClient].characterUid);
    }

    _server.QueueCommand<decltype(response)>(
      ranchClient,
      CommandId::RanchSnapshotNotify,
      [response]()
      {
        return response;
      });
  }
}

void RanchDirector::HandleCmdAction(ClientId clientId, const RanchCommandRanchCmdAction& action)
{
  RanchCommandRanchCmdActionNotify response{
    .unk0 = 2,
    .unk1 = 3,
    .unk2 = 1,};

  // TODO: Actual implementation of it
  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchCmdActionNotify,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRanchStuff(ClientId clientId, const RanchCommandRanchStuff& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = _dataDirector.GetCharacters().Get(
    clientContext.characterUid);

  if (not characterRecord)
  {
    throw std::runtime_error(
      std::format("Character [{}] not available", clientContext.characterUid));
  }

  RanchCommandRanchStuffOK response{
    command.eventId,
    command.value};

  // Todo: needs validation
  characterRecord->Mutable([&command, &response](soa::data::Character& character)
  {
    character.carrots() += command.value;
    response.totalMoney = character.carrots();
  });

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchStuffOK,
    [response]
    {
      return response;
    });
}

void RanchDirector::HandleUpdateBusyState(
  ClientId clientId,
  const RanchCommandUpdateBusyState& command)
{
  auto& clientContext = _clientContext[clientId];
  auto& ranchInstance = _ranches[clientContext.ranchUid];

  RanchCommandUpdateBusyStateNotify response {
    .characterId = clientContext.characterUid,
    .busyState = command.busyState};

  clientContext.busyState = command.busyState;

  for (auto ranchClientId : ranchInstance._clients)
  {
    _server.QueueCommand<decltype(response)>(
      ranchClientId,
      CommandId::RanchSnapshotNotify,
      [response]()
      {
        return response;
      });
  }
}

void RanchDirector::HandleSearchStallion(
  ClientId clientId,
  const RanchCommandSearchStallion& command)
{
  // TODO: Fetch data from DB according to the filters in the request
  RanchCommandSearchStallionOK response{
    .unk0 = 0,
    .unk1 = 0,
    .stallions = {
      RanchCommandSearchStallionOK::Stallion{
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
          .control = 9,
          .speed = 9,
          .strength = 9,
          .spirit = 9},
        .parts = {
          .skinId = 1,
          .maneId = 4,
          .tailId = 4,
          .faceId = 5,
        },
        .appearance = {.scale = 4, .legLength = 4, .legVolume = 5, .bodyLength = 3, .bodyVolume = 4},
        .unk11 = 5,
        .coatBonus = 0}}};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchSearchStallionOK,
    [response]()
    {
      return response;
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
  RanchCommandTryBreedingOK response{
    .uid = command.unk0, // wild guess
    .tid = command.unk1, // lmao
    .val = 0,
    .count = 0,
    .unk0 = 0,
    .parts = {
      .skinId = 1,
      .maneId = 4,
      .tailId = 4,
      .faceId = 5},
    .appearance = {.scale = 4, .legLength = 4, .legVolume = 5, .bodyLength = 3, .bodyVolume = 4},
    .stats = {.agility = 9, .control = 9, .speed = 9, .strength = 9, .spirit = 9},
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

  // TODO: Actually do something
  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchTryBreedingOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleBreedingWishlist(
  ClientId clientId,
  const RanchCommandBreedingWishlist& command)
{
  RanchCommandBreedingWishlistOK response{};

  // TODO: Actually do something
  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchBreedingWishlistOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleUpdateMountNickname(
  ClientId clientId,
  const RanchCommandUpdateMountNickname& command)
{
  // TODO: Actually do something
  RanchCommandUpdateMountNicknameOK response{
    .unk0 = command.unk0,
    .nickname = command.nickname,
    .unk1 = command.unk1,
    .unk2 = 0};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchUpdateMountNicknameOK,
    [response]()
    {
      return response;
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

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchRequestStorageOK,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestNpcDressList(ClientId clientId, const RanchCommandRequestNpcDressList& requestNpcDressList)
{
  RanchCommandRequestNpcDressListOK response{
    .unk0 = requestNpcDressList.unk0,
    .dressList = {} // TODO: Fetch dress list from somewhere
  };

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::RanchRequestNpcDressListOK,
    [response]()
    {
      return response;
    });;
}

void RanchDirector::HandleChat(
  ClientId clientId,
  const RanchCommandChat& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = _dataDirector.GetCharacters().Get(
    clientContext.characterUid);

  auto& ranchInstance = _ranches[clientContext.ranchUid];

  RanchCommandChatNotify response{
    .message = command.message};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.author = character.name();
  });

  for (const auto ranchClientId : ranchInstance._clients)
  {
    _server.QueueCommand<decltype(response)>(
      ranchClientId,
      CommandId::RanchChatNotify,
      [response]()
      {
        return response;
      });
  }
}

} // namespace alicia
