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

constexpr std::string_view DateTimeFormat = "{:%H:%M:%S %d/%m/%Y}";

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
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Mutable(
    [&clientContext](data::Character& character)
    {
      clientContext.characterUid = character.uid();

      // TODO: implement unread mail mechanics

      character.mailbox.hasNewMail() = false;
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
  spdlog::debug("[{}] ChatCmdLetterList: {} [{} {}]",
    clientId,
    command.mailboxFolder == protocol::MailboxFolder::Inbox ? "Inbox" :
      command.mailboxFolder == protocol::MailboxFolder::Sent ? "Sent" : "Unknown",
    command.request.lastMailUid,
    command.request.count);

  const auto& clientContext = _clients[clientId];

  protocol::ChatCmdLetterListAckOk response{
    .mailboxFolder = command.mailboxFolder
  };

  bool hasMoreMail{false};
  std::vector<data::Uid> mailbox{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&command, &mailbox, &hasMoreMail](const data::Character& character)
    {
      // Get the mailbox based on the command request
      std::vector<data::Uid> _mailbox{};
      if (command.mailboxFolder == protocol::MailboxFolder::Inbox)
        _mailbox = character.mailbox.inbox();
      else if (command.mailboxFolder == protocol::MailboxFolder::Sent)
        _mailbox = character.mailbox.sent();
      else
        throw std::runtime_error("Unrecognised mailbox folder.");

      // Start from the beginning of the mailbox, or from specific mailUid as per request
      auto startIter = _mailbox.begin();
      if (command.request.lastMailUid != data::InvalidUid)
      {
        startIter = std::ranges::find(_mailbox, command.request.lastMailUid);

        // Safety mechanism, just in case no mail by that UID was found
        if (startIter == _mailbox.cend())
        {
          spdlog::warn("Character {} tried to request mail after mail {} but that mail does not exist.",
            character.uid(),
            command.request.lastMailUid);
          hasMoreMail = false;
          // TODO: respond with cancel
          return;
        }
      }

      // Get remaining items left in the array, from the mailUid (or beginning)
      const auto& remaining = std::distance(
        startIter,
        _mailbox.end());

      // Copy n amounts of mail as per request
      const auto& res = std::ranges::copy_n(
        startIter,
        std::min<size_t>(
          command.request.count,
          remaining),
        std::back_inserter(mailbox));

      // Indicate that there are more mail after the current ending of response mail
      hasMoreMail = res.in != _mailbox.cend();
    });

  // Construct the response based on the mailbox in the request
  switch (command.mailboxFolder)
  {
    case protocol::MailboxFolder::Sent:
    {
      // Letter list request is for sent mails
      using SentMail = protocol::ChatCmdLetterListAckOk::SentMail;

      for (const auto& sentMailUid : mailbox)
      {
        _serverInstance.GetDataDirector().GetMail(sentMailUid).Immutable(
          [&response](const data::Mail& mail)
          {
            response.sentMails.emplace_back(SentMail{
              .mailUid = mail.uid(),
              .recipient = mail.name(),
              .content = SentMail::Content{
                .date = mail.date(),
                .body = mail.body()
              }
            });
          });
      }
      break;
    }
    case protocol::MailboxFolder::Inbox:
    {
      using InboxMail = protocol::ChatCmdLetterListAckOk::InboxMail;

      for (const auto& inboxMailUid : mailbox)
      {
        _serverInstance.GetDataDirector().GetMail(inboxMailUid).Immutable(
          [&response](const data::Mail& mail)
          {
            response.inboxMails.emplace_back(InboxMail{
              InboxMail{
                .mailUid = mail.uid(),
                .mailType = InboxMail::MailType::CanReply, // TODO: store in mail
                .mailOrigin = InboxMail::MailOrigin::Character, // TODO: store in mail
                .sender = mail.name(),
                .date = mail.date(),
                .struct0 = InboxMail::Struct0{
                  .body = mail.body()
                }
              }
            });
          });
      }
      break;
    }
    default:
    {
      spdlog::warn("[{}] ChatCmdLetterList: Unrecognised mailbox folder {}",
        clientId,
        static_cast<uint8_t>(command.mailboxFolder));
      return;
    }
  }

  // `mailbox` size here directly correlates with the loop that processes it 
  response.mailboxInfo = protocol::ChatCmdLetterListAckOk::MailboxInfo{
    .mailCount = static_cast<uint32_t>(mailbox.size()),
    .hasMoreMail = hasMoreMail
  };

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterLetterSend(
  network::ClientId clientId,
  const protocol::ChatCmdLetterSend& command)
{
  spdlog::debug("[{}] ChatCmdLetterSend: {} {}",
    clientId,
    command.recipient,
    command.body);
  
  const data::Uid& recipientCharacterUid = 
    _serverInstance.GetDataDirector().GetDataSource().IsCharacterNameUnique(command.recipient);

  if (recipientCharacterUid == data::InvalidUid)
  {
    // Character tried to send mail to a character that doesn't exist, no need to log
    // TODO: character does not exist at all, respond with cancel
    return;
  }

  // UTC now in seconds
  const auto& utcNow = std::chrono::floor<std::chrono::seconds>(util::Clock::now());
  const auto& formattedDt = std::format(DateTimeFormat, utcNow);

  // Create and store mail
  data::Uid mailUid{data::InvalidUid};
  auto mailRecord = _serverInstance.GetDataDirector().CreateMail();
  mailRecord.Mutable([&mailUid, &command, &formattedDt](data::Mail& mail)
  {
    mail.name = command.recipient;
    mail.date = formattedDt;
    mail.body = command.body;

    // Set mailUid to store in character record
    mailUid = mail.uid();
  });

  // Add the new mail to the recipient's inbox
  _serverInstance.GetDataDirector().GetCharacter(recipientCharacterUid).Mutable(
    [&mailUid](data::Character& character)
    {
      // Insert new mail to the beginning of the list
      // TODO: this operation is O(n), does dao support std::deque?
      character.mailbox.inbox().insert(
        character.mailbox.inbox().begin(),
        mailUid);

      // Set mail alarm
      character.mailbox.hasNewMail() = true;
    });

  const auto& clientContext = _clients[clientId];

  // Add the new mail to the sender's sent mailbox
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&mailUid](data::Character& character)
    {
      // Insert new mail to the beginning of the list
      // TODO: this operation is O(n), does dao support std::deque?
      character.mailbox.sent().insert(
        character.mailbox.sent().begin(),
        mailUid);
    });

  protocol::ChatCmdLetterSendAckOk response{
    .mailUid = mailUid,
    .recipient = command.recipient,
    .date = formattedDt,
    .body = command.body
  };

  _chatterServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // TODO: alert recipient of new mail if they are online
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
