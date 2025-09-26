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
  // 0 for regular players, everything else leader
  uint8_t member1{0};
  uint8_t member2{3};
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
  uint8_t gameMode{};
  //! From the table `MapBlockInfo`.
  uint16_t mapBlockId{};
  // 0 waiting room, 1 race started?
  TeamMode teamMode{};
  uint16_t missionId{};
  uint8_t unk6{};
  // 0: 3lv, 1: 12lv, 2 and beyond: nothing?
  uint8_t skillBracket{};
};

struct AcCmdCREnterRoom
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
  static void Write(const AcCmdCREnterRoom& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(AcCmdCREnterRoom& command, SourceStream& stream);
};

struct AcCmdCREnterRoomOK
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
  static void Write(const AcCmdCREnterRoomOK& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(AcCmdCREnterRoomOK& command, SourceStream& stream);
};

struct AcCmdCREnterRoomCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCREnterRoomCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(const AcCmdCREnterRoomCancel& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(AcCmdCREnterRoomCancel& command, SourceStream& stream);
};

struct AcCmdCREnterRoomNotify
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
  static void Write(const AcCmdCREnterRoomNotify& command, SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(AcCmdCREnterRoomNotify& command, SourceStream& stream);
};

struct AcCmdCRChangeRoomOptions
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
    const AcCmdCRChangeRoomOptions& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeRoomOptions& command,
    SourceStream& stream);
};

struct AcCmdCRChangeRoomOptionsNotify
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
    const AcCmdCRChangeRoomOptionsNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeRoomOptionsNotify& command,
    SourceStream& stream);
};

struct AcCmdCRLeaveRoom
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRoom;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRLeaveRoom& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLeaveRoom& command,
    SourceStream& stream);
};

struct AcCmdCRLeaveRoomOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRLeaveRoomOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLeaveRoomOK& command,
    SourceStream& stream);
};

struct AcCmdCRLeaveRoomNotify
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
    const AcCmdCRLeaveRoomNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLeaveRoomNotify& command,
    SourceStream& stream);
};

struct AcCmdCRStartRace
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
    const AcCmdCRStartRace& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRStartRace& command,
    SourceStream& stream);
};

struct AcCmdCRStartRaceNotify
{
  uint8_t gameMode{};
  bool skills{};
  // this is an oid of a special player
  uint16_t racerOid{};
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

  struct Struct1
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

    static void Write(
      const Struct1& command,
      SinkStream& stream);

    static void Read(
      Struct1& command,
      SourceStream& stream);
  } unk9{};

  struct Struct2
  {
    uint32_t unk0{};
    uint32_t unk1{};
    uint32_t unk2{};
    uint32_t unk3{};

    static void Write(
      const Struct2& command,
      SinkStream& stream);

    static void Read(
      Struct2& command,
      SourceStream& stream);
  } unk10{};

  uint16_t unk11{};
  uint8_t unk12{};

  struct Struct3
  {
    uint8_t unk0{};
    uint32_t unk1{};
    // List size specified with a byte. Max size 3
    std::vector<uint16_t> unk2{};

    static void Write(
      const Struct3& command,
      SinkStream& stream);

    static void Read(
      Struct3& command,
      SourceStream& stream);
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
    const AcCmdCRStartRaceNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRStartRaceNotify& command,
    SourceStream& stream);
};

struct AcCmdCRStartRaceCancel
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
    const AcCmdCRStartRaceCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRStartRaceCancel& command,
    SourceStream& stream);
};

struct AcCmdUserRaceTimer
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
    const AcCmdUserRaceTimer& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceTimer& command,
    SourceStream& stream);
};

struct AcCmdUserRaceTimerOK
{
  uint64_t clientTimestamp{};
  uint64_t serverTimestamp{};

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceTimerOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceTimerOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceTimerOK& command,
    SourceStream& stream);
};

struct AcCmdCRLoadingComplete
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLoadingComplete;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRLoadingComplete& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLoadingComplete& command,
    SourceStream& stream);
};

struct AcCmdCRLoadingCompleteNotify
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
    const AcCmdCRLoadingCompleteNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLoadingCompleteNotify& command,
    SourceStream& stream);
};

struct AcCmdCRChat
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
    const AcCmdCRChat& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChat& command,
    SourceStream& stream);
};

struct AcCmdCRChatNotify
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
    const AcCmdCRChatNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChatNotify& command,
    SourceStream& stream);
};

struct AcCmdCRReadyRace
{
  static Command GetCommand()
  {
    return Command::AcCmdCRReadyRace;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRReadyRace& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRReadyRace& command,
    SourceStream& stream);
};

struct AcCmdCRReadyRaceNotify
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
    const AcCmdCRReadyRaceNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRReadyRaceNotify& command,
    SourceStream& stream);
};

struct AcCmdUserRaceCountdown
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
    const AcCmdUserRaceCountdown& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceCountdown& command,
    SourceStream& stream);
};

struct AcCmdUserRaceFinal
{
  int16_t oid{};
  uint32_t member2{};
  float member3{};

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceFinal;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceFinal& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceFinal& command,
    SourceStream& stream);
};

struct AcCmdUserRaceFinalNotify
{
  int16_t oid{};
  uint32_t member2{};

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceFinal;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceFinalNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceFinalNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRaceResult
{
  uint8_t member1{};
  uint32_t member2{};
  uint32_t member3{};
  uint32_t member4{};
  uint32_t member5{};
  uint32_t member6{};
  uint32_t member7{};
  uint32_t member8{};
  uint32_t member9{};
  //! Max count 32
  std::vector<uint32_t> member10{};
  uint8_t member11{};
  uint32_t member12{};
  uint16_t member13{};
  uint8_t member14{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRaceResult;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRaceResult& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRaceResult& command,
    SourceStream& stream);
};

struct AcCmdCRRaceResultOK
{
  uint8_t member1{};
  uint64_t member2{};
  uint16_t member3{};
  uint16_t member4{};
  uint8_t member5{};
  uint32_t member6{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRaceResultOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRaceResultOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRaceResultOK& command,
    SourceStream& stream);
};

struct AcCmdRCRaceResultNotify
{
  struct ScoreInfo
  {
    uint32_t uid{1};
    std::string name{"abc"};
    //! Time in milliseconds.
    uint32_t courseTime{0};
    uint32_t member4{0};
    uint32_t experience{0};
    uint32_t member6{0};
    uint32_t carrots{0};
    uint32_t level{0};
    uint32_t member9{0};
    uint32_t member10{0};
    uint16_t member11{0};
    uint16_t member12{0};
    //! Time in milliseconds.
    uint32_t recordTimeDifference{0};
    uint32_t member14{3000};
    uint32_t member15{4000};
    AcCmdCRStartRaceNotify::Struct2 member16{
      .unk0 = 100,
      .unk1 = 200,
      .unk2 = 300,
      .unk3 = 400};
    uint32_t bitset{0xFFFFFFFF};
    std::string member18{"something"};
    uint16_t member19{0};
    uint8_t member20{0};
    uint32_t bonusCarrots{0};
    uint32_t member22{0};
    AcCmdCRStartRaceNotify::Struct1 member23{};
    uint32_t member24{0};
    uint8_t member25{0};
    uint32_t member26{0};
    uint32_t member27{0};
  };

  //! Max 16 entries, short as size
  std::vector<ScoreInfo> scores{};
  AcCmdCRStartRaceNotify::Struct3 member2{};

  uint32_t member3{};
  uint32_t member4{};

  static Command GetCommand()
  {
    return Command::AcCmdRCRaceResultNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCRaceResultNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCRaceResultNotify& command,
    SourceStream& stream);
};

struct AcCmdCRP2PResult
{
  uint16_t oid{};
  uint32_t member2{};
  std::array<std::string, 3> podium{};

  static Command GetCommand()
  {
    return Command::AcCmdCRP2PResult;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRP2PResult& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRP2PResult& command,
    SourceStream& stream);
};

struct AcCmdUserRaceP2PResult
{
  struct Something
  {
    uint16_t oid{};
    uint8_t member2{};
  };

  //! Max 16 entries.
  std::vector<Something> member1;

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceP2PResult;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceP2PResult& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceP2PResult& command,
    SourceStream& stream);
};

struct AcCmdGameRaceP2PResult
{
  struct Something
  {
    uint16_t oid{};
    uint8_t member2{};
  };

  //! Max 16 entries.
  std::vector<Something> member1;

  static Command GetCommand()
  {
    return Command::AcCmdGameRaceP2PResult;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdGameRaceP2PResult& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdGameRaceP2PResult& command,
    SourceStream& stream);
};

struct AcCmdCRAwardStart
{
  uint32_t member1;

  static Command GetCommand()
  {
    return Command::AcCmdCRAwardStart;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRAwardStart& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRAwardStart& command,
    SourceStream& stream);
};

struct AcCmdCRAwardEnd
{
  static Command GetCommand()
  {
    return Command::AcCmdCRAwardEnd;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRAwardEnd& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRAwardEnd& command,
    SourceStream& stream);
};

struct AcCmdRCAwardNotify
{
  uint32_t member1;

  static Command GetCommand()
  {
    return Command::AcCmdRCAwardNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCAwardNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCAwardNotify& command,
    SourceStream& stream);
};

struct AcCmdCRAwardEndNotify
{
  static Command GetCommand()
  {
    return Command::AcCmdCRAwardEndNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRAwardEndNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRAwardEndNotify& command,
    SourceStream& stream);
};

struct AcCmdCRStarPointGet
{
  uint16_t characterOid; // oid?
  uint32_t unk1;
  uint32_t gainedBoostAmount;

  static Command GetCommand()
  {
    return Command::AcCmdCRStarPointGet;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRStarPointGet& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRStarPointGet& command,
    SourceStream& stream);
};

struct AcCmdCRStarPointGetOK
{
  uint16_t characterOid; // oid?
  uint32_t boosterGauge;
  uint8_t unk2;

  static Command GetCommand()
  {
    return Command::AcCmdCRStarPointGetOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRStarPointGetOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRStarPointGetOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestSpur
{
  uint16_t characterOid;
  uint8_t activeBoosters;
  uint8_t comboBreak; // combo break?

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestSpur;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestSpur& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestSpur& command,
    SourceStream& stream);
};

struct AcCmdCRRequestSpurOK
{
  uint16_t characterOid;
  uint8_t activeBoosters;
  uint32_t unk2; // current star point? (gauge)
  uint8_t comboBreak;

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestSpurOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestSpurOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestSpurOK& command,
    SourceStream& stream);
};

struct AcCmdCRHurdleClearResult
{
  uint16_t characterOid;
  enum class HurdleClearType : uint8_t
  {
    Perfect = 0,
    Good = 1,
    DoubleJumpOrGlide = 2,
    Collision = 3
  };
  HurdleClearType hurdleClearType;

  static Command GetCommand()
  {
    return Command::AcCmdCRHurdleClearResult;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRHurdleClearResult& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHurdleClearResult& command,
    SourceStream& stream);
};

struct AcCmdCRHurdleClearResultOK
{
  uint16_t characterOid;
  protocol::AcCmdCRHurdleClearResult::HurdleClearType hurdleClearType;
  //! Max combo is 99
  uint32_t jumpCombo;
  uint32_t unk3;

  static Command GetCommand()
  {
    return Command::AcCmdCRHurdleClearResultOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRHurdleClearResultOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHurdleClearResultOK& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // RACE_MESSAGE_DEFINES_HPP
