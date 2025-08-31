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

  for (data::Uid onlineCharacterUid : _serverInstance.GetRanchDirector().GetOnlineCharacters())
  {
    const auto onlineCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
      onlineCharacterUid);

    auto& friendo = response.friends.emplace_back();
    onlineCharacterRecord.Immutable([&friendo](const data::Character& onlineCharacter)
    {
      friendo.name = onlineCharacter.name();
      friendo.status = onlineCharacter.isRanchLocked()
        ? protocol::ChatCmdLoginAckOK::Friend::Status::Offline
        : protocol::ChatCmdLoginAckOK::Friend::Status::Online;
      friendo.uid = onlineCharacter.uid();
      friendo.categoryUid = OnlinePlayersCategoryUid;

      // todo: get the ranch/room information
      friendo.ranchUid = onlineCharacter.uid();
    });
  }

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

} // namespace server
