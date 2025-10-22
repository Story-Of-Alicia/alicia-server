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

void AcCmdCLLogin::Write(
  const AcCmdCLLogin& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLLogin::Read(
  AcCmdCLLogin& command,
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
    .Write(command.notice)
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

  //
  stream.Write(command.settings);

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
    .Write(struct5.guildRole)
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

void AcCmdCLLoginCancel::Write(
  const AcCmdCLLoginCancel& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.reason));
}

void AcCmdCLLoginCancel::Read(
  AcCmdCLLoginCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLShowInventory::Write(
  const AcCmdCLShowInventory& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLShowInventory::Read(
  AcCmdCLShowInventory& command,
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

void AcCmdCLCreateNickname::Write(
  const AcCmdCLCreateNickname& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLCreateNickname::Read(
  AcCmdCLCreateNickname& command,
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

void AcCmdCLRequestLeagueInfo::Write(
  const AcCmdCLRequestLeagueInfo& command,
  SinkStream& stream)
{
}

void AcCmdCLRequestLeagueInfo::Read(
  AcCmdCLRequestLeagueInfo& command,
  SourceStream& stream)
{
}

void AcCmdCLRequestLeagueInfoOK::Write(
  const AcCmdCLRequestLeagueInfoOK& command,
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

void AcCmdCLRequestLeagueInfoOK::Read(
  AcCmdCLRequestLeagueInfoOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestLeagueInfoCancel::Write(
  const AcCmdCLRequestLeagueInfoCancel& command,
  SinkStream& stream)
{
}

void AcCmdCLRequestLeagueInfoCancel::Read(
  AcCmdCLRequestLeagueInfoCancel& command,
  SourceStream& stream)
{
}

void AcCmdCLAchievementCompleteList::Write(
  const AcCmdCLAchievementCompleteList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLAchievementCompleteList::Read(
  AcCmdCLAchievementCompleteList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
}

void AcCmdCLAchievementCompleteListOK::Write(
  const AcCmdCLAchievementCompleteListOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
  stream.Write(static_cast<uint16_t>(command.achievements.size()));
  for (const auto& achievement : command.achievements)
  {
    stream.Write(achievement);
  }
}

void AcCmdCLAchievementCompleteListOK::Read(
  AcCmdCLAchievementCompleteListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterChannel::Write(
  const AcCmdCLEnterChannel& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterChannel::Read(
  AcCmdCLEnterChannel& command,
  SourceStream& stream)
{
  stream.Read(command.channel);
}

void AcCmdCLEnterChannelOK::Write(
  const AcCmdCLEnterChannelOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void AcCmdCLEnterChannelOK::Read(
  AcCmdCLEnterChannelOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterChannelCancel::Write(
  const AcCmdCLEnterChannelCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLEnterChannelCancel::Read(
  AcCmdCLEnterChannelCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLLeaveChannel::Write(
  const AcCmdCLLeaveChannel& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLLeaveChannel::Read(
  AcCmdCLLeaveChannel& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCLLeaveChannelOK::Write(
  const AcCmdCLLeaveChannelOK& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLLeaveChannelOK::Read(
  AcCmdCLLeaveChannelOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRoomList::Write(
  const AcCmdCLRoomList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRoomList::Read(
  AcCmdCLRoomList& command,
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
  stream.Write(value.uid)
    .Write(value.name)
    .Write(value.playerCount)
    .Write(value.maxPlayerCount)
    .Write(value.isLocked)
    .Write(value.unk0)
    .Write(value.unk1)
    .Write(value.map)
    .Write(value.state)
    .Write(value.unk2)
    .Write(value.unk3)
    .Write(value.skillBracket)
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
    .Write(command.gameMode)
    .Write(command.teamMode)
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

void AcCmdCLMakeRoom::Write(
  const AcCmdCLMakeRoom& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLMakeRoom::Read(
  AcCmdCLMakeRoom& command,
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

void AcCmdCLMakeRoomOK::Write(
  const AcCmdCLMakeRoomOK& command,
  SinkStream& stream)
{
  stream.Write(command.roomUid)
    .Write(command.oneTimePassword)
    .Write(htonl(command.raceServerAddress))
    .Write(command.raceServerPort)
    .Write(command.unk2);
}

void AcCmdCLMakeRoomOK::Read(
  AcCmdCLMakeRoomOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLMakeRoomCancel::Write(
  const AcCmdCLMakeRoomCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}

void AcCmdCLMakeRoomCancel::Read(
  AcCmdCLMakeRoomCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterRoom::Write(
  const AcCmdCLEnterRoom& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterRoom::Read(
  AcCmdCLEnterRoom& command,
  SourceStream& stream)
{
  stream.Read(command.roomUid)
    .Read(command.password)
    .Read(command.member3);
}

void AcCmdCLEnterRoomOK::Write(
  const AcCmdCLEnterRoomOK& command,
  SinkStream& stream)
{
  stream.Write(command.roomUid)
    .Write(command.oneTimePassword)
    .Write(htonl(command.raceServerAddress))
    .Write(command.raceServerPort)
    .Write(command.member6);
}

void AcCmdCLEnterRoomOK::Read(
  AcCmdCLEnterRoomOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterRoomCancel::Write(
  const AcCmdCLEnterRoomCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void AcCmdCLEnterRoomCancel::Read(
  AcCmdCLEnterRoomCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLLeaveRoom::Write(
  const AcCmdCLLeaveRoom& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLLeaveRoom::Read(
  AcCmdCLLeaveRoom& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCLLeaveRoomOK::Write(
  const AcCmdCLLeaveRoomOK& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLLeaveRoomOK::Read(
  AcCmdCLLeaveRoomOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestQuestList::Write(
  const AcCmdCLRequestQuestList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestQuestList::Read(
  AcCmdCLRequestQuestList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
}

void AcCmdCLRequestQuestListOK::Write(
  const AcCmdCLRequestQuestListOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
  stream.Write(static_cast<uint16_t>(command.quests.size()));
  for (const auto& quest : command.quests)
  {
    stream.Write(quest);
  }
}

void AcCmdCLRequestQuestListOK::Read(
  AcCmdCLRequestQuestListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestDailyQuestList::Write(
  const AcCmdCLRequestDailyQuestList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestDailyQuestList::Read(
  AcCmdCLRequestDailyQuestList& command,
  SourceStream& stream)
{
  stream.Read(command.val0);
}

void AcCmdCLRequestDailyQuestListOK::Write(
  const AcCmdCLRequestDailyQuestListOK& command,
  SinkStream& stream)
{
  stream.Write(command.val0);
  stream.Write(command.questCount);
  // TODO for all quests in char file write quests(we assume count is 0 for now
  //stream.Write(command.testQuest);

  stream.Write(command.dailyQuestCount);
  stream.Write(command.dailyQuest1);
  stream.Write(command.dailyQuest2);
  stream.Write(command.dailyQuest3);
}

void AcCmdCLRequestDailyQuestListOK::Read(
  AcCmdCLRequestDailyQuestListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestDailyQuestListOK::DailyQuest::Write(const DailyQuest& value, SinkStream& stream)
{
  stream.Write(value.questId)
    .Write(value.unk_1)
    .Write(value.unk_2)
    .Write(value.unk_3);
}

void AcCmdCLRequestDailyQuestListOK::DailyQuest::Read(DailyQuest& value, SourceStream& stream)
{
  stream.Read(value.questId)
    .Read(value.unk_1)
    .Read(value.unk_2)
    .Read(value.unk_3);
}

void AcCmdCLRequestDailyQuestListOK::Quest::Write(const Quest& value, SinkStream& stream)
{
  stream.Write(value.field0)
    .Write(value.field1)
    .Write(value.field2)
    .Write(value.field3)
    .Write(value.field4)
    .Write(value.field5)
    .Write(value.field6)
    .Write(value.field7);
}

void AcCmdCLRequestDailyQuestListOK::Quest::Read(Quest& value, SourceStream& stream)
{
  stream.Read(value.field0)
    .Read(value.field1)
    .Read(value.field2)
    .Read(value.field3)
    .Read(value.field4)
    .Read(value.field5)
    .Read(value.field6)
    .Read(value.field7);
}

void AcCmdCLEnterRanch::Write(
  const AcCmdCLEnterRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterRanch::Read(
  AcCmdCLEnterRanch& command,
  SourceStream& stream)
{
  stream.Read(command.rancherUid)
    .Read(command.unk1)
    .Read(command.unk2);
}

void AcCmdCLEnterRanchOK::Write(
  const AcCmdCLEnterRanchOK& command,
  SinkStream& stream)
{
  stream.Write(command.rancherUid)
    .Write(command.otp)
    .Write(htonl(command.ranchAddress))
    .Write(command.ranchPort);
}

void AcCmdCLEnterRanchOK::Read(
  AcCmdCLEnterRanchOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLEnterRanchCancel::Write(
  const AcCmdCLEnterRanchCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}

void AcCmdCLEnterRanchCancel::Read(
  AcCmdCLEnterRanchCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLGetMessengerInfo::Write(
  const AcCmdCLGetMessengerInfo& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLGetMessengerInfo::Read(
  AcCmdCLGetMessengerInfo& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCLGetMessengerInfoOK::Write(
  const AcCmdCLGetMessengerInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.code)
    .Write(command.ip)
    .Write(command.port);
}

void AcCmdCLGetMessengerInfoOK::Read(
  AcCmdCLGetMessengerInfoOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLGetMessengerInfoCancel::Write(
  const AcCmdCLGetMessengerInfoCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLGetMessengerInfoCancel::Read(
  AcCmdCLGetMessengerInfoCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLCheckWaitingSeqno::Write(
  const AcCmdCLCheckWaitingSeqno& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLCheckWaitingSeqno::Read(
  AcCmdCLCheckWaitingSeqno& command,
  SourceStream& stream)
{
  stream.Read(command.uid);
}

void AcCmdCLCheckWaitingSeqnoOK::Write(
  const AcCmdCLCheckWaitingSeqnoOK& command,
  SinkStream& stream)
{
  stream.Write(command.time)
    .Write(command.position);
}

void AcCmdCLCheckWaitingSeqnoOK::Read(
  AcCmdCLCheckWaitingSeqnoOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestSpecialEventList::Write(
  const AcCmdCLRequestSpecialEventList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCLRequestSpecialEventList::Read(
  AcCmdCLRequestSpecialEventList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
}

void AcCmdCLRequestSpecialEventListOK::Write(
  const AcCmdCLRequestSpecialEventListOK& command,
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

void AcCmdCLRequestSpecialEventListOK::Read(
  AcCmdCLRequestSpecialEventListOK& command,
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

void AcCmdClientNotify::Write(
  const AcCmdClientNotify& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdClientNotify::Read(
  AcCmdClientNotify& command,
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

void AcCmdCLEnterRanchRandomly::Write(
  const AcCmdCLEnterRanchRandomly& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLEnterRanchRandomly::Read(
  AcCmdCLEnterRanchRandomly& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCLFeatureCommand::Write(
  const AcCmdCLFeatureCommand& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLFeatureCommand::Read(
  AcCmdCLFeatureCommand& command,
  SourceStream& stream)
{
  stream.Read(command.command);
}

void AcCmdCLRequestFestivalResult::Write(
  const AcCmdCLRequestFestivalResult& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestFestivalResult::Read(
  AcCmdCLRequestFestivalResult& command,
  SourceStream& stream)
{
  stream.Read(command.member1);
}

void AcCmdCLRequestFestivalResultOK::Write(
  const AcCmdCLRequestFestivalResultOK& command,
  SinkStream& stream)
{
  throw std::runtime_error("Command needs to be discovered");
}

void AcCmdCLRequestFestivalResultOK::Read(
  AcCmdCLRequestFestivalResultOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestPersonalInfo::Write(
  const AcCmdCLRequestPersonalInfo& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestPersonalInfo::Read(
  AcCmdCLRequestPersonalInfo& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.type);
}

void AcCmdLCPersonalInfo::Basic::Write(const Basic& command, SinkStream& stream)
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

void AcCmdLCPersonalInfo::Basic::Read(Basic& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdLCPersonalInfo::CourseInformation::Write(const CourseInformation& command, SinkStream& stream)
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

void AcCmdLCPersonalInfo::CourseInformation::Read(CourseInformation& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdLCPersonalInfo::Eight::Write(const Eight& command, SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.member1.size()));
  for (const auto& entry : command.member1)
  {
    stream.Write(entry.member1)
      .Write(entry.member2);
  }
}

void AcCmdLCPersonalInfo::Eight::Read(Eight& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdLCPersonalInfo::Write(const AcCmdLCPersonalInfo& command, SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.type);

  switch (command.type)
  {
    case AcCmdCLRequestPersonalInfo::Type::Basic:
      {
        stream.Write(command.basic);
        break;
      }
    case AcCmdCLRequestPersonalInfo::Type::Courses:
      {
        stream.Write(command.courseInformation);
        break;
      }
    case AcCmdCLRequestPersonalInfo::Type::Eight:
      {
        stream.Write(command.eight);
        break;
      }
  }
}

void AcCmdLCPersonalInfo::Read(
  AcCmdLCPersonalInfo& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLSetIntroduction::Write(
  const AcCmdCLSetIntroduction& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLSetIntroduction::Read(
  AcCmdCLSetIntroduction& command,
  SourceStream& stream)
{
  stream.Read(command.introduction);
}

void AcCmdCLUpdateSystemContent::Write(
  const AcCmdCLUpdateSystemContent& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLUpdateSystemContent::Read(
  AcCmdCLUpdateSystemContent& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.key)
    .Read(command.value);
}

void AcCmdLCUpdateSystemContent::Write(
  const AcCmdLCUpdateSystemContent& command,
  SinkStream& stream)
{
  stream.Write(command.systemContent);
}

void AcCmdLCUpdateSystemContent::Read(
  AcCmdLCUpdateSystemContent& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLEnterRoomQuickStop::Write(
  const AcCmdCLEnterRoomQuickStop& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLEnterRoomQuickStop::Read(
  AcCmdCLEnterRoomQuickStop& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCLEnterRoomQuickStopOK::Write(
  const AcCmdCLEnterRoomQuickStopOK& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLEnterRoomQuickStopOK::Read(
  AcCmdCLEnterRoomQuickStopOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLEnterRoomQuickStopCancel::Write(
  const AcCmdCLEnterRoomQuickStopCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLEnterRoomQuickStopCancel::Read(
  AcCmdCLEnterRoomQuickStopCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestFestivalPrize::Write(
  const AcCmdCLRequestFestivalPrize& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestFestivalPrize::Read(
  AcCmdCLRequestFestivalPrize& command,
  SourceStream& stream)
{
  stream.Read(command.member1);
}

void AcCmdCLRequestFestivalPrizeOK::Write(
  const AcCmdCLRequestFestivalPrizeOK& command,
  SinkStream& stream)
{
  // todo: discover
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestFestivalPrizeOK::Read(
  AcCmdCLRequestFestivalPrizeOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestFestivalPrizeCancel::Write(
  const AcCmdCLRequestFestivalPrizeCancel& command,
  SinkStream& stream)
{
  // todo: discover
  throw std::runtime_error("Not implemented");
}

void AcCmdCLRequestFestivalPrizeCancel::Read(
  AcCmdCLRequestFestivalPrizeCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLQueryServerTime::Write(
  const AcCmdCLQueryServerTime& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLQueryServerTime::Read(
  AcCmdCLQueryServerTime& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCLQueryServerTimeOK::Write(
  const AcCmdCLQueryServerTimeOK& command,
  SinkStream& stream)
{
  stream.Write(command.lobbyTime.dwLowDateTime)
    .Write(command.lobbyTime.dwHighDateTime);
}

void AcCmdCLQueryServerTimeOK::Read(
  AcCmdCLQueryServerTimeOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLChangeRanchOption::Read(
  AcCmdCLChangeRanchOption& command,
  SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2);
}

void AcCmdCLChangeRanchOptionOK::Write(
  const AcCmdCLChangeRanchOptionOK& command,
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

void AcCmdCLUpdateUserSettings::Write(
  const AcCmdCLUpdateUserSettings& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLUpdateUserSettings::Read(
  AcCmdCLUpdateUserSettings& command,
  SourceStream& stream)
{
  stream.Read(command.settings);
}

void AcCmdCLUpdateUserSettingsOK::Write(
  const AcCmdCLUpdateUserSettingsOK& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLUpdateUserSettingsOK::Read(
  AcCmdCLUpdateUserSettingsOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLEnterRoomQuick::Write(
  const AcCmdCLEnterRoomQuick& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCLEnterRoomQuick::Read(
  AcCmdCLEnterRoomQuick& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.member2);
}

void AcCmdCLEnterRoomQuickCancel::Write(
  const AcCmdCLEnterRoomQuickCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCLEnterRoomQuickCancel::Read(
  AcCmdCLEnterRoomQuickCancel& command,
  SourceStream& stream)
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

void AcCmdLCSkillCardPresetList::Read(
  AcCmdLCSkillCardPresetList& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdLCSkillCardPresetList::Write(
  const AcCmdLCSkillCardPresetList& command,
  SinkStream& stream)
{
  stream.Write(command.speedActiveSetId)
    .Write(command.magicActiveSetId);

  stream.Write(static_cast<uint8_t>(command.skillSets.size()));
  for (const auto& skillSet : command.skillSets)
  {
    stream.Write(skillSet);
  }
}

void AcCmdLCInviteGuildJoin::Read(
  AcCmdLCInviteGuildJoin& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdLCInviteGuildJoin::Write(
  const AcCmdLCInviteGuildJoin& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.inviterCharacterUid)
    .Write(command.inviterCharacterName)
    .Write(command.unk3)
    .Write(command.guild);
}

void AcCmdLCInviteGuildJoinCancel::Read(
  AcCmdLCInviteGuildJoinCancel& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.inviterCharacterUid)
    .Read(command.inviterCharacterName)
    .Read(command.unk3)
    .Read(command.guild)
    .Read(command.error);
}

void AcCmdLCInviteGuildJoinCancel::Write(
  const AcCmdLCInviteGuildJoinCancel& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdLCInviteGuildJoinOK::Read(
  AcCmdLCInviteGuildJoinOK& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.inviterCharacterUid)
    .Read(command.inviterCharacterName)
    .Read(command.unk3)
    .Read(command.guild);
}

void AcCmdLCInviteGuildJoinOK::Write(
  const AcCmdLCInviteGuildJoinOK& command,
  SinkStream& stream)
{
  // TODO: Return this back to the client to confirm join?
  throw std::runtime_error("Not implemented");
}

} // namespace server::protocol
