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

void RanchCommandUseItem::Write(
  const RanchCommandUseItem& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUseItem::Read(
  RanchCommandUseItem& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid)
    .Read(command.always1)
    .Read(command.always1too)
    .Read(command.play);
}

void RanchCommandUseItemOK::ActionTwoBytes::Write(
  const ActionTwoBytes& action,
  SinkStream& stream)
{
  stream.Write(action.unk0)
    .Write(action.play);
}

void RanchCommandUseItemOK::ActionTwoBytes::Read(
  ActionTwoBytes& action,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUseItemOK::ActionOneByte::Write(
  const ActionOneByte& action,
  SinkStream& stream)
{
  stream.Write(action.unk0);
}

void RanchCommandUseItemOK::ActionOneByte::Read(
  ActionOneByte& action,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUseItemOK::Write(
  const RanchCommandUseItemOK& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.unk1);

  stream.Write(command.type);
  switch (command.type)
  {
    case ActionType::Empty:
      {
        break;
      }
    case ActionType::Action1:
    case ActionType::Action2:
    case ActionType::Action3:
      {
        stream.Write(command.actionTwoBytes);
        break;
      }
    case ActionType::Action4:
      {
        stream.Write(command.actionOneByte);
        break;
      }
    default:
      throw std::runtime_error("Not implemented.");
  }
}

void RanchCommandUseItemOK::Read(
  RanchCommandUseItemOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandUseItemCancel::Write(
  const RanchCommandUseItemCancel& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);
}

void RanchCommandUseItemCancel::Read(
  RanchCommandUseItemCancel& command,
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
  stream.Read(command.unk0);
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
    .Write(command.incubatorSlotOne)
    .Write(command.incubatorSlotTwo);

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

void RanchCommandSearchStallion::Write(
  const RanchCommandSearchStallion& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandSearchStallion::Read(
  RanchCommandSearchStallion& command,
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
}

void RanchCommandSearchStallionCancel::Read(
  RanchCommandSearchStallionCancel& command,
  SourceStream& stream)
{
}

void RanchCommandSearchStallionOK::Write(
  const RanchCommandSearchStallionOK& command,
  SinkStream& stream)
{
  stream.Write(command.unk0)
    .Write(command.unk1);

  stream.Write(static_cast<uint8_t>(command.stallions.size()));
  for (auto& unk : command.stallions)
  {
    stream.Write(unk.unk0)
      .Write(unk.uid)
      .Write(unk.tid)
      .Write(unk.name)
      .Write(unk.grade)
      .Write(unk.chance)
      .Write(unk.price)
      .Write(unk.unk7)
      .Write(unk.unk8)
      .Write(unk.stats.agility)
      .Write(unk.stats.control)
      .Write(unk.stats.speed)
      .Write(unk.stats.strength)
      .Write(unk.stats.spirit)
      .Write(unk.parts.skinId)
      .Write(unk.parts.maneId)
      .Write(unk.parts.tailId)
      .Write(unk.parts.faceId)
      .Write(unk.appearance.scale)
      .Write(unk.appearance.legLength)
      .Write(unk.appearance.legVolume)
      .Write(unk.appearance.bodyLength)
      .Write(unk.appearance.bodyVolume)
      .Write(unk.unk11)
      .Write(unk.coatBonus);
  }
}

void RanchCommandSearchStallionOK::Read(
  RanchCommandSearchStallionOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandEnterBreedingMarket::Write(
  const RanchCommandEnterBreedingMarket& command,
  SinkStream& stream)
{
}

void RanchCommandEnterBreedingMarket::Read(
  RanchCommandEnterBreedingMarket& command,
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
  stream.Write(static_cast<uint8_t>(command.availableHorses.size()));
  for (auto& availableHorse : command.availableHorses)
  {
    stream.Write(availableHorse.uid)
      .Write(availableHorse.tid)
      .Write(availableHorse.combo)
      .Write(availableHorse.unk1)
      .Write(availableHorse.unk2)
      .Write(availableHorse.lineage);
  }
}

void RanchCommandEnterBreedingMarketOK::Read(
  RanchCommandEnterBreedingMarketOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandTryBreeding::Write(
  const RanchCommandTryBreeding& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandTryBreeding::Read(
  RanchCommandTryBreeding& command,
  SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.unk1);
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
  throw std::runtime_error("Not implemented.");
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
    .Write(command.parts.skinId)
    .Write(command.parts.maneId)
    .Write(command.parts.tailId)
    .Write(command.parts.faceId)
    .Write(command.appearance.scale)
    .Write(command.appearance.legLength)
    .Write(command.appearance.legVolume)
    .Write(command.appearance.bodyLength)
    .Write(command.appearance.bodyVolume)
    .Write(command.stats.agility)
    .Write(command.stats.control)
    .Write(command.stats.speed)
    .Write(command.stats.strength)
    .Write(command.stats.spirit)
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

void RanchCommandUpdateMountInfoNotify::Write(
  const RanchCommandUpdateMountInfoNotify& command,
  SinkStream& stream)
{
  stream.Write(command.action)
    .Write(command.member1)
    .Write(command.horse);
}

void RanchCommandUpdateMountInfoNotify::Read(
  RanchCommandUpdateMountInfoNotify& command,
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

void RanchCommandRequestStorage::Write(
  const RanchCommandRequestStorage& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandRequestStorage::Read(RanchCommandRequestStorage& command, SourceStream& stream)
{
  stream.Read(command.category).Read(command.page);
}

void RanchCommandRequestStorageOK::Write(
  const RanchCommandRequestStorageOK& command,
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

void RanchCommandRequestStorageOK::Read(RanchCommandRequestStorageOK& command, SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandRequestStorageCancel::Write(
  const RanchCommandRequestStorageCancel& command,
  SinkStream& stream)
{
  stream.Write(command.category).Write(command.val1);
}

void RanchCommandRequestStorageCancel::Read(
  RanchCommandRequestStorageCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandGetItemFromStorage::Write(
  const RanchCommandGetItemFromStorage& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandGetItemFromStorage::Read(
  RanchCommandGetItemFromStorage& command,
  SourceStream& stream)
{
  stream.Read(command.storedItemUid);
}

void RanchCommandGetItemFromStorageOK::Write(
  const RanchCommandGetItemFromStorageOK& command,
  SinkStream& stream)
{
  stream.Write(command.storedItemUid);
  stream.Write(static_cast<uint8_t>(command.items.size()));
  for (const auto& item : command.items)
  {
    stream.Write(item);
  }
  stream.Write(command.member0);
}

void RanchCommandGetItemFromStorageOK::Read(
  RanchCommandGetItemFromStorageOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandGetItemFromStorageCancel::Write(
  const RanchCommandGetItemFromStorageCancel& command,
  SinkStream& stream)
{
  stream.Write(command.storedItemUid)
    .Write(command.status);
}

void RanchCommandGetItemFromStorageCancel::Read(
  RanchCommandGetItemFromStorageCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandCheckStorageItem::Write(
  const RanchCommandGetItemFromStorage& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented.");
}

void RanchCommandCheckStorageItem::Read(
  RanchCommandGetItemFromStorage& command,
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
    .Write(command.isBlue)
    .Write(command.unknown2);
}

void AcCmdCRRanchChatNotify::Read(
  AcCmdCRRanchChatNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandWearEquipment::Write(
  const RanchCommandWearEquipment& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandWearEquipment::Read(
  RanchCommandWearEquipment& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid)
    .Read(command.member);
}

void RanchCommandWearEquipmentOK::Write(
  const RanchCommandWearEquipmentOK& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.member);
}

void RanchCommandWearEquipmentOK::Read(
  RanchCommandWearEquipmentOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandWearEquipmentCancel::Write(
  const RanchCommandWearEquipmentCancel& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.member);
}

void RanchCommandWearEquipmentCancel::Read(
  RanchCommandWearEquipmentCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRemoveEquipment::Write(
  const RanchCommandRemoveEquipment& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRemoveEquipment::Read(
  RanchCommandRemoveEquipment& command,
  SourceStream& stream)
{
  stream.Read(command.itemUid);
}

void RanchCommandRemoveEquipmentOK::Write(
  const RanchCommandRemoveEquipmentOK& command,
  SinkStream& stream)
{
  stream.Write(command.uid);
}

void RanchCommandRemoveEquipmentOK::Read(
  RanchCommandRemoveEquipmentOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRemoveEquipmentCancel::Write(
  const RanchCommandRemoveEquipmentCancel& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.member);
}

void RanchCommandRemoveEquipmentCancel::Read(
  RanchCommandRemoveEquipmentCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandUpdateEquipmentNotify::Write(
  const RanchCommandUpdateEquipmentNotify& command,
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

void RanchCommandUpdateEquipmentNotify::Read(
  RanchCommandUpdateEquipmentNotify& command,
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
    .Write(command.member2);
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
    stream.Read(command.actionBitset);
}

void AcCmdRCUpdatePet::Write(
  const AcCmdRCUpdatePet& command,
  SinkStream& stream)
{
  stream.Write(command.petInfo)
   .Write(command.actionBitset);
}

void AcCmdRCUpdatePet::Read(
  AcCmdRCUpdatePet& command,
  SourceStream& stream)
{
  stream.Read(command.petInfo);
  if (stream.GetCursor() - stream.Size() > 4)
    stream.Read(command.actionBitset);
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

void RanchCommandRequestPetBirth::Write(
  const RanchCommandRequestPetBirth& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestPetBirth::Read(
  RanchCommandRequestPetBirth& command,
  SourceStream& stream)
{
  stream.Read(command.member1)
    .Read(command.member2)
    .Read(command.petInfo);
}

void RanchCommandRequestPetBirthOK::Write(
  const RanchCommandRequestPetBirthOK& command,
  SinkStream& stream)
{
  stream.Write(command.petBirthInfo);
}

void RanchCommandRequestPetBirthOK::Read(
  RanchCommandRequestPetBirthOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandRequestPetBirthCancel::Write(
  const RanchCommandRequestPetBirthCancel& command,
  SinkStream& stream)
{
  stream.Write(command.petInfo);
}

void RanchCommandRequestPetBirthCancel::Read(
  RanchCommandRequestPetBirthCancel& command,
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

void RanchCommandIncubateEgg::Write(
  const RanchCommandPetBirthNotify& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandIncubateEgg::Read(
  RanchCommandPetBirthNotify& command,
  SourceStream& stream)
{
  stream.Read(command.petBirthInfo);
}

void RanchCommandIncubateEggOK::Write(
  const RanchCommandIncubateEggOK& command,
  SinkStream& stream)
{
  stream.Write(command.itemUid)
    .Write(command.egg)
    .Write(command.member3);
}

void RanchCommandIncubateEggOK::Read(
  RanchCommandIncubateEggOK& command,
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

void RanchCommandHousingBuild::Write(
  const RanchCommandHousingBuild& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandHousingBuild::Read(
  RanchCommandHousingBuild& command,
  SourceStream& stream)
{
  stream.Read(command.housingTid);
}

void RanchCommandHousingBuildOK::Write(
  const RanchCommandHousingBuildOK& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.housingTid)
    .Write(command.member3);
}

void RanchCommandHousingBuildOK::Read(
  RanchCommandHousingBuildOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandHousingBuildCancel::Write(
  const RanchCommandHousingBuildCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void RanchCommandHousingBuildCancel::Read(
  RanchCommandHousingBuildCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandHousingBuildNotify::Write(
  const RanchCommandHousingBuildNotify& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.housingTid);
}

void RanchCommandHousingBuildNotify::Read(
  RanchCommandHousingBuildNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandHousingRepair::Write(
  const RanchCommandHousingRepair& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandHousingRepair::Read(
  RanchCommandHousingRepair& command,
  SourceStream& stream)
{
  stream.Read(command.housingUid);
}

void RanchCommandHousingRepairOK::Write(
  const RanchCommandHousingRepairOK& command,
  SinkStream& stream)
{
  stream.Write(command.housingUid)
    .Write(command.member2);
}

void RanchCommandHousingRepairOK::Read(
  RanchCommandHousingRepairOK& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandHousingRepairCancel::Write(
  const RanchCommandHousingRepairCancel& command,
  SinkStream& stream)
{
  stream.Write(command.status);
}

void RanchCommandHousingRepairCancel::Read(
  RanchCommandHousingRepairCancel& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void RanchCommandHousingRepairNotify::Write(
  const RanchCommandHousingRepairNotify& command,
  SinkStream& stream)
{
  stream.Write(command.member1)
    .Write(command.housingTid);
}

void RanchCommandHousingRepairNotify::Read(
  RanchCommandHousingRepairNotify& command,
  SourceStream& stream)
{
  throw std::runtime_error("Not implemented");
}
void RanchCommandMissionEvent::Write(
  const RanchCommandMissionEvent& command,
  SinkStream& stream)
{
  stream.Write(command.event)
    .Write(command.callerOid)
    .Write(command.calledOid);
}

void RanchCommandMissionEvent::Read(
  RanchCommandMissionEvent& command,
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

} // namespace server::protocol
