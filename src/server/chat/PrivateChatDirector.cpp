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
  bool requireAuthentication)
{
  auto conversationContextIter = _conversations.find(clientId);
  if (conversationContextIter == _conversations.end())
    throw std::runtime_error("Private chat client is not available");

  return conversationContextIter->second;
}

void PrivateChatDirector::Tick()
{
}

Config::PrivateChat& PrivateChatDirector::GetConfig()
{
  return _serverInstance.GetSettings().privateChat;
}

const std::optional<network::ClientId> PrivateChatDirector::GetTargetClientIdByContext(
  const ConversationContext& conversationContext) const
{
  std::optional<ClientId> clientId{};

  // Check if any character uids are invalid 
  if (conversationContext.characterUid == data::InvalidUid or
      conversationContext.targetCharacterUid == data::InvalidUid)
    return clientId;

  for (const auto& [targetClientId, targetConversationContext] : _conversations)
  {
    // Find target conversation based on invoker and target character uids
    bool isTargetConversation = 
      targetConversationContext.characterUid == conversationContext.targetCharacterUid and
      targetConversationContext.targetCharacterUid == conversationContext.characterUid;
    if (isTargetConversation)
    {
      // This is the target conversation
      clientId.emplace(targetClientId);
      return clientId;
    }
  }
  return clientId;
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
  // This arrangement is specific to private chats only
  const data::Uid targetCharacterUid = command.code;
  const data::Uid invokerCharacterUid = command.characterUid;
  const std::string& invokerCharacterName = command.characterName;
  // Always 0 for private chats
  const uint32_t unk3 = command.guildUid;

  spdlog::debug("[{}] (Private) ChatCmdEnterRoom: {} {} {} {}",
    clientId,
    targetCharacterUid,
    invokerCharacterUid,
    invokerCharacterName,
    unk3);

  // TODO: there is no authentication mechanism here
  auto& conversationContext = GetConversationContext(clientId);

  // Confirm invoking character of that uid exists
  const auto& invokerCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(invokerCharacterUid);
  if (not invokerCharacterRecord.IsAvailable())
  {
    // TODO: return cancel
    return;
  }

  // Confirm target character of that uid exists
  const auto& targetCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(targetCharacterUid);
  if (not targetCharacterRecord.IsAvailable())
  {
    // TODO: return cancel
    return;
  }

  // Check if invoker's character name provided in command matches server record
  bool isNameMatch{false};
  invokerCharacterRecord.Immutable([&isNameMatch, invokerCharacterName](const data::Character& character)
  {
    isNameMatch = invokerCharacterName == character.name();
  });

  if (not isNameMatch)
  {
    // TODO: return cancel
    return;
  }

  // Set values for the conversation context (participants)
  conversationContext.characterUid = invokerCharacterUid;
  // TODO: Can there be multiple people in a single conversation? Group chat?
  conversationContext.targetCharacterUid = targetCharacterUid;

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
