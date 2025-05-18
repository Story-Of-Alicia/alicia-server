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

#include "DataDefines.hpp"
#include "libserver/Util.hpp"

#include <array>
#include <vector>
#include <cstdint>
#include <string>

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

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandLogin& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandLogin& command, SourceStream& buffer);
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
  //GamepadOptions gamepadOptions{};

  AgeGroup ageGroup{};
  uint8_t val4{};

  struct Unk1
  {
    uint16_t val0{};

    struct Unk1Unk1 {
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
  Struct6 val17{};

  uint32_t val18{};
  uint32_t val19{};
  uint32_t val20{};

  Struct7 val21{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandLoginOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandLoginOK& command, SourceStream& buffer);
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

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandLoginCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandLoginCancel& command, SourceStream& buffer);
};


//! Serverbound show inventory command.
struct LobbyCommandShowInventory
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandShowInventory& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandShowInventory& command, SourceStream& buffer);
};

//! Clientbound show inventory response.
struct LobbyCommandShowInventoryOK
{
  std::vector<Item> items{};
  std::vector<Horse> horses{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandShowInventoryOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandShowInventoryOK& command, SourceStream& buffer);
};

//! Clientbound create nickname command.
struct LobbyCommandCreateNicknameNotify
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandCreateNicknameNotify& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandCreateNicknameNotify& command, SourceStream& buffer);
};

//! Serverbound create nickname ok command.
struct LobbyCommandCreateNicknameOK
{
  std::string nickname{};
  Character character{};
  uint32_t member3 {};
  uint16_t member4 {};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandCreateNicknameOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandCreateNicknameOK& command, SourceStream& buffer);
};

//! Serverbound create nickname cancel command.
struct LobbyCommandCreateNicknameCancel
{
  uint8_t error{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandCreateNicknameCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandCreateNicknameCancel& command, SourceStream& buffer);
};

//! Clientbound show inventory cancel response.
struct LobbyCommandShowInventoryCancel
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandShowInventoryCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandShowInventoryCancel& command, SourceStream& buffer);
};


//! Serverbound request league info command.
struct LobbyCommandRequestLeagueInfo
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestLeagueInfo& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestLeagueInfo& command, SourceStream& buffer);
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

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestLeagueInfoOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestLeagueInfoOK& command, SourceStream& buffer);
};

//! Serverbound request league info command.
struct LobbyCommandRequestLeagueInfoCancel
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestLeagueInfoCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestLeagueInfoCancel& command, SourceStream& buffer);
};


//! Serverbound achievement complete list command.
struct LobbyCommandAchievementCompleteList
{
  uint32_t unk0{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandAchievementCompleteList& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandAchievementCompleteList& command, SourceStream& buffer);
};

//! Clientbound achievement complete list response.
struct LobbyCommandAchievementCompleteListOK
{
  uint32_t unk0{};
  std::vector<Quest> achievements;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandAchievementCompleteListOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandAchievementCompleteListOK& command, SourceStream& buffer);
};

//! Serverbound enter channel command.
struct LobbyCommandEnterChannel
{
  uint8_t channel;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandEnterChannel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandEnterChannel& command, SourceStream& buffer);
};

//! Clientbound enter channel response.
struct LobbyCommandEnterChannelOK
{
  uint8_t unk0;
  uint16_t unk1;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandEnterChannelOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandEnterChannelOK& command, SourceStream& buffer);
};

//! Serverbound enter channel command.
struct LobbyCommandEnterChannelCancel
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandEnterChannelCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandEnterChannelCancel& command, SourceStream& buffer);
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
  uint16_t unk3;
  uint8_t unk4;
  uint16_t unk5;
  uint8_t unk6;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandMakeRoom& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandMakeRoom& command, SourceStream& buffer);
};

//! Clientbound make room response.
struct LobbyCommandMakeRoomOK
{
  uint32_t unk0;
  uint32_t unk1;
  uint32_t ip;
  uint16_t port;
  uint8_t unk2;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandMakeRoomOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandMakeRoomOK& command, SourceStream& buffer);
};

//! Serverbound make room command.
struct LobbyCommandMakeRoomCancel
{
  uint8_t unk0;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandMakeRoomCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandMakeRoomCancel& command, SourceStream& buffer);
};

//! Serverbound request quest list command.
struct LobbyCommandRequestQuestList
{
  uint32_t unk0{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestQuestList& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestQuestList& command, SourceStream& buffer);
};

//! Clientbound request quest list response.
struct LobbyCommandRequestQuestListOK
{
  uint32_t unk0{};
  std::vector<Quest> quests;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestQuestListOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestQuestListOK& command, SourceStream& buffer);
};

struct LobbyCommandRequestDailyQuestList
{
  uint32_t val0{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestDailyQuestList& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestDailyQuestList& command, SourceStream& buffer);
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

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestDailyQuestListOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestDailyQuestListOK& command, SourceStream& buffer);
};

//! Serverbound enter ranch command.
struct LobbyCommandEnterRanch
{
  uint32_t unk0;
  std::string unk1;
  uint8_t unk2;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandEnterRanch& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandEnterRanch& command, SourceStream& buffer);
};

//! Clientbound enter ranch response.
struct LobbyCommandEnterRanchOK
{
  uint32_t ranchUid{};
  uint32_t code{}; // probably some auth token that has to be double checked in the ranch server
  uint32_t ip{};
  uint16_t port{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandEnterRanchOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandEnterRanchOK& command, SourceStream& buffer);
};

//! Serverbound enter ranch command.
struct LobbyCommandEnterRanchCancel
{
  uint16_t unk0;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandEnterRanchCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandEnterRanchCancel& command, SourceStream& buffer);
};


//! Serverbound get messenger info command.
struct LobbyCommandGetMessengerInfo
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGetMessengerInfo& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGetMessengerInfo& command, SourceStream& buffer);
};

//! Clientbound get messenger info response.
struct LobbyCommandGetMessengerInfoOK
{
  uint32_t code;
  uint32_t ip;
  uint16_t port;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGetMessengerInfoOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGetMessengerInfoOK& command, SourceStream& buffer);
};

//! Serverbound get messenger info command.
struct LobbyCommandGetMessengerInfoCancel
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGetMessengerInfoCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGetMessengerInfoCancel& command, SourceStream& buffer);
};


//! Serverbound room list command.
struct LobbyCommandRoomList
{
  uint8_t unk0;
  uint8_t unk1;
  uint8_t unk2;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRoomList& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRoomList& command, SourceStream& buffer);
};

struct Room {
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
  struct {
    uint32_t unk0;
    std::string unk1;
    uint16_t unk2;
  } unk3;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRoomListOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRoomListOK& command, SourceStream& buffer);
};

//! Serverbound request special event list command.
struct LobbyCommandRequestSpecialEventList
{
  uint32_t unk0;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestSpecialEventList& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestSpecialEventList& command, SourceStream& buffer);
};

struct Event {
  uint16_t unk0{};
  uint32_t unk1{};
};

//! Clientbound request special event list response.
struct LobbyCommandRequestSpecialEventListOK
{
  uint32_t unk0;
  std::vector<Quest> unk1;
  std::vector<Event> unk2;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandRequestSpecialEventListOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandRequestSpecialEventListOK& command, SourceStream& buffer);
};

//! Serverbound heartbeat command.
struct LobbyCommandHeartbeat
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandHeartbeat& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandHeartbeat& command, SourceStream& buffer);
};

//! Serverboud goods message
struct LobbyCommandGoodsShopList
{
  std::array<uint8_t, 12> data;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGoodsShopList& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGoodsShopList& command, SourceStream& buffer);
};

//! Clientbound shop goods message
struct LobbyCommandGoodsShopListOK
{
  std::array<uint8_t, 12> data;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGoodsShopListOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGoodsShopListOK& command, SourceStream& buffer);
};

//! Clientbound shop goods message
struct LobbyCommandGoodsShopListCancel
{
  std::array<uint8_t, 12> data;

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGoodsShopListCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGoodsShopListCancel& command, SourceStream& buffer);
};

struct LobbyCommandInquiryTreecash
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandInquiryTreecash& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandInquiryTreecash& command, SourceStream& buffer);
};

struct LobbyCommandInquiryTreecashOK
{
  uint32_t cash{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandInquiryTreecashOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandInquiryTreecashOK& command, SourceStream& buffer);
};

struct LobbyCommandInquiryTreecashCancel
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandInquiryTreecashCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandInquiryTreecashCancel& command, SourceStream& buffer);
};

struct LobbyClientNotify
{
  // Scene state
  // 1 - success
  // 2 - first cancel
  // 3 - repeated cancel
  uint16_t val0{};
  // Additional payload, for the success its always zero.
  // For the cancel it is the retry count
  uint32_t val1{};

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandInquiryTreecashCancel& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandInquiryTreecashCancel& command, SourceStream& buffer);
};

struct LobbyCommandGuildPartyList
{
  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGuildPartyList& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGuildPartyList& command, SourceStream& buffer);
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

  //! Writes the command to a provided sink buffer.
  //! @param command Command.
  //! @param buffer Sink buffer.
  static void Write(
    const LobbyCommandGuildPartyListOK& command, SinkStream& buffer);

  //! Reader a command from a provided source buffer.
  //! @param command Command.
  //! @param buffer Source buffer.
  static void Read(
    LobbyCommandGuildPartyListOK& command, SourceStream& buffer);
};

// TODO: AcCmdCLRequestPersonalInfo, others

} // namespace alica

#endif //LOBBY_MESSAGE_DEFINES_HPP
