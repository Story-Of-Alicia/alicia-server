//
// Created by rgnter on 30/08/2025.
//

#ifndef MESSENGERDIRECTOR_HPP
#define MESSENGERDIRECTOR_HPP

#include <libserver/network/chatter/ChatterServer.hpp>
#include <libserver/data/DataDefinitions.hpp>

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
  struct ClientContext
  {
    //! Whether the client is authenticated.
    bool isAuthenticated{false};
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Messenger status of the client.
    protocol::Status status{protocol::Status::Hidden};
  };

  Config::Messenger& GetConfig();

  void HandleClientConnected(network::ClientId clientId) override;
  void HandleClientDisconnected(network::ClientId clientId) override;

  void HandleChatterLogin(
    network::ClientId clientId,
    const protocol::ChatCmdLogin& command) override;

  void HandleChatterLetterList(
    network::ClientId clientId,
    const protocol::ChatCmdLetterList& command) override;

  void HandleChatterLetterSend(
    network::ClientId clientId,
    const protocol::ChatCmdLetterSend& command) override;

  void HandleChatterGuildLogin(
    network::ClientId clientId,
    const protocol::ChatCmdGuildLogin& command) override;

  ChatterServer _chatterServer;
  ServerInstance& _serverInstance;

  std::unordered_map<network::ClientId, ClientContext> _clients;
};

} // namespace server

#endif //MESSENGERDIRECTOR_HPP
