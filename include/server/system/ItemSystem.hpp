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

#ifndef ITEMSYSTEM_HPP
#define ITEMSYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>

namespace server
{

class ServerInstance;

class ItemSystem
{
public:
  struct ConsumeVerdict
  {
    //! Flag indicating whether the item was actually consumed.
    bool itemConsumed{false};
    //! UID of the item that was consumer.
    data::Uid itemUid{data::InvalidUid};
    //! Remaining item count.
    uint32_t remainingItemCount{0};
  };

  explicit ItemSystem(ServerInstance& serverInstance);

  ItemSystem(const ItemSystem&) = delete;
  ItemSystem& operator=(ItemSystem&) = delete;
  ItemSystem(ItemSystem&&) = delete;
  ItemSystem& operator=(ItemSystem&&) = delete;

  //! Gets an item from character's inventory or equipment.
  //! @param character Character.
  //! @param itemTid TID of the item.
  //! @return The UID of the item, or `InvalidUid` if no such item is found.
  [[nodiscard]] data::Uid GetItem(
    data::Character& character,
    data::Tid itemTid) const noexcept;

  //! Adds an item(s) to character's inventory or stacks it
  //! with an already existing item.
  //! @param character Character.
  //! @param itemTid TID of the item.
  //! @param count Count of items.
  //! @return The UID of the item stack that the item was added to,
  //!         or the UID of the item that was created.
  [[maybe_unused]] data::Uid AddItem(
    data::Character& character,
    data::Tid itemTid,
    uint32_t count) const noexcept;

  //! Adds a rented item to character's inventory or extends the rent
  //! of an already existing item.
  //! @param character Character.
  //! @param itemTid TID of the item.
  //! @param duration Rent duration.
  //! @return The UID of the item stack that the item was added to,
  //!         or the UID of the item that was created.
  [[maybe_unused]] data::Uid AddItem(
    data::Character& character,
    data::Tid itemTid,
    std::chrono::seconds duration) const noexcept;

  //! Removes an item(s) from character's inventory.
  //! @param character Character.
  //! @param itemTid TID of the item.
  void RemoveItem(
    data::Character& character,
    data::Tid itemTid) const noexcept;

  //! Consumes an item from character's inventory.
  //! @param character Character.
  //! @param itemTid TID of the item.
  //! @param count Count of items to consume.
  //! @return Verdict of consumption.
  [[nodiscard]] ConsumeVerdict ConsumeItem(
    data::Character& character,
    data::Tid itemTid,
    uint32_t count = 1) const noexcept;

  //! Counts how many of an item the character holds in their inventory.
  //! Equipment is not counted, since only inventory items can be consumed.
  //! @param character Character.
  //! @param itemTid TID of the item.
  //! @returns The total count across every stack of that item.
  [[nodiscard]] uint32_t CountItem(
    const data::Character& character,
    data::Tid itemTid) const noexcept;

  //! Checks whether an item is present in character's inventory or equipment.
  //! @param character Character.
  //! @param itemTid TID of the item.
  //! @retval `true` if character does have the item.
  //! @retval `false` if the character does not have the item.
  [[nodiscard]] bool HasItem(
    const data::Character& character,
    data::Tid itemTid) const noexcept;

  //! Checks whether an item is present in character's inventory or equipment.
  //! @param character Character.
  //! @param itemUid UID of the item.
  //! @retval `true` if character does have the item.
  //! @retval `false` if the character does not have the item.
  [[nodiscard]] bool HasItemInstance(
    const data::Character& character,
    data::Uid itemUid) const noexcept;

  //! Collects and erases expired items from character's inventory and equipment.
  //! @param character Character.
  //! @return Collection of items that expired and were erased.
  [[nodiscard]] std::vector<data::Item> CollectAndEraseExpiredItems(
    data::Character& character) const noexcept;

private:
  ServerInstance& _serverInstance;

  bool CheckExpired(data::Uid itemUid);
};

} // namespace server

#endif //ITEMSYSTEM_HPP
