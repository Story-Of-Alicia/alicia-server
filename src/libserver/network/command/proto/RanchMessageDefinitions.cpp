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

#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"

#include <cassert>
#include <algorithm>
#include <format>

namespace server::protocol
{

void AcCmdCRUseItem::Write(
  const AcCmdCRUseItem& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRUseItem::Read(
  AcCmdCRUseItem& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid)
    .Read(command.always1)
    .Read(command.horseUid)
    .Read(command.playSuccessLevel);
}

void AcCmdCRUseItemOK::Write(
  const AcCmdCRUseItemOK& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.updatedItemCount)
    .Write(command.type);

  if (command.type == ActionType::Generic)
    return;

  stream.Write(command.experiencePoints);

  if (command.type == ActionType::Feed
    || command.type == ActionType::Wash
    || command.type == ActionType::Play)
  {
    stream.Write(command.playSuccessLevel);
  }
}

void AcCmdCRUseItemOK::Read(
  AcCmdCRUseItemOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRUseItemCancel::Write(
  const AcCmdCRUseItemCancel& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.rewardExperience);
}

void AcCmdCRUseItemCancel::Read(
  AcCmdCRUseItemCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandMountFamilyTree::Write(
  const RanchCommandMountFamilyTree& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandMountFamilyTree::Read(
  RanchCommandMountFamilyTree& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid);
}

void RanchCommandMountFamilyTreeOK::Write(
  const RanchCommandMountFamilyTreeOK& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.ancestors.size()));
  for (auto& item : command.ancestors)
  {
    stream.Write(item.id)
      .Write(item.name)
      .Write(item.grade)
      .Write(item.skinId);
  }
}

void RanchCommandMountFamilyTreeOK::Read(
  RanchCommandMountFamilyTreeOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandMountFamilyTreeCancel::Write(
  const RanchCommandMountFamilyTreeCancel& command,
  SinkStream& stream)
{
}

void RanchCommandMountFamilyTreeCancel::Read(
  RanchCommandMountFamilyTreeCancel& command,
  SourceStream& stream)
{
}

void AcCmdCREnterRanch::Write(
  const AcCmdCREnterRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCREnterRanch::Read(
  AcCmdCREnterRanch& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.otp)
    .Read(command.rancherUid);
}

void AcCmdCREnterRanchOK::Write(
  const AcCmdCREnterRanchOK& command,
  SinkStream& stream)
{
  assert(command.rancherUid != 0
    && command.rancherName.length() <= 16
    && command.ranchName.size() <= 60);

  stream.Write(command.rancherUid)
    .Write(command.rancherName)
    .Write(command.ranchName);

  // Write the ranch horses
  assert(command.horses.size() <= 10);
  const auto ranchHorseCount = std::min(command.horses.size(), size_t{10});

  stream.Write(static_cast<uint8_t>(ranchHorseCount));
  for (std::size_t idx = 0; idx < ranchHorseCount; ++idx)
  {
    stream.Write(command.horses[idx]);
  }

  // Write the ranch characters
  assert(command.characters.size() <= 20);
  const auto ranchCharacterCount = std::min(command.characters.size(), size_t{20});

  stream.Write(static_cast<uint8_t>(ranchCharacterCount));
  for (std::size_t idx = 0; idx < ranchCharacterCount; ++idx)
  {
    stream.Write(command.characters[idx]);
  }

  stream.Write(command.member6)
    .Write(command.scramblingConstant)
    .Write(command.ranchProgress);

  // Write the ranch housing
  assert(command.housing.size() <= 13);
  const auto housingCount = std::min(command.housing.size(), size_t{13});

  stream.Write(static_cast<uint8_t>(housingCount));
  for (std::size_t idx = 0; idx < housingCount; ++idx)
  {
    stream.Write(command.housing[idx]);
  }

  stream.Write(command.horseSlots)
    .Write(command.member11)
    .Write(command.bitset)
    .Write(command.incubatorSlots)
    .Write(command.incubatorUseCount);

  for (const auto& egg : command.incubator)
  {
    stream.Write(egg);
  }

  stream.Write(command.league)
    .Write(command.member17);
}

void AcCmdCREnterRanchOK::Read(
  AcCmdCREnterRanchOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandEnterRanchCancel::Write(
  const RanchCommandEnterRanchCancel& command,
  SinkStream& stream)
{
}

void RanchCommandEnterRanchCancel::Read(
  RanchCommandEnterRanchCancel& command,
  SourceStream& stream)
{
}

void RanchCommandEnterRanchNotify::Write(
  const RanchCommandEnterRanchNotify& command,
  SinkStream& stream)
{
  stream.Write(command.character);
}

void RanchCommandEnterRanchNotify::Read(
  RanchCommandEnterRanchNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRanchSnapshot::FullSpatial::Write(
  const FullSpatial& structure,
  SinkStream& stream)
{
  stream.Write(structure.ranchIndex)
    .Write(structure.time)
    .Write(structure.action)
    .Write(structure.timer);

  for (const auto& byte : structure.member4)
  {
    stream.Write(byte);
  }

  for (const auto& byte : structure.matrix)
  {
    stream.Write(byte);
  }

  stream.Write(structure.velocityX)
    .Write(structure.velocityY)
    .Write(structure.velocityZ);
}

void AcCmdCRRanchSnapshot::FullSpatial::Read(
  FullSpatial& structure,
  SourceStream& stream)
{
  stream.Read(structure.ranchIndex)
    .Read(structure.time)
    .Read(structure.action)
    .Read(structure.timer);

  for (auto& byte : structure.member4)
  {
    stream.Read(byte);
  }

  for (auto& byte : structure.matrix)
  {
    stream.Read(byte);
  }

  stream.Read(structure.velocityX)
    .Read(structure.velocityY)
    .Read(structure.velocityZ);
}

void AcCmdCRRanchSnapshot::PartialSpatial::Write(
  const PartialSpatial& structure,
  SinkStream& stream)
{
  stream.Write(structure.ranchIndex)
    .Write(structure.time)
    .Write(structure.action)
    .Write(structure.timer);

  for (const auto& byte : structure.member4)
  {
    stream.Write(byte);
  }

  for (const auto& byte : structure.matrix)
  {
    stream.Write(byte);
  }
}

void AcCmdCRRanchSnapshot::PartialSpatial::Read(
  PartialSpatial& structure,
  SourceStream& stream)
{
  stream.Read(structure.ranchIndex)
    .Read(structure.time)
    .Read(structure.action)
    .Read(structure.timer);

  for (auto& byte : structure.member4)
  {
    stream.Read(byte);
  }

  for (auto& byte : structure.matrix)
  {
    stream.Read(byte);
  }
}

void AcCmdCRRanchSnapshot::Write(
  const AcCmdCRRanchSnapshot& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRanchSnapshot::Read(
  AcCmdCRRanchSnapshot& command,
  SourceStream& stream)
{
  stream.Read(command.type);

  switch (command.type)
  {
    case Full:
      {
        stream.Read(command.full);
        break;
      }
    case Partial:
      {
        stream.Read(command.partial);
        break;
      }
    default:
      {
        throw std::runtime_error(
          std::format(
            "Update type {} not implemented",
            static_cast<uint32_t>(command.type)));
      }
  }
}

void RanchCommandRanchSnapshotNotify::Write(
  const RanchCommandRanchSnapshotNotify& command,
  SinkStream& stream)
{
  stream.Write(command.ranchIndex)
    .Write(command.type);

  switch (command.type)
  {
    case AcCmdCRRanchSnapshot::Full:
      {
        stream.Write(command.full);
        break;
      }
    case AcCmdCRRanchSnapshot::Partial:
      {
        stream.Write(command.partial);
        break;
      }
    default:
      {
        throw std::runtime_error(
          std::format("Update type {} not implemented", static_cast<uint32_t>(command.type)));
      }
  }
}

void RanchCommandRanchSnapshotNotify::Read(
  RanchCommandRanchSnapshotNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRanchCmdAction::Write(
  const AcCmdCRRanchCmdAction& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRanchCmdAction::Read(
  AcCmdCRRanchCmdAction& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);

  auto length = stream.Size() - stream.GetCursor();
  command.snapshot.resize(length);
  stream.Read(command.snapshot.data(), length);
}

void RanchCommandRanchCmdActionNotify::Write(
  const RanchCommandRanchCmdActionNotify& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2);
}

void RanchCommandRanchCmdActionNotify::Read(
  RanchCommandRanchCmdActionNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUpdateBusyState::Write(
  const RanchCommandUpdateBusyState& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUpdateBusyState::Read(
  RanchCommandUpdateBusyState& command,
  SourceStream& stream)
{
  stream.Read(command.busyState);
}

void RanchCommandUpdateBusyStateNotify::Write(
  const RanchCommandUpdateBusyStateNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.busyState);
}

void RanchCommandUpdateBusyStateNotify::Read(
  RanchCommandUpdateBusyStateNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRLeaveRanch::Write(
  const AcCmdCRLeaveRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRLeaveRanch::Read(
  AcCmdCRLeaveRanch& command,
  SourceStream& stream)
{
  // Empty.
}

void AcCmdCRLeaveRanchOK::Write(
  const AcCmdCRLeaveRanchOK& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCRLeaveRanchOK::Read(
  AcCmdCRLeaveRanchOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRLeaveRanchNotify::Write(
  const AcCmdCRLeaveRanchNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterId);
}

void AcCmdCRLeaveRanchNotify::Read(
  AcCmdCRLeaveRanchNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandHeartbeat::Write(
  const RanchCommandHeartbeat& command,
  SinkStream& stream)
{
}

void RanchCommandHeartbeat::Read(
  RanchCommandHeartbeat& command,
  SourceStream& stream) {}

void RanchCommandRanchStuff::Write(
  const RanchCommandRanchStuff& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandRanchStuff::Read(
  RanchCommandRanchStuff& command,
  SourceStream& stream)
{
  stream.Read(command.eventId)
    .Read(command.value);
}

void RanchCommandRanchStuffOK::Write(
  const RanchCommandRanchStuffOK& command,
  SinkStream& stream)
{
  stream.Write(command.eventId)
    .Write(command.moneyIncrement)
    .Write(command.totalMoney);
}

void RanchCommandRanchStuffOK::Read(
  RanchCommandRanchStuffOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRSearchStallion::Write(
  const AcCmdCRSearchStallion& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRSearchStallion::Read(
  AcCmdCRSearchStallion& command,
  SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2)
    .Read(command.unk3)
    .Read(command.unk4)
    .Read(command.unk5)
    .Read(command.unk6)
    .Read(command.unk7)
    .Read(command.unk8);

  for (size_t i = 0; i < 3; i++)
  {
    uint8_t listSize;
    stream.Read(listSize);
    for (size_t j = 0; j < listSize; j++)
    {
      uint32_t value;
      stream.Read(value);
      command.unk9[i].push_back(value);
    }
  }

  stream.Read(command.unk10);
}

void RanchCommandSearchStallionCancel::Write(
  const RanchCommandSearchStallionCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void RanchCommandSearchStallionCancel::Read(
  RanchCommandSearchStallionCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRegisterStallion::Write(
  const AcCmdCRRegisterStallion& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRegisterStallion::Read(
  AcCmdCRRegisterStallion& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid)
    .Read(command.carrots);
}

void AcCmdCRRegisterStallionOK::Write(
  const AcCmdCRRegisterStallionOK& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid);
}

void AcCmdCRRegisterStallionOK::Read(
  AcCmdCRRegisterStallionOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRegisterStallionCancel::Write(
  const RanchCommandRegisterStallionCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void RanchCommandRegisterStallionCancel::Read(
  RanchCommandRegisterStallionCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallion::Write(
  const AcCmdCRUnregisterStallion& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallion::Read(
  AcCmdCRUnregisterStallion& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid);
}

void AcCmdCRUnregisterStallionOK::Write(
  const AcCmdCRUnregisterStallionOK& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCRUnregisterStallionOK::Read(
  AcCmdCRUnregisterStallionOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandUnregisterStallionCancel::Write(
  const RanchCommandUnregisterStallionCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void RanchCommandUnregisterStallionCancel::Read(
  RanchCommandUnregisterStallionCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallionEstimateInfo::Write(
  const AcCmdCRUnregisterStallionEstimateInfo& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallionEstimateInfo::Read(
  AcCmdCRUnregisterStallionEstimateInfo& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid);
}

void AcCmdCRUnregisterStallionEstimateInfoOK::Write(
  const AcCmdCRUnregisterStallionEstimateInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.timesMated)
    .Write(command.matingCompensation)
    .Write(command.member4)
    .Write(command.matingPrice);
}

void AcCmdCRUnregisterStallionEstimateInfoOK::Read(
  AcCmdCRUnregisterStallionEstimateInfoOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallionEstimateInfoCancel::Write(
  const AcCmdCRUnregisterStallionEstimateInfoCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void AcCmdCRUnregisterStallionEstimateInfoCancel::Read(
  AcCmdCRUnregisterStallionEstimateInfoCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandSearchStallionOK::Write(
  const RanchCommandSearchStallionOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);

  assert(command.stallions.size() <= 10);
  const uint8_t count = std::min(
    static_cast<uint8_t>(command.stallions.size()), uint8_t{10});

  stream.Write(count);
  for (uint8_t idx = 0; idx < count; ++idx)
  {
    const auto& stallion = command.stallions[idx];
    stream.Write(stallion.member1)
      .Write(stallion.uid)
      .Write(stallion.tid)
      .Write(stallion.name)
      .Write(stallion.grade)
      .Write(stallion.chance)
      .Write(stallion.matePrice)
      .Write(stallion.unk7)
      .Write(stallion.expiresAt)
      .Write(stallion.stats)
      .Write(stallion.parts)
      .Write(stallion.appearance)
      .Write(stallion.unk11)
      .Write(stallion.lineage);
  }
}

void RanchCommandSearchStallionOK::Read(
  RanchCommandSearchStallionOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCREnterBreedingMarket::Write(
  const AcCmdCREnterBreedingMarket& command,
  SinkStream& stream)
{
}

void AcCmdCREnterBreedingMarket::Read(
  AcCmdCREnterBreedingMarket& command,
  SourceStream& stream)
{
}

void RanchCommandEnterBreedingMarketCancel::Write(
  const RanchCommandEnterBreedingMarketCancel& command,
  SinkStream& stream)
{
}

void RanchCommandEnterBreedingMarketCancel::Read(
  RanchCommandEnterBreedingMarketCancel& command,
  SourceStream& stream)
{
}

void RanchCommandEnterBreedingMarketOK::Write(
  const RanchCommandEnterBreedingMarketOK& command,
  SinkStream& stream)
{
  const uint8_t count = std::min(
    static_cast<uint8_t>(command.stallions.size()),
    uint8_t{10});

  stream.Write(count);
  for (uint8_t idx = 0; idx < count; ++idx)
  {
    const auto& stallion = command.stallions[idx];
    stream.Write(stallion.uid)
      .Write(stallion.tid)
      .Write(stallion.combo)
      .Write(stallion.unk1)
      .Write(stallion.unk2)
      .Write(stallion.lineage);
  }
}

void RanchCommandEnterBreedingMarketOK::Read(
  RanchCommandEnterBreedingMarketOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRTryBreeding::Write(
  const AcCmdCRTryBreeding& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRTryBreeding::Read(
  AcCmdCRTryBreeding& command,
  SourceStream& stream)
{
  stream.Read(command.mareUid)
    .Read(command.stallionUid);
}

void RanchCommandTryBreedingCancel::Write(
  const RanchCommandTryBreedingCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.unk4)
    .Write(command.unk5);
}

void RanchCommandTryBreedingCancel::Read(
  RanchCommandTryBreedingCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBreedingAbandon::Write(
  const AcCmdCRBreedingAbandon& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBreedingAbandon::Read(
  AcCmdCRBreedingAbandon& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid);
}

void RanchCommandBreedingAbandonOK::Write(
  const AcCmdCRBreedingAbandon& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid);
}

void RanchCommandBreedingAbandonOK::Read(
  AcCmdCRBreedingAbandon& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandBreedingAbandonCancel::Write(
  const AcCmdCRBreedingAbandon& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid);
}

void RanchCommandBreedingAbandonCancel::Read(
  AcCmdCRBreedingAbandon& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandTryBreedingOK::Write(
  const RanchCommandTryBreedingOK& command,
  SinkStream& stream)
{
  stream.Write(command.uid)
    .Write(command.tid)
    .Write(command.val)
    .Write(command.count)
    .Write(command.unk0)
    .Write(command.parts)
    .Write(command.appearance)
    .Write(command.stats)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.unk4)
    .Write(command.unk5)
    .Write(command.unk6)
    .Write(command.unk7)
    .Write(command.unk8)
    .Write(command.unk9)
    .Write(command.unk10);
}

void RanchCommandTryBreedingOK::Read(
  RanchCommandTryBreedingOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandBreedingWishlist::Write(
  const RanchCommandBreedingWishlist& command,
  SinkStream& stream)
{
}

void RanchCommandBreedingWishlist::Read(
  RanchCommandBreedingWishlist& command,
  SourceStream& stream)
{
}

void RanchCommandBreedingWishlistCancel::Write(
  const RanchCommandBreedingWishlistCancel& command,
  SinkStream& stream)
{
}

void RanchCommandBreedingWishlistCancel::Read(
  RanchCommandBreedingWishlistCancel& command,
  SourceStream& stream)
{
}

void RanchCommandBreedingWishlistOK::Write(
  const RanchCommandBreedingWishlistOK& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.wishlist.size()));
  for (auto& wishlistElement : command.wishlist)
  {
    stream.Write(wishlistElement.unk0)
      .Write(wishlistElement.uid)
      .Write(wishlistElement.tid)
      .Write(wishlistElement.unk1)
      .Write(wishlistElement.unk2)
      .Write(wishlistElement.unk3)
      .Write(wishlistElement.unk4)
      .Write(wishlistElement.unk5)
      .Write(wishlistElement.unk6)
      .Write(wishlistElement.unk7)
      .Write(wishlistElement.unk8)
      .Write(wishlistElement.stats)
      .Write(wishlistElement.parts)
      .Write(wishlistElement.appearance)
      .Write(wishlistElement.unk9)
      .Write(wishlistElement.unk10)
      .Write(wishlistElement.unk11);
  }
}

void RanchCommandBreedingWishlistOK::Read(
  RanchCommandBreedingWishlistOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUpdateMountNickname::Write(
  const RanchCommandUpdateMountNickname& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUpdateMountNickname::Read(
  RanchCommandUpdateMountNickname& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid)
    .Read(command.name)
    .Read(command.unk1);
}

void RanchCommandUpdateMountNicknameCancel::Write(
  const RanchCommandUpdateMountNicknameCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}

void RanchCommandUpdateMountNicknameCancel::Read(
  RanchCommandUpdateMountNicknameCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdRCUpdateMountInfoNotify::Write(
  const AcCmdRCUpdateMountInfoNotify& command,
  SinkStream& stream)
{
  stream.Write(command.action)
    .Write(command.member1)
    .Write(command.horse);
}

void AcCmdRCUpdateMountInfoNotify::Read(
  AcCmdRCUpdateMountInfoNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUpdateMountNicknameOK::Write(
  const RanchCommandUpdateMountNicknameOK& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid)
    .Write(command.nickname)
    .Write(command.unk1)
    .Write(command.unk2);
}

void RanchCommandUpdateMountNicknameOK::Read(
  RanchCommandUpdateMountNicknameOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRequestStorage::Write(
  const AcCmdCRRequestStorage& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRequestStorage::Read(AcCmdCRRequestStorage& command, SourceStream& stream)
{
  stream.Read(command.category).Read(command.page);
}

void AcCmdCRRequestStorageOK::Write(
  const AcCmdCRRequestStorageOK& command,
  SinkStream& stream)
{
  stream.Write(command.category)
    .Write(command.page)
    .Write(command.pageCountAndNotification);

  stream.Write(static_cast<uint8_t>(command.storedItems.size()));
  for (const auto& storedItem : command.storedItems)
  {
    stream.Write(storedItem);
  }
}

void AcCmdCRRequestStorageOK::Read(AcCmdCRRequestStorageOK& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRequestStorageCancel::Write(
  const AcCmdCRRequestStorageCancel& command,
  SinkStream& stream)
{
  stream.Write(command.category).Write(command.val1);
}

void AcCmdCRRequestStorageCancel::Read(
  AcCmdCRRequestStorageCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRGetItemFromStorage::Write(
  const AcCmdCRGetItemFromStorage& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRGetItemFromStorage::Read(
  AcCmdCRGetItemFromStorage& command,
  SourceStream& stream)
{
  stream.Read(command.storedItemUid);
}

void AcCmdCRGetItemFromStorageOK::Write(
  const AcCmdCRGetItemFromStorageOK& command,
  SinkStream& stream)
{
  stream.Write(command.storageItemUid);
  stream.Write(static_cast<uint8_t>(command.items.size()));
  for (const auto& item : command.items)
  {
    stream.Write(item);
  }
  stream.Write(command.updatedCarrots);
}

void AcCmdCRGetItemFromStorageOK::Read(
  AcCmdCRGetItemFromStorageOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRGetItemFromStorageCancel::Write(
  const AcCmdCRGetItemFromStorageCancel& command,
  SinkStream& stream)
{
  stream.Write(command.storedItemUid)
    .Write(command.status);
}

void AcCmdCRGetItemFromStorageCancel::Read(
  AcCmdCRGetItemFromStorageCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandCheckStorageItem::Write(
  const AcCmdCRGetItemFromStorage& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandCheckStorageItem::Read(
  AcCmdCRGetItemFromStorage& command,
  SourceStream& stream)
{
  stream.Read(command.storedItemUid);
}

void RanchCommandRequestNpcDressList::Write(
  const RanchCommandRequestNpcDressList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandRequestNpcDressList::Read(
  RanchCommandRequestNpcDressList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
}

void RanchCommandRequestNpcDressListOK::Write(
  const RanchCommandRequestNpcDressListOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
  stream.Write(static_cast<uint8_t>(command.dressList.size()));
  for (const auto& item : command.dressList)
  {
    stream.Write(item);
  }
}

void RanchCommandRequestNpcDressListOK::Read(
  RanchCommandRequestNpcDressListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandRequestNpcDressListCancel::Write(
  const RanchCommandRequestNpcDressListCancel& command,
  SinkStream& stream)
{
  // Empty
}

void RanchCommandRequestNpcDressListCancel::Read(
  RanchCommandRequestNpcDressListCancel& command,
  SourceStream& stream)
{
  // Empty
}

void AcCmdCRRanchChat::Write(
  const AcCmdCRRanchChat& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRanchChat::Read(
  AcCmdCRRanchChat& command,
  SourceStream& stream)
{
  stream.Read(command.message)
    .Read(command.unknown)
    .Read(command.unknown2);
}

void AcCmdCRRanchChatNotify::Write(
  const AcCmdCRRanchChatNotify& command,
  SinkStream& stream)
{
  stream.Write(command.author)
    .Write(command.message)
    .Write(command.isSystem)
    .Write(command.unknown2);
}

void AcCmdCRRanchChatNotify::Read(
  AcCmdCRRanchChatNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRWearEquipment::Write(
  const AcCmdCRWearEquipment& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRWearEquipment::Read(
  AcCmdCRWearEquipment& command,
  SourceStream& stream)
{
  stream.Read(command.equipmentUid)
    .Read(command.member);
}

void AcCmdCRWearEquipmentOK::Write(
  const AcCmdCRWearEquipmentOK& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.member);
}

void AcCmdCRWearEquipmentOK::Read(
  AcCmdCRWearEquipmentOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRWearEquipmentCancel::Write(
  const AcCmdCRWearEquipmentCancel& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.member);
}

void AcCmdCRWearEquipmentCancel::Read(
  AcCmdCRWearEquipmentCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRemoveEquipment::Write(
  const AcCmdCRRemoveEquipment& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRemoveEquipment::Read(
  AcCmdCRRemoveEquipment& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid);
}

void AcCmdCRRemoveEquipmentOK::Write(
  const AcCmdCRRemoveEquipmentOK& command,
  SinkStream& stream)
{
  stream.Write(command.uid);
}

void AcCmdCRRemoveEquipmentOK::Read(
  AcCmdCRRemoveEquipmentOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRemoveEquipmentCancel::Write(
  const AcCmdCRRemoveEquipmentCancel& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.member);
}

void AcCmdCRRemoveEquipmentCancel::Read(
  AcCmdCRRemoveEquipmentCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateEquipmentNotify::Write(
  const AcCmdCRUpdateEquipmentNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid);

  stream.Write(static_cast<uint8_t>(command.characterEquipment.size()));
  for (const auto& item : command.characterEquipment)
  {
    stream.Write(item);
  }

  stream.Write(static_cast<uint8_t>(command.mountEquipment.size()));
  for (const auto& item : command.mountEquipment)
  {
    stream.Write(item);
  }

  stream.Write(command.mount);
}

void AcCmdCRUpdateEquipmentNotify::Read(
  AcCmdCRUpdateEquipmentNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandSetIntroductionNotify::Write(
  const RanchCommandSetIntroductionNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.introduction);
}

void RanchCommandSetIntroductionNotify::Read(
  RanchCommandSetIntroductionNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandCreateGuild::Write(
  const RanchCommandCreateGuild& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandCreateGuild::Read(
  RanchCommandCreateGuild& command,
  SourceStream& stream)
{
  stream.Read(command.name)
    .Read(command.description);
}

void RanchCommandCreateGuildOK::Write(
  const RanchCommandCreateGuildOK& command,
  SinkStream& stream)
{
  stream.Write(command.uid)
    .Write(command.updatedCarrots);
}

void RanchCommandCreateGuildOK::Read(
  RanchCommandCreateGuildOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandCreateGuildCancel::Write(
  const RanchCommandCreateGuildCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status)
    .Write(command.member2);
}

void RanchCommandCreateGuildCancel::Read(
  RanchCommandCreateGuildCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestGuildInfo::Write(
  const RanchCommandRequestGuildInfo& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}
void RanchCommandRequestGuildInfo::Read(
  RanchCommandRequestGuildInfo& command,
  SourceStream& stream)
{
  // Empty.
}

void RanchCommandRequestGuildInfoOK::GuildInfo::Write(
  const GuildInfo& command,
  SinkStream& stream)
{
  stream.Write(command.uid)
    .Write(command.member1)
    .Write(command.member2)
    .Write(command.member3)
    .Write(command.member4)
    .Write(command.member5)
    .Write(command.name)
    .Write(command.description)
    .Write(command.member8)
    .Write(command.member9)
    .Write(command.member10)
    .Write(command.member11);
}

void RanchCommandRequestGuildInfoOK::GuildInfo::Read(
  GuildInfo& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestGuildInfoOK::Write(
  const RanchCommandRequestGuildInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.guildInfo);
}

void RanchCommandRequestGuildInfoOK::Read(
  RanchCommandRequestGuildInfoOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestGuildInfoCancel::Write(
  const RanchCommandRequestGuildInfoCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void RanchCommandRequestGuildInfoCancel::Read(
  RanchCommandRequestGuildInfoCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdatePet::Write(
  const AcCmdCRUpdatePet& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdatePet::Read(
  AcCmdCRUpdatePet& command,
  SourceStream& stream)
{
  stream.Read(command.petInfo);
  if (stream.GetCursor() - stream.Size() > 4)
    stream.Read(command.itemUid);
}

void AcCmdRCUpdatePet::Write(
  const AcCmdRCUpdatePet& command,
  SinkStream& stream)
{
  stream.Write(command.petInfo)
   .Write(command.itemUid);
}

void AcCmdRCUpdatePet::Read(
  AcCmdRCUpdatePet& command,
  SourceStream& stream)
{
  stream.Read(command.petInfo);
  if (stream.GetCursor() - stream.Size() > 4)
    stream.Read(command.itemUid);
}

void AcCmdRCUpdatePetCancel::Write(
  const AcCmdRCUpdatePetCancel& command,
  SinkStream& stream)
{
  stream.Write(command.petInfo)
    .Write(command.member2)
    .Write(command.member3);
}

void AcCmdRCUpdatePetCancel::Read(
  AcCmdRCUpdatePetCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBoostIncubateInfoList::Write(
  const AcCmdCRBoostIncubateInfoList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBoostIncubateInfoList::Read(
  AcCmdCRBoostIncubateInfoList& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.member2);
}

void AcCmdCRBoostIncubateInfoListOK::Write(
  const AcCmdCRBoostIncubateInfoListOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.count);
  // for loop to write vector of 2 uint32_t
  
}

void AcCmdCRBoostIncubateInfoListOK::Read(
  AcCmdCRBoostIncubateInfoListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBoostIncubateEgg::Write(
  const AcCmdCRBoostIncubateEgg& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBoostIncubateEgg::Read(
  AcCmdCRBoostIncubateEgg& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid)
    .Read(command.incubatorSlot);
}

void AcCmdCRBoostIncubateEggOK::Write(
  const AcCmdCRBoostIncubateEggOK& command,
  SinkStream& stream)
{
  stream.Write(command.item)
    .Write(command.incubatorSlot)
    .Write(command.egg);
}

void AcCmdCRBoostIncubateEggOK::Read(
  AcCmdCRBoostIncubateEggOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestPetBirth::Write(
  const AcCmdCRRequestPetBirth& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestPetBirth::Read(
  AcCmdCRRequestPetBirth& command,
  SourceStream& stream)
{
  stream.Read(command.eggLevel)
    .Read(command.incubatorSlot)
    .Read(command.petInfo);
}

void AcCmdCRRequestPetBirthOK::Write(
  const AcCmdCRRequestPetBirthOK& command,
  SinkStream& stream)
{
  stream.Write(command.petBirthInfo);
}

void AcCmdCRRequestPetBirthOK::Read(
  AcCmdCRRequestPetBirthOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestPetBirthNotify::Write(
  const AcCmdCRRequestPetBirthNotify& command,
  SinkStream& stream)
{
  stream.Write(command.petBirthInfo);
}

void AcCmdCRRequestPetBirthNotify::Read(
  AcCmdCRRequestPetBirthNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestPetBirthCancel::Write(
  const AcCmdCRRequestPetBirthCancel& command,
  SinkStream& stream)
{
  stream.Write(command.petInfo);
}

void AcCmdCRRequestPetBirthCancel::Read(
  AcCmdCRRequestPetBirthCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandPetBirthNotify::Write(
  const RanchCommandPetBirthNotify& command,
  SinkStream& stream)
{
  stream.Write(command.petBirthInfo);
}

void RanchCommandPetBirthNotify::Read(
  RanchCommandPetBirthNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRIncubateEgg::Write(
  const AcCmdCRIncubateEgg& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRIncubateEgg::Read(
  AcCmdCRIncubateEgg& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid)
    .Read(command.itemTid)
    .Read(command.incubatorSlot);
}

void AcCmdCRIncubateEggOK::Write(
  const AcCmdCRIncubateEggOK& command,
  SinkStream& stream)
{
  stream.Write(command.incubatorSlot)
    .Write(command.egg)
    .Write(command.member3);
}

void AcCmdCRIncubateEggOK::Read(
  AcCmdCRIncubateEggOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRIncubateEggNotify::Write(
  const AcCmdCRIncubateEggNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.incubatorSlot)
    .Write(command.egg)
    .Write(command.member3);
}

void AcCmdCRIncubateEggNotify::Read(
  AcCmdCRIncubateEggNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRIncubateEggCancel::Write(
  const AcCmdCRIncubateEggCancel& command,
  SinkStream& stream)
{
  stream.Write(command.cancel)
  .Write(command.itemUid)
  .Write(command.itemTid)
  .Write(command.incubatorSlot);
}

void AcCmdCRIncubateEggCancel::Read(
  AcCmdCRIncubateEggCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandUserPetInfos::Write(
  const RanchCommandUserPetInfos& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandUserPetInfos::Read(
  RanchCommandUserPetInfos& command,
  SourceStream& stream)
{
  // Empty.
}

void RanchCommandUserPetInfosOK::Write(
  const RanchCommandUserPetInfosOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.petCount)
    .Write(command.member3);
  for (const auto& pet : command.pets)
  {
    stream.Write(pet);
  }
}

void RanchCommandUserPetInfosOK::Read(
  RanchCommandUserPetInfosOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandAchievementUpdateProperty::Write(
  const RanchCommandAchievementUpdateProperty& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandAchievementUpdateProperty::Read(
  RanchCommandAchievementUpdateProperty& command,
  SourceStream& stream)
{
  stream.Read(command.achievementEvent)
    .Read(command.member2);
}

void AcCmdCRHousingBuild::Write(
  const AcCmdCRHousingBuild& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingBuild::Read(
  AcCmdCRHousingBuild& command,
  SourceStream& stream)
{
  stream.Read(command.housingTid);
}

void AcCmdCRHousingBuildOK::Write(
  const AcCmdCRHousingBuildOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.housingTid)
    .Write(command.member3);
}

void AcCmdCRHousingBuildOK::Read(
  AcCmdCRHousingBuildOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingBuildCancel::Write(
  const AcCmdCRHousingBuildCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void AcCmdCRHousingBuildCancel::Read(
  AcCmdCRHousingBuildCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingBuildNotify::Write(
  const AcCmdCRHousingBuildNotify& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.housingId);
}

void AcCmdCRHousingBuildNotify::Read(
  AcCmdCRHousingBuildNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingRepair::Write(
  const AcCmdCRHousingRepair& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingRepair::Read(
  AcCmdCRHousingRepair& command,
  SourceStream& stream)
{
  stream.Read(command.housingUid);
}

void AcCmdCRHousingRepairOK::Write(
  const AcCmdCRHousingRepairOK& command,
  SinkStream& stream)
{
  stream.Write(command.housingUid)
    .Write(command.member2);
}

void AcCmdCRHousingRepairOK::Read(
  AcCmdCRHousingRepairOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingRepairCancel::Write(
  const AcCmdCRHousingRepairCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void AcCmdCRHousingRepairCancel::Read(
  AcCmdCRHousingRepairCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingRepairNotify::Write(
  const AcCmdCRHousingRepairNotify& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.housingTid);
}

void AcCmdCRHousingRepairNotify::Read(
  AcCmdCRHousingRepairNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}
void AcCmdRCMissionEvent::Write(
  const AcCmdRCMissionEvent& command,
  SinkStream& stream)
{
  stream.Write(command.event)
    .Write(command.callerOid)
    .Write(command.calledOid);
}

void AcCmdRCMissionEvent::Read(
  AcCmdRCMissionEvent& command,
  SourceStream& stream)
{
  stream.Read(command.event)
    .Read(command.callerOid)
    .Read(command.calledOid);
}

void RanchCommandKickRanch::Write(
  const RanchCommandKickRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandKickRanch::Read(
  RanchCommandKickRanch& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid);
}

void RanchCommandKickRanchOK::Write(
  const RanchCommandKickRanchOK& command,
  SinkStream& stream)
{
  // Empty.
}

void RanchCommandKickRanchOK::Read(
  RanchCommandKickRanchOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandKickRanchCancel::Write(
  const RanchCommandKickRanchCancel& command,
  SinkStream& stream)
{
  // Empty.
}

void RanchCommandKickRanchCancel::Read(
  RanchCommandKickRanchCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandKickRanchNotify::Write(
  const RanchCommandKickRanchNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid);
}

void RanchCommandKickRanchNotify::Read(
  RanchCommandKickRanchNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandOpCmd::Write(
  const RanchCommandOpCmd& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandOpCmd::Read(
  RanchCommandOpCmd& command,
  SourceStream& stream)
{
  stream.Read(command.command);
}

void RanchCommandOpCmdOK::Write(
  const RanchCommandOpCmdOK& command,
  SinkStream& stream)
{
  stream.Write(command.feedback)
    .Write(command.observerState);
}

void RanchCommandOpCmdOK::Read(
  RanchCommandOpCmdOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestLeagueTeamList::Write(
  const RanchCommandRequestLeagueTeamList& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestLeagueTeamList::Read(
  RanchCommandRequestLeagueTeamList& command,
  SourceStream& stream)
{
  // Empty.
}

void RanchCommandRequestLeagueTeamListOK::Write(
  const RanchCommandRequestLeagueTeamListOK& command,
  SinkStream& stream)
{
  stream.Write(command.season);
  stream.Write(command.league);
  stream.Write(command.group);
  stream.Write(command.points);
  stream.Write(command.rank);
  stream.Write(command.previousRank);
  stream.Write(command.breakPoints);
  stream.Write(command.unk7);
  stream.Write(command.unk8);
  stream.Write(command.lastWeekLeague);
  stream.Write(command.lastWeekGroup);
  stream.Write(command.lastWeekRank);
  stream.Write(command.lastWeekAvailable);
  stream.Write(command.unk13);

  stream.Write(static_cast<uint8_t>(command.members.size()));

  for (const auto& member : command.members)
  {
    stream.Write(member.uid)
      .Write(member.points)
      .Write(member.name);
  }
}

void RanchCommandRequestLeagueTeamListOK::Read(
  RanchCommandRequestLeagueTeamListOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRecoverMount::Write(
  const AcCmdCRRecoverMount& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRecoverMount::Read(
  AcCmdCRRecoverMount& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid);
}

void AcCmdCRRecoverMountOK::Write(
  const AcCmdCRRecoverMountOK& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid)
    .Write(command.stamina)
    .Write(command.updatedCarrots);
}

void AcCmdCRRecoverMountOK::Read(
  AcCmdCRRecoverMountOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRecoverMountCancel::Write(
  const AcCmdCRRecoverMountCancel& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid);
}

void AcCmdCRRecoverMountCancel::Read(
  AcCmdCRRecoverMountCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRWithdrawGuildMember::Write(
  const AcCmdCRWithdrawGuildMember& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRWithdrawGuildMember::Read(
  AcCmdCRWithdrawGuildMember& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.member1);
}

void AcCmdCRWithdrawGuildMemberOK::Write(
  const AcCmdCRWithdrawGuildMemberOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}

void AcCmdCRWithdrawGuildMemberOK::Read(
  AcCmdCRWithdrawGuildMemberOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRWithdrawGuildMemberCancel::Write(
  const AcCmdCRWithdrawGuildMemberCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void AcCmdCRWithdrawGuildMemberCancel::Read(
  AcCmdCRWithdrawGuildMemberCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRCheckStorageItem::Read(
  AcCmdCRCheckStorageItem& command,
  SourceStream& stream)
{
  stream.Read(command.storedItemUid);
}

void AcCmdCRCheckStorageItem::Write(
  const AcCmdCRCheckStorageItem& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeAge::Read(
  AcCmdCRChangeAge& command,
  SourceStream& stream)
{
  stream.Read(command.age);
}

void AcCmdCRChangeAge::Write(
  const AcCmdCRChangeAge& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeAgeCancel::Read(
  AcCmdCRChangeAgeCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeAgeCancel::Write(
  const AcCmdCRChangeAgeCancel& command,
  SinkStream& stream)
{
}

void AcCmdCRChangeAgeOK::Read(
  AcCmdCRChangeAgeOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeAgeOK::Write(
  const AcCmdCRChangeAgeOK& command,
  SinkStream& stream)
{
  stream.Write(command.age);
}

void AcCmdRCChangeAgeNotify::Read(
  AcCmdRCChangeAgeNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCChangeAgeNotify::Write(
  const AcCmdRCChangeAgeNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.age);
}

void AcCmdCRHideAge::Read(
  AcCmdCRHideAge& command,
  SourceStream& stream)
{
  stream.Read(command.option);
}

void AcCmdCRHideAge::Write(
  const AcCmdCRHideAge& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHideAgeCancel::Read(
  AcCmdCRHideAgeCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHideAgeCancel::Write(
  const AcCmdCRHideAgeCancel& command,
  SinkStream& stream)
{
}

void AcCmdCRHideAgeOK::Read(
  AcCmdCRHideAgeOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHideAgeOK::Write(
  const AcCmdCRHideAgeOK& command,
  SinkStream& stream)
{
  stream.Write(command.option);
}

void AcCmdRCHideAgeNotify::Read(
  AcCmdRCHideAgeNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCHideAgeNotify::Write(
  const AcCmdRCHideAgeNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.option);
}

void AcCmdCRStatusPointApply::Write(
  const AcCmdCRStatusPointApply& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRStatusPointApply::Read(
  AcCmdCRStatusPointApply& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid)
  .Read(command.stats);
}

void AcCmdCRStatusPointApplyOK::Write(
  const AcCmdCRStatusPointApplyOK& command,
  SinkStream& stream)
{
  // empty
}

void AcCmdCRStatusPointApplyOK::Read(
  AcCmdCRStatusPointApplyOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRStatusPointApplyCancel::Write(
  const AcCmdCRStatusPointApplyCancel& command,
  SinkStream& stream)
{
  // empty
}

void AcCmdCRStatusPointApplyCancel::Read(
  AcCmdCRStatusPointApplyCancel& command,
  SourceStream& stream)
{
  // empty
}

} // namespace server::protocol

