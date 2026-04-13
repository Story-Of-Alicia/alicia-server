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

#ifndef COMMON_MESSAGE_DEFINES_HPP
#define COMMON_MESSAGE_DEFINES_HPP

#include "libserver/network/command/CommandProtocol.hpp"
#include <libserver/util/Stream.hpp>

#include <string>

namespace server::protocol
{

struct AcCmdCRInviteUser
{
  uint32_t recipientCharacterUid{};
  std::string recipientCharacterName{};

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteUser;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteUser& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteUser& command,
    SourceStream& stream);
};

struct AcCmdCRInviteUserCancel : AcCmdCRInviteUser
{
  // Identical to `AcCmdCRInviteUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteUserCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteUserCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteUserCancel& command,
    SourceStream& stream);
};

struct AcCmdCRInviteUserOK : AcCmdCRInviteUser
{
  // Identical to `AcCmdCRInviteUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteUserOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteUserOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteUserOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestUser
{
  bool force{};
  std::string characterName{};
  uint32_t roomUid{};
  uint32_t ranchUid{};
  
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestUser;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestUser& command,
    SinkStream& stream);
  
  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestUser& command,
    SourceStream& stream);
};

struct AcCmdCRRequestUserOK : AcCmdCRRequestUser
{
  // Identical to `AcCmdCRRequestUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestUserOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestUserOK& command,
    SinkStream& stream);
  
  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestUserOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestUserCancel : AcCmdCRRequestUser
{
  // Identical to `AcCmdCRRequestUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestUserCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestUserCancel& command,
    SinkStream& stream);
  
  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestUserCancel& command,
    SourceStream& stream);
};

struct AcCmdRCRequestUser : AcCmdCRRequestUser
{
  // Identical to `AcCmdCRRequestUser`

  static Command GetCommand()
  {
    return Command::AcCmdRCRequestUser;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCRequestUser& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCRequestUser& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // LOBBY_MESSAGE_DEFINES_HPP
