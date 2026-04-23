#ifndef RACESNAPSHOT_HPP
#define RACESNAPSHOT_HPP

#include <libserver/data/DataDefinitions.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace server
{

struct RaceSnapshot
{
  struct ClientInfo
  {
    data::Uid characterUid{};
    std::string userName;
    std::string characterName;
  };
  struct RoomInfo
  {
    uint32_t roomUid{};
    std::string roomName;
    std::string stage;
    int64_t stageStartMs{};
    std::string gameMode;
    std::string teamMode;
    uint32_t maxPlayers{};
    uint16_t selectedCourseId{};
    uint16_t mapBlockId{};
    std::string masterName;
    std::vector<ClientInfo> clients;
  };
  size_t totalClients{};
  std::vector<RoomInfo> rooms;
};

} // namespace server

#endif // RACESNAPSHOT_HPP
