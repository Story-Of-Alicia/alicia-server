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
  auto charaterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not charaterRecord)
    throw std::runtime_error("Couldn't check character item, character not available");
  data::Uid itemUid = data::InvalidUid;
  charaterRecord.Immutable([this, &itemUid, itemTid](const data::Character& character)
  {
    const auto itemRecords = _serverInstance.GetDataDirector().GetItemCache().Get(
      character.inventory());

    for (const auto& itemRecord : *itemRecords)
    {
      itemRecord.Immutable([&itemUid, itemTid](const data::Item& item)
      {
        if (item.tid() == itemTid)
        {
          itemUid = item.uid();
        }
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
  auto charaterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not charaterRecord)
    throw std::runtime_error("Couldn't check character item, character not available");

  data::Uid newItemUid = data::InvalidUid;
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

void ItemSystem::AddItem(
  data::Uid itemUid,
  uint32_t value)
{
  auto itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
    throw std::runtime_error("Couldn't add item, item not available");

  itemRecord.Mutable([value, this](data::Item& item)
  {
    const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(
      item.tid());
    if (itemTemplate->type == registry::Item::Type::Temporary)
      item.expiresAt() += std::chrono::hours(24 * value);
    else if (itemTemplate->type == registry::Item::Type::Consumable)
      item.count() += value;
    else
      throw std::runtime_error("Couldn't add item, item is not stackable");
  });
}

void ItemSystem::ConsumeItem(
  data::Uid characterUid,
  data::Uid itemUid,
  uint32_t itemCount)
{
  const auto& itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
    throw std::runtime_error("Couldn't consume item, item not available");

  itemRecord.Mutable([this, itemCount, characterUid](data::Item& item)
  {
    const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(item.tid());
    if (itemTemplate->type != registry::Item::Type::Consumable)
      throw std::runtime_error("Couldn't consume item, item is not consumable");

    // Check if enough item count is available
    if (item.count() < itemCount)
      throw std::runtime_error("Couldn't consume item, not enough item count");

    item.count() -= itemCount;
    if (item.count() <= 0)
    {
      ItemSystem::RemoveItem(characterUid, item.uid());
    }
  });
}

bool ItemSystem::CheckExpired(data::Uid itemUid)
{
  const auto itemRecord = _serverInstance.GetDataDirector().GetItem(itemUid);
  if (not itemRecord)
    throw std::runtime_error("Couldn't check item expiration, item not available");

  bool isExpired = false;

  itemRecord.Immutable([this, &isExpired](const data::Item& item)
  {
    const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(item.tid());
    if (itemTemplate->type == registry::Item::Type::Temporary && item.expiresAt() <= data::Clock::now())
      isExpired = true;
  });

  return isExpired;
}

void ItemSystem::RemoveItem(
  data::Uid characterUid,
  data::Uid itemUid)
{
  const auto & charaterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not charaterRecord)
    throw std::runtime_error("Couldn't remove item, character not available");

  charaterRecord.Mutable([this, itemUid](data::Character& character)
  {
    character.inventory().erase(
      std::remove(character.inventory().begin(), character.inventory().end(), itemUid),
      character.inventory().end());
  });

  _serverInstance.GetDataDirector().GetItemCache().Delete(itemUid);
};

} // namespace server