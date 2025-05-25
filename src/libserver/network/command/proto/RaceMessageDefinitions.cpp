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

namespace alicia
{

void WritePlayerRacer(SinkStream& stream, const PlayerRacer& playerRacer)
{
  stream.Write(static_cast<uint8_t>(playerRacer.characterEquipment.size()));

  for (const Item& item : playerRacer.characterEquipment)
  {
    stream.Write(item);
  }

  stream.Write(playerRacer.character)
    .Write(playerRacer.horse)
    .Write(playerRacer.unk0);
}

void WriteRacer(SinkStream& stream, const Racer& racer)
{
  stream.Write(racer.unk0)
    .Write(racer.unk1)
    .Write(racer.level)
    .Write(racer.exp)
    .Write(racer.uid)
    .Write(racer.name)
    .Write(racer.unk5)
    .Write(racer.unk6)
    .Write(racer.bitset)
    .Write(racer.isNPC);

  if (racer.isNPC)
  {
    stream.Write(racer.npcRacer.value());
  }
  else
  {
    WritePlayerRacer(stream, racer.playerRacer.value());
  }

  stream.Write(racer.unk8.unk0)
    .Write(racer.unk8.anotherPlayerRelatedThing.mountUid)
    .Write(racer.unk8.anotherPlayerRelatedThing.val1)
    .Write(racer.unk8.anotherPlayerRelatedThing.val2);
  stream.Write(racer.yetAnotherPlayerRelatedThing.val0)
    .Write(racer.yetAnotherPlayerRelatedThing.val1)
    .Write(racer.yetAnotherPlayerRelatedThing.val2)
    .Write(racer.yetAnotherPlayerRelatedThing.val3);
  stream.Write(racer.playerRelatedThing.val0)
    .Write(racer.playerRelatedThing.val1)
    .Write(racer.playerRelatedThing.val2)
    .Write(racer.playerRelatedThing.val3)
    .Write(racer.playerRelatedThing.val4)
    .Write(racer.playerRelatedThing.val5)
    .Write(racer.playerRelatedThing.val6);
  stream.Write(racer.unk9);
  stream.Write(racer.unk10)
    .Write(racer.unk11)
    .Write(racer.unk12)
    .Write(racer.unk13);
}

void WriteRoomDescription(SinkStream& stream, const RoomDescription& roomDescription)
{
  stream.Write(roomDescription.name)
    .Write(roomDescription.val_between_name_and_desc)
    .Write(roomDescription.description)
    .Write(roomDescription.unk1)
    .Write(roomDescription.unk2)
    .Write(roomDescription.unk3)
    .Write(roomDescription.unk4)
    .Write(roomDescription.missionId)
    .Write(roomDescription.unk6)
    .Write(roomDescription.unk7);
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
  stream.Read(command.roomUid)
    .Read(command.otp)
    .Read(command.characterUid);
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

  stream.Write(command.unk0)
    .Write(command.unk1);

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
  stream.Write(command.unk0);
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
    stream.Read(command.val_between_name_and_desc);
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
    stream.Read(command.map);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk5)
  {
    stream.Read(command.raceStarted);
  }
}

void RaceCommandChangeRoomOptionsNotify::Write(
  const RaceCommandChangeRoomOptionsNotify& command,
  SinkStream& stream)
{
  stream.Write(command.optionsBitfield);
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk0)
  {
    stream.Write(command.option0);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk1)
  {
    stream.Write(command.option1);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk2)
  {
    stream.Write(command.option2);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk3)
  {
    stream.Write(command.option3);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk4)
  {
    stream.Write(command.option4);
  }
  if ((uint16_t)command.optionsBitfield & (uint16_t)RoomOptionType::Unk5)
  {
    stream.Write(command.option5);
  }
}

void RaceCommandChangeRoomOptionsNotify::Read(
  RaceCommandChangeRoomOptionsNotify& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
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
  stream.Write(command.gamemode)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.map);

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

void UserRaceTimer::Write(
  const UserRaceTimer& command,
  SinkStream& stream)
{
  throw std::logic_error("Not implemented.");
}

void UserRaceTimer::Read(
  UserRaceTimer& command,
  SourceStream& stream)
{
  stream.Read(command.timestamp);
}

void UserRaceTimerOK::Write(
  const UserRaceTimerOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void UserRaceTimerOK::Read(
  UserRaceTimerOK& command,
  SourceStream& stream)
{
  throw std::logic_error("Not implemented.");
}

} // namespace alicia
