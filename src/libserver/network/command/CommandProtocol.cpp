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

#include "libserver/network/command/CommandProtocol.hpp"

#include <unordered_map>

namespace server::protocol
{

namespace
{

//! Commands IDs mapped to the command names.
const std::unordered_map<Command, std::string_view> commands = {
  {Command::LobbyLogin, "LobbyLogin"},
  {Command::LobbyLoginOK, "LobbyLoginOK"},
  {Command::LobbyLoginCancel, "LobbyLoginCancel"},

  {Command::LobbyRoomList, "LobbyRoomList"},
  {Command::LobbyRoomListOK, "LobbyRoomListOK"},

  {Command::LobbyMakeRoom, "LobbyMakeRoom"},
  {Command::LobbyMakeRoomOK, "LobbyMakeRoomOK"},
  {Command::LobbyMakeRoomCancel, "LobbyMakeRoomCancel"},

  {Command::LobbyEnterRoom, "LobbyEnterRoom"},
  {Command::LobbyEnterRoomOK, "LobbyEnterRoomOK"},
  {Command::LobbyEnterRoomCancel, "LobbyEnterRoomCancel"},

  {Command::LobbyHeartbeat, "LobbyHeartbeat"},

  {Command::LobbyCreateNicknameNotify, "LobbyCreateNicknameNotify"},
  {Command::LobbyCreateNickname, "LobbyCreateNicknameOK"},
  {Command::LobbyCreateNicknameCancel, "LobbyCreateNicknameCancel"},

  {Command::LobbyEnterChannel, "LobbyEnterChannel"},
  {Command::LobbyEnterChannelOK, "LobbyEnterChannelOK"},
  {Command::LobbyEnterChannelCancel, "LobbyEnterChannelCancel"},

  {Command::LobbyShowInventory, "LobbyShowInventory"},
  {Command::LobbyShowInventoryOK, "LobbyShowInventoryOK"},
  {Command::LobbyShowInventoryCancel, "LobbyShowInventoryCancel"},

  // Inventory
  {Command::LobbyAchievementCompleteList, "LobbyAchievementCompleteList"},
  {Command::LobbyAchievementCompleteListOK, "LobbyAchievementCompleteListOK"},
  {Command::LobbyAchievementCompleteListCancel, "LobbyAchievementCompleteListCancel"},

  // RequestDailyQuestList
  {Command::LobbyRequestDailyQuestList, "LobbyRequestDailyQuestList"},
  {Command::LobbyRequestDailyQuestListOK, "LobbyRequestDailyQuestListOK"},
  {Command::LobbyRequestDailyQuestListCancel, "LobbyRequestDailyQuestListCancel"},

  // RequestLeagueInfo
  {Command::LobbyRequestLeagueInfo, "LobbyRequestLeagueInfo"},
  {Command::LobbyRequestLeagueInfoOK, "LobbyRequestLeagueInfoOK"},
  {Command::LobbyRequestLeagueInfoCancel, "LobbyRequestLeagueInfoCancel"},

  // RequestQuestList
  {Command::LobbyRequestQuestList, "LobbyRequestQuestList"},
  {Command::LobbyRequestQuestListOK, "LobbyRequestQuestListOK"},
  {Command::LobbyRequestQuestListCancel, "LobbyRequestQuestListCancel"},

  // RequestSpecialEventList
  {Command::LobbyRequestSpecialEventList, "LobbyRequestSpecialEventList"},
  {Command::LobbyRequestSpecialEventListOK, "LobbyRequestSpecialEventListOK"},

  // EnterRanch
  {Command::LobbyEnterRanch, "LobbyEnterRanch"},
  {Command::LobbyEnterRanchOK, "LobbyEnterRanchOK"},
  {Command::LobbyEnterRanchCancel, "LobbyEnterRanchCancel"},

  {Command::LobbyGetMessengerInfo, "LobbyGetMessengerInfo"},
  {Command::LobbyGetMessengerInfoOK, "LobbyGetMessengerInfoOK"},
  {Command::LobbyGetMessengerInfoCancel, "LobbyGetMessengerInfoCancel"},

  {Command::LobbyClientNotify, "LobbyClientNotify"},

  // GoodsShop
  {Command::LobbyGoodsShopList, "LobbyGoodsShopList"},
  {Command::LobbyGoodsShopListOK, "LobbyGoodsShopListOK"},
  {Command::LobbyGoodsShopListCancel, "LobbyGoodsShopListCancel"},

  // InquiryTreecash
  {Command::LobbyInquiryTreecash, "LobbyInquiryTreecash"},
  {Command::LobbyInquiryTreecashOK, "LobbyInquiryTreecashOK"},
  {Command::LobbyInquiryTreecashCancel, "LobbyInquiryTreecashCancel"},

  // GuildPartyList
  {Command::LobbyGuildPartyList, "LobbyGuildPartyList"},
  {Command::LobbyGuildPartyListOK, "LobbyGuildPartyListOK"},

  {Command::LobbyEnterRandomRanch, "LobbyEnterRandomRanch"},

  {Command::LobbyRequestPersonalInfo, "LobbyRequestPersonalInfo"},
  {Command::LobbyPersonalInfo, "LobbyPersonalInfo"},

  {Command::LobbySetIntroduction, "LobbySetIntroduction"},

  {Command::LobbyUpdateSystemContent, "LobbyUpdateSystemContent"},
  {Command::LobbyUpdateSystemContentNotify, "LobbyUpdateSystemContentNotify"},

  {Command::RanchEnterRanch, "RanchEnter"},
  {Command::RanchEnterRanchCancel, "RanchEnterCancel"},
  {Command::RanchEnterRanchNotify, "RanchEnterNotify"},
  {Command::RanchEnterRanchOK, "RanchEnterRanchOK"},

  {Command::RanchLeaveRanch, "RanchLeave"},
  {Command::RanchLeaveRanchOK, "RanchLeave"},

  {Command::RanchLeaveRanchNotify, "RanchLeaveNotify"},

  {Command::RanchHeartbeat, "RanchHeartbeat"},

  {Command::RanchSnapshot, "RanchSnapshot"},
  {Command::RanchSnapshotNotify, "RanchSnapshotNotify"},

  {Command::RanchCmdAction, "RanchCmdAction"},
  {Command::RanchCmdActionNotify, "RanchCmdActionNotify"},

  {Command::RanchStuff, "RanchStuff"},
  {Command::RanchStuffOK, "RanchStuffOK"},

  {Command::RanchUpdateBusyState, "RanchUpdateBusyState"},
  {Command::RanchUpdateBusyStateNotify, "RanchUpdateBusyStateNotify"},

  {Command::RanchSearchStallion, "RanchSearchStallion"},
  {Command::RanchSearchStallionOK, "RanchSearchStallionOK"},
  {Command::RanchSearchStallionCancel, "RanchSearchStallionCancel"},

  {Command::RanchEnterBreedingMarket, "RanchEnterBreedingMarket"},
  {Command::RanchEnterBreedingMarketOK, "RanchEnterBreedingMarketOK"},
  {Command::RanchEnterBreedingMarketCancel, "RanchEnterBreedingMarketCancel"},

  {Command::RanchTryBreeding, "RanchTryBreeding"},
  {Command::RanchTryBreedingOK, "RanchTryBreedingOK"},
  {Command::RanchTryBreedingCancel, "RanchTryBreedingCancel"},

  {Command::RanchBreedingWishlist, "RanchBreedingWishlist"},
  {Command::RanchBreedingWishlistCancel, "RanchBreedingWishlistCancel"},
  {Command::RanchBreedingWishlistOK, "RanchBreedingWishlistOK"},

  {Command::RanchUpdateMountNickname, "RanchUpdateMountNickname"},
  {Command::RanchUpdateMountNicknameOK, "RanchUpdateMountNicknameOK"},
  {Command::RanchUpdateMountNicknameCancel, "RanchUpdateMountNicknameCancel"},

  {Command::RanchUpdateMountInfoNotify, "RanchUpdateMountInfoNotify"},

  {Command::RanchRequestStorage, "RanchRequestStorage"},
  {Command::RanchRequestStorageOK, "RanchRequestStorageOK"},
  {Command::RanchRequestStorageCancel, "RanchRequestStorageCancel"},

  {Command::RanchGetItemFromStorage, "RanchRequestStorage"},
  {Command::RanchGetItemFromStorageOK, "RanchGetItemFromStorageOK"},
  {Command::RanchGetItemFromStorageCancel, "RanchGetItemFromStorageCancel"},

  {Command::RanchRequestNpcDressList, "RanchRequestNpcDressList"},
  {Command::RanchRequestNpcDressListCancel, "RanchRequestNpcDressListCancel"},
  {Command::RanchRequestNpcDressListOK, "RanchRequestNpcDressListOK"},

  {Command::RanchChat, "RanchChat"},
  {Command::RanchChatNotify, "RanchChatNotify"},

  {Command::RanchKickRanch, "RanchChat"},
  {Command::RanchKickRanchOK, "RanchChatNotify"},
  {Command::RanchKickRanchCancel, "RanchChat"},
  {Command::RanchKickRanchNotify, "RanchChatNotify"},

  {Command::RanchWearEquipment, "RanchWearEquipment"},
  {Command::RanchWearEquipmentOK, "RanchWearEquipmentOK"},
  {Command::RanchWearEquipmentCancel, "RanchWearEquipmentCancel"},

  {Command::RanchRemoveEquipment, "RanchWearEquipment"},
  {Command::RanchRemoveEquipmentOK, "RanchWearEquipmentOK"},
  {Command::RanchRemoveEquipmentCancel, "RanchWearEquipmentCancel"},

  {Command::RanchUseItem, "RanchUseItem"},
  {Command::RanchUseItemOK, "RanchUseItemOK"},
  {Command::RanchUseItemCancel, "RanchUseItemCancel"},

  {Command::RanchUpdateEquipmentNotify, "RanchUpdateEquipmentNotify"},

  {Command::RanchSetIntroductionNotify, "RanchSetIntroductionNotify"},

  {Command::RanchCreateGuild, "RanchCreateGuild"},
  {Command::RanchCreateGuildOK, "RanchCreateGuildOK"},
  {Command::RanchCreateGuildCancel, "RanchCreateGuildCancel"},

  {Command::RanchRequestGuildInfo, "RanchRequestGuildInfo"},
  {Command::RanchRequestGuildInfoOK, "RanchRequestGuildInfoOK"},
  {Command::RanchRequestGuildInfoCancel, "RanchRequestGuildInfoCancel"},

  {Command::RanchIncubateEgg, "RanchIncubateEgg"},
  {Command::RanchIncubateEggOK, "RanchIncubateEggOK"},
  {Command::RanchIncubateEggCancel, "RanchIncubateEggCancel"},
  {Command::RanchIncubateEggNotify, "RanchIncubateEggNotify"},

  {Command::RanchUpdatePet, "RanchUpdatePet"},
  {Command::RanchUpdatePetCancel, "RanchUpdatePetCancel"},

  {Command::RanchRequestPetBirth, "RanchRequestPetBirth"},
  {Command::RanchRequestPetBirthOK, "RanchRequestPetBirthOK"},
  {Command::RanchRequestPetBirthCancel, "RanchRequestPetBirthCancel"},

  {Command::RanchPetBirthNotify, "RanchPetBirthNotify"},

  {Command::RanchAchievementUpdateProperty, "RanchAchievementUpdateProperty"},

  {Command::RanchHousingBuild, "RanchHousingBuild"},
  {Command::RanchHousingBuildOK, "RanchHousingBuildOK"},
  {Command::RanchHousingBuildCancel, "RanchHousingBuildCancel"},

  {Command::RanchHousingRepair, "RanchHousingRepair"},
  {Command::RanchHousingRepairOK, "RanchHousingRepairOK"},
  {Command::RanchHousingRepairCancel, "RanchHousingRepairCancel"},

  {Command::RanchMissionEvent, "RanchMissionEvent"},

  {Command::RanchOpCmd, "RanchOpCmd"},
  {Command::RanchOpCmdOK, "RanchOpCmdOK"},

  {Command::RaceEnterRoom, "RaceEnterRoom"},
  {Command::RaceEnterRoomNotify, "RaceEnterRoomNotify"},
  {Command::RaceEnterRoomOK, "RaceEnterRoomOK"},
  {Command::RaceEnterRoomCancel, "RaceEnterRoomCancel"},

  {Command::RaceChangeRoomOptions, "RaceChangeRoomOptions"},
  {Command::RaceChangeRoomOptionsNotify, "RaceChangeRoomOptionsNotify"},

  {Command::RaceStartRace, "RaceStartRace"},
  {Command::RaceStartRaceNotify, "RaceStartRaceNotify"},
  {Command::RaceStartRaceCancel, "RaceStartRaceCancel"},

  {Command::RaceUserRaceTimer, "UserRaceTimer"},
  {Command::RaceUserRaceTimerOK, "UserRaceTimerOK"},

  {Command::RaceLoadingComplete, "RaceLoadingComplete"},
  {Command::RaceLoadingCompleteNotify, "RaceLoadingCompleteNotify"},

  {Command::RaceChat, "RaceChat"},
  {Command::RaceChatNotify, "RaceChatNotify"},

  {Command::RaceReadyRace, "RaceReadyRace"},
  {Command::RaceReadyRaceNotify, "RaceReadyRaceNotify"},

  {Command::RaceCountdown, "RaceCountdown"},
};

} // namespace

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
  // `ResolveSceneLockBit & 0xFFFF`
  const uint16_t id = magic.id & 0xFFFF;
  const uint32_t length = BufferSize << 16 | magic.length;

  uint32_t encoded = length;
  encoded = (encoded & 0x3FFF | encoded << 14) & 0xFFFF;
  encoded = ((encoded & 0xF | 0xFF80) << 8 | length >> 4 & 0xFF | encoded & 0xF000) & 0xFFFF;
  encoded |= (encoded ^ id) << 16;
  return encoded;
}

std::string_view GetCommandName(Command command)
{
  const auto commandIter = commands.find(command);
  return commandIter == commands.cend() ? "n/a" : commandIter->second;
}

} // namespace server
