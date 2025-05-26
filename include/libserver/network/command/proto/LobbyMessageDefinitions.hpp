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
#include "libserver/util/Util.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace alicia
{

//! Serverbound login command.
struct LobbyCommandLogin
{
  uint16_t constant0{0x00};
  uint16_t constant1{0x00};
  std::string loginId{};
  uint32_t memberNo{0x00};
  std::string authKey{};
  uint8_t val0{};

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

//! Clientbound login OK command.
struct LobbyCommandLoginOK
{
  // filetime
  WinFileTime lobbyTime{};
  uint32_t val0{};

  uint32_t selfUid{};
  std::string nickName{};
  std::string motd{};
  Gender profileGender{Gender::Unspecified};
  std::string status{};

  std::vector<Item> characterEquipment{};
  std::vector<Item> horseEquipment{};

  uint16_t level{};
  int32_t carrots{};
  uint32_t val1{};
  uint32_t val2{};
  uint8_t val3{};

  //! Option type mask.
  OptionType optionType{};
  KeyboardOptions keyboardOptions{};
  MacroOptions macroOptions{};
  uint32_t valueOptions{};
  // GamepadOptions gamepadOptions{};

  AgeGroup ageGroup{};
  uint8_t hideAge{};

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
  std::vector<Unk1> val5; // max 17

  // 256 characters max
  std::string val6{};

  uint32_t address{};
  uint16_t port{};
  uint32_t scramblingConstant{};

  Character character{};
  Horse horse{};

  struct Struct0
  {
    struct Unk2
    {
      uint32_t val0{};
      uint32_t val1{};
    };
    std::vector<Unk2> values;
  } val7{};

  // std::bitset
  //! Bit 2: Has played before
  uint32_t val8{};

  struct Struct1
  {
    uint16_t val0{};
    uint16_t val1{};
    uint16_t val2{};
  } val9{};

  uint32_t val10{};

  struct Struct2
  {
    uint8_t val0{};
    uint32_t val1{};
    uint16_t val2{};
  } val11{};

  struct Struct3
  {
    struct Unk
    {
      uint8_t val0{};
      uint8_t val1{};
    };

    std::vector<Unk> values;
  } val12{};

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
  Struct5 val15{};
  uint8_t val16{};

  // Something with rental horse
  Struct6 val17{};

  uint32_t val18{};
  uint32_t val19{};
  uint32_t val20{};

  Struct7 val21{};

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

//! Cancel reason for login.
enum class LoginCancelReason : uint8_t
{
  InvalidUser = 1,
  Duplicated = 2,
  InvalidVersion = 3,
  InvalidEquipment = 4,
  InvalidLoginId = 5,
  DisconnectYourself = 6,
};

//! Clientbound login CANCEL command.
struct LobbyCommandLoginCancel
{
  LoginCancelReason reason{0x00};

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

//! Clientbound create nickname command.
struct LobbyCommandCreateNicknameNotify
{
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

//! Clientbound show inventory cancel response.
struct LobbyCommandShowInventoryCancel
{
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

//! Serverbound request league info command.
struct LobbyCommandRequestLeagueInfo
{
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
  uint8_t unk0;
  uint8_t unk1;
  uint32_t unk2;
  uint32_t unk3;
  uint8_t unk4;
  uint8_t unk5;
  uint32_t unk6;
  uint32_t unk7;
  uint8_t unk8;
  uint8_t unk9;
  uint32_t unk10;
  uint8_t unk11;
  uint8_t unk12;
  uint8_t unk13;

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
  uint8_t unk0;
  uint16_t unk1;

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

//! Serverbound make room command.
struct LobbyCommandMakeRoom
{
  // presumably
  std::string name;
  std::string description;
  uint8_t unk0;
  uint8_t unk1;
  uint8_t unk2;
  uint16_t missionId;
  uint8_t unk3;
  uint16_t bitset;
  uint8_t unk4;

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
  uint32_t roomUid;
  uint32_t otp;
  uint32_t ip;
  uint16_t port;
  uint8_t unk2;

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
  uint8_t unk0;

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

//! Serverbound request quest list command.
struct LobbyCommandRequestQuestList
{
  uint32_t unk0{};

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
  uint32_t unk0;
  std::string unk1;
  uint8_t unk2;

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
  uint32_t ranchUid{};
  uint32_t code{}; // probably some auth token that has to be double checked in the ranch server
  uint32_t ip{};
  uint16_t port{};

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

//! Serverbound room list command.
struct LobbyCommandRoomList
{
  uint8_t unk0;
  uint8_t unk1;
  uint8_t unk2;

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

struct Room
{
  uint32_t id;
  std::string name;
  uint8_t playerCount;
  uint8_t maxPlayers;
  uint8_t isLocked;
  uint8_t unk0;
  uint8_t unk1;
  uint16_t map;
  uint8_t hasStarted;
  uint16_t unk2;
  uint8_t unk3;
  uint8_t level; // 0: 3lv, 1: 12lv, 2 and beyond: nothing
  uint32_t unk4;
};

//! Clientbound room list response.
struct LobbyCommandRoomListOK
{
  uint8_t unk0;
  uint8_t unk1;
  uint8_t unk2;
  std::vector<Room> rooms;
  struct
  {
    uint32_t unk0;
    std::string unk1;
    uint16_t unk2;
  } unk3;

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

//! Serverbound request special event list command.
struct LobbyCommandRequestSpecialEventList
{
  uint32_t unk0;

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
  std::vector<Quest> unk1;
  std::vector<Event> unk2;

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

// TODO: AcCmdCLRequestPersonalInfo, others

} // namespace alicia

#endif // LOBBY_MESSAGE_DEFINES_HPP
