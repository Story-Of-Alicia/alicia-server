#ifndef RANCHSNAPSHOT_HPP
#define RANCHSNAPSHOT_HPP

#include <libserver/data/DataDefinitions.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace server
{

struct RanchSnapshot
{
  struct ClientInfo
  {
    data::Uid characterUid{};
    std::string userName;
    std::string characterName;
  };
  struct RanchInfo
  {
    data::Uid rancherUid{};
    std::string ranchName;
    std::vector<ClientInfo> clients;
  };
  size_t totalClients{};
  std::vector<RanchInfo> ranches;
};

} // namespace server

#endif // RANCHSNAPSHOT_HPP
