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

#ifndef RANCH_MESSAGE_DEFINES_HPP
#define RANCH_MESSAGE_DEFINES_HPP

#include "CommonStructureDefinitions.hpp"
#include "libserver/network/command/CommandProtocol.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace server::protocol
{

struct RanchCommandHeartbeat
{
  static Command GetCommand()
  {
    return Command::AcCmdCRHeartbeat;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHeartbeat& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHeartbeat& command,
    SourceStream& stream);
};

struct AcCmdCREnterRanch
{
  uint32_t characterUid{};
  uint32_t otp{};
  uint32_t rancherUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCREnterRanch;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCREnterRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCREnterRanch& command,
    SourceStream& stream);
};

struct AcCmdCREnterRanchOK
{
  uint32_t rancherUid{};
  std::string rancherName{};
  std::string ranchName{};

  //! Horses on the ranch.
  std::vector<RanchHorse> horses{};
  //! Characters on the ranch.
  std::vector<RanchCharacter> characters{};

  uint64_t member6{0};
  uint32_t scramblingConstant{0};
  uint32_t ranchProgress{614090};

  // List size as a byte. Max length 13
  std::vector<Housing> housing{};

  uint8_t horseSlots{5};
  uint32_t member11{};

  enum class Bitset : uint32_t
  {
    IsLocked = 2,
  } bitset{};

  uint32_t incubatorSlotOne{2};
  uint32_t incubatorSlotTwo{1};

  std::array<Egg, 3> incubator;

  League league{};
  uint32_t member17{};

  static Command GetCommand()
  {
    return Command::AcCmdCREnterRanchOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCREnterRanchOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCREnterRanchOK& command,
    SourceStream& stream);
};

struct RanchCommandEnterRanchCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCREnterRanchCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandEnterRanchCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandEnterRanchCancel& command,
    SourceStream& stream);
};

struct RanchCommandEnterRanchNotify
{
  RanchCharacter character{};

  static Command GetCommand()
  {
    return Command::AcCmdCREnterRanchNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandEnterRanchNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandEnterRanchNotify& command,
    SourceStream& stream);
};

struct AcCmdCRLeaveRanch
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRanch;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRLeaveRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLeaveRanch& command,
    SourceStream& stream);
};

struct AcCmdCRLeaveRanchOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRanchOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRLeaveRanchOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLeaveRanchOK& command,
    SourceStream& stream);
};

struct AcCmdCRLeaveRanchNotify
{
  uint32_t characterId{}; // Probably

  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveRanchNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRLeaveRanchNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRLeaveRanchNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRanchChat
{
  std::string message;
  uint8_t unknown{};
  uint8_t unknown2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchChat;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRanchChat& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRanchChat& command,
    SourceStream& stream);
};

struct AcCmdCRRanchChatNotify
{
  std::string author;
  std::string message;
  uint8_t isBlue{};
  uint8_t unknown2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchChatNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRanchChatNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRanchChatNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRanchSnapshot
{
  enum Type : uint8_t
  {
    Full = 0,
    Partial = 1
  };

  struct FullSpatial
  {
    uint16_t ranchIndex{};
    uint32_t time{};
    //! A bitset.
    uint64_t action{};
    uint16_t timer{};
    std::array<std::byte, 12> member4{};
    std::array<std::byte, 16> matrix{};
    float velocityX{};
    float velocityY{};
    float velocityZ{};

    static void Write(const FullSpatial& structure, SinkStream& stream);
    static void Read(FullSpatial& structure, SourceStream& stream);
  };

  struct PartialSpatial
  {
    uint16_t ranchIndex{};
    uint32_t time{};
    //! A bitset.
    uint64_t action{};
    uint16_t timer{};
    std::array<std::byte, 12> member4{};
    std::array<std::byte, 16> matrix{};

    static void Write(const PartialSpatial& structure, SinkStream& stream);
    static void Read(PartialSpatial& structure, SourceStream& stream);
  };

  Type type{};
  FullSpatial full{};
  PartialSpatial partial{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchSnapshot;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRanchSnapshot& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRanchSnapshot& command,
    SourceStream& stream);
};

struct RanchCommandRanchSnapshotNotify
{
  uint16_t ranchIndex{};

  AcCmdCRRanchSnapshot::Type type{};
  AcCmdCRRanchSnapshot::FullSpatial full{};
  AcCmdCRRanchSnapshot::PartialSpatial partial{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchSnapshotNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRanchSnapshotNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRanchSnapshotNotify& command,
    SourceStream& stream);
};

struct RanchCommandEnterBreedingMarket
{
  static Command GetCommand()
  {
    return Command::AcCmdCREnterBreedingMarket;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandEnterBreedingMarket& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandEnterBreedingMarket& command,
    SourceStream& stream);
};

struct RanchCommandEnterBreedingMarketOK
{
  // List size specified with a uint8_t. Max size 10
  struct AvailableHorse
  {
    uint32_t uid{};
    uint32_t tid{};
    // Counts of successful breeds (>:o) in succession.
    uint8_t combo{};
    uint32_t unk1{};

    uint8_t unk2{};
    // Basically weighted score of number of ancestors that share the same coat as the horse.
    // Ancestors of first generation add two points to lineage,
    // ancestors of the second generation add one point to the lineage,
    // while the horse itself adds 1.
    uint8_t lineage{};
  };
  std::vector<AvailableHorse> availableHorses{};

  static Command GetCommand()
  {
    return Command::AcCmdCREnterBreedingMarketOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandEnterBreedingMarketOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandEnterBreedingMarketOK& command,
    SourceStream& stream);
};

struct RanchCommandEnterBreedingMarketCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCREnterBreedingMarketCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandEnterBreedingMarketCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandEnterBreedingMarketCancel& command,
    SourceStream& stream);
};

struct RanchCommandLeaveBreedingMarket
{
  static Command GetCommand()
  {
    return Command::AcCmdCRLeaveBreedingMarket;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandLeaveBreedingMarket& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandLeaveBreedingMarket& command,
    SourceStream& stream);
};

struct RanchCommandSearchStallion
{
  uint32_t unk0{};
  uint8_t unk1{};
  uint8_t unk2{};
  uint8_t unk3{};
  uint8_t unk4{};
  uint8_t unk5{};
  uint8_t unk6{};
  uint8_t unk7{};
  uint8_t unk8{};

  // Nested list size specified with a uint8_t. Max size 3
  std::array<std::vector<uint32_t>, 3> unk9{};

  uint8_t unk10{};

  static Command GetCommand()
  {
    return Command::AcCmdCRSearchStallion;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandSearchStallion& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandSearchStallion& command,
    SourceStream& stream);
};

struct RanchCommandSearchStallionOK
{
  // Possibly some paging values?
  // For example, current page/number of pages
  uint32_t unk0{};
  uint32_t unk1{};

  struct Stallion
  {
    std::string unk0{};
    uint32_t uid{};
    uint32_t tid{};
    std::string name{};
    uint8_t grade{};
    uint8_t chance{};
    uint32_t price{};
    uint32_t unk7{};
    uint32_t unk8{};
    Horse::Stats stats{};
    Horse::Parts parts{};
    Horse::Appearance appearance{};
    uint8_t unk11{};
    uint8_t coatBonus{};
  };

  // List size specified with a uint8_t. Max size 10
  std::vector<Stallion> stallions{};

  static Command GetCommand()
  {
    return Command::AcCmdCRSearchStallionOK;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandSearchStallionOK& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandSearchStallionOK& command,
    SourceStream& stream);
};

struct RanchCommandSearchStallionCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRSearchStallionCancel;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandSearchStallionCancel& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandSearchStallionCancel& command,
    SourceStream& stream);
};

struct RanchCommandRegisterStallion
{
  uint32_t horseUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRegisterStallion;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRegisterStallion& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRegisterStallion& command,
    SourceStream& stream);
};

struct RanchCommandRegisterStallionOK
{
  uint32_t horseUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRegisterStallionOK;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRegisterStallionOK& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRegisterStallionOK& command,
    SourceStream& stream);
};

struct RanchCommandRegisterStallionCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRRegisterStallionCancel;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRegisterStallionCancel& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRegisterStallionCancel& command,
    SourceStream& stream);
};

struct RanchCommandUnregisterStallion
{
  uint32_t horseUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUnregisterStallion;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUnregisterStallion& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUnregisterStallion& command,
    SourceStream& stream);
};

struct RanchCommandUnregisterStallionOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRUnregisterStallionOK;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUnregisterStallionOK& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUnregisterStallionOK& command,
    SourceStream& stream);
};

struct RanchCommandUnregisterStallionCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRUnregisterStallionCancel;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUnregisterStallionCancel& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUnregisterStallionCancel& command,
    SourceStream& stream);
};

struct RanchCommandUpdateEquipmentNotify
{
  uint32_t characterUid{};
  std::vector<Item> characterEquipment;
  std::vector<Item> mountEquipment;
  Horse mount{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateEquipmentNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdateEquipmentNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdateEquipmentNotify& command,
    SourceStream& stream);
};

struct RanchCommandStatusPointApply
{
  uint32_t horseUid{};
  Horse::Stats stats{};

  static Command GetCommand()
  {
    return Command::AcCmdCRStatusPointApply;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandStatusPointApply& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandStatusPointApply& command,
    SourceStream& stream);
};

struct RanchCommandStatusPointApplyOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRStatusPointApplyOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandStatusPointApplyOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandStatusPointApplyOK& command,
    SourceStream& stream);
};

struct RanchCommandStatusPointApplyCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRStatusPointApplyCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandStatusPointApplyCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandStatusPointApplyCancel& command,
    SourceStream& stream);
};

struct RanchCommandTryBreeding
{
  uint32_t unk0{};
  uint32_t unk1{};

  static Command GetCommand()
  {
    return Command::AcCmdCRTryBreeding;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandTryBreeding& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandTryBreeding& command,
    SourceStream& stream);
};

struct RanchCommandTryBreedingOK
{
  uint32_t uid{};
  uint32_t tid{};
  uint32_t val{};
  uint32_t count{};

  uint8_t unk0{};

  Horse::Parts parts{};
  Horse::Appearance appearance{};
  Horse::Stats stats{};

  uint32_t unk1{};
  uint8_t unk2{};
  uint8_t unk3{};
  uint8_t unk4{};
  uint8_t unk5{};
  uint8_t unk6{};
  uint8_t unk7{};
  uint8_t unk8{};
  uint16_t unk9{};
  uint8_t unk10{};

  static Command GetCommand()
  {
    return Command::AcCmdCRTryBreedingOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandTryBreedingOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandTryBreedingOK& command,
    SourceStream& stream);
};

struct RanchCommandTryBreedingCancel
{
  uint8_t unk0{};
  uint32_t unk1{};
  uint8_t unk2{};
  uint8_t unk3{};
  uint8_t unk4{};
  uint8_t unk5{};

  static Command GetCommand()
  {
    return Command::AcCmdCRTryBreedingCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandTryBreedingCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandTryBreedingCancel& command,
    SourceStream& stream);
};

struct RanchCommandBreedingAbandon
{
  uint32_t horseUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRBreedingAbandon;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandBreedingAbandon& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandBreedingAbandon& command,
    SourceStream& stream);
};

struct RanchCommandBreedingAbandonOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRBreedingAbandonOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandBreedingAbandon& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandBreedingAbandon& command,
    SourceStream& stream);
};

struct RanchCommandBreedingAbandonCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRBreedingAbandonCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandBreedingAbandon& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandBreedingAbandon& command,
    SourceStream& stream);
};

struct RanchCommandAchievementUpdateProperty
{
  //! 75 - level up
  //! Table `Achievements`
  uint16_t achievementEvent{};
  uint16_t member2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRAchievementUpdateProperty;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandAchievementUpdateProperty& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandAchievementUpdateProperty& command,
    SourceStream& stream);
};

struct RanchCommandBreedingWishlist
{
  static Command GetCommand()
  {
    return Command::AcCmdCRBreedingWishlist;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandBreedingWishlist& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandBreedingWishlist& command,
    SourceStream& stream);
};

struct RanchCommandBreedingWishlistOK
{
  struct WishlistElement
  {
    std::string unk0{};
    uint32_t uid{};
    uint32_t tid{};
    uint8_t unk1{};
    std::string unk2{};
    uint8_t unk3{};
    uint32_t unk4{};
    uint32_t unk5{};
    uint32_t unk6{};
    uint32_t unk7{};
    uint32_t unk8{};
    Horse::Stats stats{};
    Horse::Parts parts{};
    Horse::Appearance appearance{};
    uint8_t unk9{};
    uint8_t unk10{};
    uint8_t unk11{};
  };

  // List length specified with a uint8_t, max size 8
  std::vector<WishlistElement> wishlist{};

  static Command GetCommand()
  {
    return Command::AcCmdCRBreedingWishlistOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandBreedingWishlistOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandBreedingWishlistOK& command,
    SourceStream& stream);
};

struct RanchCommandBreedingWishlistCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRBreedingWishlistCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandBreedingWishlistCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandBreedingWishlistCancel& command,
    SourceStream& stream);
};

struct AcCmdCRRanchCmdAction
{
  uint16_t unk0{};
  std::vector<uint8_t> snapshot{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchCmdAction;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRanchCmdAction& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRanchCmdAction& command,
    SourceStream& stream);
};

struct RanchCommandRanchCmdActionNotify
{
  uint16_t unk0{};
  uint16_t unk1{};
  uint8_t unk2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchCmdActionNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRanchCmdActionNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRanchCmdActionNotify& command,
    SourceStream& stream);
};

struct RanchCommandRanchStuff
{
  uint32_t eventId{};
  int32_t value{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchStuff;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRanchStuff& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRanchStuff& command,
    SourceStream& stream);
};

struct RanchCommandRanchStuffOK
{
  uint32_t eventId{};
  int32_t moneyIncrement{};
  int32_t totalMoney{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRanchStuffOK;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRanchStuffOK& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRanchStuffOK& command,
    SourceStream& stream);
};

struct RanchCommandUpdateBusyState
{
  uint8_t busyState{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateBusyState;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdateBusyState& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdateBusyState& command,
    SourceStream& stream);
};

struct RanchCommandUpdateBusyStateNotify
{
  uint32_t characterUid{};
  uint8_t busyState{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateBusyStateNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdateBusyStateNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdateBusyStateNotify& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct RanchCommandUpdateMountNickname
{
  uint32_t horseUid{};
  std::string name{};
  uint32_t unk1{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateMountNickname;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdateMountNickname& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdateMountNickname& command,
    SourceStream& stream);
};

//! Clientbound get messenger info response.
struct RanchCommandUpdateMountNicknameOK
{
  uint32_t horseUid{};
  std::string nickname{};
  uint32_t unk1{};
  uint32_t unk2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateMountNicknameOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdateMountNicknameOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdateMountNicknameOK& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct RanchCommandUpdateMountNicknameCancel
{
  uint8_t unk0{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateMountNicknameCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdateMountNicknameCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdateMountNicknameCancel& command,
    SourceStream& stream);
};

struct RanchCommandUpdateMountInfoNotify
{
  enum class Action
  {
    Default = 0,
    UpdateMount = 4,
    SetMountStateAndBreedData = 5,
    SomeItemManip0 = 9,
    SomeItemManip1 = 10,
    SomeItemManip2 = 12,
    SomeItemManip3 = 13,
  };

  Action action{Action::UpdateMount};
  uint8_t member1{};
  Horse horse{};

  static Command GetCommand()
  {
    return Command::AcCmdRCUpdateMountInfoNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdateMountInfoNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdateMountInfoNotify& command,
    SourceStream& stream);
};

struct RanchCommandRequestStorage
{
  enum class Category : uint8_t
  {
    Purchases,
    Gifts
  };

  Category category{};
  uint16_t page{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestStorage;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestStorage& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestStorage& command,
    SourceStream& stream);
};

struct RanchCommandRequestStorageOK
{
  RanchCommandRequestStorage::Category category{};
  uint16_t page{};
  // First bit indicates whether there's new items
  // in the storage. Other bits somehow indicate the page count.
  uint16_t pageCountAndNotification{};

  //! Max 33 elements.
  std::vector<StoredItem> storedItems{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestStorageOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestStorageOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestStorageOK& command,
    SourceStream& stream);
};

struct RanchCommandRequestStorageCancel
{
  RanchCommandRequestStorage category{};
  uint8_t val1{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestStorageCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestStorageCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestStorageCancel& command,
    SourceStream& stream);
};

struct RanchCommandGetItemFromStorage
{
  uint32_t storedItemUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRGetItemFromStorage;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandGetItemFromStorage& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandGetItemFromStorage& command,
    SourceStream& stream);
};

struct RanchCommandGetItemFromStorageOK
{
  uint32_t storedItemUid{};
  std::vector<Item> items{};
  uint32_t member0{};

  static Command GetCommand()
  {
    return Command::AcCmdCRGetItemFromStorageOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandGetItemFromStorageOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandGetItemFromStorageOK& command,
    SourceStream& stream);
};

struct RanchCommandGetItemFromStorageCancel
{
  uint32_t storedItemUid{};
  uint8_t status{};

  static Command GetCommand()
  {
    return Command::AcCmdCRGetItemFromStorageCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandGetItemFromStorageCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandGetItemFromStorageCancel& command,
    SourceStream& stream);
};

struct RanchCommandCheckStorageItem
{
  uint32_t storedItemUid{};

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandGetItemFromStorage& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandGetItemFromStorage& command,
    SourceStream& stream);
};

struct RanchCommandRequestNpcDressList
{
  uint32_t unk0{}; // NPC ID?

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestNpcDressList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestNpcDressList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestNpcDressList& command,
    SourceStream& stream);
};

struct RanchCommandRequestNpcDressListOK
{
  uint32_t unk0{}; // NPC ID?

  // List size specified with a uint8_t. Max size 10
  std::vector<Item> dressList{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestNpcDressListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestNpcDressListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestNpcDressListOK& command,
    SourceStream& stream);
};

struct RanchCommandRequestNpcDressListCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestNpcDressListCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestNpcDressListCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestNpcDressListCancel& command,
    SourceStream& stream);
};

struct RanchCommandWearEquipment
{
  uint32_t itemUid{};
  uint8_t member{};

  static Command GetCommand()
  {
    return Command::AcCmdCRWearEquipment;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandWearEquipment& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandWearEquipment& command,
    SourceStream& stream);
};

struct RanchCommandWearEquipmentOK
{
  uint32_t itemUid{};
  uint8_t member{};

  static Command GetCommand()
  {
    return Command::AcCmdCRWearEquipmentOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandWearEquipmentOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandWearEquipmentOK& command,
    SourceStream& stream);
};

struct RanchCommandWearEquipmentCancel
{
  uint32_t itemUid{};
  uint8_t member{};

  static Command GetCommand()
  {
    return Command::AcCmdCRWearEquipmentCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandWearEquipmentCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandWearEquipmentCancel& command,
    SourceStream& stream);
};

struct RanchCommandRemoveEquipment
{
  uint32_t itemUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRemoveEquipment;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRemoveEquipment& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRemoveEquipment& command,
    SourceStream& stream);
};

struct RanchCommandRemoveEquipmentOK
{
  uint32_t uid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRemoveEquipmentOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRemoveEquipmentOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRemoveEquipmentOK& command,
    SourceStream& stream);
};

struct RanchCommandRemoveEquipmentCancel
{
  uint32_t itemUid{};
  uint8_t member{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRemoveEquipmentCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRemoveEquipmentCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRemoveEquipmentCancel& command,
    SourceStream& stream);
};

struct RanchCommandSetIntroductionNotify
{
  uint32_t characterUid{};
  std::string introduction{};

  static Command GetCommand()
  {
    return Command::AcCmdRCSetIntroductionNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandSetIntroductionNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandSetIntroductionNotify& command,
    SourceStream& stream);
};

struct RanchCommandCreateGuild
{
  std::string name;
  std::string description;

  static Command GetCommand()
  {
    return Command::AcCmdCRCreateGuild;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandCreateGuild& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandCreateGuild& command,
    SourceStream& stream);
};

struct RanchCommandCreateGuildOK
{
  uint32_t uid{};
  uint32_t member2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRCreateGuildOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandCreateGuildOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandCreateGuildOK& command,
    SourceStream& stream);
};

struct RanchCommandCreateGuildCancel
{
  uint8_t status{};
  uint32_t member2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRCreateGuildCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandCreateGuildCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandCreateGuildCancel& command,
    SourceStream& stream);
};

struct RanchCommandRequestGuildInfo
{
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestGuildInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestGuildInfo& command,
    SourceStream& stream);
};

struct RanchCommandRequestGuildInfoOK
{
  struct GuildInfo
  {
    uint32_t uid{};
    uint8_t member1{};
    uint32_t member2{};
    uint32_t member3{};
    uint8_t member4{};
    uint32_t member5{};
    std::string name{};
    std::string description{};
    uint32_t member8{};
    uint32_t member9{};
    uint32_t member10{};
    uint32_t member11{};

    static void Write(
      const GuildInfo& command,
      SinkStream& stream);

    static void Read(
      GuildInfo& command,
      SourceStream& stream);
  } guildInfo;

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildInfoOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestGuildInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestGuildInfoOK& command,
    SourceStream& stream);
};

struct RanchCommandRequestGuildInfoCancel
{
  uint8_t status{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildInfoCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestGuildInfoCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestGuildInfoCancel& command,
    SourceStream& stream);
};

struct RanchCommandUpdatePet
{
  PetInfo petInfo{};
  //! optional
  uint32_t actionBitset{}; // 7 - rename

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdatePet;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUpdatePet& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUpdatePet& command,
    SourceStream& stream);
};

struct RanchCommandRequestPetBirth
{
  uint32_t member1{};
  uint32_t member2{};
  PetInfo petInfo{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestPetBirth;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestPetBirth& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestPetBirth& command,
    SourceStream& stream);
};

struct RanchCommandRequestPetBirthOK
{
  PetBirthInfo petBirthInfo{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestPetBirthOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestPetBirthOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestPetBirthOK& command,
    SourceStream& stream);
};

struct RanchCommandRequestPetBirthCancel
{
  PetInfo petInfo{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestPetBirthCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestPetBirthCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestPetBirthCancel& command,
    SourceStream& stream);
};

struct RanchCommandPetBirthNotify
{
  PetBirthInfo petBirthInfo{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestPetBirthNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandPetBirthNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandPetBirthNotify& command,
    SourceStream& stream);
};

struct RanchCommandIncubateEgg
{
  uint32_t itemUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRIncubateEgg;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandPetBirthNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandPetBirthNotify& command,
    SourceStream& stream);
};

struct RanchCommandIncubateEggOK
{
  uint32_t itemUid{};
  Egg egg{};
  // optional
  uint32_t member3{};

  static Command GetCommand()
  {
    return Command::AcCmdCRIncubateEggOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandIncubateEggOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandIncubateEggOK& command,
    SourceStream& stream);
};

struct RanchCommandUserPetInfos
{

  static Command GetCommand()
  {
    return Command::AcCmdCRUserPetInfos;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUserPetInfos& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUserPetInfos& command,
    SourceStream& stream);
};

struct RanchCommandUserPetInfosOK
{
  uint32_t member1;
  uint16_t petCount;
  uint16_t member3;
  std::vector<Pet> pets{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUserPetInfosOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUserPetInfosOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUserPetInfosOK& command,
    SourceStream& stream);
};

struct RanchCommandHousingBuild
{
  uint16_t housingTid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingBuild;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingBuild& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingBuild& command,
    SourceStream& stream);
};

struct RanchCommandHousingBuildOK
{
  uint32_t member1{};
  uint16_t housingTid{};
  uint32_t member3{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingBuildOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingBuildOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingBuildOK& command,
    SourceStream& stream);
};

struct RanchCommandHousingBuildCancel
{
  uint8_t status{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingBuildCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingBuildCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingBuildCancel& command,
    SourceStream& stream);
};

struct RanchCommandHousingBuildNotify
{
  uint32_t member1{};
  uint16_t housingTid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingBuildNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingBuildNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingBuildNotify& command,
    SourceStream& stream);
};

struct RanchCommandHousingRepair
{
  uint32_t housingUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingRepair;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingRepair& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingRepair& command,
    SourceStream& stream);
};

struct RanchCommandHousingRepairOK
{
  uint32_t housingUid{};
  uint32_t member2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingRepairOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingRepairOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingRepairOK& command,
    SourceStream& stream);
};

struct RanchCommandHousingRepairCancel
{
  uint8_t status{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingRepairCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingRepairCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingRepairCancel& command,
    SourceStream& stream);
};

struct RanchCommandHousingRepairNotify
{
  //! Ignored by the client.
  uint32_t member1{};
  uint16_t housingTid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingRepairNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandHousingRepairNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandHousingRepairNotify& command,
    SourceStream& stream);
};
struct RanchCommandMissionEvent
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
  uint32_t callerOid{};
  uint32_t calledOid{};

  static Command GetCommand()
  {
    return Command::AcCmdRCMissionEvent;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandMissionEvent& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandMissionEvent& command,
    SourceStream& stream);
};

struct RanchCommandKickRanch
{
  uint32_t characterUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRKickRanch;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandKickRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandKickRanch& command,
    SourceStream& stream);
};

struct RanchCommandKickRanchOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRKickRanchOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandKickRanchOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandKickRanchOK& command,
    SourceStream& stream);
};

struct RanchCommandKickRanchCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRKickRanchCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandKickRanchCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandKickRanchCancel& command,
    SourceStream& stream);
};

struct RanchCommandKickRanchNotify
{
  uint32_t characterUid{};

  static Command GetCommand()
  {
    return Command::AcCmdRCKickRanchNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandKickRanchNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandKickRanchNotify& command,
    SourceStream& stream);
};

struct RanchCommandOpCmd
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
    const RanchCommandOpCmd& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandOpCmd& command,
    SourceStream& stream);
};

struct RanchCommandOpCmdOK
{
  std::string feedback{};

  enum class Observer : uint32_t
  {
    Enabled = 1,
    Disabled = 2,
  } observerState;

  static Command GetCommand()
  {
    return Command::AcCmdCROpCmdOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandOpCmdOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandOpCmdOK& command,
    SourceStream& stream);
};

struct RanchCommandRequestLeagueTeamList
{
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestLeagueTeamList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestLeagueTeamList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestLeagueTeamList& command,
    SourceStream& stream);
};

struct RanchCommandRequestLeagueTeamListOK
{
  //! Table LeagueSeasonInfo
  uint8_t season{};
  //! 0 - no league info available
  uint8_t league{};
  uint32_t group{};
  uint32_t points{};
  uint8_t rank{};
  uint8_t previousRank{};
  uint32_t breakPoints{};
  uint32_t unk7{};
  uint8_t unk8{};
  uint8_t lastWeekLeague{};
  uint32_t lastWeekGroup{};
  uint8_t lastWeekRank{};
  //! 0 - last week info unavailable, 1 - item ready to claim, 2 - already claimed
  uint8_t lastWeekAvailable{};
  uint8_t unk13{};

  struct Member {
    uint32_t uid{};
    uint32_t points{};
    std::string name{};
  };
  //! Max 100 elements
  std::vector<Member> members;

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestLeagueTeamListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestLeagueTeamListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestLeagueTeamListOK& command,
    SourceStream& stream);
};

struct RanchCommandRequestLeagueTeamListCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestLeagueTeamListCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRequestLeagueTeamListCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRequestLeagueTeamListCancel& command,
    SourceStream& stream);
};

struct RanchCommandUseItem
{
  uint32_t itemUid{};
  uint16_t always1{};
  uint32_t always1too{};

  enum class Play : uint32_t
  {
    Bad = 0,
    Good = 1,
    CriticalGood = 2,
    Perfect = 3,
  };
  Play play{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUseItem;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUseItem& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUseItem& command,
    SourceStream& stream);
};

//!
struct RanchCommandUseItemOK
{
  enum class ActionType : uint32_t
  {
    Empty,
    Action1,
    Action2,
    Action3,
    Action4
  };

  struct ActionTwoBytes
  {
    uint8_t unk0{};
    RanchCommandUseItem::Play play{};

    static void Write(
      const ActionTwoBytes& action,
      SinkStream& stream);
    static void Read(
      ActionTwoBytes& action,
      SourceStream& stream);
  };

  struct ActionOneByte
  {
    uint8_t unk0{};

    static void Write(
      const ActionOneByte& action,
      SinkStream& stream);
    static void Read(
      ActionOneByte& action,
      SourceStream& stream);
  };

  uint32_t itemUid{};
  uint16_t unk1{};

  // Action points to different structures depending on type
  ActionType type{};

  ActionTwoBytes actionTwoBytes{};
  ActionOneByte actionOneByte{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUseItemOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUseItemOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUseItemOK& command,
    SourceStream& stream);
};

//!
struct RanchCommandUseItemCancel
{
  uint32_t unk0{};
  uint8_t unk1{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUseItemCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandUseItemCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandUseItemCancel& command,
    SourceStream& stream);
};

//!
struct RanchCommandMountFamilyTree
{
  uint32_t unk0;

  static Command GetCommand()
  {
    return Command::AcCmdCRMountFamilyTree;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandMountFamilyTree& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandMountFamilyTree& command,
    SourceStream& stream);
};

//!
struct RanchCommandMountFamilyTreeOK
{
  uint32_t unk0{};

  struct MountFamilyTreeItem
  {
    uint8_t unk0{};
    std::string unk1{};
    uint8_t unk2{};
    uint16_t unk3{};
  };

  // In the packet, the length is specified as a byte
  // max size 6
  std::vector<MountFamilyTreeItem> items;

  static Command GetCommand()
  {
    return Command::AcCmdCRMountFamilyTreeOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandMountFamilyTreeOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandMountFamilyTreeOK& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct RanchCommandMountFamilyTreeCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRMountFamilyTreeCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandMountFamilyTreeCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandMountFamilyTreeCancel& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // RANCH_MESSAGE_DEFINES_HPP
