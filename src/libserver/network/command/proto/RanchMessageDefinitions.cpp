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
  const AcCmdCRUseItem&,
  SinkStream&)
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
    .Write(command.remainingItemCount)
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
  AcCmdCRUseItemOK&,
  SourceStream&)
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
  AcCmdCRUseItemCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandMountFamilyTree::Write(
  const RanchCommandMountFamilyTree&,
  SinkStream&)
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
  RanchCommandMountFamilyTreeOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandMountFamilyTreeCancel::Write(
  const RanchCommandMountFamilyTreeCancel&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandMountFamilyTreeCancel::Read(
  RanchCommandMountFamilyTreeCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCREnterRanch::Write(
  const AcCmdCREnterRanch&,
  SinkStream&)
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
  AcCmdCREnterRanchOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandEnterRanchCancel::Write(
  const RanchCommandEnterRanchCancel&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandEnterRanchCancel::Read(
  RanchCommandEnterRanchCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandEnterRanchNotify::Write(
  const RanchCommandEnterRanchNotify& command,
  SinkStream& stream)
{
  stream.Write(command.character);
}

void RanchCommandEnterRanchNotify::Read(
  RanchCommandEnterRanchNotify&,
  SourceStream&)
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
  const AcCmdCRRanchSnapshot&,
  SinkStream&)
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
  RanchCommandRanchSnapshotNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRanchCmdAction::Write(
  const AcCmdCRRanchCmdAction&,
  SinkStream&)
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
  RanchCommandRanchCmdActionNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUpdateBusyState::Write(
  const RanchCommandUpdateBusyState&,
  SinkStream&)
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
  RanchCommandUpdateBusyStateNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRLeaveRanch::Write(
  const AcCmdCRLeaveRanch&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRLeaveRanch::Read(
  AcCmdCRLeaveRanch&,
  SourceStream&)
{
  // Empty.
}

void AcCmdCRLeaveRanchOK::Write(
  const AcCmdCRLeaveRanchOK&,
  SinkStream&)
{
  // Empty.
}

void AcCmdCRLeaveRanchOK::Read(
  AcCmdCRLeaveRanchOK&,
  SourceStream&)
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
  AcCmdCRLeaveRanchNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRHeartbeat::Write(
  const AcCmdCRHeartbeat&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRHeartbeat::Read(
  AcCmdCRHeartbeat&,
  SourceStream&)
{
  // Empty.
}

void RanchCommandRanchStuff::Write(
  const RanchCommandRanchStuff&,
  SinkStream&)
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
  RanchCommandRanchStuffOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRSearchStallion::Write(
  const AcCmdCRSearchStallion&,
  SinkStream&)
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
  const RanchCommandSearchStallionCancel&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandSearchStallionCancel::Read(
  RanchCommandSearchStallionCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRegisterStallion::Write(
  const AcCmdCRRegisterStallion&,
  SinkStream&)
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
  AcCmdCRRegisterStallionOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRegisterStallionCancel::Write(
  const RanchCommandRegisterStallionCancel&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandRegisterStallionCancel::Read(
  RanchCommandRegisterStallionCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallion::Write(
  const AcCmdCRUnregisterStallion&,
  SinkStream&)
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
  const AcCmdCRUnregisterStallionOK&,
  SinkStream&)
{
  // Empty.
}

void AcCmdCRUnregisterStallionOK::Read(
  AcCmdCRUnregisterStallionOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandUnregisterStallionCancel::Write(
  const RanchCommandUnregisterStallionCancel&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandUnregisterStallionCancel::Read(
  RanchCommandUnregisterStallionCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallionEstimateInfo::Write(
  const AcCmdCRUnregisterStallionEstimateInfo&,
  SinkStream&)
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
  AcCmdCRUnregisterStallionEstimateInfoOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUnregisterStallionEstimateInfoCancel::Write(
  const AcCmdCRUnregisterStallionEstimateInfoCancel&,
  SinkStream&)
{
  // Empty.
}

void AcCmdCRUnregisterStallionEstimateInfoCancel::Read(
  AcCmdCRUnregisterStallionEstimateInfoCancel&,
  SourceStream&)
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
  RanchCommandSearchStallionOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCREnterBreedingMarket::Write(
  const AcCmdCREnterBreedingMarket&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCREnterBreedingMarket::Read(
  AcCmdCREnterBreedingMarket&,
  SourceStream&)
{
  // Empty.
}

void RanchCommandEnterBreedingMarketCancel::Write(
  const RanchCommandEnterBreedingMarketCancel&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandEnterBreedingMarketCancel::Read(
  RanchCommandEnterBreedingMarketCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
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
  RanchCommandEnterBreedingMarketOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRTryBreeding::Write(
  const AcCmdCRTryBreeding&,
  SinkStream&)
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
  RanchCommandTryBreedingCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBreedingAbandon::Write(
  const AcCmdCRBreedingAbandon&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBreedingAbandon::Read(
  AcCmdCRBreedingAbandon& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid);
}

void AcCmdCRBreedingAbandonOK::Write(
  const AcCmdCRBreedingAbandon& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid);
}

void AcCmdCRBreedingAbandonOK::Read(
  AcCmdCRBreedingAbandon&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBreedingAbandonCancel::Write(
  const AcCmdCRBreedingAbandon& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid);
}

void AcCmdCRBreedingAbandonCancel::Read(
  AcCmdCRBreedingAbandon&,
  SourceStream&)
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
  RanchCommandTryBreedingOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRBreedingWishlist::Write(
  const AcCmdCRBreedingWishlist&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRBreedingWishlist::Read(
  AcCmdCRBreedingWishlist&,
  SourceStream&)
{
  // Empty.
}

void AcCmdCRBreedingWishlistCancel::Write(
  const AcCmdCRBreedingWishlistCancel&,
  SinkStream&)
{
  // Empty.
}

void AcCmdCRBreedingWishlistCancel::Read(
  AcCmdCRBreedingWishlistCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRBreedingWishlistOK::Write(
  const AcCmdCRBreedingWishlistOK& command,
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

void AcCmdCRBreedingWishlistOK::Read(
  AcCmdCRBreedingWishlistOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRUpdateMountNickname::Write(
  const AcCmdCRUpdateMountNickname&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRUpdateMountNickname::Read(
  AcCmdCRUpdateMountNickname& command,
  SourceStream& stream)
{
  stream.Read(command.horseUid)
    .Read(command.name)
    .Read(command.itemUid);
}

void AcCmdCRUpdateMountNicknameCancel::Write(
  const AcCmdCRUpdateMountNicknameCancel& command,
  SinkStream& stream)
{
  stream.Write(command.error);
}

void AcCmdCRUpdateMountNicknameCancel::Read(
  AcCmdCRUpdateMountNicknameCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdRCUpdateMountInfoNotify::Write(
  const AcCmdRCUpdateMountInfoNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid)
    .Write(command.action)
    .Write(command.horse);
}

void AcCmdRCUpdateMountInfoNotify::Read(
  AcCmdRCUpdateMountInfoNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRUpdateMountNicknameOK::Write(
  const AcCmdCRUpdateMountNicknameOK& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid)
    .Write(command.nickname)
    .Write(command.itemUid)
    .Write(command.itemCount);
}

void AcCmdCRUpdateMountNicknameOK::Read(
  AcCmdCRUpdateMountNicknameOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRequestStorage::Write(
  const AcCmdCRRequestStorage&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRequestStorage::Read(
  AcCmdCRRequestStorage& command,
  SourceStream& stream)
{
  stream.Read(command.category)
    .Read(command.page);
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

void AcCmdCRRequestStorageOK::Read(
  AcCmdCRRequestStorageOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRRequestStorageCancel::Write(
  const AcCmdCRRequestStorageCancel& command,
  SinkStream& stream)
{
  stream.Write(command.category)
    .Write(command.val1);
}

void AcCmdCRRequestStorageCancel::Read(
  AcCmdCRRequestStorageCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRGetItemFromStorage::Write(
  const AcCmdCRGetItemFromStorage&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRGetItemFromStorage::Read(
  AcCmdCRGetItemFromStorage& command,
  SourceStream& stream)
{
  stream.Read(command.storageItemUid);
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
  AcCmdCRGetItemFromStorageOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRGetItemFromStorageCancel::Write(
  const AcCmdCRGetItemFromStorageCancel& command,
  SinkStream& stream)
{
  stream.Write(command.storageItemUid)
    .Write(command.status);
}

void AcCmdCRGetItemFromStorageCancel::Read(
  AcCmdCRGetItemFromStorageCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandCheckStorageItem::Write(
  const AcCmdCRGetItemFromStorage&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandCheckStorageItem::Read(
  AcCmdCRGetItemFromStorage& command,
  SourceStream& stream)
{
  stream.Read(command.storageItemUid);
}

void RanchCommandRequestNpcDressList::Write(
  const RanchCommandRequestNpcDressList&,
  SinkStream&)
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
  RanchCommandRequestNpcDressListOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandRequestNpcDressListCancel::Write(
  const RanchCommandRequestNpcDressListCancel&,
  SinkStream&)
{
  // Empty
}

void RanchCommandRequestNpcDressListCancel::Read(
  RanchCommandRequestNpcDressListCancel&,
  SourceStream&)
{
  // Empty
}

void AcCmdCRRanchChat::Write(
  const AcCmdCRRanchChat&,
  SinkStream&)
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
  AcCmdCRRanchChatNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRWearEquipment::Write(
  const AcCmdCRWearEquipment&,
  SinkStream&)
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
  AcCmdCRWearEquipmentOK&,
  SourceStream&)
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
  AcCmdCRWearEquipmentCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRemoveEquipment::Write(
  const AcCmdCRRemoveEquipment&,
  SinkStream&)
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
  AcCmdCRRemoveEquipmentOK&,
  SourceStream&)
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
  AcCmdCRRemoveEquipmentCancel&,
  SourceStream&)
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
  AcCmdCRUpdateEquipmentNotify&,
  SourceStream&)
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
  RanchCommandSetIntroductionNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandCreateGuild::Write(
  const RanchCommandCreateGuild&,
  SinkStream&)
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
  RanchCommandCreateGuildOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRCreateGuildCancel::Write(
  const AcCmdCRCreateGuildCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status)
    .Write(command.member2);
}

void AcCmdCRCreateGuildCancel::Read(
  AcCmdCRCreateGuildCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestGuildInfo::Write(
  const RanchCommandRequestGuildInfo&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}
void RanchCommandRequestGuildInfo::Read(
  RanchCommandRequestGuildInfo&,
  SourceStream&)
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
    .Write(command.memberCount)
    .Write(command.member5)
    .Write(command.name)
    .Write(command.description)
    .Write(command.inviteCooldown)
    .Write(command.member9)
    .Write(command.member10)
    .Write(command.member11);
}

void RanchCommandRequestGuildInfoOK::GuildInfo::Read(
  GuildInfo&,
  SourceStream&)
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
  RanchCommandRequestGuildInfoOK&,
  SourceStream&)
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
  RanchCommandRequestGuildInfoCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdatePet::Write(
  const AcCmdCRUpdatePet&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdatePet::Read(
  AcCmdCRUpdatePet& command,
  SourceStream& stream)
{
  stream.Read(command.petInfo);

  uint32_t itemUid{0};
  // The client bleeds stack instead of skipping the uninitialized value.
  if (stream.GetCursor() - stream.Size() > 4)
    stream.Read(itemUid);

  command.itemUid.emplace(itemUid);
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
    .Write(command.error);
}

void AcCmdRCUpdatePetCancel::Read(
  AcCmdRCUpdatePetCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBoostIncubateInfoList::Write(
  const AcCmdCRBoostIncubateInfoList&,
  SinkStream&)
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
  AcCmdCRBoostIncubateInfoListOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBoostIncubateEgg::Write(
  const AcCmdCRBoostIncubateEgg&,
  SinkStream&)
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
  AcCmdCRBoostIncubateEggOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestPetBirth::Write(
  const AcCmdCRRequestPetBirth&,
  SinkStream&)
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
  AcCmdCRRequestPetBirthOK&,
  SourceStream&)
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
  AcCmdCRRequestPetBirthNotify&,
  SourceStream&)
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
  AcCmdCRRequestPetBirthCancel&,
  SourceStream&)
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
  RanchCommandPetBirthNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRIncubateEgg::Write(
  const AcCmdCRIncubateEgg&,
  SinkStream&)
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
  AcCmdCRIncubateEggOK&,
  SourceStream&)
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
  AcCmdCRIncubateEggNotify&,
  SourceStream&)
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
  AcCmdCRIncubateEggCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandUserPetInfos::Write(
  const RanchCommandUserPetInfos&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandUserPetInfos::Read(
  RanchCommandUserPetInfos&,
  SourceStream&)
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
  RanchCommandUserPetInfosOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRAchievementUpdateProperty::Write(
  const AcCmdCRAchievementUpdateProperty&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRAchievementUpdateProperty::Read(
  AcCmdCRAchievementUpdateProperty& command,
  SourceStream& stream)
{
  stream.Read(command.achievementEvent)
    .Read(command.member2);
}

void AcCmdCRHousingBuild::Write(
  const AcCmdCRHousingBuild&,
  SinkStream&)
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
  AcCmdCRHousingBuildOK&,
  SourceStream&)
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
  AcCmdCRHousingBuildCancel&,
  SourceStream&)
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
  AcCmdCRHousingBuildNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHousingRepair::Write(
  const AcCmdCRHousingRepair&,
  SinkStream&)
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
  AcCmdCRHousingRepairOK&,
  SourceStream&)
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
  AcCmdCRHousingRepairCancel&,
  SourceStream&)
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
  AcCmdCRHousingRepairNotify&,
  SourceStream&)
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
  const RanchCommandKickRanch&,
  SinkStream&)
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
  const RanchCommandKickRanchOK&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandKickRanchOK::Read(
  RanchCommandKickRanchOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandKickRanchCancel::Write(
  const RanchCommandKickRanchCancel&,
  SinkStream&)
{
  // Empty.
}

void RanchCommandKickRanchCancel::Read(
  RanchCommandKickRanchCancel&,
  SourceStream&)
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
  RanchCommandKickRanchNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCROpCmd::Write(
  const AcCmdCROpCmd&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCROpCmd::Read(
  AcCmdCROpCmd& command,
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
  RanchCommandOpCmdOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestLeagueTeamList::Write(
  const RanchCommandRequestLeagueTeamList&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestLeagueTeamList::Read(
  RanchCommandRequestLeagueTeamList&,
  SourceStream&)
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
  RanchCommandRequestLeagueTeamListOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRecoverMount::Write(
  const AcCmdCRRecoverMount&,
  SinkStream&)
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
  AcCmdCRRecoverMountOK&,
  SourceStream&)
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
  AcCmdCRRecoverMountCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdCRWithdrawGuildMember::Write(
  const AcCmdCRWithdrawGuildMember&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRWithdrawGuildMember::Read(
  AcCmdCRWithdrawGuildMember& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.option);
}

void AcCmdCRWithdrawGuildMemberOK::Write(
  const AcCmdCRWithdrawGuildMemberOK& command,
  SinkStream& stream)
{
  stream.Write(command.option);
}

void AcCmdCRWithdrawGuildMemberOK::Read(
  AcCmdCRWithdrawGuildMemberOK&,
  SourceStream&)
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
  AcCmdCRWithdrawGuildMemberCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCWithdrawGuildMemberNotify::Write(
  const AcCmdRCWithdrawGuildMemberNotify& command,
  SinkStream& stream)
{
  stream.Write(command.guildUid)
    .Write(command.guildMemberCharacterUid)
    .Write(command.withdrawnCharacterUid)
    .Write(command.option);
}

void AcCmdRCWithdrawGuildMemberNotify::Read(
  AcCmdRCWithdrawGuildMemberNotify&,
  SourceStream&)
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
  const AcCmdCRCheckStorageItem&,
  SinkStream&)
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
  const AcCmdCRChangeAge&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeAgeCancel::Read(
  AcCmdCRChangeAgeCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeAgeCancel::Write(
  const AcCmdCRChangeAgeCancel&,
  SinkStream&)
{
}

void AcCmdCRChangeAgeOK::Read(
  AcCmdCRChangeAgeOK&,
  SourceStream&)
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
  AcCmdRCChangeAgeNotify&,
  SourceStream&)
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
  const AcCmdCRHideAge&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHideAgeCancel::Read(
  AcCmdCRHideAgeCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRHideAgeCancel::Write(
  const AcCmdCRHideAgeCancel&,
  SinkStream&)
{
}

void AcCmdCRHideAgeOK::Read(
  AcCmdCRHideAgeOK&,
  SourceStream&)
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
  AcCmdRCHideAgeNotify&,
  SourceStream&)
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

void AcCmdCRChangeNickname::Write(
  const AcCmdCRChangeNickname&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeNickname::Read(
  AcCmdCRChangeNickname& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid)
    .Read(command.newNickname);
}

void AcCmdCRChangeNicknameCancel::Write(
  const AcCmdCRChangeNicknameCancel& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.error);
}

void AcCmdCRChangeNicknameCancel::Read(
  AcCmdCRChangeNicknameCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeNicknameOK::Write(
  const AcCmdCRChangeNicknameOK& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.remainingItemCount)
    .Write(command.newNickname);
}

void AcCmdCRStatusPointApply::Write(
  const AcCmdCRStatusPointApply& ,
  SinkStream&)
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
  const AcCmdCRStatusPointApplyOK&,
  SinkStream&)
{
  // empty
}

void AcCmdCRStatusPointApplyOK::Read(
  AcCmdCRStatusPointApplyOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRStatusPointApplyCancel::Write(
  const AcCmdCRStatusPointApplyCancel&,
  SinkStream&)
{
  // empty
}

void AcCmdCRStatusPointApplyCancel::Read(
  AcCmdCRStatusPointApplyCancel&,
  SourceStream&)
{
  // empty
}

void AcCmdCRChangeSkillCardPreset::Write(
  const AcCmdCRChangeSkillCardPreset&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRChangeSkillCardPreset::Read(
  AcCmdCRChangeSkillCardPreset& command,
  SourceStream& stream)
{
  stream.Read(command.skillSet);
}

void AcCmdCRGuildMemberList::Read(
  AcCmdCRGuildMemberList&,
  SourceStream&)
{
  // Empty.
}

void AcCmdCRGuildMemberList::Write(
  const AcCmdCRGuildMemberList&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRGuildMemberListCancel::Read(
  AcCmdCRGuildMemberListCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRGuildMemberListCancel::Write(
  const AcCmdCRGuildMemberListCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void AcCmdCRGuildMemberListOK::Read(
  AcCmdCRGuildMemberListOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRGuildMemberListOK::Write(
  const AcCmdCRGuildMemberListOK& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.members.size()));
  for (const auto& member : command.members)
  {
    stream.Write(member.memberUid)
      .Write(member.nickname)
      .Write(member.unk0)
      .Write(member.guildRole)
      .Write(member.unk2);
  }
}

void AcCmdCRRequestGuildMatchInfo::Read(
  AcCmdCRRequestGuildMatchInfo& command,
  SourceStream& stream)
{
  stream.Read(command.guildUid);
}

void AcCmdCRRequestGuildMatchInfo::Write(
  const AcCmdCRRequestGuildMatchInfo&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildMatchInfoCancel::Read(
  AcCmdCRRequestGuildMatchInfoCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildMatchInfoCancel::Write(
  const AcCmdCRRequestGuildMatchInfoCancel&,
  SinkStream&)
{
  // Empty.
}

void AcCmdCRRequestGuildMatchInfoOK::Read(
  AcCmdCRRequestGuildMatchInfoOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildMatchInfoOK::Write(
  const AcCmdCRRequestGuildMatchInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.guildUid)
    .Write(command.name)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.unk4)
    .Write(command.unk5)
    .Write(command.totalWins)
    .Write(command.totalLosses)
    .Write(command.unk8)
    .Write(command.rank)
    .Write(command.unk10)
    .Write(command.seasonalWins)
    .Write(command.seasonalLosses);
}

void AcCmdCRUpdateGuildMemberGrade::Read(
  AcCmdCRUpdateGuildMemberGrade& command,
  SourceStream& stream)
{
  stream.Read(command.characterUid)
    .Read(command.guildRole);
}

void AcCmdCRUpdateGuildMemberGrade::Write(
  const AcCmdCRUpdateGuildMemberGrade&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateGuildMemberGradeCancel::Read(
  AcCmdCRUpdateGuildMemberGradeCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateGuildMemberGradeCancel::Write(
  const AcCmdCRUpdateGuildMemberGradeCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}

void AcCmdCRUpdateGuildMemberGradeOK::Read(
  AcCmdCRUpdateGuildMemberGradeOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateGuildMemberGradeOK::Write(
  const AcCmdCRUpdateGuildMemberGradeOK&,
  SinkStream&)
{
  // Empty return
}

void AcCmdRCUpdateGuildMemberGradeNotify::Read(
  AcCmdRCUpdateGuildMemberGradeNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCUpdateGuildMemberGradeNotify::Write(
  const AcCmdRCUpdateGuildMemberGradeNotify& command,
  SinkStream& stream)
{
  stream.Write(command.guildUid)
    .Write(command.unk1)
    .Write(command.targetCharacterUid)
    .Write(command.unk3)
    .Write(command.guildRole);
}

void AcCmdCRInviteGuildJoin::Read(
  AcCmdCRInviteGuildJoin& command,
  SourceStream& stream)
{
  stream.Read(command.characterName);
}

void AcCmdCRInviteGuildJoin::Write(
  const AcCmdCRInviteGuildJoin&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteGuildJoinCancel::Read(
  AcCmdCRInviteGuildJoinCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteGuildJoinCancel::Write(
  const AcCmdCRInviteGuildJoinCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.error)
    .Write(command.unk4);
}

void AcCmdRCAcceptGuildJoinNotify::Read(
  AcCmdRCAcceptGuildJoinNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCAcceptGuildJoinNotify::Write(
  const AcCmdRCAcceptGuildJoinNotify& command,
  SinkStream& stream)
{
  stream.Write(command.guildMemberCharacterUid)
    .Write(command.newMemberCharacterUid)
    .Write(command.newMemberCharacterName);
}

void AcCmdCREmblemList::Read(
  AcCmdCREmblemList&,
  SourceStream&)
{
  // Empty
}

void AcCmdCREmblemList::Write(
  const AcCmdCREmblemList&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCREmblemListCancel::Read(
  AcCmdCREmblemListCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCREmblemListCancel::Write(
  const AcCmdCREmblemListCancel&,
  SinkStream&)
{
  // Empty
}

void AcCmdCREmblemListOK::Read(
  AcCmdCREmblemListOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCREmblemListOK::Write(
  const AcCmdCREmblemListOK& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.unk0.size()));
  for (const auto& val : command.unk0)
  {
    stream.Write(val);
  }
}

void AcCmdCRUpdateDailyQuest::Write(
  const AcCmdCRUpdateDailyQuest&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateDailyQuest::Read(
  AcCmdCRUpdateDailyQuest& command,
  SourceStream& stream)
{
  stream.Read(command.quest);
}

void AcCmdCRUpdateDailyQuestCancel::Write(
  const AcCmdCRUpdateDailyQuestCancel&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateDailyQuestCancel::Read(
  AcCmdCRUpdateDailyQuestCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateDailyQuestOK::Write(
  const AcCmdCRUpdateDailyQuestOK& command,
  SinkStream& stream)
{
  stream.Write(command.newCarrotBalance);
  stream.Write(command.quest);
  stream.Write(command.unk_1);
  stream.Write(command.unk_2);
}

void AcCmdCRUpdateDailyQuestOK::Read(
  AcCmdCRUpdateDailyQuestOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRegisterDailyQuestGroup::Write(
  const AcCmdCRRegisterDailyQuestGroup&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRegisterDailyQuestGroup::Read(
  AcCmdCRRegisterDailyQuestGroup& command,
  SourceStream& stream)
{
  uint16_t size{};
  stream.Read(size);

  command.dailyQuests.resize(size);
  for (auto& quest : command.dailyQuests)
  {
    stream.Read(quest);
  }
}

void AcCmdCRRegisterDailyQuestGroupOK::Write(
  const AcCmdCRRegisterDailyQuestGroupOK& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void AcCmdCRRegisterDailyQuestGroupOK::Read(
  AcCmdCRRegisterDailyQuestGroupOK&,
  SourceStream&)
  {
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateMountInfoOK::Write(
  const AcCmdCRUpdateMountInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.action)
    .Write(command.horse);
}

void AcCmdCRUpdateMountInfoOK::Read(
  AcCmdCRUpdateMountInfoOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdRCUpdateDailyQuestNotify::Write(
  const AcCmdRCUpdateDailyQuestNotify& command,
  SinkStream& stream)
{
  stream.Write(command.characterUid);
  stream.Write(command.questId);
  stream.Write(command.unk);
  stream.Write(command.unk0);
  stream.Write(command.unk1);
  stream.Write(command.unk2);
  stream.Write(command.unk3);
}

void AcCmdRCUpdateDailyQuestNotify::Read(
  AcCmdRCUpdateDailyQuestNotify&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented.");
}

void AcCmdRCUpdateDailyQuestNotify::Unk::Write(const Unk& value, SinkStream& stream)
{
  stream.Write(value.unk0)
    .Write(value.unk1)
    .Write(value.unk2);
}

void AcCmdRCUpdateDailyQuestNotify::Unk::Read(Unk& value, SourceStream& stream)
{
  stream.Read(value.unk0)
    .Read(value.unk1)
    .Read(value.unk2);
}

void AcCmdCRRequestDailyQuestReward::Write(
  const AcCmdCRRequestDailyQuestReward&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestDailyQuestReward::Read(
  AcCmdCRRequestDailyQuestReward& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
  stream.Read(command.unk1);
}

void AcCmdCRRequestDailyQuestRewardOK::Write(
  const AcCmdCRRequestDailyQuestRewardOK& command,
  SinkStream& stream)
{
  stream.Write(static_cast<uint8_t>(command.rewards.items.size()));

  for (auto& member : command.rewards.items)
  {
    stream.Write(member);
  }
}

void AcCmdCRRequestDailyQuestRewardOK::Read(
  AcCmdCRRequestDailyQuestRewardOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestDailyQuestRewardOK::Reward::Write(const Reward& value, SinkStream& stream)
{
  for (auto& member : value.items)
  {
    stream.Write(member);
  }
}

void AcCmdCRRequestDailyQuestRewardOK::Reward::Read(Reward& value, SourceStream& stream)
{
  for (auto& member : value.items)
  {
    stream.Read(member);
  }
}

void AcCmdCRMountInjuryHealOK::Read(
  AcCmdCRMountInjuryHealOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRMountInjuryHealOK::Write(
  const AcCmdCRMountInjuryHealOK& command,
  SinkStream& stream)
{
  stream.Write(command.horseUid)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.updatedCarrotCount);
}

void AcCmdCRRegisterQuest::Write(
  const AcCmdCRRegisterQuest&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}
void AcCmdCRConfirmItem::Write(
  const AcCmdCRConfirmItem&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRegisterQuest::Read(
  AcCmdCRRegisterQuest& command,
  SourceStream& stream)
{
  stream.Read(command.questId);
  stream.Read(command.npcId);
}

void AcCmdCRRegisterQuestOK::Write(
  const AcCmdCRRegisterQuestOK& command,
  SinkStream& stream)
{
  stream.Write(command.questId);
  stream.Write(command.progress);
  stream.Write(command.isCompleted);
}

void AcCmdCRRegisterQuestOK::Read(
  AcCmdCRRegisterQuestOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestQuestReward::Write(
  const AcCmdCRRequestQuestReward&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestQuestReward::Read(
  AcCmdCRRequestQuestReward& command,
  SourceStream& stream)
{
  stream.Read(command.unk0);
  stream.Read(command.unk1);
  stream.Read(command.unk2);
}

void AcCmdCRRequestQuestRewardOK::Write(
  const AcCmdCRRequestQuestRewardOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
  stream.Write(command.unk1);
  stream.Write(command.unk2);

  for (auto& reward : command.rewards.items)
  {
    stream.Write(reward);
  }

  stream.Write(command.unk3);

  for (auto& member : command.unk4)
  {
    stream.Write(member);
  }
}

void AcCmdCRRequestQuestRewardOK::Read(
  AcCmdCRRequestQuestRewardOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestQuestRewardOK::Unk1::Write(const Unk1& value, SinkStream& stream)
{
  stream.Write(value.unk0)
    .Write(value.unk1);
}

void AcCmdCRRequestQuestRewardOK::Unk1::Read(Unk1& value, SourceStream& stream)
{
  stream.Read(value.unk0)
    .Read(value.unk1);
}
void AcCmdCRConfirmItem::Read(
  AcCmdCRConfirmItem& command,
  SourceStream& stream)
{
  stream.Read(command.recipientCharacterName)
    .Read(command.goodsSq)
    .Read(command.member3);
}

void AcCmdCRConfirmItemOK::Write(
  const AcCmdCRConfirmItemOK& command,
  SinkStream& stream)
{
  stream.Write(command.recipientCharacterName)
    .Write(command.goodsSq)
    .Write(command.canPurchase);
}

void AcCmdCRConfirmItemOK::Read(
  AcCmdCRConfirmItemOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRConfirmItemCancel::Write(
  const AcCmdCRConfirmItemCancel& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.member2)
    .Write(command.member3);
}

void AcCmdCRConfirmItemCancel::Read(
  AcCmdCRConfirmItemCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRConfirmSetItem::Read(
  AcCmdCRConfirmSetItem& command,
  SourceStream& stream)
{
  stream.Read(command.goodsSq);
}

void AcCmdCRConfirmSetItem::Write(
  const AcCmdCRConfirmSetItem&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRConfirmSetItemOK::Read(
  AcCmdCRConfirmSetItemOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRConfirmSetItemOK::Write(
  const AcCmdCRConfirmSetItemOK& command,
  SinkStream& stream)
{
  stream.Write(command.goodsSq)
    .Write(command.result);
}

void AcCmdCRConfirmSetItemCancel::Read(
  AcCmdCRConfirmSetItemCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRConfirmSetItemCancel::Write(
  const AcCmdCRConfirmSetItemCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void AcCmdCRBuyOwnItem::Read(
  AcCmdCRBuyOwnItem& command,
  SourceStream& stream)
{
  uint8_t count{0};
  stream.Read(count);
  command.orders.resize(count);
  for (auto& order : command.orders)
  {
    stream.Read(order);
  }
}

void AcCmdCRBuyOwnItem::Write(
  const AcCmdCRBuyOwnItem&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBuyOwnItemCancel::Read(
  AcCmdCRBuyOwnItemCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBuyOwnItemCancel::Write(
  const AcCmdCRBuyOwnItemCancel& command,
  SinkStream& stream)
{
  stream.Write(command.error);
}

void AcCmdCRBuyOwnItemOK::Read(
  AcCmdCRBuyOwnItemOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRBuyOwnItemOK::Write(
  const AcCmdCRBuyOwnItemOK& command,
  SinkStream& stream)
{
  // List size in one byte.
  stream.Write(static_cast<uint8_t>(command.orderResults.size()));
  for (const auto& shopItemResult : command.orderResults)
  {
    stream.Write(shopItemResult.order)
      .Write(shopItemResult.result);
  }

  // List size in one byte.
  stream.Write(static_cast<uint8_t>(command.purchases.size()));
  for (const auto& ownedItem : command.purchases)
  {
    stream.Write(ownedItem.equipImmediately)
      .Write(ownedItem.item);
  }

  stream.Write(command.newCarrots)
    .Write(command.newCash);
}

void AcCmdCRSendGift::Read(
  AcCmdCRSendGift& command,
  SourceStream& stream)
{
  stream.Read(command.recipientCharacterName)
    .Read(command.message)
    .Read(command.order);
}

void AcCmdCRSendGift::Write(
  const AcCmdCRSendGift&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRSendGiftCancel::Read(
  AcCmdCRSendGiftCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRSendGiftCancel::Write(
  const AcCmdCRSendGiftCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unused);
}

void AcCmdCRSendGiftOK::Read(
  AcCmdCRSendGiftOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRSendGiftOK::Write(
  const AcCmdCRSendGiftOK& command,
  SinkStream& stream)
{
  stream.Write(command.giftOrderResult.order)
    .Write(command.giftOrderResult.error)
    .Write(command.carrots)
    .Write(command.cash);
}

void AcCmdCRPasswordAuth::Write(
  const AcCmdCRPasswordAuth&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRPasswordAuth::Read(
  AcCmdCRPasswordAuth& command,
  SourceStream& stream)
{
  stream.Read(command.unk1);
  if (command.unk1 == 2)
  {
    stream.Read(command.unk2)
      .Read(command.unk3);
  }
  else if (command.unk1 == 3)
  {
    stream.Read(command.unk3);
  }
}

void AcCmdCRPasswordAuthOK::Write(
  const AcCmdCRPasswordAuthOK& command,
  SinkStream& stream)
{
  stream.Write(command.action)
    .Write(command.duration);
}

void AcCmdCRPasswordAuthOK::Read(
  AcCmdCRPasswordAuthOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateMountInfo::Write(
  const AcCmdCRUpdateMountInfo&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRUpdateMountInfo::Read(
  AcCmdCRUpdateMountInfo& command,
  SourceStream& stream)
{
  stream.Read(command.action)
    .Read(command.horse);
}

void AcCmdCROpenRandomBox::Write(
  const AcCmdCROpenRandomBox&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCROpenRandomBox::Read(
  AcCmdCROpenRandomBox& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid)
    .Read(command.unk1);
}

void AcCmdCROpenRandomBoxOK::Write(
  const AcCmdCROpenRandomBoxOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.packageId)
    .Write(command.carrotsObtained)
    .Write(command.newBalance);

  stream.Write(static_cast<uint8_t>(command.items.size()));
  for (const auto& item : command.items) {
    stream.Write(item);
  }
}

void AcCmdCROpenRandomBoxOK::Read(
  AcCmdCROpenRandomBoxOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCROpenRandomBoxCancel::Write(
  const AcCmdCROpenRandomBoxCancel& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.error);
}
void AcCmdCROpenRandomBoxCancel::Read(
  AcCmdCROpenRandomBoxCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRCheckPCBangGiftItem::Write(
  const AcCmdCRCheckPCBangGiftItem&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}
void AcCmdCRCheckPCBangGiftItem::Read(
  AcCmdCRCheckPCBangGiftItem&,
  SourceStream&)
{
  //empty
}
void AcCmdCRCheckPCBangGiftItemOK::Write(
  const AcCmdCRCheckPCBangGiftItemOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0);
}
void AcCmdCRCheckPCBangGiftItemOK::Read(
  AcCmdCRCheckPCBangGiftItemOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestPCBangGift::Write(
  const AcCmdCRRequestPCBangGift&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}
void AcCmdCRRequestPCBangGift::Read(
  AcCmdCRRequestPCBangGift& command,
  SourceStream& stream)
{
  stream.Read(command.gift);
}
void AcCmdCRRequestPCBangGiftOK::Write(
  const AcCmdCRRequestPCBangGiftOK& command,
  SinkStream& stream)
{
  stream.Write(command.gift);
  stream.Write(static_cast<uint8_t>(command.items.size()));

  for (const auto& item : command.items) {
    stream.Write(item);
  }
}
void AcCmdCRRequestPCBangGiftOK::Read(
  AcCmdCRRequestPCBangGiftOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRMountRentInfo::Write(
  const AcCmdCRMountRentInfo&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}
void AcCmdCRMountRentInfo::Read(
  AcCmdCRMountRentInfo&,
  SourceStream&)
{
  // empty
}
void AcCmdCRMountRentInfoOK::Write(
  const AcCmdCRMountRentInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.rent)
    .Write(command.unk2)
    .Write(command.unk3);
}
void AcCmdCRMountRentInfoOK::Read(
  AcCmdCRMountRentInfoOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRMountRent::Write(
  const AcCmdCRMountRent&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
};

void AcCmdCRMountRent::Read(
  AcCmdCRMountRent& command,
  SourceStream& stream)
{
  stream.Read(command.rent)
    .Read(command.status);
};

void AcCmdCRMountRentOK::Write(
  const AcCmdCRMountRentOK& command,
  SinkStream& stream)
{
  stream.Write(command.rent)
    .Write(command.status);
};

void AcCmdCRMountRentOK::Read(
  AcCmdCRMountRentOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
};

void AcCmdCRMountRentReset::Write(
  const AcCmdCRMountRentReset&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
};

void AcCmdCRMountRentReset::Read(
  AcCmdCRMountRentReset&,
  SourceStream&)
{
  //Empty
};

void AcCmdCRMountRentResetOK::Write(
  const AcCmdCRMountRentResetOK&,
  SinkStream&)
{
  //Empty
};

void AcCmdCRMountRentResetOK::Read(
  AcCmdCRMountRentResetOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
};

} // namespace server::protocol

