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
  stream.Write(racer.pet.uid)
    .Write(racer.pet.tid)
    .Write(racer.pet.name)
    .Write(racer.pet.val3);
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

void RaceCommandEnterRoom::Write(
  const RaceCommandEnterRoom& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandEnterRoom::Read(
  RaceCommandEnterRoom& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.otp)
    .Read(command.roomUid);
}

void RaceCommandEnterRoomOK::Write(
  const RaceCommandEnterRoomOK& command,
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

void RaceCommandEnterRoomOK::Read(
  RaceCommandEnterRoomOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandEnterRoomCancel::Write(
  const RaceCommandEnterRoomCancel& command,
  SinkStream& stream)
{
}

void RaceCommandEnterRoomCancel::Read(
  RaceCommandEnterRoomCancel& command,
  SourceStream& stream)
{
}

void RaceCommandEnterRoomNotify::Write(
  const RaceCommandEnterRoomNotify& command,
  SinkStream& stream)
{
  WriteRacer(stream, command.racer);
  stream.Write(command.averageTimeRecord);
}

void RaceCommandEnterRoomNotify::Read(
  RaceCommandEnterRoomNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandChangeRoomOptions::Write(
  const RaceCommandChangeRoomOptions& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandChangeRoomOptions::Read(
  RaceCommandChangeRoomOptions& command,
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

void RaceCommandChangeRoomOptionsNotify::Write(
  const RaceCommandChangeRoomOptionsNotify& command,
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

void RaceCommandChangeRoomOptionsNotify::Read(
  RaceCommandChangeRoomOptionsNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandLeaveRoom::Write(
  const RaceCommandLeaveRoom& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandLeaveRoom::Read(
  RaceCommandLeaveRoom& command,
  SourceStream& stream)
{
  // Empty
}
void RaceCommandLeaveRoomOK::Write(
  const RaceCommandLeaveRoomOK& command,
  SinkStream& stream)
{
  // Empty
}

void RaceCommandLeaveRoomOK::Read(
  RaceCommandLeaveRoomOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandLeaveRoomNotify::Write(
  const RaceCommandLeaveRoomNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterId);
  stream.Write(command.unk0);
}

void RaceCommandLeaveRoomNotify::Read(
  RaceCommandLeaveRoomNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RaceCommandStartRace::Write(
  const RaceCommandStartRace& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandStartRace::Read(
  RaceCommandStartRace& command,
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

void RaceCommandStartRaceNotify::Write(
  const RaceCommandStartRaceNotify& command,
  SinkStream& stream)
{
  stream.Write(command.gameMode)
    .Write(command.skills)
    .Write(command.someonesOid)
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

  stream.Write(command.ip)
    .Write(command.port)
    .Write(command.unk6)
    .Write(command.unk9.unk0)
    .Write(command.unk9.unk1)
    .Write(command.unk9.unk2)
    .Write(command.unk9.unk3);

  stream.Write(static_cast<uint8_t>(command.unk9.unk4.size()));
  for (const auto& element : command.unk9.unk4)
  {
    stream.Write(element);
  }

  stream.Write(command.unk9.unk5)
    .Write(command.unk9.unk6)
    .Write(command.unk9.unk7)
    .Write(command.unk9.unk8)
    .Write(command.unk9.unk9)
    .Write(command.unk9.unk10)
    .Write(command.unk9.unk11);

  stream.Write(command.unk10.unk0)
    .Write(command.unk10.unk1)
    .Write(command.unk10.unk2)
    .Write(command.unk10.unk3);

  stream.Write(command.unk11)
    .Write(command.unk12);

  stream.Write(command.unk13.unk0)
    .Write(command.unk13.unk1);

  stream.Write(static_cast<uint8_t>(command.unk13.unk2.size()));
  for (const auto& element : command.unk13.unk2)
  {
    stream.Write(element);
  }

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

void RaceCommandStartRaceNotify::Read(
  RaceCommandStartRaceNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandStartRaceCancel::Write(
  const RaceCommandStartRaceCancel& command,
  SinkStream& stream)
{
  stream.Write(command.reason);
}

void RaceCommandStartRaceCancel::Read(
  RaceCommandStartRaceCancel& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandUserRaceTimer::Write(
  const RaceCommandUserRaceTimer& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandUserRaceTimer::Read(
  RaceCommandUserRaceTimer& command,
  SourceStream& stream)
{
  stream.Read(command.timestamp);
}

void RaceCommandUserRaceTimerOK::Write(
  const RaceCommandUserRaceTimerOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void RaceCommandUserRaceTimerOK::Read(
  RaceCommandUserRaceTimerOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandLoadingComplete::Write(
  const RaceCommandLoadingComplete& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandLoadingComplete::Read(
  RaceCommandLoadingComplete& command,
  SourceStream& stream)
{
  // Empty.
}

void RaceCommandLoadingCompleteNotify::Write(
  const RaceCommandLoadingCompleteNotify& command,
  SinkStream& stream)
{
  stream.Write(command.oid);
}

void RaceCommandLoadingCompleteNotify::Read(
  RaceCommandLoadingCompleteNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void RaceCommandChat::Write(
  const RaceCommandChat& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RaceCommandChat::Read(
  RaceCommandChat& command,
  SourceStream& stream)
{
  stream.Read(command.message)
    .Read(command.unknown);
}

void RaceCommandChatNotify::Write(
  const RaceCommandChatNotify& command,
  SinkStream& stream)
{
  stream.Write(command.author)
    .Write(command.message)
    .Write(command.unknown);
}

void RaceCommandChatNotify::Read(
  RaceCommandChatNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RaceCommandUpdatePet::Write(
  const RaceCommandUpdatePet& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RaceCommandUpdatePet::Read(
  RaceCommandUpdatePet& command,
  SourceStream& stream)
{
  stream.Read(command.petInfo);
  if (stream.GetCursor() - stream.Size() > 4)
    stream.Read(command.member2);
}

void RaceCommandUpdatePetCancel::Write(
  const RaceCommandUpdatePetCancel& command,
  SinkStream& stream)
{
  stream.Write(command.petInfo)
    .Write(command.member2)
    .Write(command.member3);
}

void RaceCommandUpdatePetCancel::Read(
  RaceCommandUpdatePetCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RaceCommandReadyRace::Write(
  const RaceCommandReadyRace& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RaceCommandReadyRace::Read(
  RaceCommandReadyRace& command,
  SourceStream& stream)
{
  // Empty.
}

void RaceCommandReadyRaceNotify::Write(
  const RaceCommandReadyRaceNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.ready);
}

void RaceCommandReadyRaceNotify::Read(
  RaceCommandReadyRaceNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RaceCommandCountdown::Write(
  const RaceCommandCountdown& command,
  SinkStream& stream)
{
  stream.Write(command.timestamp);
}

void RaceCommandCountdown::Read(
  RaceCommandCountdown& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

} // namespace server::protocol
