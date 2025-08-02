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

#include "CommonStructureDefinitions.hpp"
#include "libserver/network/command/CommandProtocol.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace server::protocol
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

struct Avatar
{
  // List length specified with a uint8_t
  std::vector<Item> characterEquipment{};
  Character character{};
  Horse mount{};
  uint32_t unk0{};
};

//! Racer
struct Racer
{
  uint8_t member1{1};
  uint8_t member2{2};
  uint32_t level{};
  uint32_t oid{};
  uint32_t uid{};
  std::string name{};
  uint8_t unk5{3};
  uint32_t unk6{4};
  bool isHidden{};
  bool isNPC{};

  std::optional<Avatar> avatar{};
  std::optional<uint32_t> npcTid{};

  struct
  {
    uint8_t unk0{};
    Rent rent{};
  } unk8{};

  Pet pet{};
  Guild guild{};
  League unk9{};
  uint8_t unk10{5};
  uint8_t unk11{6};
  uint8_t unk12{7};
  uint8_t unk13{8};
};

struct RoomDescription
{
  std::string name{};
  uint8_t playerCount{};
  std::string description{};
  uint8_t unk1{};
  GameMode gameMode{};
  //! From the table `MapBlockInfo`.
  uint16_t mapBlockId{};
  // 0 waiting room, 1 race started?
  TeamMode teamMode{};
  uint16_t missionId{};
  uint8_t unk6{};
  // 0: 3lv, 1: 12lv, 2 and beyond: nothing?
  uint8_t skillBracket{};
};

struct RaceCommandEnterRoom
{
  uint32_t characterUid{};
  uint32_t otp{};
  uint32_t roomUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCREnterRoom;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(const RaceCommandEnterRoom& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(RaceCommandEnterRoom& command, SourceStream& stream);
};

struct RaceCommandEnterRoomOK
{
  // List size specified with a uint32_t. Max size 10
  std::vector<Racer> racers{};

  uint8_t nowPlaying{};
  uint32_t uid{};
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

  static Command GetCommand()
  {
    return Command::AcCmdCREnterRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(const RaceCommandEnterRoomOK& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(RaceCommandEnterRoomOK& command, SourceStream& stream);
};

struct RaceCommandEnterRoomCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCREnterRoomCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(const RaceCommandEnterRoomCancel& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(RaceCommandEnterRoomCancel& command, SourceStream& stream);
};

struct RaceCommandEnterRoomNotify
{
  Racer racer{};
  uint32_t averageTimeRecord{};

  static Command GetCommand()
  {
    return Command::AcCmdCREnterRoomNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(const RaceCommandEnterRoomNotify& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(RaceCommandEnterRoomNotify& command, SourceStream& stream);
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
  std::string name{};
  uint8_t playerCount{};
  std::string description{};
  uint8_t option3{};
  uint16_t mapBlockId{};
  uint8_t hasRaceStarted{};

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeRoomOptions;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandChangeRoomOptions& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandChangeRoomOptions& command,
    SourceStream& stream);
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
  std::string name{};
  uint8_t playerCount{};
  std::string description{};
  uint8_t option3{};
  uint16_t mapBlockId{};
  uint8_t hasRaceStarted{};

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeRoomOptionsNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandChangeRoomOptionsNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandChangeRoomOptionsNotify& command,
    SourceStream& stream);
};

struct RaceCommandLeaveRoom
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRoom;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandLeaveRoom& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandLeaveRoom& command,
    SourceStream& stream);
};
struct RaceCommandLeaveRoomOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandLeaveRoomOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandLeaveRoomOK& command,
    SourceStream& stream);
};
struct RaceCommandLeaveRoomNotify
{
  uint32_t characterId{};
  uint32_t unk0{};
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRoomNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandLeaveRoomNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandLeaveRoomNotify& command,
    SourceStream& stream);
};

struct RaceCommandStartRace
{
  // List size specified with a byte. Max size 10 (potentially)
  std::vector<uint16_t> unk0{};

  static Command GetCommand()
  {
    return Command::AcCmdCRStartRace;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandStartRace& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandStartRace& command,
    SourceStream& stream);
};

struct RaceCommandStartRaceNotify
{
  GameMode gameMode{};
  bool skills{};
  // this is an oid of a special player
  uint16_t someonesOid{};
  uint32_t member4{}; // Room ID?
  uint16_t mapBlockId{};

  // List size specified with a uint8_t. Max size 10
  struct Racer
  {
    uint16_t oid{};
    std::string name{};
    uint8_t unk2{};
    uint8_t unk3{};
    uint16_t unk4{};
    uint32_t p2dId{};
    uint16_t unk6{}; // Index?
    uint32_t unk7{};
  };
  std::vector<Racer> racers{};

  uint32_t ip{};
  uint16_t port{};

  uint8_t unk6{};

  struct
  {
    uint16_t unk0{};
    uint8_t unk1{};
    uint8_t unk2{};
    uint32_t unk3{};
    // List size specified with a uint8_t. Max size 20
    std::vector<uint32_t> unk4{};

    // If isBusy == 3?
    uint16_t unk5{};
    uint16_t unk6{};
    uint16_t unk7{};
    uint16_t unk8{};
    uint16_t unk9{};

    uint8_t unk10{};
    uint32_t unk11{};
  } unk9{};

  struct
  {
    uint32_t unk0{};
    uint32_t unk1{};
    uint32_t unk2{};
    uint32_t unk3{};
  } unk10{};

  uint16_t unk11{};
  uint8_t unk12{};

  struct
  {
    uint8_t unk0{};
    uint32_t unk1{};
    // List size specified with a byte. Max size 3
    std::vector<uint16_t> unk2{};
  } unk13{};

  uint8_t unk14{};
  uint32_t unk15{};
  uint32_t unk16{};
  uint8_t unk17{};

  // List size specified with a byte. Max size 8
  struct Unk18Element
  {
    uint16_t unk0{};
    // List size specified with a byte. Max size 3
    std::vector<uint32_t> unk1{};
  };
  std::vector<Unk18Element> unk18{};

  static Command GetCommand()
  {
    return Command::AcCmdCRStartRaceNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandStartRaceNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandStartRaceNotify& command,
    SourceStream& stream);
};

struct RaceCommandStartRaceCancel
{
  uint8_t reason{};

  static Command GetCommand()
  {
    return Command::AcCmdCRStartRaceCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandStartRaceCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandStartRaceCancel& command,
    SourceStream& stream);
};

struct RaceCommandUserRaceTimer
{
  // count of 100ns intervals since the system start
  uint64_t timestamp{}; // potentially

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceTimer;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandUserRaceTimer& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandUserRaceTimer& command,
    SourceStream& stream);
};

struct RaceCommandUserRaceTimerOK
{
  uint64_t unk0{};
  uint64_t unk1{};

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceTimerOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandUserRaceTimerOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandUserRaceTimerOK& command,
    SourceStream& stream);
};

struct RaceCommandLoadingComplete
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLoadingComplete;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandLoadingComplete& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandLoadingComplete& command,
    SourceStream& stream);
};

struct RaceCommandLoadingCompleteNotify
{
  uint16_t oid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRLoadingCompleteNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandLoadingCompleteNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandLoadingCompleteNotify& command,
    SourceStream& stream);
};

struct RaceCommandChat
{
  std::string message;
  uint8_t unknown{};

  static Command GetCommand()
  {
    return Command::AcCmdCRChat;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandChat& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandChat& command,
    SourceStream& stream);
};

struct RaceCommandChatNotify
{
  std::string author;
  std::string message;
  uint8_t unknown{};

  static Command GetCommand()
  {
    return Command::AcCmdCRChatNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandChatNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandChatNotify& command,
    SourceStream& stream);
};

struct RaceCommandUpdatePet
{
  PetInfo petInfo{};
  //! optional
  uint32_t member2{};
  static Command GetCommand()
  {
    return Command::AcCmdRCUpdatePet;
  }
  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandUpdatePet& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandUpdatePet& command,
    SourceStream& stream);
};

struct RaceCommandUpdatePetCancel
{
  PetInfo petInfo{};
  uint32_t member2{};
  uint8_t member3{};

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandUpdatePetCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandUpdatePetCancel& command,
    SourceStream& stream);
};

struct RaceCommandReadyRace
{
  static Command GetCommand()
  {
    return Command::AcCmdCRReadyRace;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandReadyRace& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandReadyRace& command,
    SourceStream& stream);
};

struct RaceCommandReadyRaceNotify
{
  uint32_t characterUid{};
  uint8_t ready{};

  static Command GetCommand()
  {
    return Command::AcCmdCRReadyRaceNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandReadyRaceNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandReadyRaceNotify& command,
    SourceStream& stream);
};

struct RaceCommandCountdown
{
  int64_t timestamp{}; // potentially

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceCountdown;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RaceCommandCountdown& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RaceCommandCountdown& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // RACE_MESSAGE_DEFINES_HPP
