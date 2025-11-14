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

#include "server/system/ItemSystem.hpp"

#include <spdlog/spdlog.h>

#include "server/ServerInstance.hpp"
#include "libserver/data/DataDirector.hpp"
#include "libserver/registry/ItemRegistry.hpp"


namespace server
{

ItemSystem::ItemSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

bool ItemSystem::CheckCharacterHasItem(
  data::Uid characterUid,
  data::Tid itemTid)
{
  if (ItemSystem::GetItemByTid(characterUid, itemTid) != data::InvalidUid)
    return true;
  return false;
}

data::Uid ItemSystem::GetItemByTid(
  data::Uid characterUid,
  data::Tid itemTid)
{
  data::Uid itemUid = data::InvalidUid;

  auto charaterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not charaterRecord)
  {
    spdlog::debug("Couldn't check character item, character not available");
    return itemUid;
  }
  
  charaterRecord.Immutable([this, &itemUid, itemTid](const data::Character& character)
  {
    // Combine inventory and equipped items for a single search
    std::vector<data::Uid> allItemUids = character.inventory();
    allItemUids.insert(allItemUids.end(), 
      character.characterEquipment().begin(), 
      character.characterEquipment().end());

    const auto itemRecords = _serverInstance.GetDataDirector().GetItemCache().Get(allItemUids);
    if (!itemRecords)
      return;

    for (const auto& itemRecord : *itemRecords)
    {
      itemRecord.Immutable([&itemUid, itemTid](const data::Item& item)
      {
        if (item.tid() == itemTid)
          itemUid = item.uid();
      });
      
      if (itemUid != data::InvalidUid)
        break;
    }
  });
  return itemUid;
}

data::Uid ItemSystem::CreateNewItem(
  data::Uid characterUid,
  data::Tid itemTid,
  uint32_t value)
{
  data::Uid newItemUid = data::InvalidUid;

  auto charaterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not charaterRecord)
  {
    spdlog::debug("Couldn't check character item, character not available");
    return newItemUid;
  }

  if ((newItemUid = ItemSystem::GetItemByTid(characterUid, itemTid)) != data::InvalidUid)
  {
    ItemSystem::AddItem(newItemUid, value);
    return newItemUid;
  }

  const auto createdItemRecord = _serverInstance.GetDataDirector().CreateItem();
  createdItemRecord.Mutable([this, itemTid, value, &newItemUid](data::Item& item)
  {
    const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(itemTid);
    newItemUid = item.uid();
    item.tid() = itemTid;
    item.count() = 1;

    if (itemTemplate->type == registry::Item::Type::Temporary)
      item.expiresAt() = data::Clock::now() + std::chrono::hours(24 * value);
    else if (itemTemplate->type == registry::Item::Type::Consumable)
      item.count() = value;
  });

  charaterRecord.Mutable([this, &newItemUid](data::Character& character)
  {
    character.inventory().emplace_back(newItemUid);
  });
  return newItemUid;
}

data::Uid ItemSystem::EmplaceItem(
  data::Uid characterUid,
  data::Uid itemUid)
{
  auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not characterRecord)
  {
    return data::InvalidUid;
  }

  const auto itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
  {
    return data::InvalidUid;
  }

  data::Tid itemTid = 0;
  uint32_t itemCount = 0;
  itemRecord.Immutable([&itemTid, &itemCount](const data::Item& item)
  {
    itemTid = item.tid();
    itemCount = item.count();
  });

  // Check if this specific item UID is already in the character's inventory
  bool itemAlreadyInInventory = false;
  characterRecord.Immutable([&itemAlreadyInInventory, itemUid](const data::Character& character)
  {
    itemAlreadyInInventory = std::ranges::contains(character.inventory(), itemUid);
  });

  // If already in inventory, take no further action
  if (itemAlreadyInInventory)
  {
    spdlog::debug("Item {} already exists in character {}'s inventory", itemUid, characterUid);
    return itemUid;
  }

  // If an item with the same TID exists, try to stack
  const data::Uid existingItemUid = GetItemByTid(characterUid, itemTid);
  if (existingItemUid != data::InvalidUid)
  {
    // Try to stack with the existing item
    const auto addResult = AddItem(existingItemUid, itemCount);
    
    if (addResult == ItemSystem::ReturnType::SUCCESS)
    {
      // Successfully stacked, delete the original item
      _serverInstance.GetDataDirector().GetItemCache().Delete(itemUid);
      return existingItemUid;
    }
    
    return data::InvalidUid;
  }

  // No existing item with this TID - add as new item to inventory
  characterRecord.Mutable([itemUid](data::Character& character)
  {
    character.inventory().emplace_back(itemUid);
  });

  return itemUid;
}

ItemSystem::ReturnType ItemSystem::AddItem(
  data::Uid itemUid,
  uint32_t value)
{
  auto itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
  {
    spdlog::debug("Couldn't add item, item not available");
    return ItemSystem::ReturnType::NOT_FOUND;
  }
  ItemSystem::ReturnType result = ItemSystem::ReturnType::SUCCESS;
  itemRecord.Mutable([value, this, &result](data::Item& item)
  {
    const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(
      item.tid());
    if (itemTemplate->type == registry::Item::Type::Temporary)
      item.expiresAt() += std::chrono::hours(24 * value);
    else if (itemTemplate->type == registry::Item::Type::Consumable)
      item.count() += value;
    else
    {
      spdlog::debug("Couldn't add item, item is not stackable");
      result = ItemSystem::ReturnType::NOT_STACKABLE;
    }
  });

  return result;
}

ItemSystem::ReturnType ItemSystem::ConsumeItem(
  data::Uid characterUid,
  data::Uid itemUid,
  uint32_t itemCount)
{
  const auto& itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
  {
    spdlog::debug("Couldn't consume item, item not available");
    return ItemSystem::ReturnType::NOT_FOUND;
  }

  ItemSystem::ReturnType result = ItemSystem::ReturnType::SUCCESS;

  itemRecord.Mutable([this, itemCount, characterUid, &result](data::Item& item)
  {
    const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(item.tid());
    if (itemTemplate->type != registry::Item::Type::Consumable)
    {
      spdlog::debug("Couldn't consume item, item is not consumable");
      result = ItemSystem::ReturnType::NOT_STACKABLE;
      return;
    }

    // Check if enough item count is available
    if (item.count() < itemCount)
    {
      spdlog::debug("Couldn't consume item, not enough item count");
      result = ItemSystem::ReturnType::INSUFFICIENT_QUANTITY;
      return;
    }

    item.count() -= itemCount;
    if (item.count() <= 0)
    {
      //forward result of removal
      result = ItemSystem::RemoveItem(characterUid, item.uid());
      return;
    }
  });

  return result;
}

ItemSystem::ReturnType ItemSystem::RemoveItem(
  data::Uid characterUid,
  data::Uid itemUid)
{
  const auto & charaterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not charaterRecord)
  {
    spdlog::debug("Couldn't check character item, character not available");
    return ItemSystem::ReturnType::NOT_FOUND;
  }

  charaterRecord.Mutable([this, itemUid](data::Character& character)
  {
    character.inventory().erase(
      std::remove(character.inventory().begin(), character.inventory().end(), itemUid),
      character.inventory().end());
  });

  _serverInstance.GetDataDirector().GetItemCache().Delete(itemUid);
  return ItemSystem::ReturnType::SUCCESS;
};

uint32_t ItemSystem::GetItemCount(data::Uid itemUid)
{
  const auto& itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
  {
    spdlog::debug("Couldn't get item count, item not available");
    return 0;
  }

  uint32_t itemCount = 0;
  itemRecord.Immutable([&itemCount](const data::Item& item)
  {
    itemCount = item.count();
  });

  return itemCount;
}

bool ItemSystem::CheckExpired(data::Uid itemUid)
{
  const auto itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
    spdlog::debug("Couldn't check item expiration, item not available");

  bool isExpired = false;

  itemRecord.Immutable([this, &isExpired](const data::Item& item)
  {
    const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(item.tid());
    if (itemTemplate->type == registry::Item::Type::Temporary && item.expiresAt() <= data::Clock::now())
      isExpired = true;
  });

  return isExpired;
}

} // namespace server