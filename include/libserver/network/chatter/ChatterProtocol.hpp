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
#include <string_view>

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
  ChatCmdLogin = 0x1,
  ChatCmdLoginAckOK = 0x2,
  ChatCmdLoginAckCancel = 0x3,
  ChatCmdBuddyAdd = 0x4,
  ChatCmdBuddyAddAckOk = 0x5,
  ChatCmdBuddyAddAckCancel = 0x6,
  ChatCmdBuddyAddRequestTrs = 0x7,
  ChatCmdBuddyAddReply = 0x8,
  ChatCmdBuddyDelete = 0x9,
  ChatCmdBuddyDeleteAckOk = 0xA,
  ChatCmdBuddyDeleteAckCancel = 0xB,
  ChatCmdBuddyMove = 0xC,
  ChatCmdBuddyMoveAckOk = 0xD,
  ChatCmdBuddyMoveAckCancel = 0xE,
  ChatCmdGroupAdd = 0xF,
  ChatCmdGroupAddAckOk = 0x10,
  ChatCmdGroupAddAckCancel = 0x11,
  ChatCmdGroupRename = 0x12,
  ChatCmdGroupRenameAckOk = 0x13,
  ChatCmdGroupRenameAckCancel = 0x14,
  ChatCmdGroupDelete = 0x15,
  ChatCmdGroupDeleteAckOk = 0x16,
  ChatCmdGroupDeleteAckCancel = 0x17,
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
  ChatCmdChatInvite = 0x27,
  ChatCmdChatInvitationTrs = 0x28,
  ChatCmdEnterRoom = 0x29,
  ChatCmdEnterRoomAckOk = 0x2A,
  ChatCmdEnterRoomAckCancel = 0x2B,
  ChatCmdEnterBuddyTrs = 0x2C,
  ChatCmdLeaveBuddyTrs = 0x2D,
  ChatCmdChat = 0x2E,
  ChatCmdChatTrs = 0x2F,
  ChatCmdInputState = 0x30,
  ChatCmdInputStateTrs = 0x31,
  ChatCmdEndChatTrs = 0x32,
  ChatCmdErrorTrs = 0x33,
  ChatCmdGameInvite = 0x34,
  ChatCmdGameInviteAck = 0x35,
  ChatCmdGameInviteTrs = 0x36,
  ChatCmdChannelChatTrs = 0x37,
  ChatCmdChannelInfo = 0x38,
  ChatCmdChannelInfoAckOk = 0x39,

  ChatCmdGuildChannelChatTrs = 0x3E,
  ChatCmdUpdateGuildInfo = 0x3F,
  ChatCmdGuildLogin = 0x40,
  ChatCmdGuildLoginAckOK = 0x41,
  ChatCmdGuildLoginAckCancel = 0x42,
  ChatCmdGuildLogout = 0x43,
  ChatCmdUpdateGuildMemberStateTrs = 0x44,

  ChatCmdChannelInfoGuildRoomAckOk = 0x46,

  ChatCmdChangeLetterOption = 0x4B,
  ChatCmdChangeLetterOptionAckOk = 0x4C
};

std::string_view GetChatterCommandName(server::protocol::ChatterCommand command);

} // namespace server::protocol

#endif //CHATTERPROTOCOL_HPP
