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

  explicit ItemSystem(ServerInstance& serverInstance);

  bool CheckCharacterHasItem(data::Uid characterUid, data::Tid itemTid);
  data::Uid GetItemByTid(data::Uid characterUid, data::Tid itemTid);
  //! value is either itemCount for stackable items or duration in days for expirable items
  data::Uid CreateNewItem(data::Uid characterUid, data::Tid itemTid, uint32_t value);
  void AddItem(data::Uid itemUid, uint32_t value);
  void ConsumeItem(data::Uid characterUid, data::Uid itemUid, uint32_t itemCount);
  void RemoveItem(data::Uid characterUid, data::Uid itemUid);

private:
  ServerInstance& _serverInstance;

  bool CheckExpired(data::Uid itemUid);
};

} // namespace server

#endif //ITEMSYSTEM_HPP
