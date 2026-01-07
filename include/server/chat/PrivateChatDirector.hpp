//
// Created by SergeantSerk on 30/12/2025.
//

#ifndef PRIVATECHATDIRECTOR_HPP
#define PRIVATECHATDIRECTOR_HPP

#include <libserver/network/chatter/ChatterServer.hpp>
#include <libserver/data/DataDefinitions.hpp>

#include "server/Config.hpp"

namespace server
{

//! Chat client OTP constant
constexpr uint32_t PrivateChatOtpConstant = 0x14E05CE5;

class ServerInstance;

class PrivateChatDirector
  : private IChatterServerEventsHandler
{
private:
  struct ConversationContext
  {
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Uid of the target character in this conversation.
    data::Uid targetCharacterUid{data::InvalidUid};
  };

public:
  explicit PrivateChatDirector(ServerInstance& serverInstance);

  //! Get chat config.
  //! @return Chat config.
  [[nodiscard]] Config::PrivateChat& GetConfig();

  void Initialize();
  void Terminate();
  ConversationContext& GetConversationContext(
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

  std::unordered_map<network::ClientId, ConversationContext> _conversations;
};

} // namespace server

#endif //PRIVATECHATDIRECTOR_HPP
