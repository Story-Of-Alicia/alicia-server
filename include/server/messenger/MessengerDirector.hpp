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

//! Messenger client OTP constant
constexpr uint32_t MessengerOtpConstant = 0xBABF67A0;

class ServerInstance;

class MessengerDirector
  : private IChatterServerEventsHandler
{
private:
  struct ClientContext
  {
    //! Whether the client is authenticated.
    bool isAuthenticated{false};
    //! The otp code used to authenticate this client.
    std::optional<uint32_t> otpCode{};
    //! Unique ID of the client's character.
    data::Uid characterUid{data::InvalidUid};
    //! Online presence of the client.
    protocol::Presence presence{};
  };

  struct Client
  {
    network::ClientId clientId{};
    ClientContext clientContext{};
  };

public:
  explicit MessengerDirector(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();

  //! Get messenger config.
  //! @return Messenger config.
  [[nodiscard]] Config::Messenger& GetConfig();

  ClientContext& GetClientContext(
    network::ClientId clientId,
    bool requireAuthentication = true);

  const std::optional<Client> GetClientByCharacterUid(const data::Uid characterUid) const;
  const bool IsCharacterOnline(const data::Uid characterUid) const;

  void Tick();

private:
  

  void HandleClientConnected(network::ClientId clientId) override;
  void HandleClientDisconnected(network::ClientId clientId) override;

  // Handler methods for chatter commands
  void HandleChatterLogin(
    network::ClientId clientId,
    const protocol::ChatCmdLogin& command);

  void HandleChatterBuddyAdd(
    network::ClientId clientId,
    const protocol::ChatCmdBuddyAdd& command);

  void HandleChatterBuddyAddReply(
    network::ClientId clientId,
    const protocol::ChatCmdBuddyAddReply& command);

  void HandleChatterBuddyDelete(
    network::ClientId clientId,
    const protocol::ChatCmdBuddyDelete& command);

  void HandleChatterBuddyMove(
    network::ClientId clientId,
    const protocol::ChatCmdBuddyMove& command);

  void HandleChatterGroupAdd(
    network::ClientId clientId,
    const protocol::ChatCmdGroupAdd& command);

  void HandleChatterGroupRename(
    network::ClientId clientId,
    const protocol::ChatCmdGroupRename& command);

  void HandleChatterGroupDelete(
    network::ClientId clientId,
    const protocol::ChatCmdGroupDelete& command);

  void HandleChatterLetterList(
    network::ClientId clientId,
    const protocol::ChatCmdLetterList& command);

  void HandleChatterLetterSend(
    network::ClientId clientId,
    const protocol::ChatCmdLetterSend& command);

  void HandleChatterLetterRead(
    network::ClientId clientId,
    const protocol::ChatCmdLetterRead& command);

  void HandleChatterLetterDelete(
    network::ClientId clientId,
    const protocol::ChatCmdLetterDelete& command);

  void HandleChatterUpdateState(
    network::ClientId clientId,
    const protocol::ChatCmdUpdateState& command);

  void HandleChatterChatInvite(
    network::ClientId clientId,
    const protocol::ChatCmdChatInvite& command);

  void HandleChatterGameInvite(
    network::ClientId clientId,
    const protocol::ChatCmdGameInvite& command);

  void HandleChatterChannelInfo(
    network::ClientId clientId,
    const protocol::ChatCmdChannelInfo& command);

  void HandleChatterGuildLogin(
    network::ClientId clientId,
    const protocol::ChatCmdGuildLogin& command);

  ChatterServer _chatterServer;
  ServerInstance& _serverInstance;

  std::unordered_map<network::ClientId, ClientContext> _clients;
};

} // namespace server

#endif //MESSENGERDIRECTOR_HPP
