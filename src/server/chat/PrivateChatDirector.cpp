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

  // TODO: implement the closing/termination of private chats

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
  const uint32_t unk3 = command.guildUid;

  spdlog::debug("[{}] (Private) ChatCmdEnterRoom: {} {} {} {}",
    clientId,
    targetCharacterUid,
    invokerCharacterUid,
    invokerCharacterName,
    unk3);

  // TODO: there is no authentication mechanism here,
  // discover if there is any supported by the client
  auto& conversationContext = GetConversationContext(clientId);

  // TODO: validate uids
  // TODO: validate target character name matches the one supplied in the command

  conversationContext.characterUid = invokerCharacterUid;
  // TODO: Can there be multiple people in a single conversation? Group chat?
  conversationContext.targetCharacterUid = targetCharacterUid;

  std::string targetCharacterName{};
  _serverInstance.GetDataDirector().GetCharacter(targetCharacterUid).Immutable(
    [&targetCharacterName](const data::Character& character)
    {
      targetCharacterName = character.name();
    });

  // TODO: discover response ack
  protocol::ChatCmdEnterRoomAckOk response{
    .unk1 = {
      protocol::ChatCmdEnterRoomAckOk::Struct0{
        .unk0 = invokerCharacterUid,
        .unk1 = invokerCharacterName
      },
      protocol::ChatCmdEnterRoomAckOk::Struct0{
        .unk0 = targetCharacterUid,
        .unk1 = targetCharacterName
      }
    },
  };
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
    .unk0 = conversationContext.characterUid, // TODO: this might be the target character uid
    .message = command.message};

  // Send message to invoker
  _chatterServer.QueueCommand<decltype(notify)>(clientId, [notify](){ return notify; });

  bool sent{false};
  for (const auto& [targetClientId, targetConversationContext] : _conversations)
  {
    if (targetConversationContext.characterUid == conversationContext.targetCharacterUid)
    {
      sent = true;
      _chatterServer.QueueCommand<decltype(notify)>(targetClientId, [notify](){ return notify; });
      break;
    }
  }

  // TODO: implement check if message not sent
  
  // for (const auto& [onlineClientId, onlineClientContext] : _clients)
  // {
  //   // Skip unauthenticated clients
  //   if (not onlineClientContext.isAuthenticated)
  //     continue;

  //   _chatterServer.QueueCommand<decltype(notify)>(onlineClientId, [notify](){ return notify; });
  // }
}

void PrivateChatDirector::HandleChatterInputState(
  network::ClientId clientId,
  const protocol::ChatCmdInputState& command)
{
  spdlog::debug("[{}] ChatCmdInputState: {}",
    clientId,
    command.state);
  
  // TODO: discover the purpose of this command and implement
  // observed values:
  // - 03 when closing private chat window
}

} // namespace server
