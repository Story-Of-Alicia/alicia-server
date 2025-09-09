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

#ifndef LOBBY_MESSAGE_DEFINES_HPP
#define LOBBY_MESSAGE_DEFINES_HPP

#include "CommonStructureDefinitions.hpp"
#include "libserver/network/command/CommandProtocol.hpp"
#include "libserver/util/Util.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace server::protocol
{

struct LobbyCommandLogin
{
  uint16_t constant0{0x00};
  uint16_t constant1{0x00};
  std::string loginId{};
  uint32_t memberNo{0x00};
  std::string authKey{};
  uint8_t val0{};

  static Command GetCommand()
  {
    return Command::AcCmdCLLogin;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandLogin& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandLogin& command,
    SourceStream& stream);
};

struct LobbyCommandLoginOK
{
  // filetime
  util::WinFileTime lobbyTime{};
  uint32_t member0{};

  uint32_t uid{};
  //! Max length 16
  std::string name{};
  //! Max length 255
  std::string motd{};
  Gender gender{Gender::Unspecified};
  //! Max length 255
  std::string introduction{};

  //! Max 16 elements.
  std::vector<Item> characterEquipment{};
  //! Max 16 elements.
  std::vector<Item> mountEquipment{};

  uint16_t level{};
  int32_t carrots{};

  uint32_t val1{};

  enum class Role : uint32_t
  {
    User = 0,
    PowerUser = 1,
    GameMaster = 2
  };
  Role role{};

  uint8_t val3{};

  //! Option type mask.
  OptionType optionType{};
  KeyboardOptions keyboardOptions{};
  MacroOptions macroOptions{};
  uint32_t valueOptions{};
  // GamepadOptions gamepadOptions{};

  uint8_t age{};
  uint8_t hideGenderAndAge{};

  struct Unk1
  {
    uint16_t val0{};

    struct Unk1Unk1
    {
      uint32_t val1{};
      uint32_t val2{};
    };
    std::vector<Unk1Unk1> val1{};
  };
  //! If this is not set, crash happens in preview image generation.
  std::vector<Unk1> val5; // max 17

  // 256 characters max
  std::string val6{};

  uint32_t ranchAddress{};
  uint16_t ranchport{};
  uint32_t scramblingConstant{};

  Character character{};
  Horse horse{};

  struct SystemContent
  {
    std::unordered_map<uint32_t, uint32_t> values;

    static void Write(
      const SystemContent& command,
      SinkStream& stream);

    static void Read(
      SystemContent& command,
      SourceStream& stream);
  } systemContent{};

  enum AvatarBitset : uint32_t
  {
    HasPlayerBefore = 2,
  };
  // std::bitset
  //! Bit 2: Has played before
  AvatarBitset bitfield{};

  struct Struct1
  {
    uint16_t val0{};
    uint16_t val1{};
    uint16_t val2{};
  } val9{};

  uint32_t val10{};

  struct ManagementSkills
  {
    uint8_t val0{};
    //Can be found in table CareSkillLevel, max 2675
    uint32_t progress{};
    uint16_t points{};
  } managementSkills{};

  struct SkillRanks
  {
    struct Skill
    {
      //Can be found in table CareSkillInfo
      uint8_t id{};
      uint8_t rank{};
    };

    std::vector<Skill> values;
  } skillRanks{};

  struct Struct4
  {
    struct Unk
    {
      uint16_t val0{};
      uint8_t val1{};
      uint8_t val2{};
    };

    std::vector<Unk> values;
  } val13{};

  uint32_t val14{};
  Guild guild{};
  uint8_t val16{};

  // Something with rental horse
  Rent val17{};

  uint32_t val18{};
  uint32_t val19{};
  uint32_t val20{};

  Pet pet{};

  static Command GetCommand()
  {
    return Command::AcCmdCLLoginOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandLoginOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandLoginOK& command,
    SourceStream& stream);
};

//! Clientbound login CANCEL command.
struct LobbyCommandLoginCancel
{
  //! Cancel reason for login.
  enum class Reason : uint8_t
  {
    Generic = 0,
    InvalidUser = 1,
    Duplicated = 2,
    InvalidVersion = 3,
    InvalidEquipment = 4,
    InvalidLoginId = 5,
    DisconnectYourself = 6,
  };

  Reason reason{0x00};

  static Command GetCommand()
  {
    return Command::AcCmdCLLoginCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandLoginCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandLoginCancel& command,
    SourceStream& stream);
};

//! Serverbound show inventory command.
struct LobbyCommandShowInventory
{
  static Command GetCommand()
  {
    return Command::AcCmdCLShowInventory;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandShowInventory& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandShowInventory& command,
    SourceStream& stream);
};

//! Clientbound show inventory response.
struct LobbyCommandShowInventoryOK
{
  std::vector<Item> items{};
  std::vector<Horse> horses{};

  static Command GetCommand()
  {
    return Command::AcCmdCLShowInventoryOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandShowInventoryOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandShowInventoryOK& command,
    SourceStream& stream);
};

//! Clientbound show inventory cancel response.
struct LobbyCommandShowInventoryCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLShowInventoryCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandShowInventoryCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandShowInventoryCancel& command,
    SourceStream& stream);
};

//! Clientbound create nickname command.
struct LobbyCommandCreateNicknameNotify
{
  static Command GetCommand()
  {
    return Command::AcCmdCLCreateNicknameNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandCreateNicknameNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandCreateNicknameNotify& command,
    SourceStream& stream);
};

//! Serverbound create nickname ok command.
struct LobbyCommandCreateNickname
{
  std::string nickname{};
  Character character{};
  uint32_t unk0{};

  static Command GetCommand()
  {
    return Command::AcCmdCLCreateNickname;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandCreateNickname& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandCreateNickname& command,
    SourceStream& stream);
};

//! Serverbound create nickname cancel command.
struct LobbyCommandCreateNicknameCancel
{
  uint8_t error{};

  static Command GetCommand()
  {
    return Command::AcCmdCLCreateNicknameCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandCreateNicknameCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandCreateNicknameCancel& command,
    SourceStream& stream);
};

//! Serverbound request league info command.
struct LobbyCommandRequestLeagueInfo
{
  static Command GetCommand()
  {
    return Command::AcCmdCLRequestLeagueInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestLeagueInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestLeagueInfo& command,
    SourceStream& stream);
};

//! Clientbound request league info response.
struct LobbyCommandRequestLeagueInfoOK
{
  //! Table LeagueSeasonInfo
  uint8_t season{};
  uint8_t league{};
  uint32_t unk2{};
  uint32_t unk3{};
  uint8_t rankingPercentile{};
  uint8_t unk5{};
  uint32_t unk6{};
  uint32_t unk7{};
  uint8_t unk8{};
  //! Table LeagueItemInfo Row GradeType
  uint8_t leagueReward{};
  uint32_t place{};
  uint8_t rank{};
  //! 1 - ready to claim, 2 - claimed
  uint8_t claimedReward{};
  uint8_t unk13{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestLeagueInfoOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestLeagueInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestLeagueInfoOK& command,
    SourceStream& stream);
};

//! Serverbound request league info command.
struct LobbyCommandRequestLeagueInfoCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLRequestLeagueInfoCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestLeagueInfoCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestLeagueInfoCancel& command,
    SourceStream& stream);
};

//! Serverbound achievement complete list command.
struct LobbyCommandAchievementCompleteList
{
  uint32_t unk0{};

  static Command GetCommand()
  {
    return Command::AcCmdCLAchievementCompleteList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandAchievementCompleteList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandAchievementCompleteList& command,
    SourceStream& stream);
};

//! Clientbound achievement complete list response.
struct LobbyCommandAchievementCompleteListOK
{
  uint32_t unk0{};
  std::vector<Quest> achievements;

  static Command GetCommand()
  {
    return Command::AcCmdCLAchievementCompleteListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandAchievementCompleteListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandAchievementCompleteListOK& command,
    SourceStream& stream);
};

//! Serverbound enter channel command.
struct LobbyCommandEnterChannel
{
  uint8_t channel;

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterChannel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterChannel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterChannel& command,
    SourceStream& stream);
};

//! Clientbound enter channel response.
struct LobbyCommandEnterChannelOK
{
  uint8_t unk0{};
  uint16_t unk1{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterChannelOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterChannelOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterChannelOK& command,
    SourceStream& stream);
};

//! Serverbound enter channel command.
struct LobbyCommandEnterChannelCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterChannelCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterChannelCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterChannelCancel& command,
    SourceStream& stream);
};


//! Serverbound room list command.
struct LobbyCommandRoomList
{
  uint8_t page;
  GameMode gameMode;
  TeamMode teamMode;

  static Command GetCommand()
  {
    return Command::AcCmdCLRoomList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRoomList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRoomList& command,
    SourceStream& stream);
};

//! Clientbound room list response.
struct LobbyCommandRoomListOK
{
  uint8_t page{};
  uint8_t unk1{};
  uint8_t unk2{};

  struct Room
  {
    uint32_t id{};
    std::string name{};
    uint8_t playerCount{};
    uint8_t maxPlayers{};
    uint8_t isLocked{};
    uint8_t unk0{};
    uint8_t unk1{};
    uint16_t map{};
    uint8_t hasStarted{};
    uint16_t unk2{};
    uint8_t unk3{};
    uint8_t level{}; // 0: 3lv, 1: 12lv, 2 and beyond: nothing
    uint32_t unk4{};

    static void Write(const Room& value, SinkStream& stream);
    static void Read(Room& value, SourceStream& stream);
  };

  std::vector<Room> rooms;

  struct
  {
    uint32_t unk0{};
    std::string unk1{};
    uint16_t unk2{};
  } unk3{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRoomListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRoomListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRoomListOK& command,
    SourceStream& stream);
};

//! Serverbound make room command.
struct LobbyCommandMakeRoom
{
  // presumably
  std::string name;
  std::string password;
  uint8_t playerCount;
  GameMode gameMode;
  TeamMode teamMode;
  uint16_t missionId;
  uint8_t unk3;

  enum class ModifiedSet : uint16_t
  {
    ChangeName = 1,
    ChangePlayerCount = 2,
    ChangeMode = 8
  };
  ModifiedSet bitset;

  uint8_t unk4;

  static Command GetCommand()
  {
    return Command::AcCmdCLMakeRoom;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandMakeRoom& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandMakeRoom& command,
    SourceStream& stream);
};

//! Clientbound make room response.
struct LobbyCommandMakeRoomOK
{
  uint32_t roomUid{};
  uint32_t otp{};
  uint32_t address{};
  uint16_t port{};
  uint8_t unk2{};

  static Command GetCommand()
  {
    return Command::AcCmdCLMakeRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandMakeRoomOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandMakeRoomOK& command,
    SourceStream& stream);
};

//! Serverbound make room command.
struct LobbyCommandMakeRoomCancel
{
  uint8_t unk0{};

  static Command GetCommand()
  {
    return Command::AcCmdCLMakeRoomCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandMakeRoomCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandMakeRoomCancel& command,
    SourceStream& stream);
};

struct LobbyCommandEnterRoom
{
  uint32_t roomUid{};
  std::string password{};
  uint32_t member3{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoom;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterRoom& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterRoom& command,
    SourceStream& stream);
};

struct LobbyCommandEnterRoomOK
{
  uint32_t roomUid{};
  uint32_t otp{};
  uint32_t address{};
  uint16_t port{};
  uint8_t member6{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterRoomOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterRoomOK& command,
    SourceStream& stream);
};

struct LobbyCommandEnterRoomCancel
{
  uint8_t status{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterRoomCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterRoomCancel& command,
    SourceStream& stream);
};

//! Serverbound request quest list command.
struct LobbyCommandRequestQuestList
{
  uint32_t unk0{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestQuestList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestQuestList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestQuestList& command,
    SourceStream& stream);
};

//! Clientbound request quest list response.
struct LobbyCommandRequestQuestListOK
{
  uint32_t unk0{};
  std::vector<Quest> quests;

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestQuestListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestQuestListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestQuestListOK& command,
    SourceStream& stream);
};

struct LobbyCommandRequestDailyQuestList
{
  uint32_t val0{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestDailyQuestList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestDailyQuestList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestDailyQuestList& command,
    SourceStream& stream);
};

struct LobbyCommandRequestDailyQuestListOK
{
  uint32_t val0{};
  //! Size specified with uint16
  std::vector<Quest> quests;

  struct Unk
  {
    uint16_t val0{};
    uint32_t val1{};
    uint8_t val2{};
    uint8_t val3{};
  };
  //! Size specified with uint16
  std::vector<Unk> val1;

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestDailyQuestListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestDailyQuestListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestDailyQuestListOK& command,
    SourceStream& stream);
};

//! Serverbound enter ranch command.
struct LobbyCommandEnterRanch
{
  uint32_t rancherUid;
  std::string unk1;
  uint8_t unk2;

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRanch;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterRanch& command,
    SourceStream& stream);
};

//! Clientbound enter ranch response.
struct LobbyCommandEnterRanchOK
{
  uint32_t rancherUid{};
  uint32_t otp{};
  uint32_t ranchAddress{};
  uint16_t ranchPort{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRanchOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterRanchOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterRanchOK& command,
    SourceStream& stream);
};

//! Serverbound enter ranch command.
struct LobbyCommandEnterRanchCancel
{
  uint16_t unk0;

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRanchCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterRanchCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterRanchCancel& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct LobbyCommandGetMessengerInfo
{
  static Command GetCommand()
  {
    return Command::AcCmdCLGetMessengerInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGetMessengerInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGetMessengerInfo& command,
    SourceStream& stream);
};

//! Clientbound get messenger info response.
struct LobbyCommandGetMessengerInfoOK
{
  uint32_t code;
  uint32_t ip;
  uint16_t port;

  static Command GetCommand()
  {
    return Command::AcCmdCLGetMessengerInfoOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGetMessengerInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGetMessengerInfoOK& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct LobbyCommandGetMessengerInfoCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLGetMessengerInfoCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGetMessengerInfoCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGetMessengerInfoCancel& command,
    SourceStream& stream);
};

//! Serverbound request special event list command.
struct LobbyCommandRequestSpecialEventList
{
  uint32_t unk0;

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestSpecialEventList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestSpecialEventList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestSpecialEventList& command,
    SourceStream& stream);
};

struct Event
{
  uint16_t unk0{};
  uint32_t unk1{};
};

//! Clientbound request special event list response.
struct LobbyCommandRequestSpecialEventListOK
{
  uint32_t unk0;
  std::vector<Quest> quests;
  std::vector<Event> events;

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestSpecialEventListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestSpecialEventListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestSpecialEventListOK& command,
    SourceStream& stream);
};

//! Serverbound heartbeat command.
struct LobbyCommandHeartbeat
{
  static Command GetCommand()
  {
    return Command::AcCmdCLHeartbeat;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandHeartbeat& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandHeartbeat& command,
    SourceStream& stream);
};

//! Serverboud goods message
struct LobbyCommandGoodsShopList
{
  std::array<uint8_t, 12> data;

  static Command GetCommand()
  {
    return Command::AcCmdCLGoodsShopList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGoodsShopList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGoodsShopList& command,
    SourceStream& stream);
};

//! Clientbound shop goods message
struct LobbyCommandGoodsShopListOK
{
  std::array<uint8_t, 12> data;

  static Command GetCommand()
  {
    return Command::AcCmdCLGoodsShopListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGoodsShopListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGoodsShopListOK& command,
    SourceStream& stream);
};

//! Clientbound shop goods message
struct LobbyCommandGoodsShopListCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLGoodsShopListCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGoodsShopListCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGoodsShopListCancel& command,
    SourceStream& stream);
};

struct LobbyCommandInquiryTreecash
{
  static Command GetCommand()
  {
    return Command::AcCmdCLInquiryTreecash;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandInquiryTreecash& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandInquiryTreecash& command,
    SourceStream& stream);
};

struct LobbyCommandInquiryTreecashOK
{
  uint32_t cash{};

  static Command GetCommand()
  {
    return Command::AcCmdCLInquiryTreecashOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandInquiryTreecashOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandInquiryTreecashOK& command,
    SourceStream& stream);
};

struct LobbyCommandInquiryTreecashCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLInquiryTreecashCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandInquiryTreecashCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandInquiryTreecashCancel& command,
    SourceStream& stream);
};

struct LobbyCommandClientNotify
{
  // Scene state
  // 1 - success
  // 2 - first cancel
  // 3 - repeated cancel
  uint16_t val0{};
  // Additional payload, for the success its always zero.
  // For the cancel it is the retry count
  uint32_t val1{};

  static Command GetCommand()
  {
    return Command::AcCmdClientNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandClientNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandClientNotify& command,
    SourceStream& stream);
};

struct LobbyCommandGuildPartyList
{
  static Command GetCommand()
  {
    return Command::AcCmdCLGuildPartyList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGuildPartyList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGuildPartyList& command,
    SourceStream& stream);
};

struct LobbyCommandGuildPartyListOK
{
  struct Member
  {
    uint32_t val0{};
    uint32_t val1{};
    std::string val3{};
    uint32_t val4{};
    uint32_t val5{};
    uint32_t val6{};
    uint32_t val7{};
    uint8_t val8{};
    uint32_t val9{};
  };
  std::vector<Member> members;

  static Command GetCommand()
  {
    return Command::AcCmdCLGuildPartyListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandGuildPartyListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandGuildPartyListOK& command,
    SourceStream& stream);
};

struct LobbyCommandEnterRandomRanch
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRanchRandomly;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandEnterRandomRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandEnterRandomRanch& command,
    SourceStream& stream);
};

struct LobbyCommandRequestPersonalInfo
{
  enum class Type : uint32_t
  {
    Basic = 6,
    Courses = 7,
    Eight = 8
  };

  uint32_t characterUid{};
  Type type{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestPersonalInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandRequestPersonalInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandRequestPersonalInfo& command,
    SourceStream& stream);
};

struct LobbyCommandPersonalInfo
{
  uint32_t characterUid{};
  LobbyCommandRequestPersonalInfo::Type type{};

  struct Basic
  {
    //! Store in metres, displayed in kilometres
    uint32_t distanceTravelled{};
    //! Whole number, divided by 10 for the floating point.
    uint32_t topSpeed{};
    //! Whole number, divided by 10 for the floating point.
    uint32_t longestGlidingDistance{};
    float jumpSuccessRate{};
    float perfectJumpSuccessRate{};
    uint16_t speedSingleWinCombo{};
    uint16_t speedTeamWinCombo{};
    uint16_t magicSingleWinCombo{};
    uint16_t magicTeamWinCombo{};
    float averageRank{};
    float completionRate{};
    float member12{};
    uint32_t highestCarnivalPrize{};
    uint16_t member14{};
    uint16_t member15{};
    uint16_t member16{};
    std::string introduction{};
    uint32_t level{};
    //! Level progress as dictated by LevelInfo table in libconfig
    uint32_t levelProgress{};
    std::string member20{};
    uint16_t perfectBoostCombo{};
    uint16_t perfectJumpCombo{};
    uint16_t magicDefenseCombo{};
    float member24{};
    float member25{};
    float member26{};
    std::string guildName{};
    uint8_t member28{};
    uint8_t member29{};

    static void Write(const Basic& command, SinkStream& stream);
    static void Read(Basic& command, SourceStream& stream);
  } basic{};

  struct CourseInformation
  {
    uint32_t totalGames{};
    uint32_t totalSpeedGames{};
    uint32_t totalMagicGames{};

    struct Course
    {
      uint16_t courseId{};
      //! Measured in milliseconds
      uint32_t recordTime{};
      //! Unclear if times raced or times won, needs confirming/fact checking
      uint32_t timesRaced{};
      std::array<std::byte, 12> member4{};
    };
    // max 255
    std::vector<Course> courses{};

    static void Write(const CourseInformation& command, SinkStream& stream);
    static void Read(CourseInformation& command, SourceStream& stream);
  } courseInformation{};

  struct Eight
  {
    struct Unk
    {
      uint32_t member1{};
      uint32_t member2{};
    };
    // max 255
    std::vector<Unk> member1{};

    static void Write(const Eight& command, SinkStream& stream);
    static void Read(Eight& command, SourceStream& stream);
  } eight{};

  static Command GetCommand()
  {
    return Command::AcCmdLCPersonalInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandPersonalInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandPersonalInfo& command,
    SourceStream& stream);
};

struct LobbyCommandSetIntroduction
{
  std::string introduction{};

  static Command GetCommand()
  {
    return Command::AcCmdCLSetIntroduction;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandSetIntroduction& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandSetIntroduction& command,
    SourceStream& stream);
};

struct LobbyCommandUpdateSystemContent
{
  uint8_t member1{};
  uint32_t key{};
  uint32_t value{};

  static Command GetCommand()
  {
    return Command::AcCmdCLUpdateSystemContent;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandUpdateSystemContent& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandUpdateSystemContent& command,
    SourceStream& stream);
};

struct LobbyCommandUpdateSystemContentNotify
{
  LobbyCommandLoginOK::SystemContent systemContent;

  static Command GetCommand()
  {
    return Command::AcCmdLCUpdateSystemContent;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandUpdateSystemContentNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandUpdateSystemContentNotify& command,
    SourceStream& stream);
};

struct LobbyCommandChangeRanchOption
{
  uint32_t unk0{};
  uint16_t unk1{};
  uint8_t unk2{};

  static Command GetCommand()
  {
    return Command::AcCmdCLChangeRanchOption;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandChangeRanchOption& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandChangeRanchOption& command,
    SourceStream& stream);
};

struct LobbyCommandChangeRanchOptionOK
{
  uint32_t unk0{};
  uint16_t unk1{};
  uint8_t unk2{};

  static Command GetCommand()
  {
    return Command::AcCmdCLChangeRanchOptionOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const LobbyCommandChangeRanchOptionOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    LobbyCommandChangeRanchOptionOK& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // LOBBY_MESSAGE_DEFINES_HPP
