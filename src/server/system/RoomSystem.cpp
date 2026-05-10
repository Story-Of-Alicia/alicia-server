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

#include "server/system/RoomSystem.hpp"

#include <cassert>
#include <ranges>
#include <stdexcept>

namespace server
{

void RoomSystem::CreateRoom(const std::function<void(Room&)>& consumer)
{
  std::unique_lock roomsLock(_roomsLock);
  const auto roomUid = ++_sequencedId;
  const auto [it, inserted] = _rooms.try_emplace(
    roomUid,
    std::move(Room(roomUid)));
  assert(inserted);
  
  auto& [room, roomMutex] = it->second;
  roomsLock.unlock();

  std::scoped_lock lock(roomMutex);
  consumer(room);
}

void RoomSystem::GetRoom(const uint32_t uid, const std::function<void(Room&)>& consumer)
{
  std::unique_lock roomsLock(_roomsLock);
  const auto it = _rooms.find(uid);
  if (it == _rooms.end())
    throw std::runtime_error("Room does not exist");

  auto& [room, roomMutex] = it->second;
  roomsLock.unlock();

  std::scoped_lock lock(roomMutex);
  consumer(it->second.room);
}

bool RoomSystem::RoomExists(uint32_t uid)
{
  std::scoped_lock lock(_roomsLock);
  return _rooms.contains(uid);
}

std::vector<Room::Snapshot> RoomSystem::GetRoomsSnapshot()
{
  std::scoped_lock roomsLock(_roomsLock);

  std::vector<Room::Snapshot> rooms;
  for (auto& entry : _rooms)
  {
    std::scoped_lock roomLock(entry.second.mutex);
    rooms.emplace_back(entry.second.room.GetRoomSnapshot());
  }

  return rooms;
}

void RoomSystem::DeleteRoom(uint32_t uid)
{
  std::scoped_lock lock(_roomsLock);
  const auto it = _rooms.find(uid);
  if (it == _rooms.end())
    throw std::runtime_error("Room does not exist");
  _rooms.erase(it);
}

} // namespace server
