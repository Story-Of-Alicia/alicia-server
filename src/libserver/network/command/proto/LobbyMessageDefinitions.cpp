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

#include "libserver/network/command/proto/LobbyMessageDefinitions.hpp"

namespace server::protocol
{

void LobbyCommandLogin::Write(
  const LobbyCommandLogin& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandLogin::Read(
  LobbyCommandLogin& command,
  SourceStream& stream)
{
  stream.Read(command.constant0)
    .Read(command.constant1)
    .Read(command.loginId)
    .Read(command.memberNo)
    .Read(command.authKey)
    .Read(command.val0);
}

void LobbyCommandLoginOK::SystemContent::Write(const SystemContent& command, SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.values.size()));
  for (const auto& [key, value] : command.values)
  {
    stream.Write(key)
      .Write(value);
  }
}

void LobbyCommandLoginOK::SystemContent::Read(SystemContent& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandLoginOK::Write(
  const LobbyCommandLoginOK& command,
  SinkStream& stream)
{
  stream.Write(command.lobbyTime.dwLowDateTime)
    .Write(command.lobbyTime.dwHighDateTime)
    .Write(command.member0);

  // Profile
  stream.Write(command.uid)
    .Write(command.name)
    .Write(command.motd)
    .Write(static_cast<uint8_t>(command.gender))
    .Write(command.introduction);

  // Character equipment
  assert(command.characterEquipment.size() <= 16);
  const uint8_t characterEquipmentCount = std::min(
    command.characterEquipment.size(),
    size_t{16});

  stream.Write(characterEquipmentCount);
  for (size_t idx = 0; idx < characterEquipmentCount; ++idx)
  {
    const auto& item = command.characterEquipment[idx];
    stream.Write(item);
  }

  // Mount equipment
  assert(command.mountEquipment.size() <= 16);
  const uint8_t mountEquipmentCount = std::min(
    command.mountEquipment.size(),
    size_t{16});

  stream.Write(mountEquipmentCount);
  for (size_t idx = 0; idx < mountEquipmentCount; ++idx)
  {
    const auto& item = command.mountEquipment[idx];
    stream.Write(item);
  }

  //
  stream.Write(command.level)
    .Write(command.carrots)
    .Write(command.val1)
    .Write(command.role)
    .Write(command.val3);

  // Options
  // Option type mask
  const auto optionTypeMask = static_cast<uint32_t>(
    command.optionType);
  stream.Write(optionTypeMask);

  // Write the keyboard options if specified in the option type mask.
  if (optionTypeMask & static_cast<uint32_t>(OptionType::Keyboard))
  {
    const auto& keyboard = command.keyboardOptions;
    stream.Write(static_cast<uint8_t>(keyboard.bindings.size()));

    for (const auto& binding : keyboard.bindings)
    {
      stream.Write(binding.index)
        .Write(binding.type)
        .Write(binding.key);
    }
  }

  // Write the macro options if specified in the option type mask.
  if (optionTypeMask & static_cast<uint32_t>(OptionType::Macros))
  {
    const auto& macros = command.macroOptions;

    for (const auto& macro : macros.macros)
    {
      stream.Write(macro);
    }
  }

  // Write the value option if specified in the option type mask.
  if (optionTypeMask & static_cast<uint32_t>(OptionType::Value))
  {
    stream.Write(command.valueOptions);
  }

  // ToDo: Write the gamepad options.

  stream.Write(static_cast<uint8_t>(command.age))
    .Write(command.hideGenderAndAge);

  //
  stream.Write(static_cast<uint8_t>(command.missions.size()));
  for (const auto& val : command.missions)
  {
    stream.Write(val.id);

    stream.Write(static_cast<uint8_t>(val.progress.size()));
    for (const auto& nestedVal : val.progress)
    {
      stream.Write(nestedVal.id)
        .Write(nestedVal.value);
    }
  }

  stream.Write(command.val6);

  stream.Write(command.ranchAddress)
    .Write(command.ranchPort)
    .Write(command.scramblingConstant);

  stream.Write(command.character)
    .Write(command.horse);

  stream.Write(command.systemContent)
    .Write(command.bitfield);

  // Struct2
  const auto& struct1 = command.val9;
  stream.Write(struct1.val0)
    .Write(struct1.val1)
    .Write(struct1.val2);

  stream.Write(command.val10);

  const auto& managementSkills = command.managementSkills;
  stream.Write(managementSkills.val0)
    .Write(managementSkills.progress)
    .Write(managementSkills.points);

  const auto& skillRanks = command.skillRanks;
  stream.Write(
    static_cast<uint8_t>(skillRanks.values.size()));
  for (const auto& value : skillRanks.values)
  {
    stream.Write(value.id)
      .Write(value.rank);
  }

  const auto& struct4 = command.val13;
  stream.Write(
    static_cast<uint8_t>(struct4.values.size()));
  for (const auto& value : struct4.values)
  {
    stream.Write(value.val0)
      .Write(value.val1)
      .Write(value.val2);
  }

  stream.Write(command.val14);

  // Guild
  const auto& struct5 = command.guild;
  stream.Write(struct5.uid)
    .Write(struct5.val1)
    .Write(struct5.val2)
    .Write(struct5.name)
    .Write(struct5.val4)
    .Write(struct5.val5)
    .Write(struct5.val6);

  stream.Write(command.val16);

  // Rent
  const auto& struct6 = command.val17;
  stream.Write(struct6.mountUid)
    .Write(struct6.val1)
    .Write(struct6.val2);

  stream.Write(command.val18)
    .Write(command.val19)
    .Write(command.val20);

  // Pet
  stream.Write(command.pet);
}

void LobbyCommandLoginOK::Read(
  LobbyCommandLoginOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandLoginCancel::Write(
  const LobbyCommandLoginCancel& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.reason));
}

void LobbyCommandLoginCancel::Read(
  LobbyCommandLoginCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandShowInventory::Write(
  const LobbyCommandShowInventory& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandShowInventory::Read(
  LobbyCommandShowInventory& command,
  SourceStream& stream)
{
  // Empty,
}

void LobbyCommandShowInventoryOK::Write(
  const LobbyCommandShowInventoryOK& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.items.size()));
  for (const auto& item : command.items)
  {
    stream.Write(item);
  }

  stream.Write(static_cast<uint8_t>(command.horses.size()));
  for (const auto& horse : command.horses)
  {
    stream.Write(horse);
  }
}

void LobbyCommandShowInventoryOK::Read(
  LobbyCommandShowInventoryOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandCreateNicknameNotify::Write(
  const LobbyCommandCreateNicknameNotify& command,
  SinkStream& stream)
{
  // Empty.
}

void LobbyCommandCreateNicknameNotify::Read(
  LobbyCommandCreateNicknameNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandCreateNickname::Write(
  const LobbyCommandCreateNickname& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandCreateNickname::Read(
  LobbyCommandCreateNickname& command,
  SourceStream& stream)
{
  stream.Read(command.nickname)
    .Read(command.character)
    .Read(command.unk0);
}

void LobbyCommandCreateNicknameCancel::Write(
  const LobbyCommandCreateNicknameCancel& command,
  SinkStream& stream)
{
  stream.Write(command.error);
}

void LobbyCommandCreateNicknameCancel::Read(
  LobbyCommandCreateNicknameCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandShowInventoryCancel::Write(
  const LobbyCommandShowInventoryCancel& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandShowInventoryCancel::Read(
  LobbyCommandShowInventoryCancel& command,
  SourceStream& stream)
{
}

void LobbyCommandRequestLeagueInfo::Write(
  const LobbyCommandRequestLeagueInfo& command,
  SinkStream& stream)
{
}

void LobbyCommandRequestLeagueInfo::Read(
  LobbyCommandRequestLeagueInfo& command,
  SourceStream& stream)
{
}

void LobbyCommandRequestLeagueInfoOK::Write(
  const LobbyCommandRequestLeagueInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.season)
    .Write(command.league)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.rankingPercentile)
    .Write(command.unk5)
    .Write(command.unk6)
    .Write(command.unk7)
    .Write(command.unk8)
    .Write(command.leagueReward)
    .Write(command.place)
    .Write(command.rank)
    .Write(command.claimedReward)
    .Write(command.unk13);
}

void LobbyCommandRequestLeagueInfoOK::Read(
  LobbyCommandRequestLeagueInfoOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRequestLeagueInfoCancel::Write(
  const LobbyCommandRequestLeagueInfoCancel& command,
  SinkStream& stream)
{
}

void LobbyCommandRequestLeagueInfoCancel::Read(
  LobbyCommandRequestLeagueInfoCancel& command,
  SourceStream& stream)
{
}

void LobbyCommandAchievementCompleteList::Write(
  const LobbyCommandAchievementCompleteList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandAchievementCompleteList::Read(
  LobbyCommandAchievementCompleteList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
}

void LobbyCommandAchievementCompleteListOK::Write(
  const LobbyCommandAchievementCompleteListOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
  stream.Write(static_cast<uint16_t>(command.achievements.size()));
  for (const auto& achievement : command.achievements)
  {
    stream.Write(achievement);
  }
}

void LobbyCommandAchievementCompleteListOK::Read(
  LobbyCommandAchievementCompleteListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterChannel::Write(
  const LobbyCommandEnterChannel& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterChannel::Read(
  LobbyCommandEnterChannel& command,
  SourceStream& stream)
{
  stream.Read(command.channel);
}

void LobbyCommandEnterChannelOK::Write(
  const LobbyCommandEnterChannelOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void LobbyCommandEnterChannelOK::Read(
  LobbyCommandEnterChannelOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterChannelCancel::Write(
  const LobbyCommandEnterChannelCancel& command,
  SinkStream& stream)
{
}

void LobbyCommandEnterChannelCancel::Read(
  LobbyCommandEnterChannelCancel& command,
  SourceStream& stream)
{
}

void LobbyCommandRoomList::Write(
  const LobbyCommandRoomList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRoomList::Read(
  LobbyCommandRoomList& command,
  SourceStream& stream)
{
  stream.Read(command.page)
    .Read(command.gameMode)
    .Read(command.teamMode);
}

void LobbyCommandRoomListOK::Room::Write(
  const Room& value,
  SinkStream& stream)
{
  stream.Write(value.id)
    .Write(value.name)
    .Write(value.playerCount)
    .Write(value.maxPlayers)
    .Write(value.isLocked)
    .Write(value.unk0)
    .Write(value.unk1)
    .Write(value.map)
    .Write(value.hasStarted)
    .Write(value.unk2)
    .Write(value.unk3)
    .Write(value.level)
    .Write(value.unk4);
}

void LobbyCommandRoomListOK::Room::Read(
  Room& value,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRoomListOK::Write(
  const LobbyCommandRoomListOK& command,
  SinkStream& stream)
{
  stream.Write(command.page)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(static_cast<uint8_t>(command.rooms.size()));
  for (const auto& room : command.rooms)
  {
    stream.Write(room);
  }
  stream.Write(command.unk3.unk0)
    .Write(command.unk3.unk1)
    .Write(command.unk3.unk2);
}

void LobbyCommandRoomListOK::Read(
  LobbyCommandRoomListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandMakeRoom::Write(
  const LobbyCommandMakeRoom& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandMakeRoom::Read(
  LobbyCommandMakeRoom& command,
  SourceStream& stream)
{
  stream.Read(command.name)
    .Read(command.password)
    .Read(command.playerCount)
    .Read(command.gameMode)
    .Read(command.teamMode)
    .Read(command.missionId)
    .Read(command.unk3)
    .Read(command.bitset)
    .Read(command.unk4);
}

void LobbyCommandMakeRoomOK::Write(
  const LobbyCommandMakeRoomOK& command,
  SinkStream& stream)
{
  stream.Write(command.roomUid)
    .Write(command.otp)
    .Write(htonl(command.address))
    .Write(command.port)
    .Write(command.unk2);
}

void LobbyCommandMakeRoomOK::Read(
  LobbyCommandMakeRoomOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandMakeRoomCancel::Write(
  const LobbyCommandMakeRoomCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}

void LobbyCommandMakeRoomCancel::Read(
  LobbyCommandMakeRoomCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterRoom::Write(
  const LobbyCommandEnterRoom& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterRoom::Read(
  LobbyCommandEnterRoom& command,
  SourceStream& stream)
{
  stream.Read(command.roomUid)
    .Read(command.password)
    .Read(command.member3);
}

void LobbyCommandEnterRoomOK::Write(
  const LobbyCommandEnterRoomOK& command,
  SinkStream& stream)
{
  stream.Write(command.roomUid)
    .Write(command.otp)
    .Write(htonl(command.address))
    .Write(command.port)
    .Write(command.member6);
}

void LobbyCommandEnterRoomOK::Read(
  LobbyCommandEnterRoomOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterRoomCancel::Write(
  const LobbyCommandEnterRoomCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void LobbyCommandEnterRoomCancel::Read(
  LobbyCommandEnterRoomCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRequestQuestList::Write(
  const LobbyCommandRequestQuestList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRequestQuestList::Read(
  LobbyCommandRequestQuestList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
}

void LobbyCommandRequestQuestListOK::Write(
  const LobbyCommandRequestQuestListOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
  stream.Write(static_cast<uint16_t>(command.quests.size()));
  for (const auto& quest : command.quests)
  {
    stream.Write(quest);
  }
}

void LobbyCommandRequestQuestListOK::Read(
  LobbyCommandRequestQuestListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRequestDailyQuestList::Write(
  const LobbyCommandRequestDailyQuestList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRequestDailyQuestList::Read(
  LobbyCommandRequestDailyQuestList& command,
  SourceStream& stream)
{
  stream.Read(command.val0);
}

void LobbyCommandRequestDailyQuestListOK::Write(
  const LobbyCommandRequestDailyQuestListOK& command,
  SinkStream& stream)
{
  stream.Write(command.val0);
  stream.Write(static_cast<uint16_t>(command.quests.size()));

  for (const auto& quest : command.quests)
  {
    stream.Write(quest);
  }
  stream.Write(
    static_cast<uint16_t>(command.val1.size()));
  for (const auto& entry : command.val1)
  {
    stream.Write(entry.val0)
      .Write(entry.val1)
      .Write(entry.val2)
      .Write(entry.val3);
  }
}

void LobbyCommandRequestDailyQuestListOK::Read(
  LobbyCommandRequestDailyQuestListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterRanch::Write(
  const LobbyCommandEnterRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterRanch::Read(
  LobbyCommandEnterRanch& command,
  SourceStream& stream)
{
  stream.Read(command.rancherUid)
    .Read(command.unk1)
    .Read(command.unk2);
}

void LobbyCommandEnterRanchOK::Write(
  const LobbyCommandEnterRanchOK& command,
  SinkStream& stream)
{
  stream.Write(command.rancherUid)
    .Write(command.otp)
    .Write(htonl(command.ranchAddress))
    .Write(command.ranchPort);
}

void LobbyCommandEnterRanchOK::Read(
  LobbyCommandEnterRanchOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandEnterRanchCancel::Write(
  const LobbyCommandEnterRanchCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}

void LobbyCommandEnterRanchCancel::Read(
  LobbyCommandEnterRanchCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandGetMessengerInfo::Write(
  const LobbyCommandGetMessengerInfo& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandGetMessengerInfo::Read(
  LobbyCommandGetMessengerInfo& command,
  SourceStream& stream)
{
  // Empty.
}

void LobbyCommandGetMessengerInfoOK::Write(
  const LobbyCommandGetMessengerInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.code)
    .Write(command.ip)
    .Write(command.port);
}

void LobbyCommandGetMessengerInfoOK::Read(
  LobbyCommandGetMessengerInfoOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandGetMessengerInfoCancel::Write(
  const LobbyCommandGetMessengerInfoCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void LobbyCommandGetMessengerInfoCancel::Read(
  LobbyCommandGetMessengerInfoCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}
void LobbyCommandRequestSpecialEventList::Write(
  const LobbyCommandRequestSpecialEventList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandRequestSpecialEventList::Read(
  LobbyCommandRequestSpecialEventList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
}

void LobbyCommandRequestSpecialEventListOK::Write(
  const LobbyCommandRequestSpecialEventListOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);

  stream.Write(static_cast<uint16_t>(command.quests.size()));
  for (const auto& quest : command.quests)
  {
    stream.Write(quest);
  }

  stream.Write(static_cast<uint16_t>(command.events.size()));
  for (const auto& event : command.events)
  {
    stream.Write(event.unk0)
      .Write(event.unk1);
  }
}

void LobbyCommandRequestSpecialEventListOK::Read(
  LobbyCommandRequestSpecialEventListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLHeartbeat::Write(
  const AcCmdCLHeartbeat& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLHeartbeat::Read(
  AcCmdCLHeartbeat& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCLGoodsShopList::Write(
  const AcCmdCLGoodsShopList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLGoodsShopList::Read(
  AcCmdCLGoodsShopList& command,
  SourceStream& stream)
{
  for (auto& data : command.data)
  {
    stream.Read(data);
  }
}

void AcCmdCLGoodsShopListOK::Write(
  const AcCmdCLGoodsShopListOK& command,
  SinkStream& stream)
{
  for (const auto& data : command.data)
  {
    stream.Write(data);
  }
}

void AcCmdCLGoodsShopListOK::Read(
  AcCmdCLGoodsShopListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLGoodsShopListCancel::Write(
  const AcCmdCLGoodsShopListCancel&
    command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLGoodsShopListCancel::Read(
  AcCmdCLGoodsShopListCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdLCGoodsShopListData::Write(
  const AcCmdLCGoodsShopListData& command,
  SinkStream& stream)
{
  for (const auto& b : command.member1)
  {
    stream.Write(b);
  }

  stream.Write(command.member2)
    .Write(command.member3);

  stream.Write(static_cast<uint32_t>(command.data.size()));
  for (const auto & b : command.data)
  {
    stream.Write(b);
  }
}

void AcCmdLCGoodsShopListData::Read(
  AcCmdLCGoodsShopListData& command,
  SourceStream& stream)
{
}

void AcCmdCLInquiryTreecash::Write(
  const AcCmdCLInquiryTreecash& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLInquiryTreecash::Read(
  AcCmdCLInquiryTreecash& command,
  SourceStream& stream)
{
  // Empty.
}

void LobbyCommandInquiryTreecashOK::Write(
  const LobbyCommandInquiryTreecashOK& command,
  SinkStream& stream)
{
  stream.Write(command.cash);
}

void LobbyCommandInquiryTreecashOK::Read(
  LobbyCommandInquiryTreecashOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandInquiryTreecashCancel::Write(
  const LobbyCommandInquiryTreecashCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void LobbyCommandInquiryTreecashCancel::Read(
  LobbyCommandInquiryTreecashCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandClientNotify::Write(
  const LobbyCommandClientNotify& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandClientNotify::Read(
  LobbyCommandClientNotify& command,
  SourceStream& stream)
{
  stream.Read(command.val0).Read(command.val1);
}

void LobbyCommandGuildPartyList::Write(
  const LobbyCommandGuildPartyList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandGuildPartyList::Read(
  LobbyCommandGuildPartyList& command,
  SourceStream& stream)
{
  // Empty.
}

void LobbyCommandGuildPartyListOK::Write(
  const LobbyCommandGuildPartyListOK& command,
  SinkStream& stream)
{
  assert(command.members.empty());
  // todo: Write members
  stream.Write(static_cast<uint8_t>(command.members.size()));
}

void LobbyCommandGuildPartyListOK::Read(
  LobbyCommandGuildPartyListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandEnterRandomRanch::Write(
  const LobbyCommandEnterRandomRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandEnterRandomRanch::Read(
  LobbyCommandEnterRandomRanch& command,
  SourceStream& stream)
{
  // Empty.
}

void LobbyCommandRequestPersonalInfo::Write(
  const LobbyCommandRequestPersonalInfo& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandRequestPersonalInfo::Read(
  LobbyCommandRequestPersonalInfo& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.type);
}

void LobbyCommandPersonalInfo::Basic::Write(const Basic& command, SinkStream& stream)
{
  stream.Write(command.distanceTravelled)
    .Write(command.topSpeed)
    .Write(command.longestGlidingDistance)
    .Write(command.jumpSuccessRate)
    .Write(command.perfectJumpSuccessRate)
    .Write(command.speedSingleWinCombo)
    .Write(command.speedTeamWinCombo)
    .Write(command.magicSingleWinCombo)
    .Write(command.magicTeamWinCombo)
    .Write(command.averageRank)
    .Write(command.completionRate)
    .Write(command.member12)
    .Write(command.highestCarnivalPrize)
    .Write(command.member14)
    .Write(command.member15)
    .Write(command.member16)
    .Write(command.introduction)
    .Write(command.level)
    .Write(command.levelProgress)
    .Write(command.member20)
    .Write(command.perfectBoostCombo)
    .Write(command.perfectJumpCombo)
    .Write(command.magicDefenseCombo)
    .Write(command.member24)
    .Write(command.member25)
    .Write(command.member26)
    .Write(command.guildName)
    .Write(command.member28)
    .Write(command.member29);
}

void LobbyCommandPersonalInfo::Basic::Read(Basic& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandPersonalInfo::CourseInformation::Write(const CourseInformation& command, SinkStream& stream)
{
  stream.Write(command.totalGames)
    .Write(command.totalSpeedGames)
    .Write(command.totalMagicGames);

  stream.Write(static_cast<uint8_t>(command.courses.size()));
  for (const auto& entry : command.courses)
  {
    stream.Write(entry.courseId)
      .Write(entry.timesRaced)
      .Write(entry.recordTime);

    for (const auto& byte : entry.member4)
    {
      stream.Write(byte);
    }
  }
}

void LobbyCommandPersonalInfo::CourseInformation::Read(CourseInformation& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandPersonalInfo::Eight::Write(const Eight& command, SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.member1.size()));
  for (const auto& entry : command.member1)
  {
    stream.Write(entry.member1)
      .Write(entry.member2);
  }
}

void LobbyCommandPersonalInfo::Eight::Read(Eight& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandPersonalInfo::Write(const LobbyCommandPersonalInfo& command, SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.type);

  switch (command.type)
  {
    case LobbyCommandRequestPersonalInfo::Type::Basic:
      {
        stream.Write(command.basic);
        break;
      }
    case LobbyCommandRequestPersonalInfo::Type::Courses:
      {
        stream.Write(command.courseInformation);
        break;
      }
    case LobbyCommandRequestPersonalInfo::Type::Eight:
      {
        stream.Write(command.eight);
        break;
      }
  }
}

void LobbyCommandPersonalInfo::Read(
  LobbyCommandPersonalInfo& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandSetIntroduction::Write(
  const LobbyCommandSetIntroduction& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandSetIntroduction::Read(
  LobbyCommandSetIntroduction& command,
  SourceStream& stream)
{
  stream.Read(command.introduction);
}

void LobbyCommandUpdateSystemContent::Write(
  const LobbyCommandUpdateSystemContent& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandUpdateSystemContent::Read(
  LobbyCommandUpdateSystemContent& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.key)
    .Read(command.value);
}

void LobbyCommandUpdateSystemContentNotify::Write(
  const LobbyCommandUpdateSystemContentNotify& command,
  SinkStream& stream)
{
  stream.Write(command.systemContent);
}

void LobbyCommandUpdateSystemContentNotify::Read(
  LobbyCommandUpdateSystemContentNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandChangeRanchOption::Read(
  LobbyCommandChangeRanchOption& command,
  SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2);
}

void LobbyCommandChangeRanchOptionOK::Write(
  const LobbyCommandChangeRanchOptionOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2);
}

void AcCmdLCOpKick::Write(const AcCmdLCOpKick& command, SinkStream& stream)
{
  // Empty.
}

void AcCmdLCOpKick::Read(AcCmdLCOpKick& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdLCOpMute::Write(const AcCmdLCOpMute& command, SinkStream& stream)
{
  stream.Write(command.duration);
}

void AcCmdLCOpMute::Read(AcCmdLCOpMute& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdLCNotice::Write(const AcCmdLCNotice& command, SinkStream& stream)
{
  stream.Write(command.notice);
}

void AcCmdLCNotice::Read(AcCmdLCNotice& command, SourceStream& stream)
{
    throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestMountInfo::Write(
  const AcCmdCLRequestMountInfo& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestMountInfo::Read(
  AcCmdCLRequestMountInfo& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid);
}

void AcCmdCLRequestMountInfoOK::Write(
  const AcCmdCLRequestMountInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid);
  stream.Write(static_cast<uint8_t>(command.mountInfos.size()));
  for (const auto& mountInfo : command.mountInfos)
  {
    stream.Write(mountInfo.horseUid)
      .Write(mountInfo.boostsInARow)
      .Write(mountInfo.winsSpeedSingle)
      .Write(mountInfo.winsSpeedTeam)
      .Write(mountInfo.winsMagicSingle)
      .Write(mountInfo.winsMagicTeam)
      .Write(mountInfo.totalDistance)
      .Write(mountInfo.topSpeed)
      .Write(mountInfo.longestGlideDistance)
      .Write(mountInfo.participated)
      .Write(mountInfo.cumulativePrize)
      .Write(mountInfo.biggestPrize);
  }
}

} // namespace server::protocol
