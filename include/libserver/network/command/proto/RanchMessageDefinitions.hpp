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

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace alicia
{

//!
struct RanchCommandUseItem
{
  uint32_t unk0{};
  uint16_t unk1{};
  uint32_t unk2{};
  uint32_t unk3{};

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
  uint32_t unk0{};
  uint16_t unk1{};

  // Action points to different structures depending on type
  uint32_t type{};
  // todo: std::variant instead of a pointer, or just template the struct
  void* action = nullptr;

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

struct UseItemType0Action
{
  // Empty
};

struct UseItemType1And2And3Action
{
  uint8_t unk0{};
  uint8_t unk1{};
};

struct UseItemType4Action
{
  uint8_t unk0{};
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

struct RanchCommandEnterRanch
{
  uint32_t characterUid{};
  uint32_t otp{};
  uint32_t ranchUid{};

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const RanchCommandEnterRanch& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    RanchCommandEnterRanch& command,
    SourceStream& stream);
};

//! Clientbound get messenger info response.
struct RanchCommandEnterRanchOK
{
  uint32_t ranchId{};
  std::string unk0{};
  std::string ranchName{};

  // Both lists' lengths are specified as bytes in the packet
  // Indexes across both lists cant be shared.
  // If the horse list takes indexes 0, 1 and 2
  // the player list must use indexes 3, 4 and 5.
  std::vector<RanchHorse> horses{};
  std::vector<RanchCharacter> characters{};

  uint64_t unk1{};
  uint32_t unk2{};
  uint32_t unk3{};

  struct Unk4
  {
    uint32_t unk0{};
    uint16_t unk1{};
    uint32_t unk2{};
  };

  // List size as a byte. Max length 13
  std::vector<Unk4> unk4{};

  uint8_t unk5{};
  uint32_t unk6{};
  uint32_t unk7{}; // bitset

  uint32_t unk8{};
  uint32_t unk9{};

  struct Unk10
  {
    uint32_t horseTID{};
    uint32_t unk0{};
    uint32_t unk1{};
    uint8_t unk2{};
    uint32_t unk3{};
    uint32_t unk4{};
    uint32_t unk5{};
    uint32_t unk6{};
    uint32_t unk7{};
  };

  std::array<Unk10, 3> unk10;

  RanchUnk11 unk11{};

  uint32_t unk12{};

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

//! Serverbound get messenger info command.
struct RanchCommandEnterRanchCancel
{
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

//! Serverbound get messenger info command.
struct RanchCommandEnterRanchNotify
{
  RanchCharacter character{};

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

//! Serverbound get messenger info command.
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

//! Clientbound get messenger info response.
struct RanchCommandRanchSnapshotNotify
{
  uint16_t ranchIndex{};

  RanchCommandRanchSnapshot::Type type{};
  RanchCommandRanchSnapshot::FullSpatial full{};
  RanchCommandRanchSnapshot::PartialSpatial partial{};

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

//! Serverbound get messenger info command.
struct RanchCommandRanchCmdAction
{
  uint16_t unk0{};
  std::vector<uint8_t> snapshot{};

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

//! Serverbound get messenger info command.
struct RanchCommandUpdateBusyState
{
  uint8_t busyState{};

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
    uint32_t unk1{}; // owner? horse id?
    uint32_t unk2{}; // likely either of these in either order
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
    uint8_t unk0{};
    uint32_t unk1{};
    uint8_t unk2{};
    uint8_t unk3{};
  };
  std::vector<AvailableHorse> availableHorses{};

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
  uint32_t unk0{};
  std::string nickname{};
  uint32_t unk1{};

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
  uint32_t unk0{};
  std::string nickname{};
  uint32_t unk1{};
  uint32_t unk2{};

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

struct RanchCommandRequestStorage
{
  uint8_t val0{};
  uint16_t val1{};

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
  uint8_t val0{};
  uint16_t val1{};
  uint16_t val2{};

  struct Unk
  {
    uint32_t uid{};
    uint32_t val1{};
    uint8_t val2{};
    uint32_t val3{};
    uint32_t val4{};
    uint32_t val5{};
    uint32_t val6{};
    std::string sender;
    std::string message;
    //! [0000'00][00'0000]'[0000'0000]'[0000]'[0000'0000'0000]
    //! [minute] [hour] [day] [month] [year]
    uint32_t dateAndTime{};
  };
  //! Max 33 elements.
  std::vector<Unk> val3{};

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
  uint8_t val0{};
  uint8_t val1{};

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

struct RanchCommandRequestNpcDressList
{
  uint32_t unk0{}; // NPC ID?

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

// TODO Quest commands: RanchCommandUpdateDailyQuest, RanchCommandEmblemList, etc.

} // namespace alicia

#endif // RANCH_MESSAGE_DEFINES_HPP
