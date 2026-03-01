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

#include "server/chat/AllChatDirector.hpp"

#include "server/ServerInstance.hpp"

#include <boost/container_hash/hash.hpp>

namespace server
{

AllChatDirector::AllChatDirector(ServerInstance& serverInstance)
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

void AllChatDirector::Initialize()
{
  spdlog::debug(
    "All chat server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _chatterServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void AllChatDirector::Terminate()
{
  _chatterServer.EndHost();
}

AllChatDirector::ClientContext& AllChatDirector::GetClientContext(
  const network::ClientId clientId,
  bool requireAuthentication)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("All chat client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("All chat client is not authenticated");

  return clientContext;
}

void AllChatDirector::Tick()
{
}

Config::AllChat& AllChatDirector::GetConfig()
{
  return _serverInstance.GetSettings().allChat;
}

void AllChatDirector::HandleClientConnected(network::ClientId clientId)
{
  spdlog::debug("Client {} connected to the all chat server from {}",
    clientId,
    _chatterServer.GetClientAddress(clientId).to_string());
  _clients.try_emplace(clientId);
}

void AllChatDirector::HandleClientDisconnected(network::ClientId clientId)
{
  spdlog::debug("Client {} disconnected from the all chat server", clientId);
  _clients.erase(clientId);
}

void AllChatDirector::HandleChatterEnterRoom(
  network::ClientId clientId,
  const protocol::ChatCmdEnterRoom& command)
{
  auto& clientContext = GetClientContext(clientId, false);

  // Generate identity hash based on the character uid from the command and
  // the chat otp constant
  size_t identityHash = std::hash<uint32_t>()(command.characterUid);
  boost::hash_combine(identityHash, AllChatOtpConstant);

  // Authorise the code received in the command against the calculated identity hash
  clientContext.isAuthenticated = _serverInstance.GetOtpSystem().AuthorizeCode(
    identityHash,
    command.code);

  if (not clientContext.isAuthenticated)
  {
    // Client failed chat authentication
    // Do not log with `command.name` (character name) to prevent some form of string manipulation in spdlog
    spdlog::warn("Client '{}' tried to login to all chat as character '{}' but failed authentication with auth code '{}'",
      clientId,
      command.characterUid,
      command.code);

    protocol::ChatCmdEnterRoomAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::ChatLoginFailed};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });

    // TODO: confirm the cancel command is sent before disconnecting the client
    _chatterServer.DisconnectClient(clientId);
    return;
  }

  // Sets client context character uid to the one provided by the client
  // This value is assured by the server to be correct (if it passes authentication) as
  // the server hashes the character uid and then the director's otp constant to compute the code.
  clientContext.characterUid = command.characterUid;

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

void AllChatDirector::HandleChatterChat(
  network::ClientId clientId,
  const protocol::ChatCmdChat& command)
{
  const auto& clientContext = GetClientContext(clientId);

  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  std::string characterName{};
  bool isGameMaster = false;
  characterRecord.Immutable(
    [&characterName, &isGameMaster](const data::Character& character)
    {
      characterName = character.name();
      isGameMaster = character.role() == data::Character::Role::GameMaster;
    });

  const auto userName = _serverInstance.GetLobbyDirector().GetUserByCharacterUid(
    clientContext.characterUid).userName;

  spdlog::info("[Global] {} ({}): {}",
    characterName,
    userName,
    command.message);

  const auto verdict = _serverInstance.GetChatSystem().ProcessChatMessage(
    clientContext.characterUid, command.message);

  if (verdict.commandVerdict)
  {
    for (const auto& response : verdict.commandVerdict->result)
    {
      _chatterServer.QueueCommand<protocol::ChatCmdChannelChatTrs>(clientId, [response]()
      {
        return protocol::ChatCmdChannelChatTrs{
          .messageAuthor = "",
          .message = response,
          .role = protocol::ChatCmdChat::Role::GameMaster};
      });
    }

    return;
  }

  if (verdict.isMuted)
  {
    _chatterServer.QueueCommand<protocol::ChatCmdChannelChatTrs>(clientId, [verdict]()
    {
      return protocol::ChatCmdChannelChatTrs{
        .messageAuthor = "",
        .message = verdict.message,
        .role = protocol::ChatCmdChat::Role::GameMaster};
    });

    return;
  }

  // ChatCmdChatTrs did not work in any way shape or form, the handler seemed to just do nothing
  // Opted for ChatCmdChannelChatTrs for global chat
  protocol::ChatCmdChannelChatTrs notify{
    .messageAuthor = isGameMaster
      ? std::format("[GM] {}", characterName)
      : characterName,
    .message = command.message,
    .role = protocol::ChatCmdChat::Role::User};
  
  for (const auto& [onlineClientId, onlineClientContext] : _clients)
  {
    // Skip unauthenticated clients
    if (not onlineClientContext.isAuthenticated)
      continue;

    _chatterServer.QueueCommand<decltype(notify)>(onlineClientId, [notify]()
    {
      return notify;
    });
  }
}

void AllChatDirector::HandleChatterInputState(
  network::ClientId clientId,
  const protocol::ChatCmdInputState& command)
{
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
