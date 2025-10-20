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

struct AcCmdCLLogin
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
    const AcCmdCLLogin& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLLogin& command,
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
  std::string notice{};
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

  Settings settings{};

  struct Mission
  {
    uint16_t id{};

    struct Progress
    {
      uint32_t id{};
      uint32_t value{};
    };
    std::vector<Progress> progress{};
  };

  //! Max 17
  std::vector<Mission> missions{};

  // 256 characters max
  std::string val6{};

  uint32_t ranchAddress{};
  uint16_t ranchPort{};
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
    HasPlayedBefore = 2,
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
struct AcCmdCLLoginCancel
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
    const AcCmdCLLoginCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLLoginCancel& command,
    SourceStream& stream);
};

//! Serverbound show inventory command.
struct AcCmdCLShowInventory
{
  static Command GetCommand()
  {
    return Command::AcCmdCLShowInventory;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLShowInventory& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLShowInventory& command,
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

struct AcCmdCLCreateNickname
{
  std::string nickname{};
  Character character{};
  uint32_t requestedHorseTid{};

  static Command GetCommand()
  {
    return Command::AcCmdCLCreateNickname;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLCreateNickname& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLCreateNickname& command,
    SourceStream& stream);
};

struct AcCmdCLCreateNicknameCancel
{
  enum class Reason : uint8_t
  {
    ServerError = 0,
    InvalidRequestedNotLoggedIn = 1,
    DuplicateCharacterName = 2,
    InvalidCharacterName = 3
  } error{};

  static Command GetCommand()
  {
    return Command::AcCmdCLCreateNicknameCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLCreateNicknameCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLCreateNicknameCancel& command,
    SourceStream& stream);
};

//! Serverbound request league info command.
struct AcCmdCLRequestLeagueInfo
{
  static Command GetCommand()
  {
    return Command::AcCmdCLRequestLeagueInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestLeagueInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestLeagueInfo& command,
    SourceStream& stream);
};

//! Clientbound request league info response.
struct AcCmdCLRequestLeagueInfoOK
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
    const AcCmdCLRequestLeagueInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestLeagueInfoOK& command,
    SourceStream& stream);
};

//! Serverbound request league info command.
struct AcCmdCLRequestLeagueInfoCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLRequestLeagueInfoCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestLeagueInfoCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestLeagueInfoCancel& command,
    SourceStream& stream);
};

//! Serverbound achievement complete list command.
struct AcCmdCLAchievementCompleteList
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
    const AcCmdCLAchievementCompleteList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLAchievementCompleteList& command,
    SourceStream& stream);
};

//! Clientbound achievement complete list response.
struct AcCmdCLAchievementCompleteListOK
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
    const AcCmdCLAchievementCompleteListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLAchievementCompleteListOK& command,
    SourceStream& stream);
};

//! Serverbound enter channel command.
struct AcCmdCLEnterChannel
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
    const AcCmdCLEnterChannel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterChannel& command,
    SourceStream& stream);
};

//! Clientbound enter channel response.
struct AcCmdCLEnterChannelOK
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
    const AcCmdCLEnterChannelOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterChannelOK& command,
    SourceStream& stream);
};

//! Serverbound enter channel command.
struct AcCmdCLEnterChannelCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterChannelCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterChannelCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterChannelCancel& command,
    SourceStream& stream);
};

struct AcCmdCLLeaveChannel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLLeaveChannel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLLeaveChannel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLLeaveChannel& command,
    SourceStream& stream);
};

struct AcCmdCLLeaveChannelOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCLLeaveChannelOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLLeaveChannelOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLLeaveChannelOK& command,
    SourceStream& stream);
};

//! Serverbound room list command.
struct AcCmdCLRoomList
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
    const AcCmdCLRoomList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRoomList& command,
    SourceStream& stream);
};

//! Clientbound room list response.
struct LobbyCommandRoomListOK
{
  uint8_t page{};
  GameMode gameMode{};
  TeamMode teamMode{};

  struct Room
  {
    uint32_t uid{};
    std::string name{};
    uint8_t playerCount{};
    uint8_t maxPlayerCount{};
    uint8_t isLocked{};
    uint8_t unk0{};
    uint8_t unk1{};
    uint16_t map{};
    //! State of the room
    //! Internally a bool
    enum State : uint8_t
    {
      Waiting = 0,
      Playing = 1
    } state{State::Waiting};
    uint16_t unk2{};
    uint8_t unk3{};

    enum class SkillBracket : uint8_t
    {
      Newbies = 0,
      Level12 = 1,
      Experienced = 2,
    } skillBracket{SkillBracket::Experienced};
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
struct AcCmdCLMakeRoom
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
    const AcCmdCLMakeRoom& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLMakeRoom& command,
    SourceStream& stream);
};

//! Clientbound make room response.
struct AcCmdCLMakeRoomOK
{
  uint32_t roomUid{};
  uint32_t oneTimePassword{};
  uint32_t raceServerAddress{};
  uint16_t raceServerPort{};
  uint8_t unk2{};

  static Command GetCommand()
  {
    return Command::AcCmdCLMakeRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLMakeRoomOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLMakeRoomOK& command,
    SourceStream& stream);
};

//! Serverbound make room command.
struct AcCmdCLMakeRoomCancel
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
    const AcCmdCLMakeRoomCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLMakeRoomCancel& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoom
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
    const AcCmdCLEnterRoom& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoom& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoomOK
{
  uint32_t roomUid{};
  uint32_t oneTimePassword{};
  uint32_t raceServerAddress{};
  uint16_t raceServerPort{};
  uint8_t member6{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRoomOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoomOK& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoomCancel
{
  // please verify these values
  enum class Status : uint8_t
  {
    NotLogin = 1,
    CR_NOT_IN_CHANNEL = 2,
    CR_BUSY_PREVIOUS = 3,
    CR_ALREADY_ROOM = 4,
    CR_INVALID_ROOM = 5,
    CR_CROWDED_ROOM = 6,
    CR_VERSION_ERROR = 7,
    CR_LOST_ROOM = 8,
    CR_LOST_SERVER = 9,
    CR_AUTH_ERROR = 10,
    CR_BAD_PASSWORD = 11,
    CR_PLAYING_ROOM = 12,
    CR_PRACTICE_ROOM = 13,
    CR_PRACTICE_ROOM2 = 14,
    CR_PRACTICE_ROOM_SPEEDTEAM = 15,
    CR_PRACTICE_ROOM_MAGICTEAM = 16,
  } status{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRoomCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoomCancel& command,
    SourceStream& stream);
};

struct AcCmdCLLeaveRoom
{
  static Command GetCommand()
  {
    return Command::AcCmdCLLeaveRoom;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLLeaveRoom& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLLeaveRoom& command,
    SourceStream& stream);
};

struct AcCmdCLLeaveRoomOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCLLeaveRoomOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLLeaveRoomOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLLeaveRoomOK& command,
    SourceStream& stream);
};

//! Serverbound request quest list command.
struct AcCmdCLRequestQuestList
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
    const AcCmdCLRequestQuestList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestQuestList& command,
    SourceStream& stream);
};

//! Clientbound request quest list response.
struct AcCmdCLRequestQuestListOK
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
    const AcCmdCLRequestQuestListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestQuestListOK& command,
    SourceStream& stream);
};

struct AcCmdCLRequestDailyQuestList
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
    const AcCmdCLRequestDailyQuestList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestDailyQuestList& command,
    SourceStream& stream);
};

struct AcCmdCLRequestDailyQuestListOK
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
    const AcCmdCLRequestDailyQuestListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestDailyQuestListOK& command,
    SourceStream& stream);
};

//! Serverbound enter ranch command.
struct AcCmdCLEnterRanch
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
    const AcCmdCLEnterRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRanch& command,
    SourceStream& stream);
};

//! Clientbound enter ranch response.
struct AcCmdCLEnterRanchOK
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
    const AcCmdCLEnterRanchOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRanchOK& command,
    SourceStream& stream);
};

//! Serverbound enter ranch command.
struct AcCmdCLEnterRanchCancel
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
    const AcCmdCLEnterRanchCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRanchCancel& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct AcCmdCLGetMessengerInfo
{
  static Command GetCommand()
  {
    return Command::AcCmdCLGetMessengerInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLGetMessengerInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLGetMessengerInfo& command,
    SourceStream& stream);
};

//! Clientbound get messenger info response.
struct AcCmdCLGetMessengerInfoOK
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
    const AcCmdCLGetMessengerInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLGetMessengerInfoOK& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct AcCmdCLGetMessengerInfoCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLGetMessengerInfoCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLGetMessengerInfoCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLGetMessengerInfoCancel& command,
    SourceStream& stream);
};

struct AcCmdCLCheckWaitingSeqno
{
  uint32_t uid{};

  static Command GetCommand()
  {
    return Command::AcCmdCLCheckWaitingSeqno;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLCheckWaitingSeqno& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLCheckWaitingSeqno& command,
    SourceStream& stream);
};

struct AcCmdCLCheckWaitingSeqnoOK
{
  uint32_t time{};
  uint32_t position{};

  static Command GetCommand()
  {
    return Command::AcCmdCLCheckWaitingSeqnoOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLCheckWaitingSeqnoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLCheckWaitingSeqnoOK& command,
    SourceStream& stream);
};

//! Serverbound request special event list command.
struct AcCmdCLRequestSpecialEventList
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
    const AcCmdCLRequestSpecialEventList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestSpecialEventList& command,
    SourceStream& stream);
};

struct Event
{
  uint16_t unk0{};
  uint32_t unk1{};
};

//! Clientbound request special event list response.
struct AcCmdCLRequestSpecialEventListOK
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
    const AcCmdCLRequestSpecialEventListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestSpecialEventListOK& command,
    SourceStream& stream);
};

//! Serverbound heartbeat command.
struct AcCmdCLHeartbeat
{
  static Command GetCommand()
  {
    return Command::AcCmdCLHeartbeat;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLHeartbeat& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLHeartbeat& command,
    SourceStream& stream);
};

//! Serverboud goods message
struct AcCmdCLGoodsShopList
{
  //! Timestamp of the shop cached by the client
  util::Clock::time_point cachedShopTimestamp{};

  static Command GetCommand()
  {
    return Command::AcCmdCLGoodsShopList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLGoodsShopList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLGoodsShopList& command,
    SourceStream& stream);
};

//! Clientbound shop goods message
struct AcCmdCLGoodsShopListOK
{
  //! New shop timestamp
  util::Clock::time_point shopTimestamp{};

  static Command GetCommand()
  {
    return Command::AcCmdCLGoodsShopListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLGoodsShopListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLGoodsShopListOK& command,
    SourceStream& stream);
};

//! Clientbound shop goods message
struct AcCmdCLGoodsShopListCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLGoodsShopListCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLGoodsShopListCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLGoodsShopListCancel& command,
    SourceStream& stream);
};

struct AcCmdLCGoodsShopListData
{
  util::Clock::time_point timestamp;
  uint8_t member2;
  uint8_t member3;
  std::vector<std::byte> data;

  static Command GetCommand()
  {
    return Command::AcCmdLCGoodsShopListData;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCGoodsShopListData& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCGoodsShopListData& command,
    SourceStream& stream);
};

struct AcCmdCLInquiryTreecash
{
  static Command GetCommand()
  {
    return Command::AcCmdCLInquiryTreecash;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLInquiryTreecash& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLInquiryTreecash& command,
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

struct AcCmdClientNotify
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
    const AcCmdClientNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdClientNotify& command,
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

struct AcCmdCLEnterRanchRandomly
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRanchRandomly;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRanchRandomly& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRanchRandomly& command,
    SourceStream& stream);
};

struct AcCmdCLFeatureCommand
{
  std::string command;

  static Command GetCommand()
  {
    return Command::AcCmdCLFeatureCommand;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLFeatureCommand& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLFeatureCommand& command,
    SourceStream& stream);
};

struct AcCmdCLRequestFestivalResult
{
  uint32_t member1{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestFestivalResult;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestFestivalResult& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestFestivalResult& command,
    SourceStream& stream);
};

struct AcCmdCLRequestFestivalResultOK
{
  // todo: members

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestFestivalResultOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestFestivalResultOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestFestivalResultOK& command,
    SourceStream& stream);
};

struct AcCmdCLRequestPersonalInfo
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
    const AcCmdCLRequestPersonalInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestPersonalInfo& command,
    SourceStream& stream);
};

struct AcCmdLCPersonalInfo
{
  uint32_t characterUid{};
  AcCmdCLRequestPersonalInfo::Type type{};

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
    uint32_t level{60};
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
    const AcCmdLCPersonalInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCPersonalInfo& command,
    SourceStream& stream);
};

struct AcCmdCLSetIntroduction
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
    const AcCmdCLSetIntroduction& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLSetIntroduction& command,
    SourceStream& stream);
};

struct AcCmdCLUpdateSystemContent
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
    const AcCmdCLUpdateSystemContent& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLUpdateSystemContent& command,
    SourceStream& stream);
};

struct AcCmdLCUpdateSystemContent
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
    const AcCmdLCUpdateSystemContent& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCUpdateSystemContent& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoomQuickStop
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomQuickStop;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRoomQuickStop& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoomQuickStop& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoomQuickStopOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomQuickStopOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRoomQuickStopOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoomQuickStopOK& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoomQuickStopCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomQuickStopCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRoomQuickStopCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoomQuickStopCancel& command,
    SourceStream& stream);
};

struct AcCmdCLRequestFestivalPrize
{
  uint32_t member1{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestFestivalPrize;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestFestivalPrize& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestFestivalPrize& command,
    SourceStream& stream);
};

struct AcCmdCLQueryServerTime
{
  static Command GetCommand()
  {
    return Command::AcCmdCLQueryServerTime;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLQueryServerTime& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLQueryServerTime& command,
    SourceStream& stream);
};

struct AcCmdCLQueryServerTimeOK
{
  util::WinFileTime lobbyTime{};

  static Command GetCommand()
  {
    return Command::AcCmdCLQueryServerTimeOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLQueryServerTimeOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLQueryServerTimeOK& command,
    SourceStream& stream);
};

struct AcCmdCLRequestFestivalPrizeOK
{
  // todo: figure out fields

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestFestivalPrizeOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestFestivalPrizeOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestFestivalPrizeOK& command,
    SourceStream& stream);
};

struct AcCmdCLRequestFestivalPrizeCancel
{
  // todo: figure out fields

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestFestivalPrizeCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestFestivalPrizeCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestFestivalPrizeCancel& command,
    SourceStream& stream);
};

struct AcCmdCLChangeRanchOption
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
    const AcCmdCLChangeRanchOption& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLChangeRanchOption& command,
    SourceStream& stream);
};

struct AcCmdCLChangeRanchOptionOK
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
    const AcCmdCLChangeRanchOptionOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLChangeRanchOptionOK& command,
    SourceStream& stream);
};

//! Unfortunately not implemented by the client.
struct AcCmdLCOpKick
{
  static Command GetCommand()
  {
    return Command::AcCmdLCOpKick;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCOpKick& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCOpKick& command,
    SourceStream& stream);
};

struct AcCmdLCOpMute
{
  uint32_t duration{};

  static Command GetCommand()
  {
    return Command::AcCmdLCOpMute;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCOpMute& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCOpMute& command,
    SourceStream& stream);
};

struct AcCmdLCNotice
{
  std::string notice;

  static Command GetCommand()
  {
    return Command::AcCmdLCNotice;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCNotice& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCNotice& command,
    SourceStream& stream);
};


struct AcCmdCLRequestMountInfo{
  uint32_t characterUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestMountInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestMountInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestMountInfo& command,
    SourceStream& stream);
};

struct AcCmdCLRequestMountInfoOK
{
  uint32_t characterUid{};
  struct MountInfo
  {
    uint32_t horseUid{};

    uint16_t boostsInARow{};
    uint16_t winsSpeedSingle{};
    uint16_t winsSpeedTeam{};
    uint16_t winsMagicSingle{};
    uint16_t winsMagicTeam{};

    // Store in metres, displayed in kilometres
    uint32_t totalDistance{};
    // Whole number, divided by 10 for the floating point.
    uint32_t topSpeed{};
    // Whole number, divided by 10 for the floating point.
    uint32_t longestGlideDistance{};

    // refers to carnival participation
    uint32_t participated{};
    uint32_t cumulativePrize{};
    uint32_t biggestPrize{};
  };
  // max size 10
  std::vector<MountInfo> mountInfos{};

  static Command GetCommand()
  {
    return Command::AcCmdCLRequestMountInfoOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLRequestMountInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLRequestMountInfoOK& command,
    SourceStream& stream);
};

struct AcCmdLCSkillCardPresetList
{
  uint8_t speedActiveSetId{};
  uint8_t magicActiveSetId{};
  std::vector<SkillSet> skillSets{};

  static Command GetCommand()
  {
    return Command::AcCmdLCSkillCardPresetList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCSkillCardPresetList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCSkillCardPresetList& command,
    SourceStream& stream);
};

struct AcCmdCLUpdateUserSettings
{
  Settings settings;

  static Command GetCommand()
  {
    return Command::AcCmdCLUpdateUserSettings;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLUpdateUserSettings& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLUpdateUserSettings& command,
    SourceStream& stream);
};

struct AcCmdCLUpdateUserSettingsOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCLUpdateUserSettingsOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLUpdateUserSettingsOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLUpdateUserSettingsOK& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoomQuick
{
  uint8_t member1{};
  uint8_t member2{};

  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomQuick;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRoomQuick& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoomQuick& command,
    SourceStream& stream);
};

struct AcCmdCLEnterRoomQuickCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCLEnterRoomQuickCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCLEnterRoomQuickCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCLEnterRoomQuickCancel& command,
    SourceStream& stream);
};

struct AcCmdLCInviteGuildJoin
{
  uint32_t characterUid;
  uint32_t inviterCharacterUid;
  std::string inviterCharacterName;
  std::string unk3; // guild description?

  // sub_4be7a0
  Guild guild;

  static Command GetCommand()
  {
    return Command::AcCmdLCInviteGuildJoin;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCInviteGuildJoin& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCInviteGuildJoin& command,
    SourceStream& stream);
};

struct AcCmdLCInviteGuildJoinCancel
{
  uint32_t characterUid;
  uint32_t inviterCharacterUid;
  std::string inviterCharacterName;
  std::string unk3; // guild description?

  // sub_4be7a0
  Guild guild;

  GuildError error;

  static Command GetCommand()
  {
    return Command::AcCmdLCInviteGuildJoinCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCInviteGuildJoinCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCInviteGuildJoinCancel& command,
    SourceStream& stream);
};

struct AcCmdLCInviteGuildJoinOK
{
  uint32_t characterUid;
  uint32_t inviterCharacterUid;
  std::string inviterCharacterName;
  std::string unk3; // guild description?

  // sub_4be7a0
  Guild guild;

  static Command GetCommand()
  {
    return Command::AcCmdLCInviteGuildJoinOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdLCInviteGuildJoinOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdLCInviteGuildJoinOK& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // LOBBY_MESSAGE_DEFINES_HPP
