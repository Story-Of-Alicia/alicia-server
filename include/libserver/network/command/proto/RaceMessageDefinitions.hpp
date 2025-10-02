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
  Name = 1 << 0,
  PlayerCount = 1 << 1,
  Password = 1 << 2,
  GameMode = 1 << 3,
  MapBlockId = 1 << 4,
  NPCRace = 1 << 5,
};

enum class TeamColor : uint32_t
{
  Solo = 1,
  Red = 2,
  Blue = 3
};

struct Avatar
{
  // List length specified with a uint8_t
  std::vector<Item> equipment{};
  Character character{};
  Horse mount{};
  uint32_t unk0{};
};

//! Racer
struct Racer
{
  bool isMaster{};
  uint8_t member2{};
  uint32_t level{};
  uint32_t oid{};
  uint32_t uid{};
  std::string name{};
  bool isReady{};
  TeamColor teamColor{TeamColor::Solo};
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
  uint8_t unk10{0};
  uint8_t unk11{0};
  uint8_t unk12{};
  uint8_t unk13{};
};

struct RoomDescription
{
  std::string name{};
  uint8_t playerCount{};
  std::string password{};
  // disables/enables adv map selection
  uint8_t gameModeMaps{};
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
  uint32_t scrambleValue{};

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
  RoomOptionType optionsBitfield{};
  std::string name{};
  uint8_t playerCount{};
  std::string password{};
  uint8_t gameMode{};
  uint16_t mapBlockId{};
  uint8_t npcRace{};

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
  RoomOptionType optionsBitfield{};
  std::string name{};
  uint8_t playerCount{};
  std::string password{}; // password
  uint8_t gameMode{};
  uint16_t mapBlockId{};
  uint8_t npcRace{};

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

struct AcCmdCRChangeTeam
{
  uint32_t characterOid{};
  TeamColor teamColor{};

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeTeam;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeTeam& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeTeam& command,
    SourceStream& stream);
};

struct AcCmdCRChangeTeamOK
{
  uint32_t characterOid{};
  TeamColor teamColor{};

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeTeamOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeTeamOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeTeamOK& command,
    SourceStream& stream);
};

struct AcCmdCRChangeTeamNotify
{
  uint32_t characterOid{};
  TeamColor teamColor{};

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeTeamNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeTeamNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeTeamNotify& command,
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
  TeamMode teamMode{};
  // this is an oid of a special player
  uint16_t hostOid{};
  uint32_t member4{}; // Room ID?
  uint16_t mapBlockId{};

  // List size specified with a uint8_t. Max size 10
  struct Player
  {
    uint16_t oid{};
    std::string name{};
    uint8_t unk2{};
    uint8_t unk3{};
    uint16_t p2dId{};
    TeamColor teamColor{};
    uint16_t unk6{}; // Index?
    uint32_t unk7{};
  };
  std::vector<Player> racers{};

  uint32_t p2pRelayAddress{};
  uint16_t p2pRelayPort{};

  uint8_t unk6{};

  struct Struct1
  {
    uint16_t member1{};
    uint8_t member2{};
    uint8_t member3{};
    uint32_t member4{};
    // List size specified with a uint8_t. Max size 20
    std::vector<uint32_t> member5{};

    // only present if member4 == 3
    struct Optional
    {
      uint16_t member6{};
      uint16_t member8{};
      uint16_t member9{};
      uint16_t member10{};
      uint16_t member11{};
      uint8_t member12{};
    } optional{};

    uint32_t member13{};

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

  uint16_t missionId{};
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
  //! Carnival (FestivalMissionInfo)
  uint32_t carnivalType{};
  //! Weather (MapWeatherInfo)
  //! Snow has snow, rain only has cloudy weather
  uint32_t weatherType{};
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
  std::string message;
  std::string author;
  bool isSystem{};

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
  bool isReady{};

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
  uint32_t courseTime{};
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
  uint16_t oid{};
  uint32_t courseTime{};

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
    uint32_t uid{};
    std::string name{};
    //! Time in milliseconds.
    uint32_t courseTime{};
    uint32_t member4{};
    uint32_t experience{};
    uint32_t member6{};
    uint32_t carrots{};
    uint32_t level{};
    // this is copied as memcpy
    uint32_t member9{};
    uint32_t member10{};
    uint16_t member11{};
    uint16_t member12{};
    //! Time in milliseconds.
    uint32_t recordTimeDifference{};
    uint32_t member14{};
    uint32_t member15{};
    AcCmdCRStartRaceNotify::Struct2 achievements{};
    enum Bitset : uint32_t
    {
      Connected = 1 << 7,
      HasLevelUpBonus = 1 << 8,
      PcBang = 1 << 10,
    } bitset;
    std::string mountName{};
    uint16_t member19{};
    uint8_t member20{};
    uint32_t bonusCarrots{};
    uint32_t member22{};
    AcCmdCRStartRaceNotify::Struct1 member23{};
    uint32_t member24{};
    uint8_t member25{};
    uint32_t member26{};
    uint32_t member27{};
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
  //! Oid of the calling character
  uint16_t characterOid; // oid?
  uint32_t unk1;
  uint32_t gainedStarPoints;

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
  //! Oid of the affected character
  uint16_t characterOid;
  //! Speed/magic boost value
  uint32_t starPointValue;
  //! Only works on magic gamemode, will give magic item regardless of magic gauge
  bool giveMagicItem;

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
  uint32_t startPointValue; // current star point? (gauge)
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
  AcCmdCRHurdleClearResult::HurdleClearType hurdleClearType;
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

struct AcCmdCRStartingRate
{
  uint16_t characterOid;
  uint32_t unk1; // Forward velocity??
  uint32_t boostGained;

  static Command GetCommand()
  {
    return Command::AcCmdCRStartingRate;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRStartingRate& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRStartingRate& command,
    SourceStream& stream);
};

struct AcCmdCRRequestMagicItem
{
  uint16_t member1; // character oid?
  uint32_t member2; // item type? 0 = request random?

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestMagicItem;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestMagicItem& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestMagicItem& command,
    SourceStream& stream);
};

struct AcCmdCRRequestMagicItemOK
{
  uint16_t member1; // character oid?
  uint32_t member2; // item type?
  uint32_t member3; // star point reset?

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestMagicItemOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestMagicItemOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestMagicItemOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestMagicItemNotify
{
  uint32_t member1; // item id?
  uint16_t member2; // character oid?

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestMagicItemNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestMagicItemNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestMagicItemNotify& command,
    SourceStream& stream);
};

struct AcCmdUserRaceUpdatePos
{
  //! Character oid
  uint16_t oid{};
  //! Position
  std::array<float, 3> member2{};
  //! Rotation
  std::array<float, 3> member3{};
  //! Speed
  float member4{};
  //! 1 = In the air
  uint16_t member5{};
  //! Race track progress
  float member6{};
  //! Ticks since connected to race director?
  uint32_t member7{};

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceUpdatePos;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceUpdatePos& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceUpdatePos& command,
    SourceStream& stream);
};

struct AcCmdRCRoomCountdown
{
  float member0{};
  uint16_t member1{};

  static Command GetCommand()
  {
    return Command::AcCmdRCRoomCountdown;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCRoomCountdown& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCRoomCountdown& command,
    SourceStream& stream);
};

struct AcCmdRCRoomCountdownCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdRCRoomCountdownCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCRoomCountdownCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCRoomCountdownCancel& command,
    SourceStream& stream);
};

struct AcCmdCRChangeMasterNotify
{
  //! A character UID of the new master.
  uint32_t masterUid;

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeMasterNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeMasterNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeMasterNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRelayCommand
{
  //! Character OID of the sender
  uint16_t senderOid{};
  //! Raw relay data
  std::vector<uint8_t> relayData{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRelayCommand;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRelayCommand& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRelayCommand& command,
    SourceStream& stream);
};

struct AcCmdCRRelayCommandNotify
{
  //! Character OID of the sender
  uint16_t senderOid{};
  //! Raw relay data
  std::vector<uint8_t> relayData{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRelayCommandNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRelayCommandNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRelayCommandNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRelay
{
  //! The sender object ID.
  uint32_t senderOid;
  //! The relay data.
  std::vector<uint8_t> relayData;

  static Command GetCommand()
  {
    return Command::AcCmdCRRelay;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRelay& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRelay& command,
    SourceStream& stream);
};

struct AcCmdCRRelayNotify
{
  //! The sender object ID.
  uint32_t senderOid;
  //! The relay data.
  std::vector<uint8_t> relayData;

  static Command GetCommand()
  {
    return Command::AcCmdCRRelayNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRelayNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRelayNotify& command,
    SourceStream& stream);
};

struct AcCmdRCTeamSpurGauge
{
  uint32_t member1{};
  float member2{};
  float member3{};
  float member4{};
  uint16_t member5{};
  uint32_t member6{};

  static Command GetCommand()
  {
    return Command::AcCmdRCTeamSpurGauge;
  }
  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCTeamSpurGauge& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCTeamSpurGauge& command,
    SourceStream& stream);
};

struct AcCmdUserRaceActivateInteractiveEvent
{
  uint32_t member1{};
  uint16_t characterOid{};
  uint64_t member3{};

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceActivateInteractiveEvent;
  }
  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceActivateInteractiveEvent& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceActivateInteractiveEvent& command,
    SourceStream& stream);
};

struct AcCmdUserRaceActivateEvent
{
  uint32_t eventId{};
  uint16_t characterOid{};

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceActivateEvent;
  }
  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceActivateEvent& command,
    SinkStream& stream);
    
  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceActivateEvent& command,
    SourceStream& stream);
};

struct AcCmdCRUseMagicItem
{
  // vFunc_2
  uint16_t characterOid;
  //! Read and switch/case depends on it
  uint32_t magicItemId;

  // sub_45ed60
  struct Optional1
  {
    std::array<float, 3> member1;
    std::array<float, 3> member2;
  };
  std::optional<Optional1> optional1;

  // sub_4d5460
  struct Optional2
  {
    uint8_t size;
    std::vector<uint16_t> list;
  };
  std::optional<Optional2> optional2;

  // vFunc_4 @ 0x00698540
  uint32_t unk3;
  std::optional<float> optional3; // cast time?
  std::optional<float> optional4; // total cast time?

  static Command GetCommand()
  {
    return Command::AcCmdCRUseMagicItem;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUseMagicItem& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUseMagicItem& command,
    SourceStream& stream);
};

struct AcCmdCRUseMagicItemCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRUseMagicItemCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUseMagicItemCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUseMagicItemCancel& command,
    SourceStream& stream);
};

struct AcCmdCRUseMagicItemOK
{
  uint16_t characterOid;
  uint32_t magicItemId;

  // sub_45ed60
  std::optional<AcCmdCRUseMagicItem::Optional1> optional1;
  // sub_4d5460
  std::optional<AcCmdCRUseMagicItem::Optional2> optional2;

  uint16_t unk3;
  // TODO: is this correct type?
  float unk4;
  
  static Command GetCommand()
  {
    return Command::AcCmdCRUseMagicItemOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUseMagicItemOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUseMagicItemOK& command,
    SourceStream& stream);
};

struct AcCmdCRUseMagicItemNotify
{
  // Same struct as AcCmdCRUseMagicItem
  uint16_t characterOid;
  uint32_t magicItemId;

  // sub_45ed60
  std::optional<AcCmdCRUseMagicItem::Optional1> optional1;
  // sub_4d5460
  std::optional<AcCmdCRUseMagicItem::Optional2> optional2;

  uint32_t unk3;
  std::optional<float> optional3; // cast time?
  std::optional<float> optional4; // total cast time?
  
  static Command GetCommand()
  {
    return Command::AcCmdCRUseMagicItemNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUseMagicItemNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUseMagicItemNotify& command,
    SourceStream& stream);
};

struct AcCmdGameRaceItemSpawn
{
  uint32_t itemId{};
  uint32_t itemType{};
  std::array<float, 3> position;
  std::array<float, 4> orientation;
  bool member5{};
  float removeDelay{};

  static Command GetCommand()
  {
    return Command::AcCmdGameRaceItemSpawn;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdGameRaceItemSpawn& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdGameRaceItemSpawn& command,
    SourceStream& stream);
};

struct AcCmdUserRaceItemGet
{
  uint16_t characterOid;
  uint16_t itemId;
  uint32_t unk3;

  static Command GetCommand()
  {
    return Command::AcCmdUserRaceItemGet;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdUserRaceItemGet& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdUserRaceItemGet& command,
    SourceStream& stream);
};

struct AcCmdGameRaceItemGet
{
  uint16_t characterOid;
  uint32_t itemId;
  uint32_t itemType;

  static Command GetCommand()
  {
    return Command::AcCmdGameRaceItemGet;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdGameRaceItemGet& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdGameRaceItemGet& command,
    SourceStream& stream);
};

// Magic Targeting Commands for Bolt System
struct AcCmdCRStartMagicTarget
{
  uint16_t characterOid;

  static Command GetCommand()
  {
    return Command::AcCmdCRStartMagicTarget;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRStartMagicTarget& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRStartMagicTarget& command,
    SourceStream& stream);
};

struct AcCmdCRChangeMagicTargetNotify
{
  uint16_t characterOid;
  uint16_t targetOid;

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeMagicTargetNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeMagicTargetNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeMagicTargetNotify& command,
    SourceStream& stream);
};

struct AcCmdCRChangeMagicTargetOK
{
  uint16_t characterOid;
  uint16_t targetOid;

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeMagicTargetOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeMagicTargetOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeMagicTargetOK& command,
    SourceStream& stream);
};

struct AcCmdCRChangeMagicTargetCancel
{
  uint16_t characterOid;

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeMagicTargetCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeMagicTargetCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeMagicTargetCancel& command,
    SourceStream& stream);
};

struct AcCmdRCRemoveMagicTarget
{
  uint16_t characterOid;

  static Command GetCommand()
  {
    return Command::AcCmdRCRemoveMagicTarget;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCRemoveMagicTarget& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCRemoveMagicTarget& command,
    SourceStream& stream);
};

struct AcCmdRCMagicExpire
{
  uint16_t characterOid;
  uint32_t magicItemId;

  static Command GetCommand()
  {
    return Command::AcCmdRCMagicExpire;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCMagicExpire& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCMagicExpire& command,
    SourceStream& stream);
};

struct AcCmdRCTriggerActivate
{
  uint16_t characterOid;
  uint32_t triggerType;     // Type of trigger/animation to activate
  uint32_t triggerValue;    // Additional trigger parameter
  float duration;           // Duration of the effect

  static Command GetCommand()
  {
    return Command::AcCmdRCTriggerActivate;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCTriggerActivate& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCTriggerActivate& command,
    SourceStream& stream);
};

struct AcCmdCRActivateSkillEffect
{
  uint16_t characterOid;
  uint32_t skillId;         // What skill/effect to activate
  uint32_t unk1;            // Unknown parameter
  uint32_t unk2;            // Unknown parameter

  static Command GetCommand()
  {
    return Command::AcCmdCRActivateSkillEffect;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRActivateSkillEffect& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRActivateSkillEffect& command,
    SourceStream& stream);
};

struct AcCmdRCAddSkillEffect
{
  uint16_t characterOid;    // Target character
  uint32_t effectId;        // Effect/animation ID (knockdown, stun, etc.)
  uint32_t duration;        // Effect duration in milliseconds
  uint32_t intensity;       // Effect strength/intensity

  static Command GetCommand()
  {
    return Command::AcCmdRCAddSkillEffect;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCAddSkillEffect& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCAddSkillEffect& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // RACE_MESSAGE_DEFINES_HPP
