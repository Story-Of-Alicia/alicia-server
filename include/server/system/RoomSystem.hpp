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

#include <cstdint>
#include <string>
#include <unordered_map>

namespace server
{
enum class TeamMode : uint8_t;
}
namespace server
{
enum class GameMode : uint8_t;
}
namespace server
{

struct Room
{
  uint32_t uid{};
  std::string name;
  std::string description;
  uint16_t missionId{};
  uint16_t mapBlockId{};
  uint32_t otp{};
  uint8_t playerCount;
  GameMode gameMode;
  TeamMode teamMode;
  uint8_t unk3;
  uint16_t bitset;
  uint8_t unk4;
};

class RoomSystem
{
public:
  Room& CreateRoom();
  Room& GetRoom(uint32_t uid);
  void DeleteRoom(uint32_t uid);
  const std::unordered_map<uint32_t, Room>& GetRooms()
  {
    return _rooms;
  }

private:
  uint32_t _sequencedId = 0;
  std::unordered_map<uint32_t, Room> _rooms;
};

} // namespace server

#endif //ROOMREGISTRY_HPP
