//
// Created by rgnter on 30/08/2025.
//

#ifndef MESSENGERDIRECTOR_HPP
#define MESSENGERDIRECTOR_HPP

#include <libserver/network/chatter/ChatterServer.hpp>

#include "server/Config.hpp"

namespace server
{

class ServerInstance;

class MessengerDirector
  : private IChatterServerEventsHandler
  , private IChatterCommandHandler
{
public:
  explicit MessengerDirector(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

private:
  Config::Messenger& GetConfig();

  void HandleClientConnected(network::ClientId clientId) override;
  void HandleClientDisconnected(network::ClientId clientId) override;

  void HandleChatterLogin(
    network::ClientId clientId,
    const protocol::ChatCmdLogin& command) override;

  ChatterServer _chatterServer;
  ServerInstance& _serverInstance;
};

} // namespace server

#endif //MESSENGERDIRECTOR_HPP
