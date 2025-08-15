//
// Created by rgnter on 25/05/2025.
//

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

class RoomRegistry
{
public:
  Room& CreateRoom();
  Room& GetRoom(uint32_t uid);
  void DeleteRoom(uint32_t uid);
  const std::unordered_map<uint32_t, Room>& GetRooms()
  {
    return _rooms;
  }

  static RoomRegistry& Get()
  {
    static RoomRegistry singleton;
    return singleton;
  }

private:
  uint32_t _sequencedId = 0;
  std::unordered_map<uint32_t, Room> _rooms;
};

} // namespace server

#endif //ROOMREGISTRY_HPP
