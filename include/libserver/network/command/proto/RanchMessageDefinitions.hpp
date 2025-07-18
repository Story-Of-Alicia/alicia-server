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

//!
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
    return Command::RanchUseItem;
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
    return Command::RanchUseItemOK;
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
    return Command::RanchUseItemCancel;
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

struct MountFamilyTreeItem
{
  uint8_t unk0{};
  std::string unk1{};
  uint8_t unk2{};
  uint16_t unk3{};
};

//!
struct RanchCommandMountFamilyTree
{
  uint32_t unk0;

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

  // In the packet, the length is specified as a byte
  // max size 6
  std::vector<MountFamilyTreeItem> items;

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

struct RanchCommandRanchEnter
{
  uint32_t characterUid{};
  uint32_t otp{};
  uint32_t rancherUid{};

  static Command GetCommand()
  {
    return Command::RanchEnterRanch;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRanchEnter& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRanchEnter& command,
    SourceStream& stream);
};

struct RanchCommandEnterRanchOK
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
  uint32_t ranchProgress{100000};

  // List size as a byte. Max length 13
  std::vector<Housing> housing{};

  uint8_t horseSlots{5};
  uint32_t member11{};

  enum class Bitset : uint32_t
  {
    IsLocked = 1,
  } bitset{0};

  uint32_t incubatorSlotOne{2};
  uint32_t incubatorSlotTwo{1};

  std::array<Egg, 3> incubator;

  League league{};
  uint32_t member17{};

  static Command GetCommand()
  {
    return Command::RanchEnterRanchOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandEnterRanchOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandEnterRanchOK& command,
    SourceStream& stream);
};

struct RanchCommandEnterRanchCancel
{
  static Command GetCommand()
  {
    return Command::RanchEnterRanchCancel;
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
    return Command::RanchEnterRanchNotify;
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

struct RanchCommandRanchSnapshot
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
    return Command::RanchSnapshot;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRanchSnapshot& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRanchSnapshot& command,
    SourceStream& stream);
};

struct RanchCommandRanchSnapshotNotify
{
  uint16_t ranchIndex{};

  RanchCommandRanchSnapshot::Type type{};
  RanchCommandRanchSnapshot::FullSpatial full{};
  RanchCommandRanchSnapshot::PartialSpatial partial{};

  static Command GetCommand()
  {
    return Command::RanchSnapshotNotify;
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

struct RanchCommandRanchCmdAction
{
  uint16_t unk0{};
  std::vector<uint8_t> snapshot{};

  static Command GetCommand()
  {
    return Command::RanchCmdAction;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandRanchCmdAction& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandRanchCmdAction& command,
    SourceStream& stream);
};

//! Clientbound get messenger info response.
struct RanchCommandRanchCmdActionNotify
{
  uint16_t unk0{};
  uint16_t unk1{};
  uint8_t unk2{};

  static Command GetCommand()
  {
    return Command::RanchCmdActionNotify;
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

struct RanchCommandUpdateBusyState
{
  uint8_t busyState{};

  static Command GetCommand()
  {
    return Command::RanchUpdateBusyState;
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

//! Clientbound get messenger info response.
struct RanchCommandUpdateBusyStateNotify
{
  uint32_t characterId{};
  uint8_t busyState{};

  static Command GetCommand()
  {
    return Command::RanchUpdateBusyStateNotify;
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
struct RanchCommandLeaveRanch
{
  static Command GetCommand()
  {
    return Command::RanchLeaveRanch;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandLeaveRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandLeaveRanch& command,
    SourceStream& stream);
};

//! Clientbound get messenger info response.
struct RanchCommandLeaveRanchOK
{
  static Command GetCommand()
  {
    return Command::RanchLeaveRanchOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandLeaveRanchOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandLeaveRanchOK& command,
    SourceStream& stream);
};

//! Serverbound get messenger info command.
struct RanchCommandLeaveRanchNotify
{
  uint32_t characterId{}; // Probably

  static Command GetCommand()
  {
    return Command::RanchLeaveRanchNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandLeaveRanchNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandLeaveRanchNotify& command,
    SourceStream& stream);
};

//! Serverbound heartbeat command.
struct RanchCommandHeartbeat
{
  static Command GetCommand()
  {
    return Command::RanchHeartbeat;
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

//! Serverbound RanchStuff command.
struct RanchCommandRanchStuff
{
  uint32_t eventId{};
  int32_t value{};

  static Command GetCommand()
  {
    return Command::RanchStuff;
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

// Clientbound RanchStuffOK command.
struct RanchCommandRanchStuffOK
{
  uint32_t eventId{};
  int32_t moneyIncrement{};
  int32_t totalMoney{};

  static Command GetCommand()
  {
    return Command::RanchStuffOK;
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
    return Command::RanchSearchStallion;
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

struct RanchCommandSearchStallionCancel
{
  static Command GetCommand()
  {
    return Command::RanchSearchStallionCancel;
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
    return Command::RanchSearchStallionOK;
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

//! Serverbound get messenger info command.
struct RanchCommandEnterBreedingMarket
{
  static Command GetCommand()
  {
    return Command::RanchEnterBreedingMarket;
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

//! Clientbound get messenger info response.
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
    return Command::RanchEnterBreedingMarketOK;
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

//! Serverbound get messenger info command.
struct RanchCommandEnterBreedingMarketCancel
{
  static Command GetCommand()
  {
    return Command::RanchEnterBreedingMarketCancel;
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

//! Serverbound get messenger info command.
struct RanchCommandTryBreeding
{
  uint32_t unk0{};
  uint32_t unk1{};

  static Command GetCommand()
  {
    return Command::RanchTryBreeding;
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

//! Clientbound get messenger info response.
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
    return Command::RanchTryBreedingOK;
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

//! Serverbound get messenger info command.
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
    return Command::RanchTryBreedingCancel;
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

//! Serverbound get messenger info command.
struct RanchCommandBreedingWishlist
{
  static Command GetCommand()
  {
    return Command::RanchBreedingWishlist;
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

//! Clientbound get messenger info response.
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
    return Command::RanchBreedingWishlistOK;
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

//! Serverbound get messenger info command.
struct RanchCommandBreedingWishlistCancel
{
  static Command GetCommand()
  {
    return Command::RanchBreedingWishlistCancel;
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

//! Serverbound get messenger info command.
struct RanchCommandUpdateMountNickname
{
  uint32_t horseUid{};
  std::string name{};
  uint32_t unk1{};

  static Command GetCommand()
  {
    return Command::RanchUpdateMountNickname;
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
    return Command::RanchUpdateMountNicknameOK;
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
    return Command::RanchUpdateMountNicknameCancel;
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
    return Command::RanchUpdateMountInfoNotify;
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
    return Command::RanchRequestStorage;
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
    return Command::RanchRequestStorageOK;
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
    return Command::RanchRequestStorageOK;
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
    return Command::RanchGetItemFromStorage;
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
    return Command::RanchGetItemFromStorageOK;
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
    return Command::RanchGetItemFromStorageCancel;
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
    return Command::RanchRequestNpcDressList;
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
    return Command::RanchRequestNpcDressListOK;
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
    return Command::RanchRequestNpcDressListCancel;
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

struct RanchCommandChat
{
  std::string message;
  uint8_t unknown{};
  uint8_t unknown2{};

  static Command GetCommand()
  {
    return Command::RanchChat;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandChat& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandChat& command,
    SourceStream& stream);
};

struct RanchCommandChatNotify
{
  std::string author;
  std::string message;
  uint8_t isBlue{};
  uint8_t unknown2{};

  static Command GetCommand()
  {
    return Command::RanchChatNotify;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandChatNotify& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandChatNotify& command,
    SourceStream& stream);
};

struct RanchCommandWearEquipment
{
  uint32_t itemUid{};
  uint8_t member{};

  static Command GetCommand()
  {
    return Command::RanchWearEquipment;
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
    return Command::RanchWearEquipmentOK;
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
    return Command::RanchWearEquipmentCancel;
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
    return Command::RanchRemoveEquipment;
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
    return Command::RanchRemoveEquipmentOK;
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
    return Command::RanchRemoveEquipmentCancel;
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

struct RanchCommandUpdateEquipmentNotify
{
  uint32_t characterUid{};
  std::vector<Item> characterEquipment;
  std::vector<Item> mountEquipment;
  Horse mount{};

  static Command GetCommand()
  {
    return Command::RanchUpdateEquipmentNotify;
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

struct RanchCommandSetIntroductionNotify
{
  uint32_t characterUid{};
  std::string introduction{};

  static Command GetCommand()
  {
    return Command::RanchSetIntroductionNotify;
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
    return Command::RanchCreateGuild;
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
    return Command::RanchCreateGuildOK;
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
    return Command::RanchCreateGuildCancel;
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
    return Command::RanchRequestGuildInfo;
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
    return Command::RanchRequestGuildInfoOK;
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
    return Command::RanchRequestGuildInfoCancel;
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
  uint32_t member2{};

  static Command GetCommand()
  {
    return Command::RanchUpdatePet;
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
    return Command::RanchRequestPetBirth;
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
    return Command::RanchRequestPetBirthOK;
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
    return Command::RanchRequestPetBirthCancel;
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
    return Command::RanchPetBirthNotify;
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
    return Command::RanchIncubateEgg;
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
    return Command::RanchIncubateEggOK;
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

struct RanchCommandAchievementUpdateProperty
{
  //! 75 - level up
  //! Table `Achievements`
  uint16_t achievementEvent{};
  uint16_t member2{};

  static Command GetCommand()
  {
    return Command::RanchAchievementUpdateProperty;
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

struct RanchCommandHousingBuild
{
  uint16_t housingTid{};

  static Command GetCommand()
  {
    return Command::RanchHousingBuild;
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
    return Command::RanchHousingBuildOK;
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
    return Command::RanchHousingBuildCancel;
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
    return Command::RanchHousingBuildNotify;
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
    return Command::RanchMissionEvent;
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
    return Command::RanchKickRanch;
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
    return Command::RanchKickRanchOK;
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
    return Command::RanchKickRanchCancel;
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
    return Command::RanchKickRanchNotify;
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
    return Command::RanchOpCmd;
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
    return Command::RanchOpCmdOK;
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
    return Command::RanchRequestLeagueTeamList;
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
    return Command::RanchRequestLeagueTeamListOK;
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

} // namespace server::protocol

#endif // RANCH_MESSAGE_DEFINES_HPP
