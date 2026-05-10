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

#ifndef ROOMREGISTRY_HPP
#define ROOMREGISTRY_HPP

#include "server/room/Room.hpp"

#include <libserver/data/DataDefinitions.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace server
{

class RoomSystem
{
public:
  void CreateRoom(const std::function<void(Room&)>& consumer);
  void GetRoom(uint32_t uid, const std::function<void(Room&)>& consumer);
  bool RoomExists(uint32_t uid);
  void DeleteRoom(uint32_t uid);

  std::vector<Room::Snapshot> GetRoomsSnapshot();

private:
  struct Entry
  {
    Room room;
    std::mutex mutex{};
  };

  uint32_t _sequencedId = 0;
  std::mutex _roomsLock;
  std::unordered_map<uint32_t, Entry> _rooms;
};

} // namespace server

#endif //ROOMREGISTRY_HPP
