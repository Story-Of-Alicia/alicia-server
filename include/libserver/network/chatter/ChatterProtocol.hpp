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

#ifndef CHATTERPROTOCOL_HPP
#define CHATTERPROTOCOL_HPP

#include <cstdint>

namespace server::protocol
{

struct ChatterCommandHeader
{
  //! A length of the command payload.
  uint16_t length;
  //! An ID of the chatter command.
  uint16_t commandId;
};

enum class ChatterCommand : uint16_t
{
  ChatCmdLogin = 1,
  ChatCmdLoginAckOK = 2,
  ChatCmdLoginAckCancel = 3,

  ChatCmdLetterList = 0x18,
  ChatCmdLetterListAckOk = 0x19,
  ChatCmdLetterListAckCancel = 0x1A,
  ChatCmdLetterSend = 0x1B,
  ChatCmdLetterSendAckOk = 0x1C,
  ChatCmdLetterSendAckCancel = 0x1D,
  ChatCmdLetterRead = 0x1E,
  ChatCmdLetterReadAckOk = 0x1F,
  ChatCmdLetterReadAckCancel = 0x20,
  ChatCmdLetterDelete = 0x21,
  ChatCmdLetterDeleteAckOk = 0x22,
  ChatCmdLetterDeleteAckCancel = 0x23,
  ChatCmdLetterArriveTrs = 0x24,
  ChatCmdUpdateState = 0x25,
  ChatCmdUpdateStateTrs = 0x26,

  ChatCmdEnterRoom = 0x29,
  ChatCmdEnterRoomAckOk = 0x2a,

  ChatCmdChat = 0x2E,
  ChatCmdChatTrs = 0x2F,
  ChatCmdInputState = 0x30,
  ChatCmdInputStateTrs = 0x31,

  ChatCmdChannelChatTrs = 0x37,
  ChatCmdChannelInfo = 0x38,
  ChatCmdChannelInfoAckOk = 0x39,
  ChatCmdGuildLogin = 0x40,
  ChatCmdGuildLoginAckOK = 0x41,
  ChatCmdGuildLoginAckCancel = 0x42,

  ChatCmdUpdateGuildMemberStateTrs = 0x44,
};

} // namespace server::protocol

#endif //CHATTERPROTOCOL_HPP
