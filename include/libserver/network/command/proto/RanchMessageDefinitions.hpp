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

  //! Incubator logic:
  //! icubator slots from 0 to 3, 0 = locked, 1 = single, 2 = double, 3 = triple
  //! incubator use count should count down to 0, if broken transform into single incubator
  uint32_t incubatorSlots{2};
  uint32_t incubatorUseCount{};

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
  uint8_t isSystem{};
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

struct AcCmdCREnterBreedingMarket
{
  static Command GetCommand()
  {
    return Command::AcCmdCREnterBreedingMarket;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCREnterBreedingMarket& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCREnterBreedingMarket& command,
    SourceStream& stream);
};

struct RanchCommandEnterBreedingMarketOK
{
  struct Stallion
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

  //! Max 10 elements.
  std::vector<Stallion> stallions{};

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

struct AcCmdCRSearchStallion
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
    const AcCmdCRSearchStallion& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRSearchStallion& command,
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
    std::string member1{};
    uint32_t uid{};
    uint32_t tid{};
    std::string name{};
    uint8_t grade{};
    uint8_t chance{};
    uint32_t matePrice{};
    uint32_t unk7{};
    // 1304
    uint32_t expiresAt{};
    Horse::Stats stats{};
    Horse::Parts parts{};
    Horse::Appearance appearance{};
    uint8_t unk11{};
    uint8_t lineage{};
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

struct AcCmdCRRegisterStallion
{
  uint32_t horseUid{};
  uint32_t carrots{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRegisterStallion;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRegisterStallion& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRegisterStallion& command,
    SourceStream& stream);
};

struct AcCmdCRRegisterStallionOK
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
    const AcCmdCRRegisterStallionOK& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRegisterStallionOK& command,
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

struct AcCmdCRUnregisterStallion
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
    const AcCmdCRUnregisterStallion& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUnregisterStallion& command,
    SourceStream& stream);
};

struct AcCmdCRUnregisterStallionOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRUnregisterStallionOK;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUnregisterStallionOK& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUnregisterStallionOK& command,
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

struct AcCmdCRUnregisterStallionEstimateInfo
{
  uint32_t horseUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUnregisterStallionEstimateInfo;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUnregisterStallionEstimateInfo& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUnregisterStallionEstimateInfo& command,
    SourceStream& stream);
};

struct AcCmdCRUnregisterStallionEstimateInfoOK
{
  uint32_t member1{};
  //! A count of times the stallion mated.
  uint32_t timesMated{};
  //! An amount of carrots collected for mating.
  uint32_t matingCompensation{};
  uint32_t member4{};
  //! A price for mating.
  uint32_t matingPrice{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUnregisterStallionEstimateInfoOK;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUnregisterStallionEstimateInfoOK& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUnregisterStallionEstimateInfoOK& command,
    SourceStream& stream);
};

struct AcCmdCRUnregisterStallionEstimateInfoCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRUnregisterStallionEstimateInfoCancel;
  }

  //! Writes the command to the provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUnregisterStallionEstimateInfoCancel& command,
    SinkStream& stream);

  //! Reads a command from the provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUnregisterStallionEstimateInfoCancel& command,
    SourceStream& stream);
};

struct AcCmdCRUpdateEquipmentNotify
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
    const AcCmdCRUpdateEquipmentNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUpdateEquipmentNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRecoverMount
{
  uint32_t horseUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRecoverMount;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRecoverMount& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRecoverMount& command,
    SourceStream& stream);
};

struct AcCmdCRRecoverMountOK
{
  uint32_t horseUid{};
  uint16_t stamina{};
  uint32_t updatedCarrots{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRecoverMountOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRecoverMountOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRecoverMountOK& command,
    SourceStream& stream);
};

struct AcCmdCRRecoverMountCancel
{
  uint32_t horseUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRecoverMountCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRecoverMountCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRecoverMountCancel& command,
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

struct AcCmdCRTryBreeding
{
  uint32_t mareUid{};
  uint32_t stallionUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRTryBreeding;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRTryBreeding& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRTryBreeding& command,
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

struct AcCmdCRBreedingAbandon
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
    const AcCmdCRBreedingAbandon& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRBreedingAbandon& command,
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
    const AcCmdCRBreedingAbandon& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRBreedingAbandon& command,
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
    const AcCmdCRBreedingAbandon& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRBreedingAbandon& command,
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

struct AcCmdRCUpdateMountInfoNotify
{
  enum class Action
  {
    Default = 0,
    UpdateConditionAndName = 4,
    SetMountStateAndBreedData = 5,
    SomeItemManip0 = 9,
    SomeItemManip1 = 10,
    SomeItemManip2 = 12,
    SomeItemManip3 = 13,
  };

  Action action{Action::UpdateConditionAndName};
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
    const AcCmdRCUpdateMountInfoNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCUpdateMountInfoNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRequestStorage
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
    const AcCmdCRRequestStorage& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestStorage& command,
    SourceStream& stream);
};

struct AcCmdCRRequestStorageOK
{
  AcCmdCRRequestStorage::Category category{};
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
    const AcCmdCRRequestStorageOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestStorageOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestStorageCancel
{
  AcCmdCRRequestStorage category{};
  uint8_t val1{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestStorageCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestStorageCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestStorageCancel& command,
    SourceStream& stream);
};

struct AcCmdCRGetItemFromStorage
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
    const AcCmdCRGetItemFromStorage& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRGetItemFromStorage& command,
    SourceStream& stream);
};

struct AcCmdCRGetItemFromStorageOK
{
  uint32_t storageItemUid{};
  std::vector<Item> items{};
  //! Updates carrots as displayed on game client
  uint32_t updatedCarrots{};

  static Command GetCommand()
  {
    return Command::AcCmdCRGetItemFromStorageOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRGetItemFromStorageOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRGetItemFromStorageOK& command,
    SourceStream& stream);
};

struct AcCmdCRGetItemFromStorageCancel
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
    const AcCmdCRGetItemFromStorageCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRGetItemFromStorageCancel& command,
    SourceStream& stream);
};

struct RanchCommandCheckStorageItem
{
  uint32_t storedItemUid{};

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRGetItemFromStorage& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRGetItemFromStorage& command,
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

struct AcCmdCRWearEquipment
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
    const AcCmdCRWearEquipment& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRWearEquipment& command,
    SourceStream& stream);
};

struct AcCmdCRWearEquipmentOK
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
    const AcCmdCRWearEquipmentOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRWearEquipmentOK& command,
    SourceStream& stream);
};

struct AcCmdCRWearEquipmentCancel
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
    const AcCmdCRWearEquipmentCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRWearEquipmentCancel& command,
    SourceStream& stream);
};

struct AcCmdCRRemoveEquipment
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
    const AcCmdCRRemoveEquipment& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRemoveEquipment& command,
    SourceStream& stream);
};

struct AcCmdCRRemoveEquipmentOK
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
    const AcCmdCRRemoveEquipmentOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRemoveEquipmentOK& command,
    SourceStream& stream);
};

struct AcCmdCRRemoveEquipmentCancel
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
    const AcCmdCRRemoveEquipmentCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRemoveEquipmentCancel& command,
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
  uint32_t updatedCarrots{};

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
  //! See CDATA[ERROR_FAIL_SYSTEMERROR]
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
    // 0  = Does not request member list
    // 0  > Requests member list
    // 29 > Disables guild invite
    uint8_t memberCount{};
    uint32_t member5{};
    std::string name{};
    std::string description{};
    // Cooldown for invite in hours (TOOLTIP_INVITE_COOLTIME).
    // Essentially behaves like a boolean. 1 = disables the guild invite button
    uint32_t inviteCooldown{};
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

struct AcCmdCRWithdrawGuildMember
{
  uint32_t characterUid{};
  enum class Option : uint8_t
  {
    Leave = 0,
    Kicked = 1,
    Unk2 = 2
  } option{};

  static Command GetCommand()
  {
    return Command::AcCmdCRWithdrawGuildMember;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRWithdrawGuildMember& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRWithdrawGuildMember& command,
    SourceStream& stream);
};

struct AcCmdCRWithdrawGuildMemberOK
{
  AcCmdCRWithdrawGuildMember::Option option;
  
  static Command GetCommand()
  {
    return Command::AcCmdCRWithdrawGuildMemberOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRWithdrawGuildMemberOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRWithdrawGuildMemberOK& command,
    SourceStream& stream);
};

struct AcCmdCRWithdrawGuildMemberCancel
{
  //! See GuildStrings table in libconfig
  //! 0 from CDATA[ERROR_FAIL_SYSTEMERROR]
  uint8_t status{};

  static Command GetCommand()
  {
    return Command::AcCmdCRWithdrawGuildMemberCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRWithdrawGuildMemberCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRWithdrawGuildMemberCancel& command,
    SourceStream& stream);
};

struct AcCmdRCWithdrawGuildMemberNotify
{
  uint32_t guildUid;
  uint32_t guildMemberCharacterUid;
  uint32_t withdrawnCharacterUid;
  AcCmdCRWithdrawGuildMember::Option option; // 1 - expelled

  static Command GetCommand()
  {
    return Command::AcCmdRCWithdrawGuildMemberNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCWithdrawGuildMemberNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCWithdrawGuildMemberNotify& command,
    SourceStream& stream);
};

struct AcCmdCRUpdatePet
{
  PetInfo petInfo{};
  uint32_t itemUid; // 

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdatePet;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUpdatePet& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUpdatePet& command,
    SourceStream& stream);
};

struct AcCmdRCUpdatePet
{
  PetInfo petInfo{};
  //! optional
  uint32_t itemUid{};
  static Command GetCommand()
  {
    return Command::AcCmdRCUpdatePet;
  }
  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCUpdatePet& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCUpdatePet& command,
    SourceStream& stream);
};

struct AcCmdRCUpdatePetCancel
{
  PetInfo petInfo{};
  uint32_t member2{};
  uint8_t member3{};

  static Command GetCommand()
  {
    return Command::AcCmdRCUpdatePetCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCUpdatePetCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCUpdatePetCancel& command,
    SourceStream& stream);
};

struct AcCmdCRRequestPetBirth
{
  uint32_t eggLevel{};
  uint32_t incubatorSlot{};
  PetInfo petInfo{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestPetBirth;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestPetBirth& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestPetBirth& command,
    SourceStream& stream);
};

struct AcCmdCRRequestPetBirthOK
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
    const AcCmdCRRequestPetBirthOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestPetBirthOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestPetBirthNotify
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
    const AcCmdCRRequestPetBirthNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestPetBirthNotify& command,
    SourceStream& stream);
};

struct AcCmdCRRequestPetBirthCancel
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
    const AcCmdCRRequestPetBirthCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestPetBirthCancel& command,
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

struct AcCmdCRIncubateEgg
{
  uint32_t itemUid{};
  uint32_t itemTid{};
  uint32_t incubatorSlot{};

  static Command GetCommand()
  {
    return Command::AcCmdCRIncubateEgg;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRIncubateEgg& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRIncubateEgg& command,
    SourceStream& stream);
};

struct AcCmdCRIncubateEggOK
{
  uint32_t incubatorSlot{};
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
    const AcCmdCRIncubateEggOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRIncubateEggOK& command,
    SourceStream& stream);
};

struct AcCmdCRIncubateEggNotify
{
  uint32_t characterUid{}; // needs confirmation
  uint32_t incubatorSlot{};
  Egg egg{};
  // optional
  uint32_t member3{};

  static Command GetCommand()
  {
    return Command::AcCmdCRIncubateEggNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRIncubateEggNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRIncubateEggNotify& command,
    SourceStream& stream);
};

struct AcCmdCRIncubateEggCancel
{
  uint8_t cancel{};
  uint32_t itemUid{};
  uint32_t itemTid{};
  uint32_t incubatorSlot{};

  static Command GetCommand()
  {
    return Command::AcCmdCRIncubateEggCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRIncubateEggCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRIncubateEggCancel& command,
    SourceStream& stream);
};

struct AcCmdCRBoostIncubateInfoList
{
  uint32_t member1{};
  uint32_t member2{};

  static Command GetCommand()
  {
    return Command::AcCmdCRBoostIncubateInfoList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRBoostIncubateInfoList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRBoostIncubateInfoList& command,
    SourceStream& stream);
};

struct AcCmdCRBoostIncubateInfoListOK
{
  uint32_t member1{};
  uint16_t count{};
  //here belongs some vector 2 uint32_t

  static Command GetCommand()
  {
    return Command::AcCmdCRBoostIncubateInfoListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRBoostIncubateInfoListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRBoostIncubateInfoListOK& command,
    SourceStream& stream);
};

struct AcCmdCRBoostIncubateEgg
{
  uint32_t itemUid{}; //crystal item id
  uint32_t incubatorSlot{};

  static Command GetCommand()
  {
    return Command::AcCmdCRBoostIncubateEgg;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRBoostIncubateEgg& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRBoostIncubateEgg& command,
    SourceStream& stream);
};

struct AcCmdCRBoostIncubateEggOK
{
  Item item{};
  uint32_t incubatorSlot{};
  Egg egg{};         

  static Command GetCommand()
  {
    return Command::AcCmdCRBoostIncubateEggOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRBoostIncubateEggOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRBoostIncubateEggOK& command,
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

struct AcCmdCRHousingBuild
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
    const AcCmdCRHousingBuild& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingBuild& command,
    SourceStream& stream);
};

struct AcCmdCRHousingBuildOK
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
    const AcCmdCRHousingBuildOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingBuildOK& command,
    SourceStream& stream);
};

struct AcCmdCRHousingBuildCancel
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
    const AcCmdCRHousingBuildCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingBuildCancel& command,
    SourceStream& stream);
};

struct AcCmdCRHousingBuildNotify
{
  uint32_t member1{};
  uint16_t housingId{};

  static Command GetCommand()
  {
    return Command::AcCmdCRHousingBuildNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRHousingBuildNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingBuildNotify& command,
    SourceStream& stream);
};

struct AcCmdCRHousingRepair
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
    const AcCmdCRHousingRepair& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingRepair& command,
    SourceStream& stream);
};

struct AcCmdCRHousingRepairOK
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
    const AcCmdCRHousingRepairOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingRepairOK& command,
    SourceStream& stream);
};

struct AcCmdCRHousingRepairCancel
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
    const AcCmdCRHousingRepairCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingRepairCancel& command,
    SourceStream& stream);
};

struct AcCmdCRHousingRepairNotify
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
    const AcCmdCRHousingRepairNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHousingRepairNotify& command,
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
    const AcCmdRCMissionEvent& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCMissionEvent& command,
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

struct AcCmdCRUseItem
{
  uint32_t itemUid{};
  uint16_t always1{};
  uint32_t horseUid{};

  enum class PlaySuccessLevel : uint32_t
  {
    Bad = 0,
    Good = 1,
    Perfect = 2
  };
  PlaySuccessLevel playSuccessLevel{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUseItem;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUseItem& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUseItem& command,
    SourceStream& stream);
};

struct AcCmdCRUseItemOK
{
  enum class ActionType : uint32_t
  {
    Generic,
    Feed,
    Wash,
    Play,
    Cure
  };

  enum class PlaySuccessLevel : uint32_t
  {
    Bad = 0,
    Good = 1,
    CriticalGood = 2,
    Perfect = 3,
    CriticalPerfect = 4
  };

  //! The UID of the item used.
  uint32_t itemUid{};
  //! Updates the client-side count of the item used for care.
  //! Setting it to 0 removes the item completely.
  uint16_t updatedItemCount{};
  //! Action type.
  ActionType type{};
  //! An optional reward of experience points,
  //! only applied to `ActionType::Feed`, `ActionType::Wash`, `ActionType::Play` and `ActionType::Cure` actions.
  uint8_t experiencePoints{};
  //! An optional play success level,
  //! only applied to `ActionType::Feed`, `ActionType::Wash` and `ActionType::Play` actions.
  PlaySuccessLevel playSuccessLevel{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUseItemOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUseItemOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUseItemOK& command,
    SourceStream& stream);
};

//!
struct AcCmdCRUseItemCancel
{
  uint32_t itemUid{};
  uint8_t rewardExperience{};

  static Command GetCommand()
  {
    return Command::AcCmdCRUseItemCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUseItemCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUseItemCancel& command,
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
  struct MountFamilyTreeItem
  {
    uint8_t id{};
    std::string name{};
    uint8_t grade{};
    uint16_t skinId{};
  };

  // In the packet, the length is specified as a byte
  // max size 6
  std::vector<MountFamilyTreeItem> ancestors;

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

struct AcCmdCRCheckStorageItem
{
  uint32_t storedItemUid{};

  static Command GetCommand()
  {
    return Command::AcCmdCRCheckStorageItem;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRCheckStorageItem& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRCheckStorageItem& command,
    SourceStream& stream);
};

struct AcCmdCRChangeAgeCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRChangeAgeCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeAgeCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeAgeCancel& command,
    SourceStream& stream);
};

struct AcCmdCRChangeAge
{
  enum class Age : uint8_t {
    ElementarySchoolStudent = 12,
    MiddleSchoolStudent = 13,
    HighSchoolStudent = 16,
    Adult = 19
  } age;

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeAge;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeAge& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeAge& command,
    SourceStream& stream);
};

struct AcCmdCRChangeAgeOK
{
  protocol::AcCmdCRChangeAge::Age age; // age? status?

  static Command GetCommand()
  {
    return Command::AcCmdCRChangeAgeOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRChangeAgeOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRChangeAgeOK& command,
    SourceStream& stream);
};

struct AcCmdRCChangeAgeNotify
{
  uint32_t characterUid;
  protocol::AcCmdCRChangeAge::Age age;

  static Command GetCommand()
  {
    return Command::AcCmdRCChangeAgeNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCChangeAgeNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCChangeAgeNotify& command,
    SourceStream& stream);
};

struct AcCmdCRHideAge
{
  enum class Option : uint8_t {
    Shown = 0,
    Hidden = 1
  } option;

  static Command GetCommand()
  {
    return Command::AcCmdCRHideAge;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRHideAge& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHideAge& command,
    SourceStream& stream);
};

struct AcCmdCRHideAgeCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRHideAgeCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRHideAgeCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHideAgeCancel& command,
    SourceStream& stream);
};

struct AcCmdCRHideAgeOK
{
  protocol::AcCmdCRHideAge::Option option;

  static Command GetCommand()
  {
    return Command::AcCmdCRHideAgeOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRHideAgeOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRHideAgeOK& command,
    SourceStream& stream);
};

struct AcCmdRCHideAgeNotify
{
  uint32_t characterUid;
  protocol::AcCmdCRHideAge::Option option;

  static Command GetCommand()
  {
    return Command::AcCmdRCHideAgeNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCHideAgeNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCHideAgeNotify& command,
    SourceStream& stream);
};

struct AcCmdCRGuildMemberList
{
  static Command GetCommand()
  {
    return Command::AcCmdCRGuildMemberList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRGuildMemberList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRGuildMemberList& command,
    SourceStream& stream);
};

struct AcCmdCRGuildMemberListCancel
{
  uint8_t status; // status?

  static Command GetCommand()
  {
    return Command::AcCmdCRGuildMemberListCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRGuildMemberListCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRGuildMemberListCancel& command,
    SourceStream& stream);
};

struct AcCmdCRGuildMemberListOK
{
  struct MemberInfo {
    uint32_t memberUid;
    std::string nickname;
    uint32_t unk0;
    GuildRole guildRole;
    uint8_t unk2;
  };

  std::vector<MemberInfo> members{};
  
  static Command GetCommand()
  {
    return Command::AcCmdCRGuildMemberListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRGuildMemberListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRGuildMemberListOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildMatchInfo
{
  uint32_t guildUid;
  
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildMatchInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildMatchInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildMatchInfo& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildMatchInfoCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildMatchInfoCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildMatchInfoCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildMatchInfoCancel& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildMatchInfoOK
{
  // Guild Uid?
  uint32_t guildUid;
  // Guild name?
  std::string name;
  uint32_t unk2;
  uint32_t unk3;
  uint32_t unk4;
  uint32_t unk5;
  uint32_t totalWins;
  uint32_t totalLosses;
  uint32_t unk8;
  uint32_t rank;
  uint32_t unk10;
  uint32_t seasonalWins;
  uint32_t seasonalLosses;

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildMatchInfoOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildMatchInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildMatchInfoOK& command,
    SourceStream& stream);
};

struct AcCmdCRUpdateGuildMemberGrade
{
  uint32_t characterUid;
  GuildRole guildRole;

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateGuildMemberGrade;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUpdateGuildMemberGrade& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUpdateGuildMemberGrade& command,
    SourceStream& stream);
};

struct AcCmdCRUpdateGuildMemberGradeCancel
{
  uint8_t unk0;

  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateGuildMemberGradeCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUpdateGuildMemberGradeCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUpdateGuildMemberGradeCancel& command,
    SourceStream& stream);
};

struct AcCmdCRUpdateGuildMemberGradeOK
{
  static Command GetCommand()
  {
    return Command::AcCmdCRUpdateGuildMemberGradeOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRUpdateGuildMemberGradeOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRUpdateGuildMemberGradeOK& command,
    SourceStream& stream);
};

struct AcCmdRCUpdateGuildMemberGradeNotify
{
  uint32_t guildUid;
  uint32_t unk1;
  uint32_t targetCharacterUid;
  GuildRole unk3;
  GuildRole guildRole;

  static Command GetCommand()
  {
    return Command::AcCmdRCUpdateGuildMemberGradeNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCUpdateGuildMemberGradeNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCUpdateGuildMemberGradeNotify& command,
    SourceStream& stream);
};

struct AcCmdCRInviteGuildJoin
{
  std::string characterName;

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteGuildJoin;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteGuildJoin& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteGuildJoin& command,
    SourceStream& stream);
};

struct AcCmdCRInviteGuildJoinCancel
{
  uint32_t unk0;
  uint32_t unk1;
  std::string unk2;
  GuildError error;
  uint32_t unk4;

  static Command GetCommand()
  {
    return Command::AcCmdCRInviteGuildJoinCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRInviteGuildJoinCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRInviteGuildJoinCancel& command,
    SourceStream& stream);
};

struct AcCmdRCAcceptGuildJoinNotify
{
  uint32_t guildMemberCharacterUid;
  uint32_t newMemberCharacterUid;
  std::string newMemberCharacterName;

  static Command GetCommand()
  {
    return Command::AcCmdRCAcceptGuildJoinNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdRCAcceptGuildJoinNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdRCAcceptGuildJoinNotify& command,
    SourceStream& stream);
};

struct AcCmdCREmblemList
{
  static Command GetCommand()
  {
    return Command::AcCmdCREmblemList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCREmblemList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCREmblemList& command,
    SourceStream& stream);
};

struct AcCmdCREmblemListCancel
{
  static Command GetCommand()
  {
    return Command::AcCmdCREmblemListCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCREmblemListCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCREmblemListCancel& command,
    SourceStream& stream);
};

struct AcCmdCREmblemListOK
{
  std::vector<uint16_t> unk0{};

  static Command GetCommand()
  {
    return Command::AcCmdCREmblemListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCREmblemListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCREmblemListOK& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // RANCH_MESSAGE_DEFINES_HPP
