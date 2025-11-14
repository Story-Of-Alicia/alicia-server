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

constexpr std::string_view DateTimeFormat = "{:%H:%M:%S %d/%m/%Y} UTC";

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

MessengerDirector::ClientContext& MessengerDirector::GetClientContext(
  const network::ClientId clientId,
  bool requireAuthentication)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Messenger client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Messenger client is not authenticated");

  return clientContext;
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

  auto& clientContext = GetClientContext(clientId, false);

  // TODO: verify this request in some way
  // FIXME: authentication is always assumed to be correct
  if (false)
  {
    // Login failed, bad actor, log and return
    // Do not log with `command.name` (character name) to prevent some form of string manipulation in spdlog
    spdlog::warn("Client {} tried to login as character {} but failed authentication with auth code {}",
      clientId,
      command.characterUid,
      command.code);

    protocol::ChatCmdLoginAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::LoginFailed};

    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  clientContext.isAuthenticated = true;

  constexpr auto OnlinePlayersCategoryUid = std::numeric_limits<uint32_t>::max() - 1;

  protocol::ChatCmdLoginAckOK response{
    .groups = {{.uid = OnlinePlayersCategoryUid, .name = "Online Players"}}};

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
  bool isInboxRequested = command.mailboxFolder == protocol::MailboxFolder::Inbox;
  bool isSentRequested = command.mailboxFolder == protocol::MailboxFolder::Sent;
  spdlog::debug("[{}] ChatCmdLetterList: {} [{} {}]",
    clientId,
    isInboxRequested ? "Inbox" :
      isSentRequested ? "Sent" : "Unknown",
    command.request.lastMailUid,
    command.request.count);

  if (not isInboxRequested and not isSentRequested)
  {
    spdlog::warn("[{}] ChatCmdLetterList: requested unrecognised mailbox {}",
      clientId,
      static_cast<uint8_t>(command.mailboxFolder));

    protocol::ChatCmdLetterListAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::MailUnknownMailboxFolder};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  const auto& clientContext = GetClientContext(clientId);

  protocol::ChatCmdLetterListAckOk response{
    .mailboxFolder = command.mailboxFolder
  };

  std::string characterName{};
  bool hasMoreMail{false};
  std::vector<data::Uid> mailbox{};

  std::optional<protocol::ChatterErrorCode> errorCode{};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&command, &mailbox, &hasMoreMail, &characterName, &errorCode](const data::Character& character)
    {
      characterName = character.name();

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
          errorCode.emplace(protocol::ChatterErrorCode::MailListInvalidUid);
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

  if (errorCode.has_value())
  {
    protocol::ChatCmdLetterListAckCancel cancel{
      .errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
  }

  // Track skipped mail to subtract for final response
  uint32_t skippedMailCount{0}; 

  // Build response mailbox
  for (const data::Uid& mailUid : mailbox)
  {
    _serverInstance.GetDataDirector().GetMail(mailUid).Immutable(
      [this, &response, &skippedMailCount, folder = command.mailboxFolder](const data::Mail& mail)
      {
        // Skip soft deleted mails
        if (mail.isDeleted())
        {
          // Increment counter and return
          ++skippedMailCount;
          return;
        }

        // Get mail correspondent depending on the request
        // Mail recipient if sent mailbox or mail sender if inbox mailbox
        data::Uid correspondentUid{data::InvalidUid};
        if (folder == protocol::MailboxFolder::Sent)
          correspondentUid = mail.to();
        else if (folder == protocol::MailboxFolder::Inbox)
          correspondentUid = mail.from();

        // Get correspondent's name to render mail response
        std::string correspondentName{};
        _serverInstance.GetDataDirector().GetCharacter(correspondentUid).Immutable(
          [&correspondentName](const data::Character& character)
          {
            correspondentName = character.name();
          });

        // Format mail createdAt based on format
        const auto& createdAt = std::format(
          DateTimeFormat,
          std::chrono::floor<std::chrono::seconds>(mail.createdAt()));
        if (folder == protocol::MailboxFolder::Sent)
        {
          // Compile sent mail and add to sent mail list
          response.sentMails.emplace_back(
            protocol::ChatCmdLetterListAckOk::SentMail{
              .mailUid = mail.uid(),
              .recipient = correspondentName,
              .content = protocol::ChatCmdLetterListAckOk::SentMail::Content{
                .date = createdAt,
                .body = mail.body()
              }});
        }
        else if (folder == protocol::MailboxFolder::Inbox)
        {
          // Compile sent mail and add to sent mail list
          response.inboxMails.emplace_back(
            protocol::ChatCmdLetterListAckOk::InboxMail{
              .uid = mail.uid(),
              .type = mail.type(),
              .origin = mail.origin(),
              .sender = correspondentName,
              .date = createdAt,
              .struct0 = protocol::ChatCmdLetterListAckOk::InboxMail::Struct0{
                .body = mail.body()
              }
            });
        }
      });
  }

  // `mailbox` size here directly correlates with the loop that processes it 
  // The client is to not be made aware of any skipped mails, adjust mail count
  response.mailboxInfo = protocol::ChatCmdLetterListAckOk::MailboxInfo{
    .mailCount = static_cast<uint32_t>(mailbox.size() - skippedMailCount),
    .hasMoreMail = hasMoreMail
  };

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterLetterSend(
  network::ClientId clientId,
  const protocol::ChatCmdLetterSend& command)
{
  spdlog::debug("[{}] ChatCmdLetterSend: {} [{}]",
    clientId,
    command.recipient,
    command.body);
  
  const data::Uid& recipientCharacterUid = 
    _serverInstance.GetDataDirector().GetDataSource().IsCharacterNameUnique(command.recipient);

  if (recipientCharacterUid == data::InvalidUid)
  {
    // Character tried to send mail to a character that doesn't exist, no need to log
    protocol::ChatCmdLetterSendAckCancel cancel{
      .errorCode = protocol::ChatterErrorCode::CharacterDoesNotExist
    };

    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // TODO: enforce any character limit?
  // TODO: bad word checks and/or deny sending the letter as a result?

  const auto& clientContext = GetClientContext(clientId);

  std::string senderName{};
  data::Uid senderUid{data::InvalidUid};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&senderUid, &senderName](const data::Character& character)
    {
      senderUid = character.uid();
      senderName = character.name();
    });

  // UTC now in seconds
  const auto& utcNow = std::chrono::floor<std::chrono::seconds>(util::Clock::now());
  const auto& formattedDt = std::format(DateTimeFormat, utcNow);

  // Create and store mail
  data::Uid mailUid{data::InvalidUid};
  auto mailRecord = _serverInstance.GetDataDirector().CreateMail();
  mailRecord.Mutable([&mailUid, &command, &utcNow, &senderUid, &recipientCharacterUid](data::Mail& mail)
  {
    // Set mail parameters
    mail.from() = senderUid;
    mail.to() = recipientCharacterUid;

    mail.type() = data::Mail::MailType::CanReply;
    mail.origin() = data::Mail::MailOrigin::Character;

    mail.createdAt() = utcNow;
    mail.body() = command.body;

    // Get mailUid to store in character record
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

  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });

  // Check if recipient is online for live mail delivery
  auto client = std::ranges::find_if(
    _clients,
    [&recipientCharacterUid](const std::pair<network::ClientId, ClientContext>& client)
    {
      return client.second.characterUid == recipientCharacterUid;
    });

  if (client == _clients.cend())
    // Character is not online, all good and handled
    return;

  protocol::ChatCmdLetterArriveTrs notify{
    .mailUid = mailUid,
    .mailType = data::Mail::MailType::CanReply,
    .mailOrigin = data::Mail::MailOrigin::Character,
    .sender = senderName,
    .date = formattedDt,
    .body = command.body
  };

  const auto& recipientClientId = client->first;
  _chatterServer.QueueCommand<decltype(notify)>(recipientClientId, [notify](){ return notify; });
}

void MessengerDirector::HandleChatterLetterRead(
  network::ClientId clientId,
  const protocol::ChatCmdLetterRead& command)
{
  spdlog::debug("[{}] ChatCmdLetterRead: {} {}",
    clientId,
    command.unk0,
    command.mailUid);

  const auto& clientContext = GetClientContext(clientId);

  // Confirm if the mail even exists
  const auto& mailRecord = _serverInstance.GetDataDirector().GetMail(command.mailUid);

  std::optional<protocol::ChatterErrorCode> errorCode{};
  if (command.mailUid == data::InvalidUid)
  {
    // Character tried to request an invalid mail
    spdlog::warn("Character {} tried to request an invalid mail",
      clientContext.characterUid);
    errorCode.emplace(protocol::ChatterErrorCode::MailInvalidUid);
  }
  else if (not mailRecord.IsAvailable())
  {
    // Mail does not exist or is not available
    spdlog::warn("Character {} tried to request a mail {} that does not exist or is not available",
      clientContext.characterUid,
      command.mailUid);
    errorCode.emplace(protocol::ChatterErrorCode::MailDoesNotExistOrNotAvailable);
  }

  // If we haven't encountered any errors so far, check the mail ownership
  if (not errorCode.has_value())
  {
    // Confirm the character has such mail in its mailbox
    _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
      [&command, &errorCode](const data::Character& character)
      {
        // Check if character has this mail in their `inbox`
        const bool& hasMail = std::ranges::contains(
          character.mailbox.inbox(),
          command.mailUid);

        if (not hasMail)
        {
          // Character does not own this mail
          errorCode.emplace(protocol::ChatterErrorCode::MailDoesNotBelongToCharacter);
        }
      });
  }

  // If an error occurred along the way, respond with cancel and return
  if (errorCode.has_value())
  {
    protocol::ChatCmdLetterReadAckCancel cancel{.errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Mark letter as read
  mailRecord.Mutable([](data::Mail& mail)
  {
    mail.isRead() = true;
  });

  protocol::ChatCmdLetterReadAckOk response{
    .unk0 = command.unk0,
    .mailUid = command.mailUid
  };
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterLetterDelete(
  network::ClientId clientId,
  const protocol::ChatCmdLetterDelete& command)
{
  bool isRequestSent = command.folder == protocol::MailboxFolder::Sent;
  bool isRequestInbox = command.folder == protocol::MailboxFolder::Inbox;
  spdlog::debug("[{}] ChatCmdLetterDelete: {} {}",
    clientId,
    isRequestSent ? "Sent" :
      isRequestInbox ? "Inbox" : "Unknown",
    command.mailUid);

  const auto& clientContext = GetClientContext(clientId);
  
  Record<data::Mail> mailRecord{};
  std::optional<protocol::ChatterErrorCode> errorCode{};
  if (not isRequestSent and not isRequestInbox)
  {
    // Mailbox unrecognised
    spdlog::warn("Character {} tried to delete a mail from unrecognised mailbox {}",
      clientContext.characterUid,
      static_cast<uint8_t>(command.folder));
    errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteUnknownMailboxFolder);
  }
  else if (not (mailRecord = _serverInstance.GetDataDirector().GetMail(command.mailUid)).IsAvailable())
  {
    // Mail is not available
    spdlog::warn("Character {} tried to delete mail {} which is currently not available",
      clientContext.characterUid,
      command.mailUid);
    errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteUnknownMailboxFolder);
  }

  if (not errorCode.has_value())
  {
    // No errors yet, do ownership check
    _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
      [this, &command, &errorCode](data::Character& character)
      {
        switch (command.folder)
        {
          case protocol::MailboxFolder::Sent:
          {
            // Sent mailbox
            // Find mail
            const auto& iter = std::ranges::find(character.mailbox.sent(), command.mailUid);
            // Check if character owns this mail
            if (iter == character.mailbox.sent().cend())
            {
              errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteMailDoesNotBelongToCharacter);
              return;
            }
            break;
          }
          case protocol::MailboxFolder::Inbox:
          {
            // Inbox mailbox
            // Find mail
            const auto& iter = std::ranges::find(character.mailbox.inbox(), command.mailUid);
            // Check if character owns this mail
            if (iter == character.mailbox.inbox().cend())
            {
              errorCode.emplace(protocol::ChatterErrorCode::LetterDeleteMailDoesNotBelongToCharacter);
              return;
            }
            break;
          }
          default:
          {
            throw std::runtime_error(
              std::format(
                "Unrecognised mailbox {}",
                static_cast<uint8_t>(command.folder)));
          }
        }
      });
  }

  if (errorCode.has_value())
  {
    if (errorCode.value() == protocol::ChatterErrorCode::LetterDeleteMailDoesNotBelongToCharacter)
      spdlog::debug("Character {} tried to delete mail {} which they do not own from {} mailbox",
        clientContext.characterUid,
        command.mailUid,
        command.folder == protocol::MailboxFolder::Sent ? "Sent" :
          command.folder == protocol::MailboxFolder::Inbox ? "Inbox" :
          throw std::runtime_error(
            std::format(
              "Unrecognised mailbox {}",
              static_cast<uint8_t>(command.folder))));

    protocol::ChatCmdLetterDeleteAckCancel cancel{.errorCode = errorCode.value()};
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Mail exists and character owns this mail, soft delete
  _serverInstance.GetDataDirector().GetMail(command.mailUid).Mutable(
    [](data::Mail& mail)
    {
      mail.isDeleted() = true;
    });
  
  protocol::ChatCmdLetterDeleteAckOk response{
    .folder = command.folder,
    .mailUid = command.mailUid
  };
  _chatterServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void MessengerDirector::HandleChatterGuildLogin(
  network::ClientId clientId,
  const protocol::ChatCmdGuildLogin& command)
{
  // ChatCmdGuildLogin is sent after ChatCmdLogin
  // Assumption: the user is very likely already authenticated with messenger
  const auto& clientContext = GetClientContext(clientId);

  // Check if client belongs to the guild in the command
  data::Uid characterGuildUid{data::InvalidUid};
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&characterGuildUid](const data::Character& character)
    {
      characterGuildUid = character.guildUid();
    });

  std::optional<protocol::ChatterErrorCode> errorCode{};
  if (not clientContext.isAuthenticated)
  {
    // Client is not authenticated with chatter server
    spdlog::warn("Client {} tried to login to guild {} but is not authenticated with the chatter server.",
      clientId,
      command.guildUid);
    errorCode.emplace(protocol::ChatterErrorCode::GuildLoginClientNotAuthenticated);
  }
  else if (command.characterUid != clientContext.characterUid)
  {
    // Command `characterUid` does match the client context `characterUid 
    spdlog::warn("Client {} tried to login, who is character {}, to guild {} on behalf of another character {}",
      clientId,
      clientContext.characterUid,
      command.guildUid,
      command.characterUid);
    errorCode.emplace(protocol::ChatterErrorCode::CommandCharacterIsNotClientCharacter);
  }
  else if (characterGuildUid != command.guildUid)
  {
    // Character does not belong to the guild in the guild login
    spdlog::warn("Character {} tried to login to guild {} but character is not a guild member.",
      clientContext.characterUid,
      command.guildUid);
    errorCode.emplace(protocol::ChatterErrorCode::GuildLoginCharacterNotGuildMember);
  }

  if (errorCode.has_value())
  {
    // Some error has been encountered, respond with cancel and return
    protocol::ChatCmdGuildLoginAckCancel cancel{
      .errorCode = errorCode.value()
    };
    _chatterServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::ChatCmdGuildLoginAckOK response{};
  _serverInstance.GetDataDirector().GetGuild(command.guildUid).Immutable(
    [this, &response](const data::Guild& guild)
    {
      for (const data::Uid& guildMemberUid : guild.members())
      {
        auto& chatGuildMember = response.guildMembers.emplace_back(
          protocol::ChatCmdGuildLoginAckOK::GuildMember{
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
