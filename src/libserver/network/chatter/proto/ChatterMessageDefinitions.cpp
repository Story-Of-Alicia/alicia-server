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

  stream.Write(command.mailAlarm.mailUid)
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
}

void server::protocol::ChatCmdLoginAckCancel::Read(
  ChatCmdLoginAckCancel&,
  SourceStream&)
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

void server::protocol::ChatCmdGuildLoginOK::GuildMember::Struct2::Write(
  const ChatCmdGuildLoginOK::GuildMember::Struct2& command,
  server::SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void server::protocol::ChatCmdGuildLoginOK::GuildMember::Struct2::Read(
  ChatCmdGuildLoginOK::GuildMember::Struct2& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLoginOK::GuildMember::Write(
  const ChatCmdGuildLoginOK::GuildMember& command,
  server::SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.status)
    .Write(command.unk2);
}

void server::protocol::ChatCmdGuildLoginOK::GuildMember::Read(
  ChatCmdGuildLoginOK::GuildMember& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdGuildLoginOK::Write(
  const ChatCmdGuildLoginOK& command,
  server::SinkStream& stream)
{
  // Guild members array size (u32)
  stream.Write(static_cast<uint32_t>(command.guildMembers.size()));
  for (const auto& struct1Element : command.guildMembers)
  {
    stream.Write(struct1Element);
  }
}

void server::protocol::ChatCmdGuildLoginOK::Read(
  ChatCmdGuildLoginOK& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}
