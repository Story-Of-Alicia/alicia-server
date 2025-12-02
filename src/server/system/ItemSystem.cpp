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

data::Uid ItemSystem::GetItem(
  data::Character& character,
  data::Tid itemTid) const noexcept
{
  const auto itemRecords = _serverInstance.GetDataDirector().GetItemCache().Get(
    character.inventory());
  if (not itemRecords)
    return data::InvalidUid;

  for (const auto& itemRecord : *itemRecords)
  {
    auto foundItemUid = data::InvalidUid;
    itemRecord.Immutable(
      [&foundItemUid, &itemTid](const data::Item& item)
      {
        if (item.tid() != itemTid)
          return;

        foundItemUid = item.uid();
      });

    if (foundItemUid != data::InvalidUid)
      return foundItemUid;
  }

  return data::InvalidUid;
}

data::Uid ItemSystem::AddItem(
  data::Character& character,
  const data::Tid itemTid,
  const uint32_t count) const noexcept
{
  const auto itemUid = GetItem(character, itemTid);
  if (itemUid == data::InvalidUid)
  {
    const auto createdItemRecord = _serverInstance.GetDataDirector().CreateItem();
    if (not createdItemRecord)
    {
      spdlog::error(
        "Failed to create new item '{}' (x{}) for character '{}'",
        itemTid,
        count,
        character.name());
    }

    auto createdItemUid = data::InvalidUid;

    createdItemRecord.Mutable(
      [&itemTid, &count, &createdItemUid](data::Item& item)
      {
        item.tid() = itemTid;
        item.count() = count;
        item.duration() = std::chrono::seconds::zero();
        item.createdAt() = data::Clock::now();

        createdItemUid = item.uid();
      });

    character.inventory().emplace_back(createdItemUid);
    return createdItemUid;
  }

  const auto itemRecord = _serverInstance.GetDataDirector().GetItemCache().Get(
    itemUid);
  if (not itemRecord)
    return data::InvalidUid;

  itemRecord->Mutable(
    [&count](data::Item& item)
    {
      item.count() += count;
    });

  return itemUid;
}

data::Uid ItemSystem::AddItem(
  data::Character& character,
  data::Tid itemTid,
  std::chrono::seconds duration) const noexcept
{
  const auto itemUid = GetItem(character, itemTid);
  if (itemUid == data::InvalidUid)
  {
    const auto createdItemRecord = _serverInstance.GetDataDirector().CreateItem();
    if (not createdItemRecord)
    {
      spdlog::error(
        "Failed to create new item '{}' (xs) for character '{}'",
        itemTid,
        duration.count(),
        character.name());
    }

    auto createdItemUid = data::InvalidUid;

    createdItemRecord.Mutable(
      [&itemTid, &duration, &createdItemUid](data::Item& item)
      {
        item.tid() = itemTid;
        item.count() = 1;
        item.duration() = duration;
        item.createdAt() = data::Clock::now();

        createdItemUid = item.uid();
      });

    character.inventory().emplace_back(createdItemUid);
    return createdItemUid;
  }

  const auto itemRecord = _serverInstance.GetDataDirector().GetItemCache().Get(
    itemUid);
  if (not itemRecord)
    return data::InvalidUid;

  itemRecord->Mutable(
    [&duration](data::Item& item)
    {
      item.duration() += duration;
    });

  return itemUid;
}

void ItemSystem::RemoveItem(
  data::Character& character,
  const data::Tid itemTid) const noexcept
{
  const auto itemRecords = _serverInstance.GetDataDirector().GetItemCache().Get(
    character.inventory());
  if (not itemRecords)
    return;

  auto itemUid{data::InvalidUid};
  for (const auto& itemRecord : *itemRecords)
  {
    itemRecord.Mutable([&itemUid, &itemTid](
      data::Item& item)
    {
      if (item.tid() != itemTid)
        return;

      itemUid = item.uid();
    });

    if (itemUid != data::InvalidUid)
      break;
  }

  if (itemUid != data::InvalidUid)
  {
    const auto itemsToRemove = std::ranges::remove(character.inventory(), itemUid);
    character.inventory().erase(itemsToRemove.begin(), itemsToRemove.end());

    _serverInstance.GetDataDirector().GetItemCache().Delete(itemUid);
  }
}

ItemSystem::ConsumeVerdict ItemSystem::ConsumeItem(
  data::Character& character,
  const data::Tid itemTid,
  const uint32_t count) const noexcept
{
  const auto itemRecords = _serverInstance.GetDataDirector().GetItemCache().Get(
    character.inventory());
  if (not itemRecords)
    return {};

  for (const auto& itemRecord : *itemRecords)
  {
    ConsumeVerdict verdict{};

    itemRecord.Mutable([&verdict, &itemTid, &count](
      data::Item& item)
    {
      if (item.tid() != itemTid)
        return;

      verdict.itemUid = item.uid();

      if (static_cast<int64_t>(item.count()) - count >= 0)
      {
        item.count() = item.count() - count;
        verdict.itemConsumed = true;
        verdict.remainingItemCount = item.count();
      }
    });

    if (verdict.itemUid != data::InvalidUid)
    {
      if (verdict.remainingItemCount == 0)
      {
        _serverInstance.GetDataDirector().GetItemCache().Delete(verdict.itemUid);
        const auto itemRange = std::ranges::remove(character.inventory(), verdict.itemUid);
        character.inventory().erase(itemRange.begin(), itemRange.end());

        verdict.itemUid = data::InvalidUid;
      }

      return verdict;
    }
  }

  return {};
}

bool ItemSystem::HasItem(
  const data::Character& character,
  const data::Tid itemTid) const noexcept
{
  const auto itemRecords = _serverInstance.GetDataDirector().GetItemCache().Get(
    character.inventory());
  if (not itemRecords)
    return false;

  for (const auto& itemRecord : *itemRecords)
  {
    bool isMatch = false;
    itemRecord.Immutable([&isMatch, &itemTid](
      const data::Item& item)
    {
      isMatch = item.tid() == itemTid;
    });

    if (isMatch)
      return true;
  }

  return false;
}

} // namespace server