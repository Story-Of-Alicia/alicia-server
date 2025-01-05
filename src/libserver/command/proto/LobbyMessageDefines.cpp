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

#include "libserver/command/proto/LobbyMessageDefines.hpp"

#include <chrono>

namespace alicia
{

//! Writes item data to the buffer.
//! @param buf Sink buffer.
//! @param item Item data to write.
void WriteItem(SinkStream& buf, const Item& item)
{
  buf.Write(item.uid)
    .Write(item.tid)
    .Write(item.val)
    .Write(item.count);
}

//! Writes character data to the buffer.
//! @param buf Sink buffer.
//! @param character Character data to write.
void WriteCharacter(
  SinkStream& buf,
  const Character& character)
{
  const auto& [parts, appearance] = character;

  // Write the character parts.
  buf.Write(parts.charId)
    .Write(parts.mouthSerialId)
    .Write(parts.faceSerialId)
    .Write(parts.val0);

  // Write the character appearance.
  buf.Write(appearance.val0)
    .Write(appearance.headSize)
    .Write(appearance.height)
    .Write(appearance.thighVolume)
    .Write(appearance.legVolume)
    .Write(appearance.val1);
}

//! Reads character data from the buffer.
//! @param buf Source buffer.
//! @param character Character data to read.
void ReadCharacter(
  SourceStream& buf,
  Character& character)
{
  auto& [parts, appearance] = character;

  // Write the character parts.
  buf.Read(parts.charId)
    .Read(parts.mouthSerialId)
    .Read(parts.faceSerialId)
    .Read(parts.val0);

  // Write the character appearance.
  buf.Read(appearance.val0)
    .Read(appearance.headSize)
    .Read(appearance.height)
    .Read(appearance.thighVolume)
    .Read(appearance.legVolume)
    .Read(appearance.val1);
}

//! Writes horse data to the buffer.
//! @param buf Sink buffer.
//! @param horse Horse data to write.
void WriteHorse(
  SinkStream& buf,
  const Horse& horse)
{
  // Horse identifiers.
  buf.Write(horse.uid)
    .Write(horse.tid)
    .Write(horse.name);

  // Horse parts.
  const auto& parts = horse.parts;
  buf.Write(parts.skinId)
    .Write(parts.maneId)
    .Write(parts.tailId)
    .Write(parts.faceId);

  // Horse appearance.
  const auto& appearance = horse.appearance;
  buf.Write(appearance.scale)
    .Write(appearance.legLength)
    .Write(appearance.legVolume)
    .Write(appearance.bodyLength)
    .Write(appearance.bodyVolume);

  // Horse stats.
  const auto& stats = horse.stats;
  buf.Write(stats.agility)
    .Write(stats.spirit)
    .Write(stats.speed)
    .Write(stats.strength)
    .Write(stats.ambition);

  buf.Write(horse.rating)
    .Write(horse.clazz)
    .Write(horse.val0)
    .Write(horse.grade)
    .Write(horse.growthPoints);

  const auto& vals0 = horse.vals0;
  buf.Write(vals0.stamina)
    .Write(vals0.attractiveness)
    .Write(vals0.hunger)
    .Write(vals0.val0)
    .Write(vals0.val1)
    .Write(vals0.val2)
    .Write(vals0.val3)
    .Write(vals0.val4)
    .Write(vals0.val5)
    .Write(vals0.val6)
    .Write(vals0.val7)
    .Write(vals0.val8)
    .Write(vals0.val9)
    .Write(vals0.val10);

  const auto& vals1 = horse.vals1;
  buf.Write(vals1.val0)
    .Write(vals1.val1)
    .Write(vals1.val2)
    .Write(vals1.val3)
    .Write(vals1.val4)
    .Write(vals1.classProgression)
    .Write(vals1.val5)
    .Write(vals1.val6)
    .Write(vals1.val7)
    .Write(vals1.val8)
    .Write(vals1.val9)
    .Write(vals1.val10)
    .Write(vals1.val11)
    .Write(vals1.val12)
    .Write(vals1.val13)
    .Write(vals1.val14)
    .Write(vals1.val15);

  // Horse mastery.
  const auto& mastery = horse.mastery;
  buf.Write(mastery.magic)
    .Write(mastery.jumping)
    .Write(mastery.sliding)
    .Write(mastery.gliding);

  buf.Write(horse.val16)
    .Write(horse.val17);
}

void LobbyCommandLogin::Write(
  const LobbyCommandLogin& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandLogin::Read(
  LobbyCommandLogin& command, SourceStream& buffer)
{
  buffer.Read(command.constant0)
    .Read(command.constant1)
    .Read(command.loginId)
    .Read(command.memberNo)
    .Read(command.authKey)
    .Read(command.val0);
}

void LobbyCommandLoginOK::Write(
  const LobbyCommandLoginOK& command, SinkStream& buffer)
{
  buffer.Write(command.lobbyTime.dwLowDateTime)
    .Write(command.lobbyTime.dwHighDateTime)
    .Write(command.val0);

  // Profile
  buffer.Write(command.selfUid)
    .Write(command.nickName)
    .Write(command.motd)
    .Write(static_cast<uint8_t>(command.profileGender))
    .Write(command.status);

  // Character equipment
  buffer.Write(static_cast<uint8_t>(
    command.characterEquipment.size()));
  for (const Item& item : command.characterEquipment)
  {
    WriteItem(buffer, item);
  }

  // Horse equipment
  buffer.Write(static_cast<uint8_t>(
    command.horseEquipment.size()));
  for (const Item& item : command.horseEquipment)
  {
    WriteItem(buffer, item);
  }

  //
  buffer.Write(command.level)
    .Write(command.carrots)
    .Write(command.val1)
    .Write(command.val2)
    .Write(command.val3);

  // Options
  // Option type mask
  const auto optionTypeMask = static_cast<uint32_t>(
    command.optionType);
  buffer.Write(optionTypeMask);

  // Write the keyboard options if specified in the option type mask.
  if (optionTypeMask & static_cast<uint32_t>(OptionType::Keyboard))
  {
    const auto& keyboard = command.keyboardOptions;
    buffer.Write(static_cast<uint8_t>(
      keyboard.bindings.size()));

    for (const auto& binding : keyboard.bindings)
    {
      buffer.Write(binding.index)
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
      buffer.Write(macro);
    }
  }

  // Write the value option if specified in the option type mask.
  if (optionTypeMask & static_cast<uint32_t>(OptionType::Value))
  {
    buffer.Write(command.valueOptions);
  }

  // ToDo: Write the gamepad options.

  buffer.Write(static_cast<uint8_t>(command.ageGroup))
    .Write(command.val4);

  //
  buffer.Write(static_cast<uint8_t>(command.val5.size()));
  for (const auto& val : command.val5)
  {
    buffer.Write(val.val0);

    buffer.Write(static_cast<uint8_t>(val.val1.size()));
    for (const auto& nestedVal : val.val1)
    {
      buffer.Write(nestedVal.val1)
            .Write(nestedVal.val2);
    }
  }

  //
  buffer.Write(command.val6);

  // Write server info.
  buffer.Write(command.address)
    .Write(command.port)
    .Write(command.scramblingConstant);

  WriteCharacter(buffer, command.character);
  WriteHorse(buffer, command.horse);

  // Struct1
  const auto& struct0 = command.val7;
  buffer.Write(
    static_cast<uint8_t>(struct0.values.size()));
  for (const auto& value : struct0.values)
  {
    buffer.Write(value.val0)
      .Write(value.val1);
  }

  buffer.Write(command.val8);

  // Struct2
  const auto& struct1 = command.val9;
  buffer.Write(struct1.val0)
    .Write(struct1.val1)
    .Write(struct1.val2);

  buffer.Write(command.val10);

  const auto& struct2 = command.val11;
  buffer.Write(struct2.val0)
    .Write(struct2.val1)
    .Write(struct2.val2);

  // Struct3
  const auto& struct3 = command.val12;
  buffer.Write(
    static_cast<uint8_t>(struct3.values.size()));
  for (const auto& value : struct3.values)
  {
    buffer.Write(value.val0)
      .Write(value.val1);
  }

  // Struct4
  const auto& struct4 = command.val13;
  buffer.Write(
    static_cast<uint8_t>(struct4.values.size()));
  for (const auto& value : struct4.values)
  {
    buffer.Write(value.val0)
      .Write(value.val1)
      .Write(value.val2);
  }

  buffer.Write(command.val14);

  // Struct5
  const auto& struct5 = command.val15;
  buffer.Write(struct5.val0)
    .Write(struct5.val1)
    .Write(struct5.val2)
    .Write(struct5.val3)
    .Write(struct5.val4)
    .Write(struct5.val5)
    .Write(struct5.val6);

  buffer.Write(command.val16);

  // Struct6
  const auto& struct6 = command.val17;
  buffer.Write(struct6.mountUid)
    .Write(struct6.val1)
    .Write(struct6.val2);

  buffer.Write(command.val18)
    .Write(command.val19)
    .Write(command.val20);

  // Struct7
  const auto& struct7 = command.val21;
  buffer.Write(struct7.val0)
    .Write(struct7.val1)
    .Write(struct7.val2)
    .Write(struct7.val3);
}

void LobbyCommandLoginOK::Read(
  LobbyCommandLoginOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandLoginCancel::Write(
  const LobbyCommandLoginCancel& command,
  SinkStream& buffer)
{
  buffer.Write(static_cast<uint8_t>(command.reason));
}

void LobbyCommandLoginCancel::Read(
  LobbyCommandLoginCancel& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandShowInventory::Write(
  const LobbyCommandShowInventory& command,
  SinkStream& buffer)
{
}

void LobbyCommandShowInventory::Read(
  LobbyCommandShowInventory& command,
  SourceStream& buffer)
{
}

void LobbyCommandShowInventoryOK::Write(
  const LobbyCommandShowInventoryOK& command,
  SinkStream& buffer)
{
  buffer.Write(static_cast<uint8_t>(command.items.size()));
  for (const auto& item : command.items)
  {
    WriteItem(buffer, item);
  }

  buffer.Write(static_cast<uint8_t>(command.horses.size()));
  for (const auto& horse : command.horses)
  {
    WriteHorse(buffer, horse);
  }
}

void LobbyCommandShowInventoryOK::Read(
  LobbyCommandShowInventoryOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandCreateNicknameNotify::Write(
  const LobbyCommandCreateNicknameNotify& command, SinkStream& buffer)
{
  // Empty.
}

void LobbyCommandCreateNicknameNotify::Read(
  LobbyCommandCreateNicknameNotify& command, SourceStream& buffer)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandCreateNicknameOK::Write(
  const LobbyCommandCreateNicknameOK& command,
  SinkStream& buffer)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandCreateNicknameOK::Read(
  LobbyCommandCreateNicknameOK& command,
  SourceStream& buffer)
{
  buffer.Read(command.nickname);
  ReadCharacter(buffer, command.character);
  buffer.Read(command.unk0);
}

void LobbyCommandCreateNicknameCancel::Write(
  const LobbyCommandCreateNicknameCancel& command,
  SinkStream& buffer)
{
  buffer.Write(command.error);
}

void LobbyCommandCreateNicknameCancel::Read(
  LobbyCommandCreateNicknameCancel& command,
  SourceStream& buffer)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandShowInventoryCancel::Write(
  const LobbyCommandShowInventoryCancel& command,
  SinkStream& buffer)
{
  throw std::runtime_error("Not implemented.");
}

void LobbyCommandShowInventoryCancel::Read(
  LobbyCommandShowInventoryCancel& command,
  SourceStream& buffer)
{
}

void LobbyCommandRequestLeagueInfo::Write(
  const LobbyCommandRequestLeagueInfo& command,
  SinkStream& buffer)
{
}

void LobbyCommandRequestLeagueInfo::Read(
  LobbyCommandRequestLeagueInfo& command,
  SourceStream& buffer)
{
}

void LobbyCommandRequestLeagueInfoOK::Write(
  const LobbyCommandRequestLeagueInfoOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0)
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
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestLeagueInfoCancel::Write(
  const LobbyCommandRequestLeagueInfoCancel& command,
  SinkStream& buffer)
{
}

void LobbyCommandRequestLeagueInfoCancel::Read(
  LobbyCommandRequestLeagueInfoCancel& command,
  SourceStream& buffer)
{
}

void LobbyCommandAchievementCompleteList::Write(
  const LobbyCommandAchievementCompleteList& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandAchievementCompleteList::Read(
  LobbyCommandAchievementCompleteList& command,
  SourceStream& buffer)
{
  buffer.Read(command.unk0);
}

void LobbyCommandAchievementCompleteListOK::Write(
  const LobbyCommandAchievementCompleteListOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0);
  buffer.Write(static_cast<uint16_t>(command.achievements.size()));
  for (const auto& achievement : command.achievements)
  {
    buffer.Write(achievement.unk0)
      .Write(achievement.unk1)
      .Write(achievement.unk2)
      .Write(achievement.unk3)
      .Write(achievement.unk4);
  }
}

void LobbyCommandAchievementCompleteListOK::Read(
  LobbyCommandAchievementCompleteListOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterChannel::Write(
  const LobbyCommandEnterChannel& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterChannel::Read(
  LobbyCommandEnterChannel& command,
  SourceStream& buffer)
{
  buffer.Read(command.channel);
}

void LobbyCommandEnterChannelOK::Write(
  const LobbyCommandEnterChannelOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.unk1);
}

void LobbyCommandEnterChannelOK::Read(
  LobbyCommandEnterChannelOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterChannelCancel::Write(
  const LobbyCommandEnterChannelCancel& command,
  SinkStream& buffer)
{
}

void LobbyCommandEnterChannelCancel::Read(
  LobbyCommandEnterChannelCancel& command,
  SourceStream& buffer)
{
}

void LobbyCommandMakeRoom::Write(
  const LobbyCommandMakeRoom& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandMakeRoom::Read(
  LobbyCommandMakeRoom& command,
  SourceStream& buffer)
{
  buffer.Read(command.name)
    .Read(command.description)
    .Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2)
    .Read(command.unk3)
    .Read(command.unk4)
    .Read(command.unk5)
    .Read(command.unk6);
}

void LobbyCommandMakeRoomOK::Write(
  const LobbyCommandMakeRoomOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.ip)
    .Write(command.port)
    .Write(command.unk2);
}

void LobbyCommandMakeRoomOK::Read(
  LobbyCommandMakeRoomOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandMakeRoomCancel::Write(
  const LobbyCommandMakeRoomCancel& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0);
}

void LobbyCommandMakeRoomCancel::Read(
  LobbyCommandMakeRoomCancel& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestQuestList::Write(
  const LobbyCommandRequestQuestList& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestQuestList::Read(
  LobbyCommandRequestQuestList& command,
  SourceStream& buffer)
{
  buffer.Read(command.unk0);
}

void LobbyCommandRequestQuestListOK::Write(
  const LobbyCommandRequestQuestListOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0);
  buffer.Write(static_cast<uint16_t>(command.quests.size()));
  for (const auto& quest : command.quests)
  {
    buffer.Write(quest.unk0)
      .Write(quest.unk1)
      .Write(quest.unk2)
      .Write(quest.unk3)
      .Write(quest.unk4);
  }
}

void LobbyCommandRequestQuestListOK::Read(
  LobbyCommandRequestQuestListOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterRanch::Write(
  const LobbyCommandEnterRanch& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterRanch::Read(
  LobbyCommandEnterRanch& command,
  SourceStream& buffer)
{
  buffer.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2);
}

void LobbyCommandEnterRanchOK::Write(
  const LobbyCommandEnterRanchOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.ranchUid)
    .Write(command.code)
    .Write(command.ip)
    .Write(command.port);
}

void LobbyCommandEnterRanchOK::Read(
  LobbyCommandEnterRanchOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandEnterRanchCancel::Write(
  const LobbyCommandEnterRanchCancel& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0);
}

void LobbyCommandEnterRanchCancel::Read(
  LobbyCommandEnterRanchCancel& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandGetMessengerInfo::Write(
  const LobbyCommandGetMessengerInfo& command,
  SinkStream& buffer)
{
}

void LobbyCommandGetMessengerInfo::Read(
  LobbyCommandGetMessengerInfo& command,
  SourceStream& buffer)
{
}

void LobbyCommandGetMessengerInfoOK::Write(
  const LobbyCommandGetMessengerInfoOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.code)
    .Write(command.ip)
    .Write(command.port);
}

void LobbyCommandGetMessengerInfoOK::Read(
  LobbyCommandGetMessengerInfoOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandGetMessengerInfoCancel::Write(
  const LobbyCommandGetMessengerInfoCancel& command,
  SinkStream& buffer)
{
}

void LobbyCommandGetMessengerInfoCancel::Read(
  LobbyCommandGetMessengerInfoCancel& command,
  SourceStream& buffer)
{
}

void LobbyCommandRoomList::Write(
  const LobbyCommandRoomList& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRoomList::Read(
  LobbyCommandRoomList& command,
  SourceStream& buffer)
{
  buffer.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2);
}

void LobbyCommandRoomListOK::Write(
  const LobbyCommandRoomListOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(static_cast<uint8_t>(command.rooms.size()));
  for(const auto& room : command.rooms)
  {
    buffer.Write(room.id)
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
  buffer.Write(command.unk3.unk0)
    .Write(command.unk3.unk1)
    .Write(command.unk3.unk2);
}

void LobbyCommandRoomListOK::Read(
  LobbyCommandRoomListOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestSpecialEventList::Write(
  const LobbyCommandRequestSpecialEventList& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandRequestSpecialEventList::Read(
  LobbyCommandRequestSpecialEventList& command,
  SourceStream& buffer)
{
  buffer.Read(command.unk0);
}

void LobbyCommandRequestSpecialEventListOK::Write(
  const LobbyCommandRequestSpecialEventListOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.unk0);

  buffer.Write(static_cast<uint16_t>(command.unk1.size()));
  for (const auto& unk1Element : command.unk1)
  {
    buffer.Write(unk1Element.unk0)
      .Write(unk1Element.unk1)
      .Write(unk1Element.unk2)
      .Write(unk1Element.unk3)
      .Write(unk1Element.unk4)
      .Write(unk1Element.unk5);
  }

  buffer.Write(static_cast<uint16_t>(command.unk2.size()));
  for (const auto& unk2Element: command.unk2)
  {
    buffer.Write(unk2Element.unk0)
      .Write(unk2Element.unk1);
  }
}

void LobbyCommandRequestSpecialEventListOK::Read(
  LobbyCommandRequestSpecialEventListOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void LobbyCommandHeartbeat::Write(
  const LobbyCommandHeartbeat& command,
  SinkStream& buffer)
{
}

void LobbyCommandHeartbeat::Read(
  LobbyCommandHeartbeat& command,
  SourceStream& buffer)
{
}

} // namespace alicia::proto