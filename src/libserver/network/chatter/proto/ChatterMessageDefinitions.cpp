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

void server::protocol::Presence::Read(
  Presence& presence,
  SourceStream& stream)
{
  stream.Read(presence.status)
    .Read(presence.scene)
    .Read(presence.sceneUid);
}

void server::protocol::Presence::Write(
  const Presence& presence,
  SinkStream& stream)
{
  stream.Write(presence.status)
    .Write(presence.scene)
    .Write(presence.sceneUid);
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
      .Write(fr.scene)
      .Write(fr.sceneUid);
  }
}

void server::protocol::ChatCmdLoginAckOK::Read(
  ChatCmdLoginAckOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLoginAckCancel::Write(
  const ChatCmdLoginAckCancel& command,
  SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdLoginAckCancel::Read(
  ChatCmdLoginAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyAdd::Write(
  const ChatCmdBuddyAdd&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyAdd::Read(
  ChatCmdBuddyAdd& command,
  SourceStream& stream)
{
  stream.Read(command.characterName);
}

void server::protocol::ChatCmdBuddyAddAckOk::Write(
  const ChatCmdBuddyAddAckOk& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.characterName)
    .Write(command.unk2)
    .Write(command.status);
}

void server::protocol::ChatCmdBuddyAddAckOk::Read(
  ChatCmdBuddyAddAckOk&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyAddAckCancel::Write(
  const ChatCmdBuddyAddAckCancel& command,
  SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdBuddyAddAckCancel::Read(
  ChatCmdBuddyAddAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyAddRequestTrs::Write(
  const ChatCmdBuddyAddRequestTrs& command,
  SinkStream& stream)
{
  stream.Write(command.requestingCharacterUid)
    .Write(command.requestingCharacterName);
}

void server::protocol::ChatCmdBuddyAddRequestTrs::Read(
  ChatCmdBuddyAddRequestTrs&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyAddReply::Write(
  const ChatCmdBuddyAddReply&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyAddReply::Read(
  ChatCmdBuddyAddReply& command,
  SourceStream& stream)
{
  stream.Read(command.requestingCharacterUid)
    .Read(command.requestAccepted);
}

void server::protocol::ChatCmdBuddyDelete::Write(
  const ChatCmdBuddyDelete&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyDelete::Read(
  ChatCmdBuddyDelete& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid);
}

void server::protocol::ChatCmdBuddyDeleteAckOk::Write(
  const ChatCmdBuddyDeleteAckOk& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid);
}

void server::protocol::ChatCmdBuddyDeleteAckOk::Read(
  ChatCmdBuddyDeleteAckOk&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyDeleteAckCancel::Write(
  const ChatCmdBuddyDeleteAckCancel& command,
  SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdBuddyDeleteAckCancel::Read(
  ChatCmdBuddyDeleteAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyMove::Write(
  const ChatCmdBuddyMove&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyMove::Read(
  ChatCmdBuddyMove& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.groupUid);
}

void server::protocol::ChatCmdBuddyMoveAckOk::Write(
  const ChatCmdBuddyMoveAckOk& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.groupUid);
}

void server::protocol::ChatCmdBuddyMoveAckOk::Read(
  ChatCmdBuddyMoveAckOk&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdBuddyMoveAckCancel::Write(
  const ChatCmdBuddyMoveAckCancel& command,
  SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdBuddyMoveAckCancel::Read(
  ChatCmdBuddyMoveAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupAdd::Write(
  const ChatCmdGroupAdd&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupAdd::Read(
  ChatCmdGroupAdd& command,
  SourceStream& stream)
{
  stream.Read(command.groupName);
}

void server::protocol::ChatCmdGroupAddAckOk::Write(
  const ChatCmdGroupAddAckOk& command,
  SinkStream& stream)
{
  stream.Write(command.groupUid)
    .Write(command.groupName);
}

void server::protocol::ChatCmdGroupAddAckOk::Read(
  ChatCmdGroupAddAckOk&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupAddAckCancel::Write(
  const ChatCmdGroupAddAckCancel& command,
  SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdGroupAddAckCancel::Read(
  ChatCmdGroupAddAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupRename::Write(
  const ChatCmdGroupRename&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupRename::Read(
  ChatCmdGroupRename& command,
  SourceStream& stream)
{
  stream.Read(command.groupUid)
    .Read(command.groupName);
}

void server::protocol::ChatCmdGroupRenameAckOk::Write(
  const ChatCmdGroupRenameAckOk& command,
  SinkStream& stream)
{
  stream.Write(command.groupUid)
    .Write(command.groupName);
}

void server::protocol::ChatCmdGroupRenameAckOk::Read(
  ChatCmdGroupRenameAckOk&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupRenameAckCancel::Write(
  const ChatCmdGroupRenameAckCancel& command,
  SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdGroupRenameAckCancel::Read(
  ChatCmdGroupRenameAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupDelete::Write(
  const ChatCmdGroupDelete&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupDelete::Read(
  ChatCmdGroupDelete& command,
  SourceStream& stream)
{
  stream.Read(command.groupUid);
}

void server::protocol::ChatCmdGroupDeleteAckOk::Write(
  const ChatCmdGroupDeleteAckOk& command,
  SinkStream& stream)
{
  stream.Write(command.groupUid);
}

void server::protocol::ChatCmdGroupDeleteAckOk::Read(
  ChatCmdGroupDeleteAckOk&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGroupDeleteAckCancel::Write(
  const ChatCmdGroupDeleteAckCancel& command,
  SinkStream& stream)
{
  stream.Write(command.errorCode);
}

void server::protocol::ChatCmdGroupDeleteAckCancel::Read(
  ChatCmdGroupDeleteAckCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterList::Request::Write(
  const ChatCmdLetterList::Request&,
  server::SinkStream&)
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
  const ChatCmdLetterList&,
  server::SinkStream&)
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
  ChatCmdLetterListAckOk&,
  server::SourceStream&)
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
  ChatCmdLetterListAckCancel&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterSend::Write(
  const ChatCmdLetterSend&,
  server::SinkStream&)
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
  ChatCmdLetterSendAckOk&,
  server::SourceStream&)
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
  ChatCmdLetterSendAckCancel&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterRead::Write(
  const ChatCmdLetterRead&,
  server::SinkStream&)
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
  ChatCmdLetterReadAckOk&,
  server::SourceStream&)
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
  ChatCmdLetterReadAckCancel&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLetterDelete::Write(
  const ChatCmdLetterDelete&,
  server::SinkStream&)
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
  ChatCmdLetterDeleteAckOk&,
  server::SourceStream&)
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
  ChatCmdLetterDeleteAckCancel&,
  server::SourceStream&)
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
  ChatCmdLetterArriveTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdUpdateState::Write(
  const ChatCmdUpdateState&,
  server::SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdUpdateState::Read(
  ChatCmdUpdateState& command,
  server::SourceStream& stream)
{
  stream.Read(command.presence.status)
    .Read(command.presence.scene)
    .Read(command.presence.sceneUid);
}

void server::protocol::ChatCmdUpdateStateTrs::Write(
  const ChatCmdUpdateStateTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.affectedCharacterUid)
    .Write(command.presence.status)
    .Write(command.presence.scene)
    .Write(command.presence.sceneUid);
}

void server::protocol::ChatCmdUpdateStateTrs::Read(
  ChatCmdUpdateStateTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChatInvite::Write(
  const ChatCmdChatInvite&,
  server::SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChatInvite::Read(
  ChatCmdChatInvite& command,
  server::SourceStream& stream)
{
  uint32_t length{};
  stream.Read(length);

  command.chatParticipantUids.resize(length);
  stream.Read(command.chatParticipantUids.data(), length);
}

void server::protocol::ChatCmdChatInvitationTrs::Write(
  const ChatCmdChatInvitationTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.unk4)
    .Write(command.unk5);
}

void server::protocol::ChatCmdChatInvitationTrs::Read(
  ChatCmdChatInvitationTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdEnterRoom::Write(
  const ChatCmdEnterRoom&,
  server::SinkStream&)
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
  ChatCmdEnterRoomAckOk&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChat::Write(
  const ChatCmdChat&,
  server::SinkStream&)
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
  ChatCmdChatTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdInputState::Write(
  const ChatCmdInputState&,
  server::SinkStream&)
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
  ChatCmdInputStateTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGameInvite::Write(
  const ChatCmdGameInvite&,
  server::SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGameInvite::Read(
  ChatCmdGameInvite& command,
  server::SourceStream& stream)
{
  stream.Read(command.recipientCharacterUid);
}

void server::protocol::ChatCmdGameInviteAck::Write(
  const ChatCmdGameInviteAck& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void server::protocol::ChatCmdGameInviteAck::Read(
  ChatCmdGameInviteAck&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGameInviteTrs::Write(
  const ChatCmdGameInviteTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0);
}

void server::protocol::ChatCmdGameInviteTrs::Read(
  ChatCmdGameInviteTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChannelChatTrs::Write(
  const ChatCmdChannelChatTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.messageAuthor)
    .Write(command.message)
    .Write(command.role);
}

void server::protocol::ChatCmdChannelChatTrs::Read(
  ChatCmdChannelChatTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChannelInfo::Write(
  const ChatCmdChannelInfo&,
  server::SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChannelInfo::Read(
  ChatCmdChannelInfo&,
  server::SourceStream&)
{
  // Empty
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
  ChatCmdChannelInfoAckOk&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildChannelChatTrs::Write(
  const ChatCmdGuildChannelChatTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.guildUid)
    .Write(command.author)
    .Write(command.message)
    .Write(command.role);
}

void server::protocol::ChatCmdGuildChannelChatTrs::Read(
  ChatCmdGuildChannelChatTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLogin::Write(
  const ChatCmdGuildLogin&,
  server::SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLogin::Read(
  ChatCmdGuildLogin& command,
  server::SourceStream& stream)
{
  server::protocol::ChatCmdLogin::Read(command, stream);
}

void server::protocol::ChatCmdGuildLoginAckOK::GuildMember::Write(
  const ChatCmdGuildLoginAckOK::GuildMember& command,
  server::SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.presence);
}

void server::protocol::ChatCmdGuildLoginAckOK::GuildMember::Read(
  ChatCmdGuildLoginAckOK::GuildMember&,
  server::SourceStream&)
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
  ChatCmdGuildLoginAckOK&,
  server::SourceStream&)
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
  ChatCmdGuildLoginAckCancel&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdUpdateGuildMemberStateTrs::Write(
  const ChatCmdUpdateGuildMemberStateTrs& command,
  server::SinkStream& stream)
{
  stream.Write(command.affectedCharacterUid)
    .Write(command.presence.status)
    .Write(command.presence.scene)
    .Write(command.presence.sceneUid);
}

void server::protocol::ChatCmdUpdateGuildMemberStateTrs::Read(
  ChatCmdUpdateGuildMemberStateTrs&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdChannelInfoGuildRoomAckOk::Write(
  const ChatCmdChannelInfoGuildRoomAckOk& command,
  server::SinkStream& stream)
{
  stream.Write(command.hostname)
    .Write(command.port)
    .Write(command.code);
}

void server::protocol::ChatCmdChannelInfoGuildRoomAckOk::Read(
  ChatCmdChannelInfoGuildRoomAckOk&,
  server::SourceStream&)
{
  throw std::runtime_error("Not implemented");
}
