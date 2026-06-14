//
// Created by SergeantSerk on 30/12/2025.
//

#ifndef ALLCHATDIRECTOR_HPP
#define ALLCHATDIRECTOR_HPP

#include <libserver/network/chatter/ChatterServer.hpp>
#include <libserver/data/DataDefinitions.hpp>

#include "server/Config.hpp"

namespace server
{

//! Chat client OTP constant
constexpr uint32_t AllChatOtpConstant = 0x14E05CE5;

class ServerInstance;

class AllChatDirector
  : private IChatterServerEventsHandler
{
private:
  struct ClientContext
  {
    //! Whether the client is authenticated.
    bool isAuthenticated{false};
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Online presence of the client.
    protocol::Presence presence{};
  };

public:
  explicit AllChatDirector(ServerInstance& serverInstance);

  //! Get chat config.
  //! @return Chat config.
  [[nodiscard]] Config::AllChat& GetConfig();

  void Initialize();
  void Terminate();
  ClientContext& GetClientContext(
    network::ClientId clientId,
    bool requireAuthentication = true);
  void Tick();

private:
  void HandleClientConnected(network::ClientId clientId) override;
  void HandleClientDisconnected(network::ClientId clientId) override;

  // Handler methods for chatter commands
  void HandleChatterEnterRoom(
    network::ClientId clientId,
    const protocol::ChatCmdEnterRoom& command);
  
  void HandleChatterChat(
    network::ClientId clientId,
    const protocol::ChatCmdChat& command);

  void HandleChatterInputState(
    network::ClientId clientId,
    const protocol::ChatCmdInputState& command);

  ChatterServer _chatterServer;
  ServerInstance& _serverInstance;

  std::unordered_map<network::ClientId, ClientContext> _clients;
};

} // namespace server

#endif // ALLCHATDIRECTOR_HPP
