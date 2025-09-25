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
  stream.Write(static_cast<uint8_t>(playerRacer.characterEquipment.size()));

  for (const Item& item : playerRacer.characterEquipment)
  {
    stream.Write(item);
  }

  stream.Write(playerRacer.character)
    .Write(playerRacer.mount)
    .Write(playerRacer.unk0);
}

void WriteRacer(SinkStream& stream, const Racer& racer)
{
  stream.Write(racer.member1)
    .Write(racer.member2)
    .Write(racer.level)
    .Write(racer.oid)
    .Write(racer.uid)
    .Write(racer.name)
    .Write(racer.unk5)
    .Write(racer.unk6)
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
    .Write(roomDescription.description)
    .Write(roomDescription.unk1)
    .Write(roomDescription.gameMode)
    .Write(roomDescription.mapBlockId)
    .Write(roomDescription.teamMode)
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
    .Write(command.unk13);
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
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk0)
  {
    stream.Read(command.name);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk1)
  {
    stream.Read(command.playerCount);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk2)
  {
    stream.Read(command.description);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk3)
  {
    stream.Read(command.option3);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk4)
  {
    stream.Read(command.mapBlockId);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk5)
  {
    stream.Read(command.hasRaceStarted);
  }
}

void AcCmdCRChangeRoomOptionsNotify::Write(
  const AcCmdCRChangeRoomOptionsNotify& command,
  SinkStream& stream)
{
  stream.Write(command.optionsBitfield);
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk0)
  {
    stream.Write(command.name);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk1)
  {
    stream.Write(command.playerCount);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk2)
  {
    stream.Write(command.description);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk3)
  {
    stream.Write(command.option3);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk4)
  {
    stream.Write(command.mapBlockId);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk5)
  {
    stream.Write(command.hasRaceStarted);
  }
}

void AcCmdCRChangeRoomOptionsNotify::Read(
  AcCmdCRChangeRoomOptionsNotify& command,
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
  stream.Write(command.unk0)
   .Write(command.unk1)
   .Write(command.unk2)
   .Write(command.unk3);

  stream.Write(static_cast<uint8_t>(
    command.unk4.size()));
  for (const auto& element : command.unk4)
  {
    stream.Write(element);
  }

  stream.Write(command.unk5)
    .Write(command.unk6)
    .Write(command.unk7)
    .Write(command.unk8)
    .Write(command.unk9)
    .Write(command.unk10)
    .Write(command.unk11);
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
    .Write(command.skills)
    .Write(command.racerOid)
    .Write(command.member4)
    .Write(command.mapBlockId);

  stream.Write(static_cast<uint8_t>(command.racers.size()));
  for (const auto& element : command.racers)
  {
    stream.Write(element.oid)
      .Write(element.name)
      .Write(element.unk2)
      .Write(element.unk3)
      .Write(element.unk4)
      .Write(element.p2dId)
      .Write(element.unk6)
      .Write(element.unk7);
  }

  stream.Write(
    boost::asio::detail::socket_ops::host_to_network_long(
      command.ip))
    .Write(command.port)
    .Write(command.unk6)
    .Write(command.unk9)
    .Write(command.unk10);

  stream.Write(command.unk11)
    .Write(command.unk12)
    .Write(command.unk13);

  stream.Write(command.unk14)
    .Write(command.unk15)
    .Write(command.unk16)
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
  stream.Write(command.author)
    .Write(command.message)
    .Write(command.unknown);
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
    .Write(command.ready);
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
    .Read(command.member2)
    .Read(command.member3);
}

void AcCmdUserRaceFinalNotify::Write(
  const AcCmdUserRaceFinalNotify& command,
  SinkStream& stream)
{
  stream.Write(command.oid)
    .Write(command.member2);
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
      .Write(score.member16)
      .Write(score.bitset)
      .Write(score.member18)
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

} // namespace server::protocol
