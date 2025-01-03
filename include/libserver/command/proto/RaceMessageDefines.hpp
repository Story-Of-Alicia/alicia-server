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

#ifndef RACE_MESSAGE_DEFINES_HPP
#define RACE_MESSAGE_DEFINES_HPP

#include "DataDefines.hpp"
#include "libserver/Util.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace alicia
{

enum class RoomOptionType : uint16_t
{
  Unk0 = 1 << 0,
  Unk1 = 1 << 1,
  Unk2 = 1 << 2,
  Unk3 = 1 << 3,
  Unk4 = 1 << 4,
  Unk5 = 1 << 5,
};

struct PlayerRacer
{
  // List length specified with a uint8_t
  std::vector<Item> characterEquipment{};
  Character character{};
  Horse horse{};
  uint32_t unk0{};
};

//! Racer
struct Racer
{
  uint8_t unk0{};
  uint8_t unk1{};
  uint32_t level{};
  uint32_t exp{};
  uint32_t uid{};
  std::string name{};
  uint8_t unk5{};
  uint32_t unk6{};
  uint8_t bitset{};
  bool isNPC{};

  std::optional<PlayerRacer> playerRacer{};
  std::optional<uint32_t> npcRacer{};

  struct
  {
    uint8_t unk0{};
    Struct6 anotherPlayerRelatedThing{};
  } unk8{};
  Struct7 yetAnotherPlayerRelatedThing{};
  Struct5 playerRelatedThing{};
  RanchUnk11 unk9{};
  uint8_t unk10{};
  uint8_t unk11{};
  uint8_t unk12{};
  uint8_t unk13{};
};

struct RoomDescription
{
  std::string name{};
  uint8_t unk0{}; // room id?
  std::string description{};
  uint8_t unk1{};
  uint8_t unk2{};
  uint16_t unk3{}; // map?
  uint8_t unk4{}; // 0 waiting room, 1 race started?
  uint16_t unk5{};
  uint8_t unk6{};
  uint8_t unk7{}; // 0: 3lv, 1: 12lv, 2 and beyond: nothing? 
};

struct RaceCommandEnterRoom
{
  uint32_t roomUid{};
  uint32_t otp{};
  uint32_t characterUid{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(const RaceCommandEnterRoom& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(RaceCommandEnterRoom& command, SourceStream& buffer);
};

struct RaceCommandEnterRoomOK
{
  // List size specified with a uint32_t. Max size 10
  std::vector<Racer> racers{};
  uint8_t unk0{};
  uint32_t unk1{};
  RoomDescription roomDescription{};
  
  uint32_t unk2{};
  uint16_t unk3{};
  uint32_t unk4{};
  uint32_t unk5{};
  uint32_t unk6{};

  uint32_t unk7{};
  uint16_t unk8{};

  // unk9: structure that depends on this+0x2980 == 2 (inside unk3?)
  struct
  {
    uint32_t unk0{};
    uint16_t unk1{};
    // List size specified with a uint8_t
    std::vector<uint32_t> unk2{};
  } unk9{};

  uint32_t unk10{};
  float unk11{};
  uint32_t unk12{};
  uint32_t unk13{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(const RaceCommandEnterRoomOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(RaceCommandEnterRoomOK& command, SourceStream& buffer);
};

struct RaceCommandEnterRoomCancel
{

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(const RaceCommandEnterRoomCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(RaceCommandEnterRoomCancel& command, SourceStream& buffer);
};

struct RaceCommandEnterRoomNotify
{
  Racer racer{};
  uint32_t unk0{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(const RaceCommandEnterRoomNotify& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(RaceCommandEnterRoomNotify& command, SourceStream& buffer);
};


struct RaceCommandChangeRoomOptions
{
  // Request consists of: short as a bitfield
  //  if & 1 != 0: string
  //  if & 2 != 0: byte
  //  if & 4 != 0: string
  //  if & 8 != 0: byte
  //  if  & 16 != 0: short
  //  if  & 32 != 0: byte
  RoomOptionType optionsBitfield{};
  std::string option0{};
  uint8_t option1{};
  std::string option2{};
  uint8_t option3{};
  uint16_t option4{};
  uint8_t option5{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const RaceCommandChangeRoomOptions& command, SinkStream& buffer);

//! Reader a command from a provided source buffer.
//! @param command Command.
//! @param buffer Source buffer.
  static void Read(
    RaceCommandChangeRoomOptions& command, SourceStream& buffer);
};

struct RaceCommandChangeRoomOptionsNotify
{
  // Response consists of: short as a bitfield
  //  if & 1 != 0: string
  //  if & 2 != 0: byte
  //  if & 4 != 0: string
  //  if & 8 != 0: byte
  //  if  & 16 != 0: short
  //  if  & 32 != 0: byte
  RoomOptionType optionsBitfield{};
  std::string option0{};
  uint8_t option1{};
  std::string option2{};
  uint8_t option3{};
  uint16_t option4{};
  uint8_t option5{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const RaceCommandChangeRoomOptionsNotify& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    RaceCommandChangeRoomOptionsNotify& command, SourceStream& buffer);
};

} // namespace alicia

#endif // RACE_MESSAGE_DEFINES_HPP
