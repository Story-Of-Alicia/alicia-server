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

#include "server/messenger/MessengerDirector.hpp"

#include "server/ServerInstance.hpp"

namespace server
{

MessengerDirector::MessengerDirector(ServerInstance& serverInstance)
  : _chatterServer(*this, *this)
  , _serverInstance(serverInstance)
{
}

void MessengerDirector::Initialize()
{
  spdlog::debug(
    "Messenger server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _chatterServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void MessengerDirector::Terminate()
{
  _chatterServer.EndHost();
}

void MessengerDirector::Tick()
{
}

Config::Messenger& MessengerDirector::GetConfig()
{
  return _serverInstance.GetSettings().messenger;
}

void MessengerDirector::HandleClientConnected(network::ClientId clientId)
{
}

void MessengerDirector::HandleClientDisconnected(network::ClientId clientId)
{
}

void MessengerDirector::HandleChatterLogin(
  network::ClientId clientId,
  const protocol::ChatCmdLogin& command)
{
  constexpr auto OnlinePlayersCategoryUid = std::numeric_limits<uint32_t>::max() - 1;

  protocol::ChatCmdLoginAckOK response{
    .groups = {{.uid = OnlinePlayersCategoryUid, .name = "Online Players"}}};

  // TODO: verify this request in some way
  // Client request could be logging in as another character
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Immutable(
    [&response](const data::Character& character)
    {
      response.member1 = character.uid();
    });

  for (const auto& userInstance : _serverInstance.GetLobbyDirector().GetUsers() | std::views::values)
  {
    const auto onlineCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
      userInstance.characterUid);

    auto& friendo = response.friends.emplace_back();
    onlineCharacterRecord.Immutable([&userInstance, &friendo](const data::Character& onlineCharacter)
    {
      friendo.name = onlineCharacter.name();
      friendo.status = onlineCharacter.isRanchLocked()
        ? protocol::ChatCmdLoginAckOK::Friend::Status::Offline
        : protocol::ChatCmdLoginAckOK::Friend::Status::Online;
      friendo.uid = onlineCharacter.uid();
      friendo.categoryUid = OnlinePlayersCategoryUid;

      // todo: get the ranch/room information
      friendo.ranchUid = onlineCharacter.uid();
      friendo.roomUid = userInstance.roomUid;
    });
  }

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterGuildLogin(
  network::ClientId clientId,
  const protocol::ChatCmdGuildLogin& command)
{
  protocol::ChatCmdGuildLoginOK response{};

  // TODO: check if calling client character uid is command.characterUid
  // TODO: check if command.guildUid is the guildUid the calling character is in

  // TODO: guild record is retrieved directly from command, verify this
  _serverInstance.GetDataDirector().GetGuild(command.guildUid).Immutable(
    [&response](const data::Guild& guild)
    {
      for (const data::Uid& member : guild.members())
      {
        response.guildMembers.emplace_back(
          protocol::ChatCmdGuildLoginOK::GuildMember{
            .characterUid = member,
            .status = protocol::ChatCmdGuildLoginOK::GuildMember::Status::Online, // TODO: implement online status
            .unk2 = {
              .unk0 = 0,
              .unk1 = 0
            }
          });
      }
    });

  _chatterServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

} // namespace server
