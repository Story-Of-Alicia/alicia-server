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

#include "libserver/command/CommandProtocol.hpp"

#include <unordered_map>

namespace alicia
{

namespace
{

//! Commands IDs mapped to the command names.
const std::unordered_map<CommandId, std::string_view> commands = {
  // Login
  {CommandId::LobbyLogin, "LobbyLogin"},
  {CommandId::LobbyLoginOK, "LobbyLoginOK"},
  {CommandId::LobbyLoginCancel, "LobbyLoginCancel"},

  // Heartbeat
  {CommandId::LobbyHeartbeat, "LobbyHeartbeat"},

  // Inventory
  {CommandId::LobbyShowInventory, "LobbyShowInventory"},
  {CommandId::LobbyShowInventoryOK, "LobbyShowInventoryOK"},
  {CommandId::LobbyShowInventoryCancel, "LobbyShowInventoryCancel"},

  // Inventory
  {CommandId::LobbyAchievementCompleteList, "AchievementCompleteList"},
  {CommandId::LobbyAchievementCompleteListOK, "AchievementCompleteListOK"},
  {CommandId::LobbyAchievementCompleteListCancel, "AchievementCompleteListCancel"},

  // RequestDailyQuestList
  {CommandId::LobbyRequestDailyQuestList, "RequestDailyQuestList"},
  {CommandId::LobbyRequestDailyQuestListOK, "RequestDailyQuestListOK"},
  {CommandId::LobbyRequestDailyQuestListCancel, "RequestDailyQuestListCancel"},

  // RequestQuestList
  {CommandId::LobbyRequestQuestList, "RequestQuestList"},
  {CommandId::LobbyRequestQuestListOK, "RequestQuestListOK"},
  {CommandId::LobbyRequestQuestListCancel, "RequestQuestListCancel"},

  // RequestLeagueInfo
  {CommandId::LobbyRequestLeagueInfo, "LobbyRequestLeagueInfo"},
  {CommandId::LobbyRequestLeagueInfoOK, "LobbyRequestLeagueInfoOK"},
  {CommandId::LobbyRequestLeagueInfoCancel, "LobbyRequestLeagueInfoCancel"},
};

} // namespace anon

MessageMagic decode_message_magic(uint32_t value)
{
  MessageMagic magic;
  if (value & 1 << 15)
  {
    const uint16_t section = value & 0x3FFF;
    magic.length = (value & 0xFF) << 4 | section >> 8 & 0xF | section & 0xF000;
  }

  const uint16_t firstTwoBytes = value & 0xFFFF;
  const uint16_t secondTwoBytes = value >> 16 & 0xFFFF;
  const uint16_t xorResult = firstTwoBytes ^ secondTwoBytes;
  magic.id = ~(xorResult & 0xC000) & xorResult;

  return magic;
}

uint32_t encode_message_magic(MessageMagic magic)
{
  const uint16_t id = BufferJumbo & 0xFFFF | magic.id & 0xFFFF;
  const uint32_t length = BufferSize << 16 | magic.length;

  uint32_t encoded = length;
  encoded = (encoded & 0x3FFF | encoded << 14) & 0xFFFF;
  encoded = ((encoded & 0xF | 0xFF80) << 8 | length >> 4 & 0xFF | encoded & 0xF000) & 0xFFFF;
  encoded |= (encoded ^ id) << 16;
  return encoded;
}

std::string_view GetCommandName(CommandId command)
{
  const auto commandIter = commands.find(command);
  return commandIter == commands.cend() ? "n/a" : commandIter->second;
}

} // namespace alicia