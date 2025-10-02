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

#include "libserver/network/command/proto/RaceMessageDefinitions.hpp"

#include "libserver/network/chatter/ChatterServer.hpp"

namespace server::protocol
{

void WritePlayerRacer(SinkStream& stream, const Avatar& playerRacer)
{
  stream.Write(static_cast<uint8_t>(playerRacer.equipment.size()));

  for (const Item& item : playerRacer.equipment)
  {
    stream.Write(item);
  }

  stream.Write(playerRacer.character)
    .Write(playerRacer.mount)
    .Write(playerRacer.unk0);
}

void WriteRacer(SinkStream& stream, const Racer& racer)
{
  stream.Write(racer.isMaster)
    .Write(racer.member2)
    .Write(racer.level)
    .Write(racer.oid)
    .Write(racer.uid)
    .Write(racer.name)
    .Write(racer.isReady)
    .Write(racer.teamColor)
    .Write(racer.isHidden)
    .Write(racer.isNPC);

  if (racer.isNPC)
  {
    stream.Write(racer.npcTid.value());
  }
  else
  {
    WritePlayerRacer(stream, racer.avatar.value());
  }

  stream.Write(racer.unk8.unk0)
    .Write(racer.unk8.rent.mountUid)
    .Write(racer.unk8.rent.val1)
    .Write(racer.unk8.rent.val2);
  stream.Write(racer.pet);
  stream.Write(racer.guild.uid)
    .Write(racer.guild.val1)
    .Write(racer.guild.val2)
    .Write(racer.guild.name)
    .Write(racer.guild.val4)
    .Write(racer.guild.val5)
    .Write(racer.guild.val6);
  stream.Write(racer.unk9);
  stream.Write(racer.unk10)
    .Write(racer.unk11)
    .Write(racer.unk12)
    .Write(racer.unk13);
}

void WriteRoomDescription(SinkStream& stream, const RoomDescription& roomDescription)
{
  stream.Write(roomDescription.name)
    .Write(roomDescription.playerCount)
    .Write(roomDescription.password)
    .Write(roomDescription.gameModeMaps)
    .Write(roomDescription.teamMode)
    .Write(roomDescription.mapBlockId)
    .Write(roomDescription.gameMode)
    .Write(roomDescription.missionId)
    .Write(roomDescription.unk6)
    .Write(roomDescription.skillBracket);
}

void AcCmdCREnterRoom::Write(
  const AcCmdCREnterRoom& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCREnterRoom::Read(
  AcCmdCREnterRoom& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.otp)
    .Read(command.roomUid);
}

void AcCmdCREnterRoomOK::Write(
  const AcCmdCREnterRoomOK& command,
  SinkStream& stream)
{
  if (command.racers.size() > 10)
  {
    throw std::logic_error("Racers size is greater than 10.");
  }

  stream.Write(static_cast<uint32_t>(command.racers.size()));
  for (const auto& racer : command.racers)
  {
    WriteRacer(stream, racer);
  }

  stream.Write(command.nowPlaying)
    .Write(command.uid);

  WriteRoomDescription(stream, command.roomDescription);

  stream.Write(command.unk2)
    .Write(command.unk3)
    .Write(command.unk4)
    .Write(command.unk5)
    .Write(command.unk6);

  stream.Write(command.unk7)
    .Write(command.unk8);

  stream.Write(command.unk9.unk0)
    .Write(command.unk9.unk1)
    .Write(static_cast<uint8_t>(command.unk9.unk2.size()));
  for (const auto& unk2Element : command.unk9.unk2)
  {
    stream.Write(unk2Element);
  }

  stream.Write(command.unk10)
    .Write(command.unk11)
    .Write(command.unk12)
    .Write(command.scrambleValue);
}

void AcCmdCREnterRoomOK::Read(
  AcCmdCREnterRoomOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCREnterRoomCancel::Write(
  const AcCmdCREnterRoomCancel& command,
  SinkStream& stream)
{
}

void AcCmdCREnterRoomCancel::Read(
  AcCmdCREnterRoomCancel& command,
  SourceStream& stream)
{
}

void AcCmdCREnterRoomNotify::Write(
  const AcCmdCREnterRoomNotify& command,
  SinkStream& stream)
{
  WriteRacer(stream, command.racer);
  stream.Write(command.averageTimeRecord);
}

void AcCmdCREnterRoomNotify::Read(
  AcCmdCREnterRoomNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRChangeRoomOptions::Write(
  const AcCmdCRChangeRoomOptions& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRChangeRoomOptions::Read(
  AcCmdCRChangeRoomOptions& command,
  SourceStream& stream)
{
  stream.Read(command.optionsBitfield);
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Name)
  {
    stream.Read(command.name);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::PlayerCount)
  {
    stream.Read(command.playerCount);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Password)
  {
    stream.Read(command.password);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::GameMode)
  {
    stream.Read(command.gameMode);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::MapBlockId)
  {
    stream.Read(command.mapBlockId);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::NPCRace)
  {
    stream.Read(command.npcRace);
  }
}

void AcCmdCRChangeRoomOptionsNotify::Write(
  const AcCmdCRChangeRoomOptionsNotify& command,
  SinkStream& stream)
{
  stream.Write(command.optionsBitfield);
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Name)
  {
    stream.Write(command.name);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::PlayerCount)
  {
    stream.Write(command.playerCount);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Password)
  {
    stream.Write(command.password);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::GameMode)
  {
    stream.Write(command.gameMode);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::MapBlockId)
  {
    stream.Write(command.mapBlockId);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::NPCRace)
  {
    stream.Write(command.npcRace);
  }
}

void AcCmdCRChangeRoomOptionsNotify::Read(
  AcCmdCRChangeRoomOptionsNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRChangeTeam::Write(
  const AcCmdCRChangeTeam& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRChangeTeam::Read(
  AcCmdCRChangeTeam& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.teamColor);
}

void AcCmdCRChangeTeamOK::Write(
  const AcCmdCRChangeTeamOK& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.teamColor);
}

void AcCmdCRChangeTeamOK::Read(
  AcCmdCRChangeTeamOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRChangeTeamNotify::Write(
  const AcCmdCRChangeTeamNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.teamColor);
}

void AcCmdCRChangeTeamNotify::Read(
  AcCmdCRChangeTeamNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRLeaveRoom::Write(
  const AcCmdCRLeaveRoom& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRLeaveRoom::Read(
  AcCmdCRLeaveRoom& command,
  SourceStream& stream)
{
  // Empty
}
void AcCmdCRLeaveRoomOK::Write(
  const AcCmdCRLeaveRoomOK& command,
  SinkStream& stream)
{
  // Empty
}

void AcCmdCRLeaveRoomOK::Read(
  AcCmdCRLeaveRoomOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRLeaveRoomNotify::Write(
  const AcCmdCRLeaveRoomNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterId);
  stream.Write(command.unk0);
}

void AcCmdCRLeaveRoomNotify::Read(
  AcCmdCRLeaveRoomNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRStartRace::Write(
  const AcCmdCRStartRace& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRStartRace::Read(
  AcCmdCRStartRace& command,
  SourceStream& stream)
{
  uint8_t size;
  stream.Read(size);
  command.unk0.resize(size);
  for (auto& element : command.unk0)
  {
    stream.Read(element);
  }
}

void AcCmdCRStartRaceNotify::Struct1::Write(
  const Struct1& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
   .Write(command.member2)
   .Write(command.member3)
   .Write(command.member4);

  stream.Write(static_cast<uint8_t>(
    command.member5.size()));
  for (const auto& element : command.member5)
  {
    stream.Write(element);
  }

  if (command.member4 == 3)
  {
    stream.Write(command.optional.member6)
      .Write(command.optional.member8)
      .Write(command.optional.member9)
      .Write(command.optional.member10)
      .Write(command.optional.member11)
      .Write(command.optional.member12);
  }

  stream.Write(command.member13);
}

void AcCmdCRStartRaceNotify::Struct1::Read(
  Struct1& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRStartRaceNotify::Struct2::Write(
  const Struct2& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3);
}

void AcCmdCRStartRaceNotify::Struct2::Read(
  Struct2& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRStartRaceNotify::Struct3::Write(
  const Struct3& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);

  stream.Write(static_cast<uint8_t>(
    command.unk2.size()));
  for (const auto& element : command.unk2)
  {
    stream.Write(element);
  }
}

void AcCmdCRStartRaceNotify::Struct3::Read(
  Struct3& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRStartRaceNotify::Write(
  const AcCmdCRStartRaceNotify& command,
  SinkStream& stream)
{
  stream.Write(command.gameMode)
    .Write(command.teamMode)
    .Write(command.hostOid)
    .Write(command.member4)
    .Write(command.mapBlockId);

  stream.Write(static_cast<uint8_t>(command.racers.size()));
  for (const auto& element : command.racers)
  {
    stream.Write(element.oid)
      .Write(element.name)
      .Write(element.unk2)
      .Write(element.unk3)
      .Write(element.p2dId)
      .Write(element.teamColor)
      .Write(element.unk6)
      .Write(element.unk7);
  }

  stream.Write(
    boost::asio::detail::socket_ops::host_to_network_long(
      command.p2pRelayAddress))
    .Write(command.p2pRelayPort)
    .Write(command.unk6)
    .Write(command.unk9)
    .Write(command.unk10);

  stream.Write(command.missionId)
    .Write(command.unk12)
    .Write(command.unk13);

  stream.Write(command.unk14)
    .Write(command.carnivalType)
    .Write(command.weatherType)
    .Write(command.unk17);

  stream.Write(static_cast<uint8_t>(command.unk18.size()));
  for (const auto& element : command.unk18)
  {
    stream.Write(element.unk0)
      .Write(static_cast<uint8_t>(element.unk1.size()));
    for (const auto& subElement : element.unk1)
    {
      stream.Write(subElement);
    }
  }
}

void AcCmdCRStartRaceNotify::Read(
  AcCmdCRStartRaceNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRStartRaceCancel::Write(
  const AcCmdCRStartRaceCancel& command,
  SinkStream& stream)
{
  stream.Write(command.reason);
}

void AcCmdCRStartRaceCancel::Read(
  AcCmdCRStartRaceCancel& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdUserRaceTimer::Write(
  const AcCmdUserRaceTimer& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdUserRaceTimer::Read(
  AcCmdUserRaceTimer& command,
  SourceStream& stream)
{
  stream.Read(command.timestamp);
}

void AcCmdUserRaceTimerOK::Write(
  const AcCmdUserRaceTimerOK& command,
  SinkStream& stream)
{
  stream.Write(command.clientTimestamp)
    .Write(command.serverTimestamp);
}

void AcCmdUserRaceTimerOK::Read(
  AcCmdUserRaceTimerOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRLoadingComplete::Write(
  const AcCmdCRLoadingComplete& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRLoadingComplete::Read(
  AcCmdCRLoadingComplete& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCRLoadingCompleteNotify::Write(
  const AcCmdCRLoadingCompleteNotify& command,
  SinkStream& stream)
{
  stream.Write(command.oid);
}

void AcCmdCRLoadingCompleteNotify::Read(
  AcCmdCRLoadingCompleteNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void AcCmdCRChat::Write(
  const AcCmdCRChat& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChat::Read(
  AcCmdCRChat& command,
  SourceStream& stream)
{
  stream.Read(command.message)
    .Read(command.unknown);
}

void AcCmdCRChatNotify::Write(
  const AcCmdCRChatNotify& command,
  SinkStream& stream)
{
  stream.Write(command.message)
    .Write(command.author)
    .Write(command.isSystem);
}

void AcCmdCRChatNotify::Read(
  AcCmdCRChatNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRReadyRace::Write(
  const AcCmdCRReadyRace& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRReadyRace::Read(
  AcCmdCRReadyRace& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCRReadyRaceNotify::Write(
  const AcCmdCRReadyRaceNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.isReady);
}

void AcCmdCRReadyRaceNotify::Read(
  AcCmdCRReadyRaceNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceCountdown::Write(
  const AcCmdUserRaceCountdown& command,
  SinkStream& stream)
{
  stream.Write(command.timestamp);
}

void AcCmdUserRaceCountdown::Read(
  AcCmdUserRaceCountdown& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceFinal::Write(
  const AcCmdUserRaceFinal& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceFinal::Read(
  AcCmdUserRaceFinal& command,
  SourceStream& stream)
{
  stream.Read(command.oid)
    .Read(command.courseTime)
    .Read(command.member3);
}

void AcCmdUserRaceFinalNotify::Write(
  const AcCmdUserRaceFinalNotify& command,
  SinkStream& stream)
{
  stream.Write(command.oid)
    .Write(command.courseTime);
}

void AcCmdUserRaceFinalNotify::Read(
  AcCmdUserRaceFinalNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRaceResult::Write(
  const AcCmdCRRaceResult& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRaceResult::Read(
  AcCmdCRRaceResult& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.member2)
    .Read(command.member3)
    .Read(command.member4)
    .Read(command.member5)
    .Read(command.member6)
    .Read(command.member7)
    .Read(command.member8)
    .Read(command.member9);

  uint8_t size{};
  stream.Read(size);

  command.member10.resize(size);
  for (auto& value : command.member10)
  {
    stream.Read(value);
  }

  stream.Read(command.member11)
    .Read(command.member12)
    .Read(command.member13)
    .Read(command.member14);
}

void AcCmdCRRaceResultOK::Write(
  const AcCmdCRRaceResultOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.member2)
    .Write(command.member3)
    .Write(command.member4)
    .Write(command.member5)
    .Write(command.member6);
}

void AcCmdCRRaceResultOK::Read(
  AcCmdCRRaceResultOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCRaceResultNotify::Write(
  const AcCmdRCRaceResultNotify& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint16_t>(command.scores.size()));
  for (const auto& score : command.scores)
  {
    stream.Write(score.uid)
      .Write(score.name)
      .Write(score.courseTime)
      .Write(score.member4)
      .Write(score.experience)
      .Write(score.member6)
      .Write(score.carrots)
      .Write(score.level)
      .Write(score.member9)
      .Write(score.member10)
      .Write(score.member11)
      .Write(score.member12)
      .Write(score.recordTimeDifference)
      .Write(score.member14)
      .Write(score.member15)
      .Write(score.achievements)
      .Write(score.bitset)
      .Write(score.mountName)
      .Write(score.member19)
      .Write(score.member20)
      .Write(score.bonusCarrots)
      .Write(score.member22)
      .Write(score.member23)
      .Write(score.member24)
      .Write(score.member25)
      .Write(score.member26)
      .Write(score.member27);
  }

  stream.Write(command.member2);

  stream.Write(command.member3)
    .Write(command.member4);
}

void AcCmdRCRaceResultNotify::Read(
  AcCmdRCRaceResultNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRP2PResult::Write(
  const AcCmdCRP2PResult& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRP2PResult::Read(
  AcCmdCRP2PResult& command,
  SourceStream& stream)
{
  stream.Read(command.oid)
    .Read(command.member2);
  for (auto& podium : command.podium)
  {
    stream.Read(podium);
  }
}

void AcCmdUserRaceP2PResult::Write(
  const AcCmdUserRaceP2PResult& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceP2PResult::Read(
  AcCmdUserRaceP2PResult& command,
  SourceStream& stream)
{
  uint8_t size{};
  stream.Read(size);

  command.member1.resize(size);
  for (auto& value : command.member1)
  {
    stream.Read(value.oid)
      .Read(value.member2);
  }
}

void AcCmdGameRaceP2PResult::Write(
  const AcCmdGameRaceP2PResult& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(
    command.member1.size()));

  for (auto& value : command.member1)
  {
    stream.Write(value.oid)
      .Write(value.member2);
  }
}

void AcCmdGameRaceP2PResult::Read(
  AcCmdGameRaceP2PResult& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRAwardStart::Write(
  const AcCmdCRAwardStart& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRAwardStart::Read(
  AcCmdCRAwardStart& command,
  SourceStream& stream)
{
  stream.Read(command.member1);
}

void AcCmdCRAwardEnd::Write(
  const AcCmdCRAwardEnd& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRAwardEnd::Read(
  AcCmdCRAwardEnd& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdRCAwardNotify::Write(
  const AcCmdRCAwardNotify& command,
  SinkStream& stream)
{
  stream.Write(command.member1);
}

void AcCmdRCAwardNotify::Read(
  AcCmdRCAwardNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRAwardEndNotify::Write(
  const AcCmdCRAwardEndNotify& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCRAwardEndNotify::Read(
  AcCmdCRAwardEndNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRStarPointGet::Write(
  const AcCmdCRStarPointGet& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRStarPointGet::Read(
  AcCmdCRStarPointGet& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.unk1)
    .Read(command.gainedStarPoints);
}

void AcCmdCRStarPointGetOK::Write(
  const AcCmdCRStarPointGetOK& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.starPointValue)
    .Write(command.giveMagicItem);
}

void AcCmdCRStarPointGetOK::Read(
  AcCmdCRStarPointGetOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestSpur::Write(
  const AcCmdCRRequestSpur& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestSpur::Read(
  AcCmdCRRequestSpur& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.activeBoosters)
    .Read(command.comboBreak);
}

void AcCmdCRRequestSpurOK::Write(
  const AcCmdCRRequestSpurOK& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.activeBoosters)
    .Write(command.startPointValue)
    .Write(command.comboBreak);
}

void AcCmdCRRequestSpurOK::Read(
  AcCmdCRRequestSpurOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHurdleClearResult::Write(
  const AcCmdCRHurdleClearResult& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHurdleClearResult::Read(
  AcCmdCRHurdleClearResult& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.hurdleClearType);
}

void AcCmdCRHurdleClearResultOK::Write(
  const AcCmdCRHurdleClearResultOK& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.hurdleClearType)
    .Write(command.jumpCombo)
    .Write(command.unk3);
}

void AcCmdCRHurdleClearResultOK::Read(
  AcCmdCRHurdleClearResultOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRStartingRate::Write(
  const AcCmdCRStartingRate& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRStartingRate::Read(
  AcCmdCRStartingRate& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.unk1)
    .Read(command.boostGained);
}

void AcCmdCRRequestMagicItem::Write(
  const AcCmdCRRequestMagicItem& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestMagicItem::Read(
  AcCmdCRRequestMagicItem& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.member2);
}

void AcCmdCRRequestMagicItemOK::Write(
  const AcCmdCRRequestMagicItemOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.member2)
    .Write(command.member3);
}

void AcCmdCRRequestMagicItemOK::Read(
  AcCmdCRRequestMagicItemOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestMagicItemNotify::Write(
  const AcCmdCRRequestMagicItemNotify& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.member2);
}

void AcCmdCRRequestMagicItemNotify::Read(
  AcCmdCRRequestMagicItemNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceUpdatePos::Write(
  const AcCmdUserRaceUpdatePos& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceUpdatePos::Read(
  AcCmdUserRaceUpdatePos& command,
  SourceStream& stream)
{
  stream.Read(command.oid);

  for (auto& element : command.member2)
  {
    stream.Read(element);
  }

  for (auto& element : command.member3)
  {
    stream.Read(element);
  }

  stream.Read(command.member4)
    .Read(command.member5)
    .Read(command.member6)
    .Read(command.member7);
}

void AcCmdRCRoomCountdown::Write(
  const AcCmdRCRoomCountdown& command,
  SinkStream& stream)
{
  stream.Write(command.member0)
    .Write(command.member1);
}

void AcCmdRCRoomCountdown::Read(
  AcCmdRCRoomCountdown& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCRoomCountdownCancel::Write(
  const AcCmdRCRoomCountdownCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdRCRoomCountdownCancel::Read(
  AcCmdRCRoomCountdownCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeMasterNotify::Write(
  const AcCmdCRChangeMasterNotify& command,
  SinkStream& stream)
{
  stream.Write(command.masterUid);
}

void AcCmdCRChangeMasterNotify::Read(
  AcCmdCRChangeMasterNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRelayCommand::Write(
  const AcCmdCRRelayCommand& command,
  SinkStream& stream)
{
  stream.Write(command.senderOid);
  for (const auto& byte : command.relayData)
  {
    stream.Write(byte);
  }
}

void AcCmdCRRelayCommand::Read(
  AcCmdCRRelayCommand& command,
  SourceStream& stream)
{
  stream.Read(command.senderOid);
  
  // Read remaining bytes as relay data
  const auto remainingBytes = stream.Size() - stream.GetCursor();
  command.relayData.resize(remainingBytes);
  for (auto& byte : command.relayData)
  {
    stream.Read(byte);
  }
}

void AcCmdCRRelayCommandNotify::Write(
  const AcCmdCRRelayCommandNotify& command,
  SinkStream& stream)
{
  stream.Write(command.senderOid);
  for (const auto& byte : command.relayData)
  {
    stream.Write(byte);
  }
}

void AcCmdCRRelayCommandNotify::Read(
  AcCmdCRRelayCommandNotify& command,
  SourceStream& stream)
{
  stream.Read(command.senderOid);
  
  // Read remaining bytes as relay data
  const auto remainingBytes = stream.Size() - stream.GetCursor();
  command.relayData.resize(remainingBytes);
  for (auto& byte : command.relayData)
  {
    stream.Read(byte);
  }
}

void AcCmdCRRelay::Write(
  const AcCmdCRRelay& command,
  SinkStream& stream)
{
  stream.Write(command.senderOid);
  for (const auto& byte : command.relayData)
  {
    stream.Write(byte);
  }
}

void AcCmdCRRelay::Read(
  AcCmdCRRelay& command,
  SourceStream& stream)
{
  stream.Read(command.senderOid);
  
  // Read remaining bytes as relay data
  const auto remainingBytes = stream.Size() - stream.GetCursor();
  command.relayData.resize(remainingBytes);
  for (auto& byte : command.relayData)
  {
    stream.Read(byte);
  }
}

void AcCmdCRRelayNotify::Write(
  const AcCmdCRRelayNotify& command,
  SinkStream& stream)
{
  stream.Write(command.senderOid);
  for (const auto& byte : command.relayData)
  {
    stream.Write(byte);
  }
}

void AcCmdCRRelayNotify::Read(
  AcCmdCRRelayNotify& command,
  SourceStream& stream)
{
  stream.Read(command.senderOid);
  
  // Read remaining bytes as relay data
  const auto remainingBytes = stream.Size() - stream.GetCursor();
  command.relayData.resize(remainingBytes);
  for (auto& byte : command.relayData)
  {
    stream.Read(byte);
  }
}

void AcCmdRCTeamSpurGauge::Write(
  const AcCmdRCTeamSpurGauge& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.member2)
    .Write(command.member3)
    .Write(command.member4)
    .Write(command.member5)
    .Write(command.member6);
}

void AcCmdRCTeamSpurGauge::Read(
  AcCmdRCTeamSpurGauge& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");  
}

void AcCmdUserRaceActivateInteractiveEvent::Write(
  const AcCmdUserRaceActivateInteractiveEvent& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.characterOid)
    .Write(command.member3);
}

void AcCmdUserRaceActivateInteractiveEvent::Read(
  AcCmdUserRaceActivateInteractiveEvent& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.member3);
}

void AcCmdUserRaceActivateEvent::Write(
  const AcCmdUserRaceActivateEvent& command,
  SinkStream& stream)
{
  stream.Write(command.eventId)
    .Write(command.characterOid);
}

void AcCmdUserRaceActivateEvent::Read(
  AcCmdUserRaceActivateEvent& command,
  SourceStream& stream)
{
  stream.Read(command.eventId);
}

void AcCmdCRUseMagicItem::Write(
  const AcCmdCRUseMagicItem& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUseMagicItem::Read(
  AcCmdCRUseMagicItem& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid);
  stream.Read(command.magicItemId);

  switch(command.magicItemId)
  {
    // Case 0xA and 0xB read 2x [3x floats]
    // and then fallthrough to read uint16_t vector
    case 0xa:
    case 0xb:
    {
      auto& optional1 = command.optional1.emplace();
      for (auto& element : optional1.member1)
      {
        stream.Read(element);
      }
      for (auto& element : optional1.member2)
      {
        stream.Read(element);
      }
      [[fallthrough]];
    }
    case 0x2:
    case 0x3:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x12:
    case 0x13:
    {
      auto& optional2 = command.optional2.emplace();

      stream.Read(optional2.size);
      optional2.list.resize(optional2.size);
      for (auto& element : optional2.list)
      {
        stream.Read(element);
      }
      break;
    }
    default:
    {
      break;
    }
  }

  stream.Read(command.unk3);
  // FIXME: wtf am i switching
  switch (command.magicItemId)
  {
    case 0x2:
    case 0x3:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x12:
    case 0x13:
    {
      stream.Read(command.optional3.emplace())
        .Read(command.optional4.emplace());
      break;
    }
    default:
    {
      break;
    }
  }
}

void AcCmdCRUseMagicItemCancel::Write(
  const AcCmdCRUseMagicItemCancel& command,
  SinkStream& stream)
{
  // Empty
}

void AcCmdCRUseMagicItemCancel::Read(
  AcCmdCRUseMagicItemCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUseMagicItemOK::Write(
  const AcCmdCRUseMagicItemOK& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid);
  stream.Write(command.magicItemId);

  switch(command.magicItemId)
  {
    // Case 0xA and 0xB write 2x [3x floats]
    // and then fallthrough to read uint16_t vector
    case 0xa:
    case 0xb:
      // TODO: is this correct?
      // Assert that optional1 has value
      assert(command.optional1.has_value());

      for (auto& element : command.optional1.value().member1)
      {
        stream.Write(element);
      }
      for (auto& element : command.optional1.value().member2)
      {
        stream.Write(element);
      }
      [[fallthrough]];
    case 0x2:
    case 0x3:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x12:
    case 0x13:
    {
      // TODO: is this correct?
      // Assert that optional2 has value
      assert(command.optional2.has_value());

      // Expects vector size followed by uint16_t vector itself
      stream.Write(command.optional2.value().size);
      for (auto& element : command.optional2.value().list)
      {
        stream.Write(element);
      }
      break;
    }
    default:
    {
      break;
    }
  }

  stream.Write(command.unk3)
    .Write(command.unk4);
}

void AcCmdCRUseMagicItemOK::Read(
  AcCmdCRUseMagicItemOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdGameRaceItemSpawn::Write(
  const AcCmdGameRaceItemSpawn& command,
  SinkStream& stream)
{
  stream.Write(command.itemId)
    .Write(command.itemType);

  for (const float& axis : command.position)
  {
    stream.Write(axis);
  }

  for (const float& axis : command.orientation)
  {
    stream.Write(axis);
  }

  stream.Write(command.member5)
    .Write(command.removeDelay);
}

void AcCmdGameRaceItemSpawn::Read(
  AcCmdGameRaceItemSpawn& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceItemGet::Write(
  const AcCmdUserRaceItemGet& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdUserRaceItemGet::Read(
  AcCmdUserRaceItemGet& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.itemId)
    .Read(command.unk3);
}

void AcCmdGameRaceItemGet::Write(
  const AcCmdGameRaceItemGet& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.itemId)
    .Write(command.itemType);
}

void AcCmdGameRaceItemGet::Read(
  AcCmdGameRaceItemGet& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.itemId)
    .Read(command.itemType);
}

// Magic Targeting Commands Implementation
void AcCmdCRStartMagicTarget::Read(
  AcCmdCRStartMagicTarget& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid);
}

void AcCmdCRChangeMagicTargetNotify::Write(
  const AcCmdCRChangeMagicTargetNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.targetOid);
}

void AcCmdCRChangeMagicTargetNotify::Read(
  AcCmdCRChangeMagicTargetNotify& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.targetOid);
}

void AcCmdCRChangeMagicTargetOK::Read(
  AcCmdCRChangeMagicTargetOK& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.targetOid);
}

void AcCmdCRChangeMagicTargetCancel::Read(
  AcCmdCRChangeMagicTargetCancel& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid);
}

void AcCmdRCRemoveMagicTarget::Write(
  const AcCmdRCRemoveMagicTarget& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid);
}

void AcCmdRCMagicExpire::Write(
  const AcCmdRCMagicExpire& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid);
}

void AcCmdCRUseMagicItemNotify::Write(
  const AcCmdCRUseMagicItemNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid);
  stream.Write(command.magicItemId);

  switch(command.magicItemId)
  {
    // Case 0xA and 0xB write 2x [3x floats]
    // and then fallthrough to write uint16_t vector
    case 0xa:
    case 0xb:
      // Assert that optional1 has value
      assert(command.optional1.has_value());

      for (auto& element : command.optional1.value().member1)
      {
        stream.Write(element);
      }
      for (auto& element : command.optional1.value().member2)
      {
        stream.Write(element);
      }
      [[fallthrough]];
    case 0x2:
    case 0x3:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x12:
    case 0x13:
    {
      // Assert that optional2 has value
      assert(command.optional2.has_value());

      // Expects vector size followed by uint16_t vector itself
      stream.Write(command.optional2.value().size);
      for (auto& element : command.optional2.value().list)
      {
        stream.Write(element);
      }
      break;
    }
    default:
    {
      break;
    }
  }

  // FIXME: wtf am i switching
  switch (command.magicItemId)
  {
    case 0x2:
    case 0x3:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x12:
    case 0x13:
    {
      // Assert that optional3 and optional4 have values
      assert(command.optional3.has_value());
      assert(command.optional4.has_value());
      stream.Write(command.optional3.value())
        .Write(command.optional4.value());
      break;
    }
    default:
    {
      break;
    }
  }

  stream.Write(command.unk3);
}

void AcCmdCRUseMagicItemNotify::Read(
  AcCmdCRUseMagicItemNotify& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid);
  stream.Read(command.magicItemId);

  switch(command.magicItemId)
  {
    // Case 0xA and 0xB read 2x [3x floats]
    // and then fallthrough to read uint16_t vector
    case 0xa:
    case 0xb:
      for (auto& element : command.optional1.emplace().member1)
      {
        stream.Read(element);
      }
      for (auto& element : command.optional1.value().member2)
      {
        stream.Read(element);
      }
      [[fallthrough]];
    case 0x2:
    case 0x3:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x12:
    case 0x13:
    {
      auto& optional2 = command.optional2.emplace();

      stream.Read(optional2.size);
      optional2.list.resize(optional2.size);
      for (auto& element : optional2.list)
      {
        stream.Read(element);
      }
      break;
    }
    default:
    {
      break;
    }
  }

  stream.Read(command.unk3);
  // FIXME: wtf am i switching
  switch (command.magicItemId)
  {
    case 0x2:
    case 0x3:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x12:
    case 0x13:
    {
      stream.Read(command.optional3.emplace())
        .Read(command.optional4.emplace());
      break;
    }
    default:
    {
      break;
    }
  }
}

void AcCmdRCTriggerActivate::Write(
  const AcCmdRCTriggerActivate& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.triggerType)
    .Write(command.triggerValue)
    .Write(command.duration);
}

void AcCmdRCTriggerActivate::Read(
  AcCmdRCTriggerActivate& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.triggerType)
    .Read(command.triggerValue)
    .Read(command.duration);
}

void AcCmdCRActivateSkillEffect::Write(
  const AcCmdCRActivateSkillEffect& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.skillId)
    .Write(command.unk1)
    .Write(command.unk2);
}

void AcCmdCRActivateSkillEffect::Read(
  AcCmdCRActivateSkillEffect& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.skillId)
    .Read(command.unk1)
    .Read(command.unk2);
}

void AcCmdRCAddSkillEffect::Write(
  const AcCmdRCAddSkillEffect& command,
  SinkStream& stream)
{
  stream.Write(command.characterOid)
    .Write(command.effectId)
    .Write(command.duration)
    .Write(command.intensity);
}

void AcCmdRCAddSkillEffect::Read(
  AcCmdRCAddSkillEffect& command,
  SourceStream& stream)
{
  stream.Read(command.characterOid)
    .Read(command.effectId)
    .Read(command.duration)
    .Read(command.intensity);
}

} // namespace server::protocol
