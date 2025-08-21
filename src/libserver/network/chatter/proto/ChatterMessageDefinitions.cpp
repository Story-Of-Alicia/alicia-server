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
  const ChatCmdLogin& command,
  server::SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLogin::Read(
  ChatCmdLogin& command,
  server::SourceStream& stream)
{
  stream.Read(command.val0)
    .Read(command.name)
    .Read(command.code)
    .Read(command.val1);
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
  ChatCmdLoginAckOK& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void server::protocol::ChatCmdLoginAckCancel::Write(
  const ChatCmdLoginAckCancel& command,
  server::SinkStream& stream)
{
}

void server::protocol::ChatCmdLoginAckCancel::Read(
  ChatCmdLoginAckCancel& command,
  server::SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}
