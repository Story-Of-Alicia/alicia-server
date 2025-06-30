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
#include  "server/ServerInstance.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "libserver/util/Util.hpp"

#include <ranges>

#include <spdlog/spdlog.h>

namespace server
{

RanchDirector::RanchDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
{
  _commandServer.RegisterCommandHandler<protocol::RanchCommandRanchEnter>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRanchEnter(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandLeaveRanch>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRanchLeave(clientId);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRanchSnapshot>(
    [this](ClientId clientId, const auto& message)
    {
      HandleSnapshot(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRanchCmdAction>(
    [this](ClientId clientId, const auto& message)
    {
      HandleCmdAction(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRanchStuff>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRanchStuff(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUpdateBusyState>(
    [this](ClientId clientId, auto& command)
    {
      HandleUpdateBusyState(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandSearchStallion>(
    [this](ClientId clientId, auto& command)
    {
      HandleSearchStallion(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandEnterBreedingMarket>(
    [this](ClientId clientId, auto& command)
    {
      HandleEnterBreedingMarket(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandTryBreeding>(
    [this](ClientId clientId, auto& command)
    {
      HandleTryBreeding(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandBreedingWishlist>(
    [this](ClientId clientId, auto& command)
    {
      HandleBreedingWishlist(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUpdateMountNickname>(
    [this](ClientId clientId, auto& command)
    {
      HandleUpdateMountNickname(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestStorage>(
    [this](ClientId clientId, auto& command)
    {
      HandleRequestStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandGetItemFromStorage>(
    [this](ClientId clientId, auto& command)
    {
      HandleGetItemFromStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandChat>(
    [this](ClientId clientId, auto& command)
    {
      HandleChat(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandWearEquipment>(
    [this](ClientId clientId, auto& command)
    {
      HandleWearEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRemoveEquipment>(
    [this](ClientId clientId, auto& command)
    {
      HandleRemoveEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUseItem>(
    [this](ClientId clientId, auto& command)
    {
      HandleUseItem(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandCreateGuild>(
    [this](ClientId clientId, auto& command)
    {
      HandleCreateGuild(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestGuildInfo>(
    [this](ClientId clientId, auto& command)
    {
      HandleRequestGuildInfo(clientId, command);
    });
}

void RanchDirector::Initialize()
{
  spdlog::debug(
    "Ranch server listening on {}:{}",
    GetSettings().address.to_string(),
    GetSettings().port);

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestNpcDressList>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestNpcDressList(clientId, message);
    });

  // Host the server.
  _commandServer.BeginHost(GetSettings().address, GetSettings().port);
}

void RanchDirector::Terminate()
{
  _commandServer.EndHost();
}

void RanchDirector::Tick()
{
}

void RanchDirector::HandleClientConnected(ClientId clientId)
{
  spdlog::info("Client {} connected to the ranch", clientId);
}

void RanchDirector::HandleClientDisconnected(ClientId clientId)
{
  spdlog::info("Client {} disconnected from the ranch", clientId);

  HandleRanchLeave(clientId);
  _clientContext.erase(clientId);
}

void RanchDirector::BroadcastSetIntroductionNotify(
  uint32_t characterUid,
  const std::string& introduction)
{
  const auto& clientContext = GetClientContextByCharacterUid(characterUid);

  protocol::RanchCommandSetIntroductionNotify notify{
    .characterUid = characterUid,
    .introduction = introduction};

  for (const ClientId& ranchClientId : _ranches[clientContext.ranchUid]._clients)
  {
    // Prevent broadcast to self.
    if (ranchClientId == clientContext.characterUid)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::BroadcastUpdateMountInfoNotify(
  data::Uid characterUid,
  data::Uid horseUid)
{
  const auto& clientContext = GetClientContextByCharacterUid(characterUid);
  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
    horseUid);

  protocol::RanchCommandUpdateMountInfoNotify notify{};
  horseRecord->Immutable([&notify](const data::Horse& horse)
  {
    protocol::BuildProtocolHorse(notify.horse, horse);
  });

  for (const ClientId& ranchClientId : _ranches[clientContext.ranchUid]._clients)
  {
    // Prevent broadcast to self.
    if (ranchClientId == clientContext.characterUid)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

ServerInstance& RanchDirector::GetServerInstance()
{
  return _serverInstance;
}

Settings::RanchSettings& RanchDirector::GetSettings()
{
  return GetServerInstance().GetSettings()._ranchSettings;
}

RanchDirector::ClientContext& RanchDirector::GetClientContextByCharacterUid(
  data::Uid characterUid)
{
  for (auto& [clientId, clientContext] : _clientContext)
  {
    if (clientContext.characterUid == characterUid)
      return clientContext;
  }

  throw std::runtime_error("Character not associated with any client");
}

void RanchDirector::HandleRanchEnter(
  ClientId clientId,
  const protocol::RanchCommandRanchEnter& command)
{
  // todo verify the OTP against the character UID

  auto& clientContext = _clientContext[clientId];

  clientContext.characterUid = command.characterUid;
  clientContext.ranchUid = command.ranchUid;

  protocol::RanchCommandEnterRanchOK response{
    .ranchId = command.ranchUid,
    .unk0 = "unk0",
    .league = {
      .type = League::Type::Platinum,
      .rankingPercentile = 50}};

  // Get the ranch the user is connecting to.
  const auto ranchRecord = GetServerInstance().GetDataDirector().GetRanches().Get(
    command.ranchUid);
  if (not ranchRecord)
  {
    throw std::runtime_error(
      std::format("Ranch [{}] not available",
        command.characterUid));
  }

  ranchRecord->Immutable([&response](const data::Ranch& ranch)
  {
    response.ranchName = ranch.name();
  });

  auto& ranchInstance = _ranches[command.ranchUid];

  // Add the character to the ranch.
  ranchInstance._worldTracker.AddCharacter(
    command.characterUid);

  RanchCharacter enteringRanchPlayer;

  // Add the ranch horses.
  for (auto [horseUid, horseEntityId] : ranchInstance._worldTracker.GetHorseEntities())
  {
    auto& ranchHorse = response.horses.emplace_back();
    ranchHorse.ranchIndex = horseEntityId;

    auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(horseUid);
    if (not horseRecord)
    {
      throw std::runtime_error(
        std::format("Horse [{}] not available", horseUid));
    }

    horseRecord->Immutable([&ranchHorse](const data::Horse& horse)
    {
      protocol::BuildProtocolHorse(ranchHorse.horse, horse);
    });
  }

  // Add the ranch characters.
  for (auto [characterUid, characterEntityId] : ranchInstance._worldTracker.GetCharacterEntities())
  {
    auto& ranchCharacter = response.characters.emplace_back();
    ranchCharacter.ranchIndex = characterEntityId;
    ranchCharacter.guild = {
      .val1 = 0};

    auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(characterUid);
    if (not characterRecord)
    {
      throw std::runtime_error(
        std::format("Character [{}] not available", characterUid));
    }

    characterRecord->Immutable([this, &ranchCharacter](const data::Character& character)
    {
      ranchCharacter.uid = character.uid();
      ranchCharacter.name = character.name();
      ranchCharacter.gender = Gender::Unspecified;
      ranchCharacter.introduction = character.introduction();

      ranchCharacter.unk0 = 0;
      ranchCharacter.unk1 = 0;

      protocol::BuildProtocolCharacter(ranchCharacter.character, character);

      // Character's equipment.
      const auto equipment = GetServerInstance().GetDataDirector().GetItems().Get(
        character.characterEquipment());
      if (not equipment)
      {
        throw std::runtime_error(
          std::format(
            "Character's [{}] equipment is not available",
            character.uid()));
      }

      protocol::BuildProtocolItems(ranchCharacter.characterEquipment, *equipment);

      // Character's mount.
      const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
        character.mountUid());
      if (not mountRecord)
      {
        throw std::runtime_error(
          std::format(
            "Character's [{}] mount [{}] is not available",
            character.uid(),
            character.mountUid()));
      }

      mountRecord->Immutable([&ranchCharacter](const data::Horse& horse)
      {
        protocol::BuildProtocolHorse(ranchCharacter.mount, horse);
        ranchCharacter.rent = {
          .mountUid = horse.uid(),
          .val1 = 0x12};
      });
    });

    if (command.characterUid == characterUid)
    {
      enteringRanchPlayer = ranchCharacter;
    }
  }

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Notify to all other players of the entering player.
  const protocol::RanchCommandEnterRanchNotify ranchJoinNotification {
    .character = enteringRanchPlayer
  };

  // Iterate over all the clients connected
  // to the ranch and broadcast join notification.
  for (ClientId ranchClient : ranchInstance._clients)
  {
    _commandServer.QueueCommand<decltype(ranchJoinNotification)>(
      ranchClient,
      [ranchJoinNotification](){
        return ranchJoinNotification;
      });
  }

  ranchInstance._clients.emplace(clientId);
}

void RanchDirector::HandleRanchLeave(ClientId clientId)
{
  const auto& clientContext = _clientContext[clientId];
  auto& ranchInstance = _ranches[clientContext.ranchUid];

  protocol::RanchCommandLeaveRanchNotify notify{
    .characterId = clientContext.characterUid};

  ranchInstance._worldTracker.RemoveCharacter(notify.characterId);
  ranchInstance._clients.erase(clientId);

  for (const ClientId& ranchClientId : ranchInstance._clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::HandleSnapshot(
  ClientId clientId,
  const protocol::RanchCommandRanchSnapshot& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto& ranchInstance = _ranches[clientContext.ranchUid];

  protocol::RanchCommandRanchSnapshotNotify response {
    .ranchIndex = ranchInstance._worldTracker.GetCharacterEntityId(
      clientContext.characterUid),
    .type = command.type,
  };

  switch (command.type)
  {
    case protocol::RanchCommandRanchSnapshot::Full:
    {
      response.full = command.full;
      break;
    }
    case protocol::RanchCommandRanchSnapshot::Partial:
    {
      response.partial = command.partial;
      break;
    }
  }

  for (const auto& ranchClient : ranchInstance._clients)
  {
    // Do not broadcast to the client that sent the snapshot.
    if (ranchClient == clientId)
      continue;

    _commandServer.QueueCommand<decltype(response)>(
      ranchClient,
      [response]()
      {
        return response;
      });
  }
}

void RanchDirector::HandleCmdAction(
  ClientId clientId,
  const protocol::RanchCommandRanchCmdAction& command)
{
  protocol::RanchCommandRanchCmdActionNotify response{
    .unk0 = 2,
    .unk1 = 3,
    .unk2 = 1,};

  // TODO: Actual implementation of it
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRanchStuff(
  ClientId clientId,
  const protocol::RanchCommandRanchStuff& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  if (not characterRecord)
  {
    throw std::runtime_error(
      std::format("Character [{}] not available", clientContext.characterUid));
  }

  protocol::RanchCommandRanchStuffOK response{
    command.eventId,
    command.value};

  // Todo: needs validation
  characterRecord->Mutable([&command, &response](data::Character& character)
  {
    character.carrots() += command.value;
    response.totalMoney = character.carrots();
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });
}

void RanchDirector::HandleUpdateBusyState(
  ClientId clientId,
  const protocol::RanchCommandUpdateBusyState& command)
{
  auto& clientContext = _clientContext[clientId];
  auto& ranchInstance = _ranches[clientContext.ranchUid];

  protocol::RanchCommandUpdateBusyStateNotify response {
    .characterId = clientContext.characterUid,
    .busyState = command.busyState};

  clientContext.busyState = command.busyState;

  for (auto ranchClientId : ranchInstance._clients)
  {
    _commandServer.QueueCommand<decltype(response)>(
      ranchClientId,
      [response]()
      {
        return response;
      });
  }
}

void RanchDirector::HandleSearchStallion(
  ClientId clientId,
  const protocol::RanchCommandSearchStallion& command)
{
  protocol::RanchCommandSearchStallionOK response{
    .unk0 = 0,
    .unk1 = 0};

  const auto horseUids = GetServerInstance().GetDataDirector().GetHorses().GetKeys();
  for (const auto& horseUid : horseUids)
  {
    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
      horseUid);

    auto& stallion = response.stallions.emplace_back();
    horseRecord->Immutable([&stallion](const data::Horse& horse)
    {
      stallion.unk0 = "unk";
      stallion.uid = horse.uid();
      stallion.tid = horse.tid();

      stallion.chance = 0xFF;
      stallion.unk7 = 0xFF;
      stallion.unk8 = 0xFF;

      stallion.name = horse.name();
      stallion.grade = horse.grade();
      stallion.unk11 = 0xFF;

      protocol::BuildProtocolHorseStats(stallion.stats, horse.stats);
      protocol::BuildProtocolHorseParts(stallion.parts, horse.parts);
      protocol::BuildProtocolHorseAppearance(stallion.appearance, horse.appearance);
    });
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleEnterBreedingMarket(
  ClientId clientId,
  const protocol::RanchCommandEnterBreedingMarket& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  protocol::RanchCommandEnterBreedingMarketOK response;

  characterRecord->Immutable([this, &response](const data::Character& character)
  {
    const auto horseRecords = GetServerInstance().GetDataDirector().GetHorses().Get(
      character.horses());

    for (const auto& horseRecord : *horseRecords)
    {
      auto& protocolHorse = response.availableHorses.emplace_back();
      horseRecord.Immutable([&protocolHorse](const data::Horse& horse)
      {
        protocolHorse.uid = horse.uid();
        protocolHorse.tid = horse.tid();

        // todo figure out the rest
      });
    }
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleTryBreeding(
  ClientId clientId,
  const protocol::RanchCommandTryBreeding& command)
{
  protocol::RanchCommandTryBreedingOK response{
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
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleBreedingWishlist(
  ClientId clientId,
  const protocol::RanchCommandBreedingWishlist& command)
{
  protocol::RanchCommandBreedingWishlistOK response{};

  // TODO: Actually do something
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleUpdateMountNickname(
  ClientId clientId,
  const protocol::RanchCommandUpdateMountNickname& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  bool canRenameHorse = false;
  characterRecord->Mutable([this, &canRenameHorse, horseUid = command.horseUid](data::Character& character)
  {
    const bool ownsHorse = std::ranges::contains(character.horses(), horseUid);
    if (not ownsHorse)
      return;

    constexpr data::Tid HorseRenameItem = 45003;
    const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
      character.inventory());

    // Find the horse rename item.
    auto horseRenameItemUid = data::InvalidUid;
    for (const auto& itemRecord : *itemRecords)
    {
      itemRecord.Immutable([&horseRenameItemUid](const data::Item& item)
      {
        if (item.tid() == HorseRenameItem)
        {
          horseRenameItemUid = item.uid();
        }
      });

      // Break early if the item was found.
      if (horseRenameItemUid != data::InvalidUid)
        break;
    }

    // Find the item in the inventory.
    const auto itemInventoryIter = std::ranges::find(
      character.inventory(), horseRenameItemUid);

    // Remove the item from the inventory.
    character.inventory().erase(itemInventoryIter);
    canRenameHorse = true;
  });

  if (not canRenameHorse)
  {
    protocol::RanchCommandUpdateMountNicknameCancel response{};
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
    command.horseUid);

  horseRecord->Mutable([horseName = command.name](data::Horse& horse)
  {
    horse.name() = horseName;
  });

  protocol::RanchCommandUpdateMountNicknameOK response{
    .horseUid = command.horseUid,
    .nickname = command.name,
    .unk1 = command.unk1,
    .unk2 = 0};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  BroadcastUpdateMountInfoNotify(clientContext.characterUid, response.horseUid);
}

void RanchDirector::HandleRequestStorage(
  ClientId clientId,
  const protocol::RanchCommandRequestStorage& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  protocol::RanchCommandRequestStorageOK response{
    .category = command.category,
    .page = command.page};

  const bool showPurchases = command.category == protocol::RanchCommandRequestStorage::Category::Purchases;

  // Fill the stored items, either from the purchase category or the gift category.

  characterRecord->Immutable([this, showPurchases, page = static_cast<size_t>(command.page), &response](const data::Character& character) mutable
  {
    const auto storedItemRecords = GetServerInstance().GetDataDirector().GetStoredItems().Get(
      showPurchases ? character.purchases() : character.gifts());
    if (not storedItemRecords || storedItemRecords->empty())
      return;

    const auto pagination = std::views::chunk(*storedItemRecords, 5);
    page = std::max(std::min(page - 1, pagination.size()), size_t{0});

    response.pageCountAndNotification = pagination.size() << 2;

    // todo pagination of stored item records with std::ranges::chunks
    protocol::BuildProtocolStoredItems(response.storedItems, pagination[page ]);
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleGetItemFromStorage(
  ClientId clientId,
  const protocol::RanchCommandGetItemFromStorage& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  bool storedItemIsValid = true;

  // Try to remove the stored item from the character.
  characterRecord->Mutable([this, &storedItemIsValid, storedItemUid = command.storedItemUid](
    data::Character& character)
    {
      // The stored item is either a gift or a purchase.
      // Try to remove from both gift and purchase vectors.

      const auto storedGiftIter = std::ranges::find(character.gifts(), storedItemUid);
      if (storedGiftIter != character.gifts().cend())
      {
        character.gifts().erase(storedGiftIter);
        return;
      }

      const auto storedPurchaseIter = std::ranges::find(character.purchases(), storedItemUid);
      if (storedGiftIter != character.purchases().cend())
      {
        character.purchases().erase(storedGiftIter);
        return;
      }

      storedItemIsValid = false;
    });

  // If the stored item is invalid cancel the takeout.
  if (not storedItemIsValid)
  {
    protocol::RanchCommandGetItemFromStorageCancel response{
      .storedItemUid = command.storedItemUid,
      .status = 0};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  protocol::RanchCommandGetItemFromStorageOK response{
    .storedItemUid = command.storedItemUid,
    .member0 = 0};

  // Get the items assigned to the stored item and fill the protocol command.
  characterRecord->Mutable([this, &response](
    data::Character& character)
    {
      const auto storedItemRecord = GetServerInstance().GetDataDirector().GetStoredItems().Get(
        response.storedItemUid);

      // Collection of the items received from the stored item.
      std::vector<data::Uid> items;

      storedItemRecord->Immutable([this, &items, &response](const data::StoredItem& storedItem)
      {
        items = storedItem.items();
        const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
          items);

        protocol::BuildProtocolItems(response.items, *itemRecords);
      });

      // Add the items to the character's inventory.
      character.inventory().insert(
        character.inventory().end(),
        items.begin(),
        items.end());
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestNpcDressList(
  ClientId clientId,
  const protocol::RanchCommandRequestNpcDressList& requestNpcDressList)
{
  protocol::RanchCommandRequestNpcDressListOK response{
    .unk0 = requestNpcDressList.unk0,
    .dressList = {} // TODO: Fetch dress list from somewhere
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleChat(
  ClientId clientId,
  const protocol::RanchCommandChat& chat)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  auto& ranchInstance = _ranches[clientContext.ranchUid];

  if (chat.message.starts_with("//"))
  {
    for (const auto& response : HandleCommand(clientId, chat.message))
    {
      protocol::RanchCommandChatNotify notify{
        .author = "system",
        .message = response,
        .isBlue = true,};
      SendChat(clientId, notify);
    }

    return;
  }

  protocol::RanchCommandChatNotify response{
    .message = chat.message};

  characterRecord->Immutable([&response](const data::Character& character)
  {
    response.author = character.name();
  });

  for (const auto ranchClientId : ranchInstance._clients)
  {
    SendChat(ranchClientId, response);
  }
}

std::vector<std::string> RanchDirector::HandleCommand(
  ClientId clientId,
  const std::string& message)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  const auto command = util::TokenizeString(
    message.substr(2), ' ');

  if (command.empty())
    return {"No command provided."};

  if (command[0] == "help")
  {
    return {
      "List of available commands:",

      "//online\n"
      "Lists online players",

      "//give item <count> <tid>\n"
      "  - count - Count of items\n"
      "  - tid - TID of item",

      "//horse parts <skinId>\n"
      "              <faceId>\n"
      "              <maneId>\n"
      "              <tailId>\n"
      "  - skinId - ID of the coat\n"
      "  - faceId - ID of the face\n"
      "  - maneId - ID of the mane\n"
      "  - tailId - ID of the tail",

      "//horse appearance <scale>\n"
      "                   <legLength>\n"
      "                   <legVolume>\n"
      "                   <bodyLength>\n"
      "                   <bodyVolume>\n"
      "  - scale - Defaults to 5\n"
      "  - legLength - Defaults to 5\n"
      "  - legVolume - Defaults to 5\n"
      "  - bodyLength - Defaults to 5\n"
      "  - bodyVolume - Defaults to 5",

      "Note: to ignore any parameter,\n"
      "       simply specify 0 as the value.",
      };
  }

  if (command[0] == "online")
  {
    std::vector<std::string> response;
    response.emplace_back() = std::format(
      "Players online ({}):",
      _clientContext.size());

    for (const auto& clientContext : _clientContext)
    {
      const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
        clientContext.second.characterUid);
      characterRecord->Immutable([&response, &clientContext, clientId](const data::Character& character)
      {
        response.emplace_back() = std::format(
          "{}{}", character.name(), clientContext.first == clientId ? " (you)" : "");
      });
    }
    return response;
  }

  if (command[0] == "horse")
  {
    if (command.size() < 2)
      return {"Invalid command sub-literal. (//horse <appearance/parts>)"};

    auto mountUid = data::InvalidUid;

    if (command[1] == "parts")
    {
      if (command.size() < 6)
        return {"Invalid command arguments. (//horse parts <skinId> <faceId> <maneId> <tailId>)"};
      data::Horse::Parts parts{
        .skinId = static_cast<uint32_t>(std::atoi(command[2].c_str())),
        .faceId = static_cast<uint32_t>(std::atoi(command[3].c_str())),
        .maneId = static_cast<uint32_t>(std::atoi(command[4].c_str())),
        .tailId = static_cast<uint32_t>(std::atoi(command[5].c_str()))};

      characterRecord->Immutable([this, &mountUid, &parts](const data::Character& character)
      {
        GetServerInstance().GetDataDirector().GetHorses().Get(character.mountUid())->Mutable([&parts](data::Horse& horse)
        {
          if (parts.faceId() != 0)
            horse.parts.faceId() = parts.faceId();
          if (parts.maneId() != 0)
            horse.parts.maneId() = parts.maneId();
          if (parts.skinId() != 0)
            horse.parts.skinId() = parts.skinId();
          if (parts.tailId() != 0)
            horse.parts.tailId() = parts.tailId();
        });

        mountUid = character.mountUid();
      });

      return {"Parts set! Restart the client."};
    }

    if (command[1] == "appearance")
    {
      if (command.size() < 7)
        return {"Invalid command arguments. (//horse appearance <scale> <legLength> <legVolume> <bodyLength> <bodyVolume>)"};
      data::Horse::Appearance appearance{
        .scale = static_cast<uint32_t>(std::atoi(command[2].c_str())),
        .legLength = static_cast<uint32_t>(std::atoi(command[3].c_str())),
        .legVolume = static_cast<uint32_t>(std::atoi(command[4].c_str())),
        .bodyLength = static_cast<uint32_t>(std::atoi(command[5].c_str())),
        .bodyVolume = static_cast<uint32_t>(std::atoi(command[6].c_str()))};

      characterRecord->Immutable([this, &mountUid, &appearance](const data::Character& character)
      {
        GetServerInstance().GetDataDirector().GetHorses().Get(character.mountUid())->Mutable([&appearance](
          data::Horse& horse)
        {
          if (appearance.scale() != 0)
            horse.appearance.scale() = appearance.scale();
          if (appearance.legLength() != 0)
            horse.appearance.legLength() = appearance.legLength();
          if (appearance.legVolume() != 0)
            horse.appearance.legVolume() = appearance.legVolume();
          if (appearance.bodyLength() != 0)
            horse.appearance.bodyLength() = appearance.bodyLength();
          if (appearance.bodyVolume() != 0)
            horse.appearance.bodyVolume() = appearance.bodyVolume();
        });
        mountUid = character.mountUid();
      });
    }

    BroadcastUpdateMountInfoNotify(clientContext.characterUid, mountUid);
    return {"Parts set! Restart the client."};
  }
  if (command[0] == "give")
  {
    if (command.size() < 2)
      return {"Invalid command sub-literal. (//give <item/horse>)"};

    if (command[1] == "item")
    {
      if (command.size() < 4)
        return {"Invalid command arguments. (//give item <count> <tid>)"};

      uint32_t itemCount = std::atoi(command[2].c_str());
      data::Uid createdItemTid = std::atoi(command[3].c_str());

      // Create the item.
      auto createdItemUid = data::InvalidUid;
      const auto createdItemRecord = GetServerInstance().GetDataDirector().CreateItem();
      createdItemRecord.Mutable([createdItemTid, itemCount, &createdItemUid](data::Item& item)
      {
        item.tid() = createdItemTid;
        item.count() = itemCount;

        createdItemUid = item.uid();
      });

      // Create the stored item.
      auto giftUid = data::InvalidUid;
      const auto storedItem = GetServerInstance().GetDataDirector().CreateStoredItem();
      storedItem.Mutable([this, &giftUid, createdItemUid, createdItemTid](data::StoredItem& storedItem)
      {
        storedItem.items().emplace_back(createdItemUid);
        storedItem.sender() = "System";
        storedItem.message() = std::format("Item '{}'", createdItemTid);

        giftUid = storedItem.uid();
      });

      // Add the stored item as a gift.

      characterRecord->Mutable([giftUid](data::Character& character)
      {
        character.gifts().emplace_back(giftUid);
      });

      return {"Item given. Check your gifts in inventory!"};
    }

    if (command[1] == "horse")
    {
      if (command.size() < 3)
      {
        return {"Not implemented"};
      }
    }
  }

  return {"Invalid command"};
}

void RanchDirector::SendChat(
  ClientId clientId,
  const protocol::RanchCommandChatNotify& chat)
{
  _commandServer.QueueCommand<protocol::RanchCommandChatNotify>(
    clientId,
    [chat]()
    {
      return chat;
    });
}

void RanchDirector::HandleWearEquipment(
  ClientId clientId,
  const protocol::RanchCommandWearEquipment& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  bool equipSuccessful = false;
  characterRecord->Mutable([&command, &equipSuccessful](data::Character& character)
  {
    const bool hasEquippedItem = std::ranges::contains(
      character.inventory(), command.itemUid);
    const bool hasMountedHorse = std::ranges::contains(
      character.horses(), command.itemUid);

    // Make sure the equip UID is either a valid item or a horse.
    equipSuccessful = hasEquippedItem || hasMountedHorse;

    if (hasMountedHorse)
    {
      character.mountUid() = command.itemUid;
    }
    else if (hasEquippedItem)
    {
      const bool isItemAlreadyEquipped = std::ranges::contains(
        character.characterEquipment(), command.itemUid);
      if (not isItemAlreadyEquipped)
      {
        character.characterEquipment().emplace_back(command.itemUid);
      }
    }
  });

  if (equipSuccessful)
  {
    protocol::RanchCommandWearEquipmentOK response{
      .itemUid = command.itemUid,
      .member = command.member};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    BroadcastEquipmentUpdate(clientId);

    return;
  }

  protocol::RanchCommandWearEquipmentCancel response{
    .itemUid = command.itemUid,
    .member = command.member};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRemoveEquipment(
  ClientId clientId,
  const protocol::RanchCommandRemoveEquipment& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  characterRecord->Mutable([&command](data::Character& character)
  {
    const bool ownsItem = std::ranges::contains(
      character.inventory(), command.itemUid);

    // You can't really unequip a horse. You can only switch to a different one.
    // At least in Alicia 1.0.

    if (ownsItem)
    {
      const auto range = std::ranges::remove(
        character.characterEquipment(), command.itemUid);
      character.characterEquipment().erase(range.begin(), range.end());
    }
  });

  // We really don't need to cancel the unequip.
  // Always respond with OK.
  protocol::RanchCommandRemoveEquipmentOK response{
    .uid = command.itemUid};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  BroadcastEquipmentUpdate(clientId);
}

void RanchDirector::HandleCreateGuild(
  ClientId clientId,
  const protocol::RanchCommandCreateGuild& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  protocol::RanchCommandCreateGuildOK response{
    .uid = 0,
    .member2 = 0};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestGuildInfo(
  ClientId clientId,
  const protocol::RanchCommandRequestGuildInfo& command)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  protocol::RanchCommandRequestGuildInfoOK response{
    .guildInfo = {
      .uid = 1,
      .name = "test",
      .description = "desc"}};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::BroadcastEquipmentUpdate(ClientId clientId)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    clientContext.characterUid);

  protocol::RanchCommandUpdateEquipmentNotify notify{
    .characterUid = clientContext.characterUid};

  characterRecord->Immutable([this, &notify](const data::Character& character)
  {
    // Character equipment
    const auto characterEquipment = GetServerInstance().GetDataDirector().GetItems().Get(
      character.characterEquipment());
    protocol::BuildProtocolItems(notify.characterEquipment, *characterEquipment);

    // Mount equipment
    const auto mountEquipment = GetServerInstance().GetDataDirector().GetItems().Get(
      character.mountEquipment());
    protocol::BuildProtocolItems(notify.mountEquipment, *mountEquipment);

    // Mount
    const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
      character.mountUid());

    mountRecord->Immutable([&notify](const data::Horse& mount)
    {
      protocol::BuildProtocolHorse(notify.mount, mount);
    });
  });

  // Broadcast to all the ranch clients.
  const auto& ranchInstance = _ranches[clientContext.ranchUid];
  for (ClientId ranchClientId : ranchInstance._clients)
  {
    // Prevent broadcasting to self.
    if (ranchClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::HandleUseItem(
  ClientId clientId,
  const protocol::RanchCommandUseItem& command)
{
  protocol::RanchCommandUseItemOK response
  {
    response.itemUid = command.itemUid,
    response.unk1 = command.always1,
    response.type = protocol::RanchCommandUseItemOK::ActionType::Empty
  };

  // feed, Action 1 through 3
  //   success - both bytes zero
  //   failure - Action empty
  // toys, always Action 1 to Action 3,
  //   play success indicated by the second byte
  // brushes, always empty response
  //   success - Action empty

  spdlog::info("Play - itemUid: {}, mem1: {}, mem2: {}, {} play",
  command.itemUid,
  command.always1,
  command.always1too,
  command.play == protocol::RanchCommandUseItem::Play::Bad
    ? "Bad"
    : command.play == protocol::RanchCommandUseItem::Play::Good
      ? "Good"
      : command.play == protocol::RanchCommandUseItem::Play::CriticalGood
        ? "Critical good"
        : "Perfect");

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

} // namespace server
