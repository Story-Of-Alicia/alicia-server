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

#ifndef COMMON_MESSAGE_DEFINES_HPP
#define COMMON_MESSAGE_DEFINES_HPP

#include "libserver/network/command/CommandProtocol.hpp"
#include "libserver/network/command/proto/CommonStructureDefinitions.hpp"
#include <libserver/util/Stream.hpp>

#include <string>

namespace server::protocol
{

struct AcCmdCRInviteUser
{
  uint32_t recipientCharacterUid{};
  std::string recipientCharacterName{};

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteUser;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteUser& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteUser& command,
    SourceStream& stream);
};

struct AcCmdCRInviteUserCancel : AcCmdCRInviteUser
{
  // Identical to `AcCmdCRInviteUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteUserCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteUserCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteUserCancel& command,
    SourceStream& stream);
};

struct AcCmdCRInviteUserOK : AcCmdCRInviteUser
{
  // Identical to `AcCmdCRInviteUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteUserOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteUserOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteUserOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestUser
{
  bool force{};
  std::string characterName{};
  uint32_t roomUid{};
  uint32_t ranchUid{};
  
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestUser;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestUser& command,
    SinkStream& stream);
  
  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestUser& command,
    SourceStream& stream);
};

struct AcCmdCRRequestUserOK : AcCmdCRRequestUser
{
  // Identical to `AcCmdCRRequestUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestUserOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestUserOK& command,
    SinkStream& stream);
  
  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestUserOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestUserCancel : AcCmdCRRequestUser
{
  // Identical to `AcCmdCRRequestUser`

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestUserCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestUserCancel& command,
    SinkStream& stream);
  
  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestUserCancel& command,
    SourceStream& stream);
};

struct AcCmdRCRequestUser : AcCmdCRRequestUser
{
  // Identical to `AcCmdCRRequestUser`

  static Command GetCommand()
  {
    return Command::AcCmdRCRequestUser;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCRequestUser& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCRequestUser& command,
    SourceStream& stream);
};

//! Server-initiated, clientbound notification indicating to the client
//! progression of a quest.
//! Can be used in either ranch or race.
struct AcCmdRCUpdateQuestNotify
{
  uint32_t characterUid{};
  uint16_t questTid{};
  ObjectiveProgress objectiveProgress{};

  static Command GetCommand()
  {
    return Command::AcCmdRCUpdateQuestNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCUpdateQuestNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCUpdateQuestNotify& command,
    SourceStream& stream);
};

struct AcCmdRCUpdateDailyQuestNotify
{   
  uint32_t characterUid;
  uint16_t questId;
  ObjectiveProgress objectiveProgress;
  uint32_t carrotsReward; //used when rewardType is Carrots
  QuestRewardType rewardType{QuestRewardType::None};
  uint32_t unk2;
  uint32_t mountExp; //used when rewardType is Exp

  static Command GetCommand()
  {
    return Command::AcCmdRCUpdateDailyQuestNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCUpdateDailyQuestNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCUpdateDailyQuestNotify& command,
    SourceStream& stream);
};

struct AcCmdCROpCmd
{
  std::string command{};

  static Command GetCommand()
  {
    return Command::AcCmdCROpCmd;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCROpCmd& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCROpCmd& command,
    SourceStream& stream);
};

struct AcCmdRCUpdateMountInfoNotify
{
  // TODO: confirm these values
  enum class Action : uint8_t
  {
    // Takes horse name + type (type foal interacts with graze)
    Default = 0,
    // Has gMsgSetMountInfo/RanchCare_ResetAmends//Ranch_UpdateMountName
    // [Ranch_UpdateMountName] characterUid = 0
    // This appears to do the horse change animation
    MaybeRentHorseOrReturnToNature = 4,
    // Has gMsgSetMountState/Breed_SuccessData_MountSeed
    // [Breed_SuccessData_MountSeed] seed? = 0
    PutHorseInRentOrBreedingSystem = 5,
    // Takes potentialLevel and potentialValue
    ProgressHorsePotential = 9,
    // Just takes luck.
    SomethingWithHorseLuck = 10,
    UpdateInjuryState = 11,
    SomethingWithInjuryAndLuck = 12
  };

  uint32_t characterUid{};
  Action action{Action::Default};
  Horse horse{};

  static Command GetCommand()
  {
    return Command::AcCmdRCUpdateMountInfoNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCUpdateMountInfoNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCUpdateMountInfoNotify& command,
    SourceStream& stream);
};

struct AcCmdRCMobDead
{
  uint16_t mobOid{};

  static Command GetCommand()
  {
    return Command::AcCmdRCMobDead;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCMobDead& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCMobDead& command,
    SourceStream& stream);
};

struct AcCmdRCMissionEvent
{
  enum class Event : uint32_t
  {
    EVENT_UI_CLOSE=1,
    EVENT_PLAYER_INPUT=2,
    EVENT_PLAYER_ACTION=3,
    EVENT_ENTER_POSITION=4,
    EVENT_GET_ITEM=5,
    EVENT_USE_ITEM=6,
    EVENT_TIMER=7,
    EVENT_SCRIPT=8,
    EVENT_TRIGGER=9,
    EVENT_WAIT=10,
    EVENT_RECORD=11,
    EVENT_GAME=12,
    EVENT_CAMERA_STOP=13,
    EVENT_PATROL_END=14,
    EVENT_PATROL_NEXT=15,
    EVENT_HORSE_ACTION_END=16,
    EVENT_UI=17,
    EVENT_AREA_ENTER=18,
    EVENT_AREA_LEAVE=19,
    EVENT_NPC_CHAT=20,
    EVENT_ACTIVE_CONTENT=21,
    EVENT_PLAYER_COLLISION=22,
    EVENT_CALL_NPC=23,
    EVENT_ORDER_NPC=24,
    EVENT_CALLED_NPC=25,
    EVENT_CALL_NPC_RESULT=26,
    EVENT_NPC_FOLLOWING_END=27,
    EVENT_DEV_SET_MOUNT_CONDITION=28,
    EVENT_NPC_FOLLOW_START=29,
    EVENT_CHANGE_MOUNT=30,
    EVENT_GAME_STEP=31,
    EVENT_DEV_SET_GROUP_FORCE=32,
    EVENT_FUN_KNOCKBACK=33,
    EVENT_FUN_KNOCKBACK_INFO=34,
    EVENT_SHEEP_COIN_DROP=35,
    EVENT_WAVE_START=36,
    EVENT_WAVE_END=37
  };

  Event event{};
  uint32_t val1{};
  uint32_t val2{};

  static Command GetCommand()
  {
    return Command::AcCmdRCMissionEvent;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCMissionEvent& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCMissionEvent& command,
    SourceStream& stream);
};

struct AcCmdCRAchievementUpdateProperty
{
  uint16_t propertyKey{};
  std::string propertyValue{};

  static Command GetCommand()
  {
    return Command::AcCmdCRAchievementUpdateProperty;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRAchievementUpdateProperty& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRAchievementUpdateProperty& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // COMMON_MESSAGE_DEFINES_HPP
