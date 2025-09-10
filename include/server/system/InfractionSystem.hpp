//
// Created by rgnter on 10/09/2025.
//

#ifndef INFRACTIONSYSTEM_HPP
#define INFRACTIONSYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>

namespace server
{

class ServerInstance;

class InfractionSystem
{
public:
  struct Verdict
  {
    bool preventChatting{false};
    bool preventServerJoining{false};
    bool preventRoomJoining{false};
    bool preventBreeding{false};
  };

  explicit InfractionSystem(ServerInstance& serverInstance);

  [[nodiscard]] Verdict CheckOutstandingPunishments(const std::string& userName);

private:
  ServerInstance& _serverInstance;
};

} // namespace server

#endif //INFRACTIONSYSTEM_HPP
