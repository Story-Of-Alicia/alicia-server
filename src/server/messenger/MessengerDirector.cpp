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
  spdlog::debug("Client {} connected to the messenger server from {}",
    clientId,
    _chatterServer.GetClientAddress(clientId).to_string());
  _clients.try_emplace(clientId);
}

void MessengerDirector::HandleClientDisconnected(network::ClientId clientId)
{
  spdlog::debug("Client {} disconnected from the messenger server", clientId);

  // TODO: broadcast notify to friends & guilds that character is offline

  _clients.erase(clientId);
}

void MessengerDirector::HandleChatterLogin(
  network::ClientId clientId,
  const protocol::ChatCmdLogin& command)
{
  spdlog::debug("[{}] ChatCmdLogin: {} {} {} {}",
    clientId,
    command.characterUid,
    command.name,
    command.code,
    command.guildUid);

  auto& clientContext = _clients[clientId];

  constexpr auto OnlinePlayersCategoryUid = std::numeric_limits<uint32_t>::max() - 1;

  protocol::ChatCmdLoginAckOK response{
    .groups = {{.uid = OnlinePlayersCategoryUid, .name = "Online Players"}}};

  // TODO: verify this request in some way
  // FIXME: authentication is always assumed to be correct
  clientContext.isAuthenticated = true;

  // TODO: remember status from last login?
  clientContext.status = protocol::Status::Online;

  // Client request could be logging in as another character
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Immutable(
    [&clientContext](const data::Character& character)
    {
      clientContext.characterUid = character.uid();
    });

  response.member1 = clientContext.characterUid;

  for (const auto& userInstance : _serverInstance.GetLobbyDirector().GetUsers() | std::views::values)
  {
    const auto onlineCharacterRecord = _serverInstance.GetDataDirector().GetCharacter(
      userInstance.characterUid);

    auto& friendo = response.friends.emplace_back();
    onlineCharacterRecord.Immutable([&userInstance, &friendo](const data::Character& onlineCharacter)
    {
      friendo.name = onlineCharacter.name();
      friendo.status = onlineCharacter.isRanchLocked()
        ? protocol::Status::Offline
        : protocol::Status::Online;
      friendo.uid = onlineCharacter.uid();
      friendo.categoryUid = OnlinePlayersCategoryUid;

      // todo: get the ranch/room information
      friendo.ranchUid = onlineCharacter.uid();
      friendo.roomUid = userInstance.roomUid;
    });
  }

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterLetterList(
  network::ClientId clientId,
  const protocol::ChatCmdLetterList& command)
{
  using MailboxFolder = protocol::ChatCmdLetterList::MailboxFolder;
  spdlog::debug("[{}] ChatCmdLetterList: {} [{} {}]",
    clientId,
    command.folder == MailboxFolder::Inbox ? "Inbox" :
      command.folder == MailboxFolder::Sent ? "Sent" : "Unknown",
    command.struct0.unk0,
    command.struct0.unk1);

  protocol::ChatCmdLetterListAckOk response{};
  // Construct the response based on the mailbox in the request
  switch (command.folder)
  {
    case MailboxFolder::Sent:
    {
      // Letter list request is for sent mails
      using SentMail = protocol::ChatCmdLetterListAckOk::SentMail;
      std::vector<SentMail> sentMails{
        SentMail{
          .mailUid = 123,
          .recipient = "Recipient",
          .content = SentMail::Content{
            .date = "11:49:50 06/11/2025",
            .body = "Mail body"
          }
        }
      };

      response.mailboxFolder = MailboxFolder::Sent,
      response.struct0 = protocol::ChatCmdLetterListAckOk::Struct0{
        .mailCount = static_cast<uint32_t>(sentMails.size()),
        .unk1 = 1
      },
      response.sentMails = sentMails;
      break;
    }
    default:
    {
      spdlog::warn("[{}] ChatCmdLetterList: Unrecognised mailbox folder {}",
        clientId,
        static_cast<uint8_t>(command.folder));
      return;
    }
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
    [this, &response](const data::Guild& guild)
    {
      for (const data::Uid& guildMemberUid : guild.members())
      {
        auto& chatGuildMember = response.guildMembers.emplace_back(
          protocol::ChatCmdGuildLoginOK::GuildMember{
            .characterUid = guildMemberUid,
            .status = protocol::Status::Offline,
            .unk2 = {
              .unk0 = 0,
              .unk1 = 0
            }
          });

        // Find if the guild member is connected to the messenger server
        if (auto it = _clients.find(guildMemberUid); it != _clients.end())
        {
          // If guild member is connected, set status to the one set by the character
          chatGuildMember.status = it->second.status;
        }
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
