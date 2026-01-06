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

#include "server/chat/ChatDirector.hpp"

#include "server/ServerInstance.hpp"

namespace server
{

ChatDirector::ChatDirector(ServerInstance& serverInstance)
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

void ChatDirector::Initialize()
{
  spdlog::debug(
    "Chat server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _chatterServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void ChatDirector::Terminate()
{
  _chatterServer.EndHost();
}

ChatDirector::ClientContext& ChatDirector::GetClientContext(
  const network::ClientId clientId,
  bool requireAuthentication)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Chat client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Chat client is not authenticated");

  return clientContext;
}

void ChatDirector::Tick()
{
}

Config::Chat& ChatDirector::GetConfig()
{
  return _serverInstance.GetSettings().chat;
}

void ChatDirector::HandleClientConnected(network::ClientId clientId)
{
  spdlog::debug("Client {} connected to the chat server from {}",
    clientId,
    _chatterServer.GetClientAddress(clientId).to_string());
  _clients.try_emplace(clientId);
}

void ChatDirector::HandleClientDisconnected(network::ClientId clientId)
{
  spdlog::debug("Client {} disconnected from the chat server", clientId);
  _clients.erase(clientId);
}

void ChatDirector::HandleChatterEnterRoom(
  network::ClientId clientId,
  const protocol::ChatCmdEnterRoom& command)
{
  spdlog::debug("[{}] ChatCmdEnterRoom: {} {} {} {}",
    clientId,
    command.code,
    command.characterUid,
    command.characterName,
    command.guildUid);

  auto& clientContext = GetClientContext(clientId, false);
  // TODO:/FIXME: authenticate with code assigned in ChatCmdChannelInfo
  clientContext.characterUid = command.characterUid;
  clientContext.isAuthenticated = true;

  // TODO: discover response ack
  protocol::ChatCmdEnterRoomAckOk response{
    .unk1 = {
      protocol::ChatCmdEnterRoomAckOk::Struct0{
        .unk0 = 0,
        .unk1 = "All"
      },
      protocol::ChatCmdEnterRoomAckOk::Struct0{
        .unk0 = 1,
        .unk1 = "Guild"
      }
    },
  };
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void ChatDirector::HandleChatterChat(
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

  const auto& clientContext = GetClientContext(clientId);

  std::string name{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&name](const data::Character& character)
    {
      name = character.name();
    });

  // ChatCmdChatTrs did not work in any way shape or form, the handler seemed to just do nothing
  // Opted for ChatCmdChannelChatTrs for global chat
  protocol::ChatCmdChannelChatTrs notify{
    .messageAuthor = name,
    .message = command.message,
    .role = command.role};
  
  for (const auto& [onlineClientId, onlineClientContext] : _clients)
  {
    // Skip unauthenticated clients
    if (not onlineClientContext.isAuthenticated)
      continue;

    _chatterServer.QueueCommand<decltype(notify)>(onlineClientId, [notify](){ return notify; });
  }
}

void ChatDirector::HandleChatterInputState(
  network::ClientId clientId,
  const protocol::ChatCmdInputState& command)
{
  spdlog::debug("[{}] ChatCmdInputState: {}",
    clientId,
    command.state);

  // Note: might have to do with login state i.e. remember last online status (online/offline/away)
  const auto& clientContext = GetClientContext(clientId);

  // Get character's friends list
  std::set<data::Uid> friends{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&friends](const data::Character& character)
    {
      friends = character.contacts.groups().at(0).members;
    });

  // Prepare notify command
  protocol::ChatCmdInputStateTrs notify{
    .unk0 = clientContext.characterUid, // Assumed, unknown effect
    .state = command.state};

  for (const auto& [onlineClientId, onlineClientContext] : _clients)
  {
    // Skip unauthenticated clients
    if (not onlineClientContext.isAuthenticated)
      continue;

    // Notify friend
    if (friends.contains(onlineClientContext.characterUid))
      _chatterServer.QueueCommand<decltype(notify)>(onlineClientId, [notify](){ return notify; });
  }
}

} // namespace server
