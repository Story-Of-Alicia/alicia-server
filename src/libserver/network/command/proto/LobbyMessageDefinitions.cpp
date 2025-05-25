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

namespace alicia
{

void LobbyCommandLogin::Write(
  const LobbyCommandLogin& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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

void LobbyCommandLoginOK::Write(
  const LobbyCommandLoginOK& command,
  SinkStream& stream)
{
  stream.Write(command.lobbyTime.dwLowDateTime)
    .Write(command.lobbyTime.dwHighDateTime)
    .Write(command.val0);

  // Profile
  stream.Write(command.selfUid)
    .Write(command.nickName)
    .Write(command.motd)
    .Write(static_cast<uint8_t>(command.profileGender))
    .Write(command.status);

  // Character equipment
  stream.Write(static_cast<uint8_t>(command.characterEquipment.size()));
  for (const Item& item : command.characterEquipment)
  {
    stream.Write(item);
  }

  // Horse equipment
  stream.Write(static_cast<uint8_t>(command.horseEquipment.size()));
  for (const Item& item : command.horseEquipment)
  {
    stream.Write(item);
  }

  //
  stream.Write(command.level)
    .Write(command.carrots)
    .Write(command.val1)
    .Write(command.val2)
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

  stream.Write(static_cast<uint8_t>(command.ageGroup))
    .Write(command.val4);

  //
  stream.Write(static_cast<uint8_t>(command.val5.size()));
  for (const auto& val : command.val5)
  {
    stream.Write(val.val0);

    stream.Write(static_cast<uint8_t>(val.val1.size()));
    for (const auto& nestedVal : val.val1)
    {
      stream.Write(nestedVal.val1)
        .Write(nestedVal.val2);
    }
  }

  //
  stream.Write(command.val6);

  // Write server info.
  stream.Write(command.address)
    .Write(command.port)
    .Write(command.scramblingConstant);

  stream.Write(command.character)
    .Write(command.horse);

  // Struct1
  const auto& struct0 = command.val7;
  stream.Write(
    static_cast<uint8_t>(struct0.values.size()));
  for (const auto& value : struct0.values)
  {
    stream.Write(value.val0)
      .Write(value.val1);
  }

  stream.Write(command.val8);

  // Struct2
  const auto& struct1 = command.val9;
  stream.Write(struct1.val0)
    .Write(struct1.val1)
    .Write(struct1.val2);

  stream.Write(command.val10);

  const auto& struct2 = command.val11;
  stream.Write(struct2.val0)
    .Write(struct2.val1)
    .Write(struct2.val2);

  // Struct3
  const auto& struct3 = command.val12;
  stream.Write(
    static_cast<uint8_t>(struct3.values.size()));
  for (const auto& value : struct3.values)
  {
    stream.Write(value.val0)
      .Write(value.val1);
  }

  // Struct4
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

  // Struct5
  const auto& struct5 = command.val15;
  stream.Write(struct5.val0)
    .Write(struct5.val1)
    .Write(struct5.val2)
    .Write(struct5.val3)
    .Write(struct5.val4)
    .Write(struct5.val5)
    .Write(struct5.val6);

  stream.Write(command.val16);

  // Struct6
  const auto& struct6 = command.val17;
  stream.Write(struct6.mountUid)
    .Write(struct6.val1)
    .Write(struct6.val2);

  stream.Write(command.val18)
    .Write(command.val19)
    .Write(command.val20);

  // Struct7
  const auto& struct7 = command.val21;
  stream.Write(struct7.val0)
    .Write(struct7.val1)
    .Write(struct7.val2)
    .Write(struct7.val3);
}

void LobbyCommandLoginOK::Read(
  LobbyCommandLoginOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
}

void LobbyCommandShowInventory::Write(
  const LobbyCommandShowInventory& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
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

void LobbyCommandCreateNicknameOK::Write(
  const LobbyCommandCreateNicknameOK& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandCreateNicknameOK::Read(
  LobbyCommandCreateNicknameOK& command,
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
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.unk4)
    .Write(command.unk5)
    .Write(command.unk6)
    .Write(command.unk7)
    .Write(command.unk8)
    .Write(command.unk9)
    .Write(command.unk10)
    .Write(command.unk11)
    .Write(command.unk12)
    .Write(command.unk13);
}

void LobbyCommandRequestLeagueInfoOK::Read(
  LobbyCommandRequestLeagueInfoOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
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
    stream.Write(achievement.unk0)
      .Write(achievement.unk1)
      .Write(achievement.unk2)
      .Write(achievement.unk3)
      .Write(achievement.unk4);
  }
}

void LobbyCommandAchievementCompleteListOK::Read(
  LobbyCommandAchievementCompleteListOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterChannel::Write(
  const LobbyCommandEnterChannel& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
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

void LobbyCommandMakeRoom::Write(
  const LobbyCommandMakeRoom& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandMakeRoom::Read(
  LobbyCommandMakeRoom& command,
  SourceStream& stream)
{
  stream.Read(command.name)
    .Read(command.description)
    .Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2)
    .Read(command.missionId)
    .Read(command.unk4)
    .Read(command.bitset)
    .Read(command.unk6);
}

void LobbyCommandMakeRoomOK::Write(
  const LobbyCommandMakeRoomOK& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.otp)
    .Write(command.ip)
    .Write(command.port)
    .Write(command.unk2);
}

void LobbyCommandMakeRoomOK::Read(
  LobbyCommandMakeRoomOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestQuestList::Write(
  const LobbyCommandRequestQuestList& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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
    stream.Write(quest.unk0)
      .Write(quest.unk1)
      .Write(quest.unk2)
      .Write(quest.unk3)
      .Write(quest.unk4);
  }
}

void LobbyCommandRequestQuestListOK::Read(
  LobbyCommandRequestQuestListOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestDailyQuestList::Write(
  const LobbyCommandRequestDailyQuestList& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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
    stream.Write(quest.unk0)
      .Write(quest.unk1)
      .Write(quest.unk2)
      .Write(quest.unk3)
      .Write(quest.unk4);
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
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterRanch::Write(
  const LobbyCommandEnterRanch& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterRanch::Read(
  LobbyCommandEnterRanch& command,
  SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2);
}

void LobbyCommandEnterRanchOK::Write(
  const LobbyCommandEnterRanchOK& command,
  SinkStream& stream)
{
  stream.Write(command.ranchUid)
    .Write(command.code)
    .Write(command.ip)
    .Write(command.port);
}

void LobbyCommandEnterRanchOK::Read(
  LobbyCommandEnterRanchOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
}

void LobbyCommandGetMessengerInfo::Write(
  const LobbyCommandGetMessengerInfo& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRoomList::Write(
  const LobbyCommandRoomList& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRoomList::Read(
  LobbyCommandRoomList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2);
}

void LobbyCommandRoomListOK::Write(
  const LobbyCommandRoomListOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(static_cast<uint8_t>(command.rooms.size()));
  for (const auto& room : command.rooms)
  {
    stream.Write(room.id)
      .Write(room.name)
      .Write(room.playerCount)
      .Write(room.maxPlayers)
      .Write(room.isLocked)
      .Write(room.unk0)
      .Write(room.unk1)
      .Write(room.map)
      .Write(room.hasStarted)
      .Write(room.unk2)
      .Write(room.unk3)
      .Write(room.level)
      .Write(room.unk4);
  }
  stream.Write(command.unk3.unk0)
    .Write(command.unk3.unk1)
    .Write(command.unk3.unk2);
}

void LobbyCommandRoomListOK::Read(
  LobbyCommandRoomListOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestSpecialEventList::Write(
  const LobbyCommandRequestSpecialEventList& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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

  stream.Write(static_cast<uint16_t>(command.unk1.size()));
  for (const auto& unk1Element : command.unk1)
  {
    stream.Write(unk1Element.unk0)
      .Write(unk1Element.unk1)
      .Write(unk1Element.unk2)
      .Write(unk1Element.unk3)
      .Write(unk1Element.unk4);
  }

  stream.Write(static_cast<uint16_t>(command.unk2.size()));
  for (const auto& unk2Element : command.unk2)
  {
    stream.Write(unk2Element.unk0)
      .Write(unk2Element.unk1);
  }
}

void LobbyCommandRequestSpecialEventListOK::Read(
  LobbyCommandRequestSpecialEventListOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandHeartbeat::Write(
  const LobbyCommandHeartbeat& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandHeartbeat::Read(
  LobbyCommandHeartbeat& command,
  SourceStream& stream)
{
  // Empty.
}

void LobbyCommandGoodsShopList::Write(
  const LobbyCommandGoodsShopList& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandGoodsShopList::Read(
  LobbyCommandGoodsShopList& command,
  SourceStream& stream)
{
  for (auto& data : command.data)
  {
    stream.Read(data);
  }
}

void LobbyCommandGoodsShopListOK::Write(
  const LobbyCommandGoodsShopListOK& command,
  SinkStream& stream)
{
  for (const auto& data : command.data)
  {
    stream.Write(data);
  }
}

void LobbyCommandGoodsShopListOK::Read(
  LobbyCommandGoodsShopListOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandGoodsShopListCancel::Write(
  const LobbyCommandGoodsShopListCancel&
    command,
  SinkStream& stream)
{
  // Empty.
}

void LobbyCommandGoodsShopListCancel::Read(
  LobbyCommandGoodsShopListCancel& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandInquiryTreecash::Write(
  const LobbyCommandInquiryTreecash& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandInquiryTreecash::Read(
  LobbyCommandInquiryTreecash& command,
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
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
}

void LobbyCommandClientNotify::Write(
  const LobbyCommandClientNotify& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented.");
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
  throw std::logic_error("Not implemented");
}

} // namespace alicia
