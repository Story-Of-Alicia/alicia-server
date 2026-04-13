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

#include "libserver/network/command/proto/CommonMessageDefinitions.hpp"

namespace server::protocol
{

void AcCmdCRInviteUser::Read(
  AcCmdCRInviteUser& command,
  SourceStream& stream)
{
  stream.Read(command.recipientCharacterUid)
    .Read(command.recipientCharacterName);
}

void AcCmdCRInviteUser::Write(
  const AcCmdCRInviteUser&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteUserCancel::Read(
  AcCmdCRInviteUserCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteUserCancel::Write(
  const AcCmdCRInviteUserCancel& command,
  SinkStream& stream)
{
  stream.Write(command.recipientCharacterUid)
    .Write(command.recipientCharacterName);
}

void AcCmdCRInviteUserOK::Read(
  AcCmdCRInviteUserOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteUserOK::Write(
  const AcCmdCRInviteUserOK& command,
  SinkStream& stream)
{
  stream.Write(command.recipientCharacterUid)
    .Write(command.recipientCharacterName);
}

void AcCmdCRRequestUser::Read(
  AcCmdCRRequestUser& command,
  SourceStream& stream)
{
  stream.Read(command.force)
    .Read(command.characterName)
    .Read(command.roomUid)
    .Read(command.ranchUid);
}

void AcCmdCRRequestUser::Write(
  const AcCmdCRRequestUser&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
};

void AcCmdCRRequestUserOK::Read(
  AcCmdCRRequestUserOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestUserOK::Write(
  const AcCmdCRRequestUserOK& command,
  SinkStream& stream)
{
  stream.Write(command.force)
    .Write(command.characterName)
    .Write(command.roomUid)
    .Write(command.ranchUid);
}

void AcCmdCRRequestUserCancel::Read(
  AcCmdCRRequestUserCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestUserCancel::Write(
  const AcCmdCRRequestUserCancel& command,
  SinkStream& stream)
{
  stream.Write(command.force)
    .Write(command.characterName)
    .Write(command.roomUid)
    .Write(command.ranchUid);
}

void AcCmdRCRequestUser::Read(
  AcCmdRCRequestUser&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
};

void AcCmdRCRequestUser::Write(
  const AcCmdRCRequestUser& command,
  SinkStream& stream)
{
  stream.Write(command.force)
    .Write(command.characterName)
    .Write(command.roomUid)
    .Write(command.ranchUid);
}

} // namespace server::protocol

