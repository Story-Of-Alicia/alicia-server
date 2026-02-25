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

#include "libserver/network/chatter/ChatterProtocol.hpp"

#include <unordered_map>

namespace server::protocol
{

namespace
{

const std::unordered_map<ChatterCommand, std::string_view> commands = {
  {ChatterCommand::ChatCmdLogin, "ChatCmdLogin"},
  {ChatterCommand::ChatCmdLoginAckOK, "ChatCmdLoginAckOK"},
  {ChatterCommand::ChatCmdLoginAckCancel, "ChatCmdLoginAckCancel"},
  {ChatterCommand::ChatCmdBuddyAdd, "ChatCmdBuddyAdd"},
  {ChatterCommand::ChatCmdBuddyAddAckOk, "ChatCmdBuddyAddAckOk"},
  {ChatterCommand::ChatCmdBuddyAddAckCancel, "ChatCmdBuddyAddAckCancel"},
  {ChatterCommand::ChatCmdBuddyAddRequestTrs, "ChatCmdBuddyAddRequestTrs"},
  {ChatterCommand::ChatCmdBuddyAddReply, "ChatCmdBuddyAddReply"},
  {ChatterCommand::ChatCmdBuddyDelete, "ChatCmdBuddyDelete"},
  {ChatterCommand::ChatCmdBuddyDeleteAckOk, "ChatCmdBuddyDeleteAckOk"},
  {ChatterCommand::ChatCmdBuddyDeleteAckCancel, "ChatCmdBuddyDeleteAckCancel"},
  {ChatterCommand::ChatCmdBuddyMove, "ChatCmdBuddyMove"},
  {ChatterCommand::ChatCmdBuddyMoveAckOk, "ChatCmdBuddyMoveAckOk"},
  {ChatterCommand::ChatCmdBuddyMoveAckCancel, "ChatCmdBuddyMoveAckCancel"},
  {ChatterCommand::ChatCmdGroupAdd, "ChatCmdGroupAdd"},
  {ChatterCommand::ChatCmdGroupAddAckOk, "ChatCmdGroupAddAckOk"},
  {ChatterCommand::ChatCmdGroupAddAckCancel, "ChatCmdGroupAddAckCancel"},
  {ChatterCommand::ChatCmdGroupRename, "ChatCmdGroupRename"},
  {ChatterCommand::ChatCmdGroupRenameAckOk, "ChatCmdGroupRenameAckOk"},
  {ChatterCommand::ChatCmdGroupRenameAckCancel, "ChatCmdGroupRenameAckCancel"},
  {ChatterCommand::ChatCmdGroupDelete, "ChatCmdGroupDelete"},
  {ChatterCommand::ChatCmdGroupDeleteAckOk, "ChatCmdGroupDeleteAckOk"},
  {ChatterCommand::ChatCmdGroupDeleteAckCancel, "ChatCmdGroupDeleteAckCancel"},
  {ChatterCommand::ChatCmdLetterList, "ChatCmdLetterList"},
  {ChatterCommand::ChatCmdLetterListAckOk, "ChatCmdLetterListAckOk"},
  {ChatterCommand::ChatCmdLetterListAckCancel, "ChatCmdLetterListAckCancel"},
  {ChatterCommand::ChatCmdLetterSend, "ChatCmdLetterSend"},
  {ChatterCommand::ChatCmdLetterSendAckOk, "ChatCmdLetterSendAckOk"},
  {ChatterCommand::ChatCmdLetterSendAckCancel, "ChatCmdLetterSendAckCancel"},
  {ChatterCommand::ChatCmdLetterRead, "ChatCmdLetterRead"},
  {ChatterCommand::ChatCmdLetterReadAckOk, "ChatCmdLetterReadAckOk"},
  {ChatterCommand::ChatCmdLetterReadAckCancel, "ChatCmdLetterReadAckCancel"},
  {ChatterCommand::ChatCmdLetterDelete, "ChatCmdLetterDelete"},
  {ChatterCommand::ChatCmdLetterDeleteAckOk, "ChatCmdLetterDeleteAckOk"},
  {ChatterCommand::ChatCmdLetterDeleteAckCancel, "ChatCmdLetterDeleteAckCancel"},
  {ChatterCommand::ChatCmdLetterArriveTrs, "ChatCmdLetterArriveTrs"},
  {ChatterCommand::ChatCmdUpdateState, "ChatCmdUpdateState"},
  {ChatterCommand::ChatCmdUpdateStateTrs, "ChatCmdUpdateStateTrs"},
  {ChatterCommand::ChatCmdChatInvite, "ChatCmdChatInvite"},
  {ChatterCommand::ChatCmdChatInvitationTrs, "ChatCmdChatInvitationTrs"},
  {ChatterCommand::ChatCmdEnterRoom, "ChatCmdEnterRoom"},
  {ChatterCommand::ChatCmdEnterRoomAckOk, "ChatCmdEnterRoomAckOk"},
  {ChatterCommand::ChatCmdEnterRoomAckCancel, "ChatCmdEnterRoomAckCancel"},
  {ChatterCommand::ChatCmdEnterBuddyTrs, "ChatCmdEnterBuddyTrs"},
  {ChatterCommand::ChatCmdLeaveBuddyTrs, "ChatCmdLeaveBuddyTrs"},
  {ChatterCommand::ChatCmdChat, "ChatCmdChat"},
  {ChatterCommand::ChatCmdChatTrs, "ChatCmdChatTrs"},
  {ChatterCommand::ChatCmdInputState, "ChatCmdInputState"},
  {ChatterCommand::ChatCmdInputStateTrs, "ChatCmdInputStateTrs"},
  {ChatterCommand::ChatCmdEndChatTrs, "ChatCmdEndChatTrs"},
  {ChatterCommand::ChatCmdErrorTrs, "ChatCmdErrorTrs"},
  {ChatterCommand::ChatCmdGameInvite, "ChatCmdGameInvite"},
  {ChatterCommand::ChatCmdGameInviteAck, "ChatCmdGameInviteAck"},
  {ChatterCommand::ChatCmdGameInviteTrs, "ChatCmdGameInviteTrs"},
  {ChatterCommand::ChatCmdChannelChatTrs, "ChatCmdChannelChatTrs"},
  {ChatterCommand::ChatCmdChannelInfo, "ChatCmdChannelInfo"},
  {ChatterCommand::ChatCmdChannelInfoAckOk, "ChatCmdChannelInfoAckOk"},

  {ChatterCommand::ChatCmdGuildChannelChatTrs, "ChatCmdGuildChannelChatTrs"},
  {ChatterCommand::ChatCmdUpdateGuildInfo, "ChatCmdUpdateGuildInfo"},
  {ChatterCommand::ChatCmdGuildLogin, "ChatCmdGuildLogin"},
  {ChatterCommand::ChatCmdGuildLoginAckOK, "ChatCmdGuildLoginAckOK"},
  {ChatterCommand::ChatCmdGuildLoginAckCancel, "ChatCmdGuildLoginAckCancel"},
  {ChatterCommand::ChatCmdGuildLogout, "ChatCmdGuildLogout"},
  {ChatterCommand::ChatCmdUpdateGuildMemberStateTrs, "ChatCmdUpdateGuildMemberStateTrs"},

  {ChatterCommand::ChatCmdChannelInfoGuildRoomAckOk, "ChatCmdChannelInfoGuildRoomAckOk"},

  {ChatterCommand::ChatCmdChangeLetterOption, "ChatCmdChangeLetterOption"},
  {ChatterCommand::ChatCmdChangeLetterOptionAckOk, "ChatCmdChangeLetterOptionAckOk"}
};

} // namespace

std::string_view GetChatterCommandName(server::protocol::ChatterCommand command)
{
  const auto commandIter = commands.find(command);
  return commandIter == commands.cend() ? "n/a" : commandIter->second;
}

} // namespace server::protocol
