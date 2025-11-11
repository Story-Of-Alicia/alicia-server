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

#ifndef CHATTERMESSAGEDEFINITIONS_HPP
#define CHATTERMESSAGEDEFINITIONS_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/chatter/ChatterProtocol.hpp>
#include <libserver/util/Stream.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace server::protocol
{

enum class Status : uint8_t
{
  Hidden = 0,
  Offline = 1,
  Online = 2,
  Away = 3
};

enum class MailboxFolder : uint8_t
{
  Sent = 1,
  Inbox = 2
};

//! Custom server defined error codes. Does not correspond with any value on the client.
//! The client displays this as "Server Error (code: x)"
enum class ChatterErrorCode : uint32_t
{
  LoginFailed = 1,
  CommandCharacterIsNotClientCharacter = 2,
  CharacterDoesNotExist = 3,
  GuildLoginClientNotAuthenticated = 4,
  GuildLoginCharacterNotGuildMember = 5,
  MailInvalidUid = 6,
  MailDoesNotExistOrNotAvailable = 7,
  MailDoesNotBelongToCharacter = 8
};

struct ChatCmdLogin
{
  uint32_t characterUid{};
  std::string name{};
  uint32_t code{};
  uint32_t guildUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLogin;
  }

  static void Write(
    const ChatCmdLogin& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLogin& command,
    SourceStream& stream);
};

struct ChatCmdLoginAckOK
{
  uint32_t member1 = 0;

  struct MailAlarm
  {
    enum class Status : uint32_t
    {
      NoNewMail = 0,
      NewMail = 1
    } status{Status::NoNewMail};
    uint8_t hasMail{};
  } mailAlarm;

  struct Group
  {
    uint32_t uid{};
    std::string name{};
  };
  std::vector<Group> groups;

  struct Friend
  {
    uint32_t uid{};
    uint32_t categoryUid{};
    std::string name{};

    Status status = Status::Offline;

    // 2 - friend request popup
    uint8_t member5{};
    uint32_t roomUid{};
    uint32_t ranchUid{};
  };
  std::vector<Friend> friends;

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckOK;
  }

  static void Write(
    const ChatCmdLoginAckOK& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLoginAckOK& command,
    SourceStream& stream);
};

struct ChatCmdLoginAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckCancel;
  }

  static void Write(
    const ChatCmdLoginAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLoginAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterList
{
  MailboxFolder mailboxFolder{};

  // Likely to do with mailbox pagination
  struct Request
  {
    //! The UID of the last mail in the character mailbox.
    uint32_t lastMailUid{};
    //! Requested mail count to read.
    uint32_t count{};

    static void Write(
      const Request& command,
      SinkStream& stream);

    static void Read(
      Request& command,
      SourceStream& stream);
  } request{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterList;
  }

  static void Write(
    const ChatCmdLetterList& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterList& command,
    SourceStream& stream);
};

struct ChatCmdLetterListAckOk
{
  MailboxFolder mailboxFolder{};

  struct MailboxInfo
  {
    //! Mail count.
    uint32_t mailCount{};
    //! Indicates whether there are more mail in mailbox.
    //! `0` disables the "Show 10 more..." button.
    uint8_t hasMoreMail{};
  } mailboxInfo{};

  struct InboxMail
  {
    //! Mail UID.
    data::Uid uid{};
    data::Mail::MailType type{};
    data::Mail::MailOrigin origin{};

    //! Who sent the mail.
    std::string sender{};
    //! Date of the mail when it was sent, as a string.
    std::string date{};

    struct Struct0
    {
      //! Unknown, left for discovery later.
      std::string unk0{"struct0.unk0"};
      //! Mail body.
      std::string body{};
    } struct0{};
  };
  std::vector<InboxMail> inboxMails{};

  struct SentMail
  {
    data::Uid mailUid{};
    std::string recipient{};
    struct Content
    {
      std::string date{};
      std::string body{};
    } content{};
  };
  std::vector<SentMail> sentMails{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterListAckOk;
  }

  static void Write(
    const ChatCmdLetterListAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterListAckOk& command,
    SourceStream& stream);
};

struct ChatCmdLetterSend
{
  std::string recipient{};
  std::string body{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterSend;
  }

  static void Write(
    const ChatCmdLetterSend& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterSend& command,
    SourceStream& stream);
};

struct ChatCmdLetterSendAckOk
{
  // TODO: confirm if this is truly mailUid or relative index of 0 -> x
  data::Uid mailUid{};
  //! Recipient name.
  std::string recipient{};
  //! Client takes anything, typically "hh:mm:ss DD/MM/YYYY".
  std::string date{};
  //! Mail body.
  std::string body{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterSendAckOk;
  }

  static void Write(
    const ChatCmdLetterSendAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterSendAckOk& command,
    SourceStream& stream);
};

struct ChatCmdLetterSendAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterSendAckCancel;
  }

  static void Write(
    const ChatCmdLetterSendAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterSendAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterRead
{
  // Very possibly mailbox folder
  // Typically `2` (inbox?)
  uint8_t unk0{};
  data::Uid mailUid{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterRead;
  }

  static void Write(
    const ChatCmdLetterRead& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterRead& command,
    SourceStream& stream);
};

struct ChatCmdLetterReadAckOk
{
  //! Very possibly mailbox folder
  // Typically `2` (inbox?)
  uint8_t unk0{};

  //! UID of the mail being requested.
  data::Uid mailUid{};
  std::string unk2{"ChatCmdLetterReadAckOk.unk2"};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterReadAckOk;
  }

  static void Write(
    const ChatCmdLetterReadAckOk& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterReadAckOk& command,
    SourceStream& stream);
};

struct ChatCmdLetterReadAckCancel
{
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterReadAckCancel;
  }

  static void Write(
    const ChatCmdLetterReadAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterReadAckCancel& command,
    SourceStream& stream);
};

struct ChatCmdLetterArriveTrs
{
  // Note: almost identical to InboxMail with the extra std::string missing
  data::Uid mailUid{};
  data::Mail::MailType mailType{};
  data::Mail::MailOrigin mailOrigin{};

  std::string sender{};
  std::string date{};
  std::string body{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLetterArriveTrs;
  }

  static void Write(
    const ChatCmdLetterArriveTrs& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLetterArriveTrs& command,
    SourceStream& stream);
};

struct ChatCmdGuildLogin : ChatCmdLogin
{
  // ChatCmdGuildLogin shares the same payload as ChatCmdLogin

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGuildLogin;
  }

  static void Write(
    const ChatCmdGuildLogin& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGuildLogin& command,
    SourceStream& stream);
};

struct ChatCmdGuildLoginAckOK
{
  struct GuildMember
  {
    //! Character UID of the guild member. 
    data::Uid characterUid{};
    //! Online status of the guild member.
    //! `Status::Hidden` completely removes the status of that member.
    Status status{Status::Hidden};

    struct Struct2
    {
      uint32_t unk0{};
      uint32_t unk1{};

      static void Write(
        const Struct2& command,
        SinkStream& stream);

      static void Read(
        Struct2& command,
        SourceStream& stream);
    } unk2{};

    static void Write(
      const GuildMember& command,
      SinkStream& stream);

    static void Read(
      GuildMember& command,
      SourceStream& stream);
  };
  std::vector<GuildMember> guildMembers{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGuildLoginAckOK;
  }

  static void Write(
    const ChatCmdGuildLoginAckOK& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGuildLoginAckOK& command,
    SourceStream& stream);
};

struct ChatCmdGuildLoginAckCancel
{
  //! Custom error code.
  ChatterErrorCode errorCode{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdGuildLoginAckCancel;
  }

  static void Write(
    const ChatCmdGuildLoginAckCancel& command,
    SinkStream& stream);

  static void Read(
    ChatCmdGuildLoginAckCancel& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // CHATTERMESSAGEDEFINITIONS_HPP
