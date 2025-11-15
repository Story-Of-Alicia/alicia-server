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

#include "libserver/network/chatter/proto/ChatterMessageDefinitions.hpp"

#include <stdexcept>

void server::protocol::ChatCmdLogin::Write(
  const ChatCmdLogin&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLogin::Read(
  ChatCmdLogin& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.name)
    .Read(command.code)
    .Read(command.guildUid);
}

void server::protocol::ChatCmdLoginAckOK::Write(
  const ChatCmdLoginAckOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1);

  stream.Write(command.mailAlarm.status)
    .Write(command.mailAlarm.hasMail);

  stream.Write(static_cast<uint32_t>(command.groups.size()));
  for (auto& group : command.groups)
  {
    stream.Write(group.uid)
      .Write(group.name);
  }

  stream.Write(static_cast<uint32_t>(command.friends.size()));
  for (const auto& fr : command.friends)
  {
    stream.Write(fr.uid)
      .Write(fr.categoryUid)
      .Write(fr.name)
      .Write(fr.status)
      .Write(fr.member5)
      .Write(fr.roomUid)
      .Write(fr.ranchUid);
  }
}

void server::protocol::ChatCmdLoginAckOK::Read(
  ChatCmdLoginAckOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLoginAckCancel::Write(
  const ChatCmdLoginAckCancel&,
  SinkStream&)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdLoginAckCancel::Read(
  ChatCmdLoginAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterList::Request::Write(
  const ChatCmdLetterList::Request& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterList::Request::Read(
  ChatCmdLetterList::Request& command,
  server::SourceStream& stream)
{
  stream.Read(command.lastMailUid)
    .Read(command.count);
}

void server::protocol::ChatCmdLetterList::Write(
  const ChatCmdLetterList& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterList::Read(
  ChatCmdLetterList& command,
  server::SourceStream& stream)
{
  stream.Read(command.mailboxFolder)
    .Read(command.request);
}

void server::protocol::ChatCmdLetterListAckOk::Write(
  const ChatCmdLetterListAckOk& command,
  server::SinkStream& stream)
{
  stream.Write(command.mailboxFolder);
  // TODO: break this out into it's own struct write function
  stream.Write(command.mailboxInfo.mailCount)
    .Write(command.mailboxInfo.hasMoreMail);

  switch (command.mailboxFolder)
  {
    case MailboxFolder::Sent:
    {
      for (const auto& sentMail : command.sentMails)
      {
        // TODO: break this out into it's own struct write function
        stream.Write(sentMail.mailUid)
          .Write(sentMail.recipient);
        // TODO: break this out into it's own struct write function
        stream.Write(sentMail.content.date)
          .Write(sentMail.content.body);
      }
      break;
    }
    case MailboxFolder::Inbox:
    {
      for (const auto& mail : command.inboxMails)
      {
        stream.Write(mail.uid)
          .Write(mail.type)
          .Write(mail.origin)
          .Write(mail.sender)
          .Write(mail.date);

        // TODO: break this out into it's own struct write function
        stream.Write(mail.struct0.unk0)
          .Write(mail.struct0.body);
      }
      break;
    }
    default:
    {
      throw std::runtime_error("Not implemented");
    }
  }
}

void server::protocol::ChatCmdLetterListAckOk::Read(
  ChatCmdLetterListAckOk& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterListAckCancel::Write(
  const ChatCmdLetterListAckCancel& command,
  server::SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdLetterListAckCancel::Read(
  ChatCmdLetterListAckCancel& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterSend::Write(
  const ChatCmdLetterSend& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterSend::Read(
  ChatCmdLetterSend& command,
  server::SourceStream& stream)
{
  stream.Read(command.recipient)
    .Read(command.body);
}

void server::protocol::ChatCmdLetterSendAckOk::Write(
  const ChatCmdLetterSendAckOk& command,
  server::SinkStream& stream)
{
  stream.Write(command.mailUid)
    .Write(command.recipient)
    .Write(command.date)
    .Write(command.body);
}

void server::protocol::ChatCmdLetterSendAckOk::Read(
  ChatCmdLetterSendAckOk& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterSendAckCancel::Write(
  const ChatCmdLetterSendAckCancel& command,
  server::SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdLetterSendAckCancel::Read(
  ChatCmdLetterSendAckCancel& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterRead::Write(
  const ChatCmdLetterRead& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterRead::Read(
  ChatCmdLetterRead& command,
  server::SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.mailUid);
}

void server::protocol::ChatCmdLetterReadAckOk::Write(
  const ChatCmdLetterReadAckOk& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.mailUid)
    .Write(command.unk2);
}

void server::protocol::ChatCmdLetterReadAckOk::Read(
  ChatCmdLetterReadAckOk& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterReadAckCancel::Write(
  const ChatCmdLetterReadAckCancel& command,
  server::SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdLetterReadAckCancel::Read(
  ChatCmdLetterReadAckCancel& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterDelete::Write(
  const ChatCmdLetterDelete& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterDelete::Read(
  ChatCmdLetterDelete& command,
  server::SourceStream& stream)
{
  stream.Read(command.folder)
    .Read(command.mailUid);
}

void server::protocol::ChatCmdLetterDeleteAckOk::Write(
  const ChatCmdLetterDeleteAckOk& command,
  server::SinkStream& stream)
{
  stream.Write(command.folder)
    .Write(command.mailUid);
}

void server::protocol::ChatCmdLetterDeleteAckOk::Read(
  ChatCmdLetterDeleteAckOk& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterDeleteAckCancel::Write(
  const ChatCmdLetterDeleteAckCancel& command,
  server::SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdLetterDeleteAckCancel::Read(
  ChatCmdLetterDeleteAckCancel& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterArriveTrs::Write(
  const ChatCmdLetterArriveTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.mailUid)
    .Write(command.mailType)
    .Write(command.mailOrigin)
    .Write(command.sender)
    .Write(command.date)
    .Write(command.body);
}

void server::protocol::ChatCmdLetterArriveTrs::Read(
  ChatCmdLetterArriveTrs& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdEnterRoom::Write(
  const ChatCmdEnterRoom& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdEnterRoom::Read(
  ChatCmdEnterRoom& command,
  server::SourceStream& stream)
{
  stream.Read(command.code)
    .Read(command.characterUid)
    .Read(command.characterName)
    .Read(command.guildUid);
}

void server::protocol::ChatCmdEnterRoomAckOk::Write(
  const ChatCmdEnterRoomAckOk& command,
  server::SinkStream& stream)
{
  stream.Write(static_cast<uint32_t>(command.unk1.size()));
  for (const auto& item : command.unk1)
  {
    stream.Write(item.unk0)
      .Write(item.unk1);
  }
}

void server::protocol::ChatCmdEnterRoomAckOk::Read(
  ChatCmdEnterRoomAckOk& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChat::Write(
  const ChatCmdChat& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChat::Read(
  ChatCmdChat& command,
  server::SourceStream& stream)
{
  stream.Read(command.message)
    .Read(command.role);
}

void server::protocol::ChatCmdChatTrs::Write(
  const ChatCmdChatTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.message);
}

void server::protocol::ChatCmdChatTrs::Read(
  ChatCmdChatTrs& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdInputState::Write(
  const ChatCmdInputState& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdInputState::Read(
  ChatCmdInputState& command,
  server::SourceStream& stream)
{
  stream.Read(command.state);
}

void server::protocol::ChatCmdInputStateTrs::Write(
  const ChatCmdInputStateTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.state);
}

void server::protocol::ChatCmdInputStateTrs::Read(
  ChatCmdInputStateTrs& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChannelChatTrs::Write(
  const ChatCmdChannelChatTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2);
}

void server::protocol::ChatCmdChannelChatTrs::Read(
  ChatCmdChannelChatTrs& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChannelInfo::Write(
  const ChatCmdChannelInfo& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChannelInfo::Read(
  ChatCmdChannelInfo& command,
  server::SourceStream& stream)
{
  stream.Read(command.unk0);
}

void server::protocol::ChatCmdChannelInfoAckOk::Write(
  const ChatCmdChannelInfoAckOk& command,
  server::SinkStream& stream)
{
  stream.Write(command.hostname)
    .Write(command.port)
    .Write(command.code);
}

void server::protocol::ChatCmdChannelInfoAckOk::Read(
  ChatCmdChannelInfoAckOk& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLogin::Write(
  const ChatCmdGuildLogin& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLogin::Read(
  ChatCmdGuildLogin& command,
  server::SourceStream& stream)
{
  server::protocol::ChatCmdLogin::Read(command, stream);
}

void server::protocol::ChatCmdGuildLoginAckOK::GuildMember::Struct2::Write(
  const ChatCmdGuildLoginAckOK::GuildMember::Struct2& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void server::protocol::ChatCmdGuildLoginAckOK::GuildMember::Struct2::Read(
  ChatCmdGuildLoginAckOK::GuildMember::Struct2& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLoginAckOK::GuildMember::Write(
  const ChatCmdGuildLoginAckOK::GuildMember& command,
  server::SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.status)
    .Write(command.unk2);
}

void server::protocol::ChatCmdGuildLoginAckOK::GuildMember::Read(
  ChatCmdGuildLoginAckOK::GuildMember& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLoginAckOK::Write(
  const ChatCmdGuildLoginAckOK& command,
  server::SinkStream& stream)
{
  // Guild members array size (u32)
  stream.Write(static_cast<uint32_t>(command.guildMembers.size()));
  for (const auto& struct1Element : command.guildMembers)
  {
    stream.Write(struct1Element);
  }
}

void server::protocol::ChatCmdGuildLoginAckOK::Read(
  ChatCmdGuildLoginAckOK& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLoginAckCancel::Write(
  const ChatCmdGuildLoginAckCancel& command,
  server::SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdGuildLoginAckCancel::Read(
  ChatCmdGuildLoginAckCancel& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}
