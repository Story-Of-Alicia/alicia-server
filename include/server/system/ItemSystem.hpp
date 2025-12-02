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

#include <span>

namespace server
{

class ServerInstance;

class ItemSystem
{
public:
  explicit ItemSystem(ServerInstance& serverInstance);

  struct ConsumeVerdict
  {
    bool itemConsumed{false};
    data::Uid itemUid{data::InvalidUid};
    uint32_t remainingItemCount{0};
  };

  [[nodiscard]] data::Uid GetItem(
    data::Character& character,
    data::Tid itemTid) const noexcept;

  [[maybe_unused]] data::Uid AddItem(
    data::Character& character,
    data::Tid itemTid,
    uint32_t count) const noexcept;

  [[maybe_unused]] data::Uid AddItem(
    data::Character& character,
    data::Tid itemTid,
    std::chrono::seconds duration) const noexcept;

  void RemoveItem(
    data::Character& character,
    data::Tid itemTid) const noexcept;

  [[nodiscard]] ConsumeVerdict ConsumeItem(
    data::Character& character,
    data::Tid itemTid,
    uint32_t count = 1) const noexcept;

  [[nodiscard]] bool HasItem(
    data::Character& character,
    data::Uid itemUid) const noexcept;

  uint32_t GetItemCount(data::Uid itemUid);

private:
  ServerInstance& _serverInstance;

  bool CheckExpired(data::Uid itemUid);
};

} // namespace server

#endif //ITEMSYSTEM_HPP
