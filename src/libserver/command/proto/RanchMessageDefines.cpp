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

#include "libserver/command/proto/RanchMessageDefines.hpp"
#include "libserver/command/proto/LobbyMessageDefines.hpp"

namespace alicia
{

namespace
{

void WriteUseItemType0Action(
  SinkStream& buffer, UseItemType0Action* action)
{
}

void WriteUseItemType1And2And3Action(
  SinkStream& buffer, UseItemType1And2And3Action* action)
{
  buffer.Write(action->unk0)
    .Write(action->unk1);
}

void WriteUseItemType4Action(
  SinkStream& buffer, UseItemType4Action* action)
{
  buffer.Write(action->unk0);
}

void WriteMountFamilyTreeItem(
  SinkStream& buffer, const MountFamilyTreeItem& mountFamilyTreeItem)
{
  buffer.Write(mountFamilyTreeItem.unk0)
    .Write(mountFamilyTreeItem.unk1)
    .Write(mountFamilyTreeItem.unk2)
    .Write(mountFamilyTreeItem.unk3);
}

void WriteRanchHorse(
  SinkStream& buf, const RanchHorse& ranchHorse)
{
  buf.Write(ranchHorse.ranchIndex)
    .Write(ranchHorse.horse);
}

void WriteRanchPlayer(
  SinkStream& buf, const RanchPlayer& ranchPlayer)
{
  buf.Write(ranchPlayer.userUid)
    .Write(ranchPlayer.name)
    .Write(ranchPlayer.gender)
    .Write(ranchPlayer.unk0)
    .Write(ranchPlayer.unk1)
    .Write(ranchPlayer.description);

  buf.Write(ranchPlayer.character)
    .Write(ranchPlayer.horse);

  buf.Write(static_cast<uint8_t>(
    ranchPlayer.characterEquipment.size()));
  for (const Item& item : ranchPlayer.characterEquipment)
  {
    buf.Write(item);
  }

  // Struct5
  const auto& struct5 = ranchPlayer.playerRelatedThing;
  buf.Write(struct5.val0)
    .Write(struct5.val1)
    .Write(struct5.val2)
    .Write(struct5.val3)
    .Write(struct5.val4)
    .Write(struct5.val5)
    .Write(struct5.val6);

  buf.Write(ranchPlayer.ranchIndex)
    .Write(ranchPlayer.unk2)
    .Write(ranchPlayer.unk3);

  // Struct6
  const auto& struct6 = ranchPlayer.anotherPlayerRelatedThing;
  buf.Write(struct6.mountUid)
    .Write(struct6.val1)
    .Write(struct6.val2);

  // Struct7
  const auto& struct7 = ranchPlayer.yetAnotherPlayerRelatedThing;
  buf.Write(struct7.val0)
    .Write(struct7.val1)
    .Write(struct7.val2)
    .Write(struct7.val3);

  buf.Write(ranchPlayer.unk4)
    .Write(ranchPlayer.unk5);
}

}

void RanchCommandUseItem::Write(
  const RanchCommandUseItem& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandUseItem::Read(
  RanchCommandUseItem& command, SourceStream& buffer)
{
  buffer.Read(command.unk0)
    .Read(command.unk1)
    .Read(command.unk2)
    .Read(command.unk3);
}

void RanchCommandUseItemOK::Write(
  const RanchCommandUseItemOK& command, SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.unk1);

  buffer.Write(command.type);
  switch (command.type)
  {
  case 0:
    WriteUseItemType0Action(buffer, (UseItemType0Action*) command.action);
    break;
  case 1:
  case 2:
  case 3:
    WriteUseItemType1And2And3Action(buffer, (UseItemType1And2And3Action*) command.action);
    break;  
  case 4:
    WriteUseItemType4Action(buffer, (UseItemType4Action*) command.action);
  default:
    throw std::logic_error("Not implemented.");
  }
}

void RanchCommandUseItemOK::Read(
  RanchCommandUseItemOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandMountFamilyTree::Write(
  const RanchCommandMountFamilyTree& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandMountFamilyTree::Read(
  RanchCommandMountFamilyTree& command, SourceStream& buffer)
{
  buffer.Read(command.unk0);
}

void RanchCommandMountFamilyTreeOK::Write(
  const RanchCommandMountFamilyTreeOK& command, SinkStream& buffer)
{
  buffer.Write(command.unk0);

  buffer.Write(static_cast<uint8_t>(command.items.size()));
  for (auto& item : command.items)
  {
    WriteMountFamilyTreeItem(buffer, item);
  }
}

void RanchCommandMountFamilyTreeOK::Read(
  RanchCommandMountFamilyTreeOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandMountFamilyTreeCancel::Write(
  const RanchCommandMountFamilyTreeCancel& command, SinkStream& buffer)
{
}

void RanchCommandMountFamilyTreeCancel::Read(
  RanchCommandMountFamilyTreeCancel& command, SourceStream& buffer)
{
}

void RanchCommandEnterRanch::Write(
  const RanchCommandEnterRanch& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandEnterRanch::Read(
  RanchCommandEnterRanch& command, SourceStream& buffer)
{
  buffer.Read(command.characterUid)
    .Read(command.code)
    .Read(command.ranchUid);
}

void RanchCommandEnterRanchOK::Write(
  const RanchCommandEnterRanchOK& command, SinkStream& buffer)
{
  buffer.Write(command.ranchId)
    .Write(command.unk0)
    .Write(command.ranchName);

  buffer.Write(static_cast<uint8_t>(command.horses.size()));
  for (auto& horse : command.horses)
  {
    WriteRanchHorse(buffer, horse);
  }

  buffer.Write(static_cast<uint8_t>(command.users.size()));
  for (auto& player : command.users)
  {
    WriteRanchPlayer(buffer, player);
  }

  buffer.Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3);

  buffer.Write(static_cast<uint8_t>(command.unk4.size()));
  for (auto& unk : command.unk4)
  {
    buffer.Write(unk.unk0)
      .Write(unk.unk1)
      .Write(unk.unk2);
  }

  buffer.Write(command.unk5)
    .Write(command.unk6)
    .Write(command.unk7)
    .Write(command.unk8)
    .Write(command.unk9);

  for (auto& unk : command.unk10)
  {
    buffer.Write(unk.horseTID)
      .Write(unk.unk0)
      .Write(unk.unk1)
      .Write(unk.unk2)
      .Write(unk.unk3)
      .Write(unk.unk4)
      .Write(unk.unk5)
      .Write(unk.unk6)
      .Write(unk.unk7);
  }

  buffer.Write(command.unk11);

  buffer.Write(command.unk12);
}

void RanchCommandEnterRanchOK::Read(
  RanchCommandEnterRanchOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandEnterRanchCancel::Write(
  const RanchCommandEnterRanchCancel& command, SinkStream& buffer)
{
}

void RanchCommandEnterRanchCancel::Read(
  RanchCommandEnterRanchCancel& command, SourceStream& buffer)
{
}

void RanchCommandEnterRanchNotify::Write(
  const RanchCommandEnterRanchNotify& command, SinkStream& buffer)
{
  WriteRanchPlayer(buffer, command.player);
}

void RanchCommandEnterRanchNotify::Read(
  RanchCommandEnterRanchNotify& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRanchSnapshot::Write(
  const RanchCommandRanchSnapshot& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRanchSnapshot::Read(
  RanchCommandRanchSnapshot& command, SourceStream& buffer)
{
  buffer.Read(command.unk0);

  auto snapshotLength = buffer.Size() - buffer.GetCursor();
  command.snapshot.resize(snapshotLength);
  buffer.Read(command.snapshot.data(), snapshotLength);
}

void RanchCommandRanchSnapshotNotify::Write(
  const RanchCommandRanchSnapshotNotify& command, SinkStream& buffer)
{
  buffer.Write(command.ranchIndex);
  buffer.Write(command.unk0);
  buffer.Write(command.snapshot.data(), command.snapshot.size());
}

void RanchCommandRanchSnapshotNotify::Read(
  RanchCommandRanchSnapshotNotify& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRanchCmdAction::Write(
  const RanchCommandRanchCmdAction& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRanchCmdAction::Read(
  RanchCommandRanchCmdAction& command, SourceStream& buffer)
{
  buffer.Read(command.unk0);

  auto length = buffer.Size() - buffer.GetCursor();
  command.snapshot.resize(length);
  buffer.Read(command.snapshot.data(), length);
}

void RanchCommandRanchCmdActionNotify::Write(
  const RanchCommandRanchCmdActionNotify& command, SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2);
}

void RanchCommandRanchCmdActionNotify::Read(
  RanchCommandRanchCmdActionNotify& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandUpdateBusyState::Write(
  const RanchCommandUpdateBusyState& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandUpdateBusyState::Read(
  RanchCommandUpdateBusyState& command, SourceStream& buffer)
{
  buffer.Read(command.busyState);
}

void RanchCommandUpdateBusyStateNotify::Write(
  const RanchCommandUpdateBusyStateNotify& command, SinkStream& buffer)
{
  buffer.Write(command.characterId)
    .Write(command.busyState);
}

void RanchCommandUpdateBusyStateNotify::Read(
  RanchCommandUpdateBusyStateNotify& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandLeaveRanch::Write(
  const RanchCommandLeaveRanch& command, SinkStream& buffer)
{
}

void RanchCommandLeaveRanch::Read(
  RanchCommandLeaveRanch& command, SourceStream& buffer)
{
}

void RanchCommandLeaveRanchOK::Write(
  const RanchCommandLeaveRanchOK& command, SinkStream& buffer)
{
}

void RanchCommandLeaveRanchOK::Read(
  RanchCommandLeaveRanchOK& command, SourceStream& buffer)
{
}

void RanchCommandLeaveRanchNotify::Write(
  const RanchCommandLeaveRanchNotify& command, SinkStream& buffer)
{
  buffer.Write(command.characterId);
}

void RanchCommandLeaveRanchNotify::Read(
  RanchCommandLeaveRanchNotify& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandHeartbeat::Write(
  const RanchCommandHeartbeat& command,
  SinkStream& buffer)
{
}

void RanchCommandHeartbeat::Read(
  RanchCommandHeartbeat& command,
  SourceStream& buffer) {}

void RanchCommandRanchStuff::Write(
  const RanchCommandRanchStuff& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRanchStuff::Read(
  RanchCommandRanchStuff& command,
  SourceStream& buffer)
{
  buffer.Read(command.eventId)
    .Read(command.value);
}

void RanchCommandRanchStuffOK::Write(
  const RanchCommandRanchStuffOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.eventId)
    .Write(command.moneyIncrement)
    .Write(command.totalMoney);
}

void RanchCommandRanchStuffOK::Read(
  RanchCommandRanchStuffOK& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandSearchStallion::Write(
  const RanchCommandSearchStallion& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandSearchStallion::Read(
  RanchCommandSearchStallion& command, SourceStream& buffer)
{
  buffer.Read(command.unk0)
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
    buffer.Read(listSize);
    for (size_t j = 0; j < listSize; j++)
    {
      uint32_t value;
      buffer.Read(value);
      command.unk9[i].push_back(value);
    }
  }

  buffer.Read(command.unk10);
}

void RanchCommandSearchStallionCancel::Write(
  const RanchCommandSearchStallionCancel& command, SinkStream& buffer)
{
}

void RanchCommandSearchStallionCancel::Read(
  RanchCommandSearchStallionCancel& command, SourceStream& buffer)
{
}

void RanchCommandSearchStallionOK::Write(
  const RanchCommandSearchStallionOK& command, SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.unk1);

  buffer.Write(static_cast<uint8_t>(command.stallions.size()));
  for (auto& unk : command.stallions)
  {
    buffer.Write(unk.unk0)
      .Write(unk.unk1)
      .Write(unk.unk2)
      .Write(unk.name)
      .Write(unk.grade)
      .Write(unk.chance)
      .Write(unk.price)
      .Write(unk.unk7)
      .Write(unk.unk8)
      .Write(unk.stats.agility)
      .Write(unk.stats.spirit)
      .Write(unk.stats.speed)
      .Write(unk.stats.strength)
      .Write(unk.stats.ambition)
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
  RanchCommandSearchStallionOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}


void RanchCommandEnterBreedingMarket::Write(
  const RanchCommandEnterBreedingMarket& command, SinkStream& buffer)
{
}

void RanchCommandEnterBreedingMarket::Read(
  RanchCommandEnterBreedingMarket& command, SourceStream& buffer)
{
}


void RanchCommandEnterBreedingMarketCancel::Write(
  const RanchCommandEnterBreedingMarketCancel& command, SinkStream& buffer)
{
}

void RanchCommandEnterBreedingMarketCancel::Read(
  RanchCommandEnterBreedingMarketCancel& command, SourceStream& buffer)
{
}


void RanchCommandEnterBreedingMarketOK::Write(
  const RanchCommandEnterBreedingMarketOK& command, SinkStream& buffer)
{
  buffer.Write(static_cast<uint8_t>(command.availableHorses.size()));
  for (auto& availableHorse : command.availableHorses)
  {
    buffer.Write(availableHorse.uid)
      .Write(availableHorse.tid)
      .Write(availableHorse.unk0)
      .Write(availableHorse.unk1)
      .Write(availableHorse.unk2)
      .Write(availableHorse.unk3);
  }
}

void RanchCommandEnterBreedingMarketOK::Read(
  RanchCommandEnterBreedingMarketOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}


void RanchCommandTryBreeding::Write(
  const RanchCommandTryBreeding& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandTryBreeding::Read(
  RanchCommandTryBreeding& command, SourceStream& buffer)
{
  buffer.Read(command.unk0)
    .Read(command.unk1);
}


void RanchCommandTryBreedingCancel::Write(
  const RanchCommandTryBreedingCancel& command, SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.unk1)
    .Write(command.unk2)
    .Write(command.unk3)
    .Write(command.unk4)
    .Write(command.unk5);
}

void RanchCommandTryBreedingCancel::Read(
  RanchCommandTryBreedingCancel& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}


void RanchCommandTryBreedingOK::Write(
  const RanchCommandTryBreedingOK& command, SinkStream& buffer)
{
  buffer.Write(command.uid)
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
    .Write(command.stats.spirit)
    .Write(command.stats.speed)
    .Write(command.stats.strength)
    .Write(command.stats.ambition)
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
  RanchCommandTryBreedingOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}


void RanchCommandBreedingWishlist::Write(
  const RanchCommandBreedingWishlist& command, SinkStream& buffer)
{
}

void RanchCommandBreedingWishlist::Read(
  RanchCommandBreedingWishlist& command, SourceStream& buffer)
{
}


void RanchCommandBreedingWishlistCancel::Write(
  const RanchCommandBreedingWishlistCancel& command, SinkStream& buffer)
{
}

void RanchCommandBreedingWishlistCancel::Read(
  RanchCommandBreedingWishlistCancel& command, SourceStream& buffer)
{
}


void RanchCommandBreedingWishlistOK::Write(
  const RanchCommandBreedingWishlistOK& command, SinkStream& buffer)
{
  buffer.Write(static_cast<uint8_t>(command.wishlist.size()));
  for (auto& wishlistElement : command.wishlist)
  {
    buffer.Write(wishlistElement.unk0)
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
      .Write(wishlistElement.stats.agility)
      .Write(wishlistElement.stats.spirit)
      .Write(wishlistElement.stats.speed)
      .Write(wishlistElement.stats.strength)
      .Write(wishlistElement.stats.ambition)
      .Write(wishlistElement.parts.skinId)
      .Write(wishlistElement.parts.maneId)
      .Write(wishlistElement.parts.tailId)
      .Write(wishlistElement.parts.faceId)
      .Write(wishlistElement.appearance.scale)
      .Write(wishlistElement.appearance.legLength)
      .Write(wishlistElement.appearance.legVolume)
      .Write(wishlistElement.appearance.bodyLength)
      .Write(wishlistElement.appearance.bodyVolume)
      .Write(wishlistElement.unk9)
      .Write(wishlistElement.unk10)
      .Write(wishlistElement.unk11);
  }
}

void RanchCommandBreedingWishlistOK::Read(
  RanchCommandBreedingWishlistOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}


void RanchCommandUpdateMountNickname::Write(
  const RanchCommandUpdateMountNickname& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandUpdateMountNickname::Read(
  RanchCommandUpdateMountNickname& command, SourceStream& buffer)
{
  buffer.Read(command.unk0)
    .Read(command.nickname)
    .Read(command.unk1);
}


void RanchCommandUpdateMountNicknameCancel::Write(
  const RanchCommandUpdateMountNicknameCancel& command, SinkStream& buffer)
{
  buffer.Write(command.unk0);
}

void RanchCommandUpdateMountNicknameCancel::Read(
  RanchCommandUpdateMountNicknameCancel& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandUpdateMountNicknameOK::Write(
  const RanchCommandUpdateMountNicknameOK& command, SinkStream& buffer)
{
  buffer.Write(command.unk0)
    .Write(command.nickname)
    .Write(command.unk1)
    .Write(command.unk2);
}

void RanchCommandUpdateMountNicknameOK::Read(
  RanchCommandUpdateMountNicknameOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRequestStorage::Write(
  const RanchCommandRequestStorage& command,
  SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRequestStorage::Read(RanchCommandRequestStorage& command, SourceStream& buffer)
{
  buffer.Read(command.val0).Read(command.val1);
}

void RanchCommandRequestStorageOK::Write(
  const RanchCommandRequestStorageOK& command,
  SinkStream& buffer)
{
  buffer.Write(command.val0)
    .Write(command.val1)
    .Write(command.val2);

  buffer.Write(static_cast<uint8_t>(command.val3.size()));
  for (const auto& item : command.val3)
  {
    buffer.Write(item.uid)
      .Write(item.val1)
      .Write(item.val2)
      .Write(item.val3)
      .Write(item.val4)
      .Write(item.val5)
      .Write(item.val6)
      .Write(item.val7)
      .Write(item.val8)
      .Write(item.val9);
  }
}

void RanchCommandRequestStorageOK::Read(RanchCommandRequestStorageOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRequestStorageCancel::Write(
  const RanchCommandRequestStorageCancel& command,
  SinkStream& buffer)
{
  buffer.Write(command.val0).Write(command.val1);
}

void RanchCommandRequestStorageCancel::Read(
  RanchCommandRequestStorageCancel& command,
  SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRequestNpcDressList::Write(
  const RanchCommandRequestNpcDressList& command, SinkStream& buffer)
{
  throw std::logic_error("Not implemented.");
}

void RanchCommandRequestNpcDressList::Read(
  RanchCommandRequestNpcDressList& command, SourceStream& buffer)
{
  buffer.Read(command.unk0);
}

void RanchCommandRequestNpcDressListOK::Write(
  const RanchCommandRequestNpcDressListOK& command, SinkStream& buffer)
{
  buffer.Write(command.unk0);
  buffer.Write(static_cast<uint8_t>(command.dressList.size()));
  for (const auto& item : command.dressList)
  {
    buffer.Write(item);
  }
}

void RanchCommandRequestNpcDressListOK::Read(
  RanchCommandRequestNpcDressListOK& command, SourceStream& buffer)
{
  throw std::logic_error("Not implemented.");
}


void RanchCommandRequestNpcDressListCancel::Write(
  const RanchCommandRequestNpcDressListCancel& command, SinkStream& buffer)
{
  // Empty
}

void RanchCommandRequestNpcDressListCancel::Read(
  RanchCommandRequestNpcDressListCancel& command, SourceStream& buffer)
{
  // Empty
}


} // namespace alicia
