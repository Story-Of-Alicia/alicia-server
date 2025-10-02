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

#include "../../../include/server/system/RoomSystem.hpp"

#include <cassert>
#include <stdexcept>

namespace server
{

Room& RoomSystem::CreateRoom()
{
  const auto [it, inserted] = _rooms.try_emplace(++_sequencedId);
  assert(inserted);

  it->second.uid = _sequencedId;

  return it->second;
}

Room& RoomSystem::GetRoom(uint32_t uid)
{
  const auto it = _rooms.find(uid);
  if (it == _rooms.end())
    throw std::runtime_error("room does not exist");
  return it->second;
}

void RoomSystem::DeleteRoom(uint32_t uid)
{
  const auto it = _rooms.find(uid);
  if (it == _rooms.end())
    throw std::runtime_error("room does not exist");
  _rooms.erase(it);
}

} // namespace server
