//
// Created by SergeantSerk on 30/12/2025.
//

#ifndef PRIVATECHATDIRECTOR_HPP
#define PRIVATECHATDIRECTOR_HPP

#include <libserver/network/chatter/ChatterServer.hpp>
#include <libserver/data/DataDefinitions.hpp>

#include "server/Config.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_map>

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
    //! Whether the client is authenticated.
    bool isAuthenticated{false};
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Uid of the target character in this conversation.
    data::Uid targetCharacterUid{data::InvalidUid};
  };

  struct PendingConversation
  {
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Uid of the target character in this conversation.
    data::Uid targetCharacterUid{data::InvalidUid};
    //! Identity hash used with the OTP system.
    std::size_t identityHash{};
    //! Time when the pending conversation code expires.
    std::chrono::steady_clock::time_point expiry{};
  };

public:
  explicit PrivateChatDirector(ServerInstance& serverInstance);

  //! Get chat config.
  //! @return Chat config.
  [[nodiscard]] Config::PrivateChat& GetConfig();
  //! Grants a directed, one-use private chat code for a single participant.
  [[nodiscard]] uint32_t GrantConversationCode(
    data::Uid characterUid,
    data::Uid targetCharacterUid);

  void Initialize();
  void Terminate();
  ConversationContext& GetConversationContext(
    network::ClientId clientId,
    bool requireAuthentication = true);
  void Tick();

private:
  void HandleClientConnected(network::ClientId clientId) override;
  void HandleClientDisconnected(network::ClientId clientId) override;

  const std::optional<network::ClientId> GetTargetClientIdByContext(
    const ConversationContext& conversationContext) const;

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
  std::mutex _pendingConversationsMutex;
  //! Pending one-use private chat tickets, keyed by character uid and then OTP code.
  std::unordered_map<
    data::Uid,
    std::unordered_map<uint32_t, PendingConversation>> _pendingConversations;
};

} // namespace server

#endif //PRIVATECHATDIRECTOR_HPP
