/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "server/chat/PrivateChatDirector.hpp"

#include "server/ServerInstance.hpp"

#include <boost/container_hash/hash.hpp>

namespace server
{

PrivateChatDirector::PrivateChatDirector(ServerInstance& serverInstance)
  : _chatterServer(*this)
  , _serverInstance(serverInstance)
{
  // Register chatter command handlers
  _chatterServer.RegisterCommandHandler<protocol::ChatCmdEnterRoom>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterEnterRoom(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdChat>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterChat(clientId, command);
    });

  _chatterServer.RegisterCommandHandler<protocol::ChatCmdInputState>(
    [this](network::ClientId clientId, const auto& command)
    {
      HandleChatterInputState(clientId, command);
    });
}

void PrivateChatDirector::Initialize()
{
  spdlog::debug(
    "Private chat server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _chatterServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void PrivateChatDirector::Terminate()
{
  _chatterServer.EndHost();
}

PrivateChatDirector::ConversationContext& PrivateChatDirector::GetConversationContext(
  const network::ClientId clientId,
  const bool requireAuthentication)
{
  auto conversationContextIter = _conversations.find(clientId);
  if (conversationContextIter == _conversations.end())
    throw std::runtime_error("Private chat client is not available");

  auto& conversationContext = conversationContextIter->second;
  if (requireAuthentication && not conversationContext.isAuthenticated)
    throw std::runtime_error("Private chat client is not authenticated");

  return conversationContext;
}

void PrivateChatDirector::Tick()
{
}

Config::PrivateChat& PrivateChatDirector::GetConfig()
{
  return _serverInstance.GetSettings().privateChat;
}

uint32_t PrivateChatDirector::GrantConversationCode(
  const data::Uid characterUid,
  const data::Uid targetCharacterUid)
{
  // Store the code separately from OtpSystem so EnterRoom can resolve it back
  // to the target participant after the client presents the OTP

  // The OTP must be valid only for this directed participant pair
  std::size_t identityHash = std::hash<uint32_t>()(characterUid);
  boost::hash_combine(identityHash, targetCharacterUid);
  boost::hash_combine(identityHash, PrivateChatOtpConstant);
  const uint32_t code = _serverInstance.GetOtpSystem().GrantCode(identityHash);

  constexpr auto PrivateChatOtpLifetimeSeconds = std::chrono::seconds(30);
  const PendingConversation pendingConversation{
    .characterUid = characterUid,
    .targetCharacterUid = targetCharacterUid,
    .identityHash = identityHash,
    .expiry = std::chrono::steady_clock::now() + PrivateChatOtpLifetimeSeconds};

  std::scoped_lock lock(_pendingConversationsMutex);
  _pendingConversations[characterUid].insert_or_assign(code, pendingConversation);

  return code;
}

const std::optional<network::ClientId> PrivateChatDirector::GetTargetClientIdByContext(
  const ConversationContext& conversationContext) const
{
  // Check if any character uids are invalid 
  if (not conversationContext.isAuthenticated or
      conversationContext.characterUid == data::InvalidUid or
      conversationContext.targetCharacterUid == data::InvalidUid)
    return std::nullopt;

  for (const auto& [targetClientId, targetConversationContext] : _conversations)
  {
    // Find target conversation based on invoker and target character uids
    bool isTargetConversation = 
      targetConversationContext.isAuthenticated and
      targetConversationContext.characterUid == conversationContext.targetCharacterUid and
      targetConversationContext.targetCharacterUid == conversationContext.characterUid;
    if (isTargetConversation)
    {
      // This is the target conversation
      return targetClientId;
    }
  }
  
  return std::nullopt;
}

void PrivateChatDirector::HandleClientConnected(network::ClientId clientId)
{
  spdlog::debug("Client {} connected to the private chat server from {}",
    clientId,
    _chatterServer.GetClientAddress(clientId).to_string());
  _conversations.try_emplace(clientId);
}

void PrivateChatDirector::HandleClientDisconnected(network::ClientId clientId)
{
  spdlog::debug("Client {} disconnected from the private chat server", clientId);

  // Terminate the disconnect client's private chat instance
  protocol::ChatCmdEndChatTrs notify{};
  _chatterServer.QueueCommand<decltype(notify)>(clientId, [notify](){ return notify; });

  // Terminate the corresponding client's private chat instance
  const auto& conversationContext = _conversations.at(clientId);
  const std::optional<ClientId> targetClientId = GetTargetClientIdByContext(
    conversationContext);
  if (targetClientId.has_value())
  {
    // Target client found, disconnect
    _chatterServer.QueueCommand<decltype(notify)>(targetClientId.value(), [notify](){ return notify; });
  }

  _conversations.erase(clientId);
}

void PrivateChatDirector::HandleChatterEnterRoom(
  network::ClientId clientId,
  const protocol::ChatCmdEnterRoom& command)
{
  spdlog::debug("[{}] (Private) ChatCmdEnterRoom: {} {} {} {}",
    clientId,
    command.code,
    command.characterUid,
    command.characterName,
    command.guildUid);

  auto& conversationContext = GetConversationContext(clientId, false);

  const auto rejectPrivateChatLogin = [this, clientId]()
  {
    protocol::ChatCmdEnterRoomAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::ChatLoginFailed};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    _chatterServer.DisconnectClient(clientId);
  };

  PendingConversation pendingConversation{};
  // A code is consumed on successful auth and on OTP auth failure. Other validation
  // failures leave the ticket until its own OTP expiry rejects it.
  const auto erasePendingConversation =
    [this, characterUid = command.characterUid, code = command.code]()
    {
      std::scoped_lock lock(_pendingConversationsMutex);
      const auto characterIter = _pendingConversations.find(characterUid);
      if (characterIter == _pendingConversations.end())
        return;

      characterIter->second.erase(code);
      if (characterIter->second.empty())
        _pendingConversations.erase(characterIter);
    };

  {
    std::scoped_lock lock(_pendingConversationsMutex);

    // Resolve the client-supplied (character uid, code) pair into the pending
    // directed conversation created by MessengerDirector.
    const auto characterIter = _pendingConversations.find(command.characterUid);
    if (characterIter == _pendingConversations.cend())
    {
      spdlog::warn(
        "Client {} tried to enter private chat as character {} with unknown auth code {}",
        clientId,
        command.characterUid,
        command.code);
    }
    else if (const auto pendingConversationIter = characterIter->second.find(command.code);
             pendingConversationIter == characterIter->second.cend())
    {
      spdlog::warn(
        "Client {} tried to enter private chat as character {} with unknown auth code {}",
        clientId,
        command.characterUid,
        command.code);
    }
    else if (std::chrono::steady_clock::now() > pendingConversationIter->second.expiry)
    {
      // Mirror the short-lived OTP behavior and remove the stale ticket lazily.
      spdlog::warn(
        "Client {} tried to enter private chat as character {} with expired auth code {}",
        clientId,
        command.characterUid,
        command.code);
      characterIter->second.erase(pendingConversationIter);
      if (characterIter->second.empty())
        _pendingConversations.erase(characterIter);
    }
    else
    {
      pendingConversation = pendingConversationIter->second;
    }
  }

  if (pendingConversation.characterUid == data::InvalidUid)
  {
    rejectPrivateChatLogin();
    return;
  }

  if (command.guildUid != 0)
  {
    spdlog::warn(
      "Client {} tried to enter private chat as character {} with guild uid {}",
      clientId,
      command.characterUid,
      command.guildUid);
    rejectPrivateChatLogin();
    return;
  }

  const auto& characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    command.characterUid);
  if (not characterRecord.IsAvailable())
  {
    spdlog::warn(
      "Client {} tried to enter private chat as unavailable character {}",
      clientId,
      command.characterUid);
    rejectPrivateChatLogin();
    return;
  }

  const auto& targetCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
    pendingConversation.targetCharacterUid);
  if (not targetCharacterRecord.IsAvailable())
  {
    spdlog::warn(
      "Client {} tried to enter private chat with unavailable target character {}",
      clientId,
      pendingConversation.targetCharacterUid);
    rejectPrivateChatLogin();
    return;
  }

  bool characterNameMatches{false};
  characterRecord.Immutable(
    [&characterNameMatches, &command](const data::Character& character)
    {
      characterNameMatches = command.characterName == character.name();
    });

  if (not characterNameMatches)
  {
    spdlog::warn(
      "Client {} tried to enter private chat as character {} with an unexpected character name",
      clientId,
      command.characterUid);
    rejectPrivateChatLogin();
    return;
  }

  // Validate the server-side OTP after resolving the ticket so the code remains
  // tied to the expected participant pair.
  const bool isAuthorized = _serverInstance.GetOtpSystem().AuthorizeCode(
    pendingConversation.identityHash,
    command.code);
  if (not isAuthorized)
  {
    spdlog::warn(
      "Client {} tried to enter private chat as character {} but failed authentication with auth code {}",
      clientId,
      command.characterUid,
      command.code);

    erasePendingConversation();
    rejectPrivateChatLogin();
    return;
  }

  erasePendingConversation();

  // At this point future chat/input-state commands can use the participant pair
  // without trusting client-provided EnterRoom fields again.
  conversationContext.isAuthenticated = true;
  conversationContext.characterUid = pendingConversation.characterUid;
  conversationContext.targetCharacterUid = pendingConversation.targetCharacterUid;

  // Deserialises but has no handler
  protocol::ChatCmdEnterRoomAckOk response{};
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void PrivateChatDirector::HandleChatterChat(
  network::ClientId clientId,
  const protocol::ChatCmdChat& command)
{
  spdlog::debug("[{}] ChatCmdChat: {} [{}]",
    clientId,
    command.message,
    command.role == protocol::ChatCmdChat::Role::User ? "User" :
      command.role == protocol::ChatCmdChat::Role::Op ? "Op" :
      command.role == protocol::ChatCmdChat::Role::GameMaster ? "GameMaster" :
      std::format(
        "unknown role {}",
        static_cast<uint8_t>(command.role)));

  const auto& conversationContext = GetConversationContext(clientId);

  protocol::ChatCmdChatTrs notify{
    .characterUid = conversationContext.characterUid,
    .message = command.message};

  // Send message to invoker
  _chatterServer.QueueCommand<decltype(notify)>(clientId, [notify](){ return notify; });

  // TODO: this works instantenously for connected clients. For disconnected clients that are reconnecting,
  // buffer the message by waiting x secs to check if the target character has connected to the private chat.

  // Try send message to target character
  const std::optional<ClientId> targetClientId = GetTargetClientIdByContext(
    conversationContext);
  if (targetClientId.has_value())
  {
    // Target client found, disconnect
    _chatterServer.QueueCommand<decltype(notify)>(targetClientId.value(), [notify](){ return notify; });
  }
}

void PrivateChatDirector::HandleChatterInputState(
  network::ClientId clientId,
  const protocol::ChatCmdInputState& command)
{
  spdlog::debug("[{}] ChatCmdInputState: {}",
    clientId,
    command.state);

  // Observed values:
  // 01 - when entering the channel/chat window opens
  // 03 - when closing private chat window (unique to each conversation instance)

  // Tag 7 & 10 does not seem to implement a handler for this and just simply returns (noop)
  // Corresponding command: ChatCmdInputStateTrs

  const auto& conversationContext = GetConversationContext(clientId);
  const std::optional<ClientId> targetClientId = GetTargetClientIdByContext(
    conversationContext);

  if (not targetClientId.has_value())
    // Terminate chat client? Consider the fact that maybe they are just getting started (connecting)
    return;

  // Unconfirmed to be working, implemented nevertheless as it seemed best to use these commands for what triggers it
  if (command.state == 1)
  {
    protocol::ChatCmdEnterBuddyTrs notify{
      .unk0 = conversationContext.characterUid};
    _serverInstance.GetDataDirector().GetCharacter(conversationContext.characterUid).Immutable(
      [&notify](const data::Character& character)
      {
        notify.unk1 = character.name();
      });
    _chatterServer.QueueCommand<decltype(notify)>(targetClientId.value(), [notify](){ return notify; });
  }
  else if (command.state == 3)
  {
    protocol::ChatCmdLeaveBuddyTrs notify{
      .unk0 = conversationContext.characterUid};
    _chatterServer.QueueCommand<decltype(notify)>(targetClientId.value(), [notify](){ return notify; });
  }
}

} // namespace server
