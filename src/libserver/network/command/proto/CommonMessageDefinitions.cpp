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

#include "libserver/network/command/proto/CommonMessageDefinitions.hpp"

#include <cassert>

namespace server::protocol
{

void AcCmdCRInviteUser::Read(
  AcCmdCRInviteUser& command,
  SourceStream& stream)
{
  stream.Read(command.recipientCharacterUid)
    .Read(command.recipientCharacterName);
}

void AcCmdCRInviteUser::Write(
  const AcCmdCRInviteUser&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteUserCancel::Read(
  AcCmdCRInviteUserCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteUserCancel::Write(
  const AcCmdCRInviteUserCancel& command,
  SinkStream& stream)
{
  stream.Write(command.recipientCharacterUid)
    .Write(command.recipientCharacterName);
}

void AcCmdCRInviteUserOK::Read(
  AcCmdCRInviteUserOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRInviteUserOK::Write(
  const AcCmdCRInviteUserOK& command,
  SinkStream& stream)
{
  stream.Write(command.recipientCharacterUid)
    .Write(command.recipientCharacterName);
}

void AcCmdCRRequestGuildRankingInfoList::Read(
  AcCmdCRRequestGuildRankingInfoList& command,
  SourceStream& stream)
{
  stream.Read(command.unk0)
    .Read(command.unk1);
}

void AcCmdCRRequestGuildRankingInfoList::Write(
  const AcCmdCRRequestGuildRankingInfoList&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildRankingInfoListCancel::Read(
  AcCmdCRRequestGuildRankingInfoListCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildRankingInfoListCancel::Write(
  const AcCmdCRRequestGuildRankingInfoListCancel&,
  SinkStream&)
{
  // Empty
}

void AcCmdCRRequestGuildRankingInfoListOK::Read(
  AcCmdCRRequestGuildRankingInfoListOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildRankingInfoListOK::Write(
  const AcCmdCRRequestGuildRankingInfoListOK& command,
  SinkStream& stream)
{
  // Max 0x80 (128) ranking info in list
  assert(command.list.size() <= 0x80);
  stream.Write(static_cast<uint8_t>(command.list.size()));
  for (const auto& info : command.list)
  {
    stream.Write(info.rank)
      .Write(info.unk1)
      .Write(info.unk2)
      .Write(info.unk3)
      .Write(info.unk4)
      .Write(info.score)
      .Write(info.unk6)
      .Write(info.name)
      .Write(info.unk8)
      .Write(info.unk9);
  }
}

void AcCmdCRRequestGuildRankingInfo::Read(
  AcCmdCRRequestGuildRankingInfo& command,
  SourceStream& stream)
{
  stream.Read(command.guildName);
}

void AcCmdCRRequestGuildRankingInfo::Write(
  const AcCmdCRRequestGuildRankingInfo&,
  SinkStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildRankingInfoCancel::Read(
  AcCmdCRRequestGuildRankingInfoCancel&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildRankingInfoCancel::Write(
  const AcCmdCRRequestGuildRankingInfoCancel&,
  SinkStream&)
{
  // Empty
}

void AcCmdCRRequestGuildRankingInfoOK::Read(
  AcCmdCRRequestGuildRankingInfoOK&,
  SourceStream&)
{
  throw std::runtime_error("Not implemented");
}

void AcCmdCRRequestGuildRankingInfoOK::Write(
  const AcCmdCRRequestGuildRankingInfoOK& command,
  SinkStream& stream)
{
  stream.Write(command.guildUid)
    .Write(command.creationDate)
    .Write(command.memberCount)
    .Write(command.guildName)
    .Write(command.emblemRevision)
    .Write(command.overallRanking)
    .Write(command.totalWins)
    .Write(command.totalLosses)
    .Write(command.unk8)
    .Write(command.score)
    .Write(command.unk10)
    .Write(command.seasonalWins)
    .Write(command.seasonalLosses)
    .Write(command.unk13)
    .Write(command.leaderName);
}

} // namespace server::protocol

