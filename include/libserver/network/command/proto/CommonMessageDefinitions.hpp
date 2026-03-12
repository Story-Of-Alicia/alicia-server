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
#include <libserver/util/Stream.hpp>

#include <string>
#include <vector>

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

struct AcCmdCRRequestGuildRankingInfoList
{
  uint32_t unk0{};
  uint32_t unk1{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildRankingInfoList;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildRankingInfoList& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildRankingInfoList& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildRankingInfoListCancel
{
  // Empty

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildRankingInfoListCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildRankingInfoListCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildRankingInfoListCancel& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildRankingInfoListOK
{
  struct GuildRankInfo
  {
    //! Rank in the guild rank info list.
    //! Minimum value is `1`.
    uint32_t rank{};
    uint32_t unk1{};
    uint32_t unk2{};
    uint32_t unk3{};
    uint32_t unk4{};
    //! The score of the guild.
    //! Can be negative.
    int32_t score{};
    uint32_t unk6{};
    //! The name of the guild.
    std::string name{};
    uint32_t unk8{};
    uint32_t unk9{};
  };

  std::vector<GuildRankInfo> list{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildRankingInfoListOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildRankingInfoListOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildRankingInfoListOK& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildRankingInfo
{
  //! The name of the guild to request info of.
  std::string guildName{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildRankingInfo;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildRankingInfo& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildRankingInfo& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildRankingInfoCancel
{
  // Empty

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildRankingInfoCancel;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildRankingInfoCancel& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildRankingInfoCancel& command,
    SourceStream& stream);
};

struct AcCmdCRRequestGuildRankingInfoOK
{
  //! UID of the guild.
  //! This is the first part of the PNG in the download URL.
  uint32_t guildUid{};
  //! Date from when the guild was created.
  uint32_t creationDate{};
  //! The amount of guild members in the guild.
  uint32_t memberCount{};
  //! Name of the guild.
  std::string guildName{};
  //! Emblem revision (ID).
  //! This is the second part of the PNG in the download URL.
  uint16_t emblemRevision{};
  //! Overall ranking of the guild, in the server.
  uint32_t overallRanking{};
  //! Total wins.
  uint32_t totalWins{};
  //! Total losses.
  uint32_t totalLosses{};
  uint32_t unk8{};
  //! Guild score.
  uint32_t score{};
  uint32_t unk10{};
  //! Wins this season.
  uint32_t seasonalWins{};
  //! Losses this season.
  uint32_t seasonalLosses{};
  uint32_t unk13{};
  //! Leader of the guild.
  std::string leaderName{};

  static Command GetCommand()
  {
    return Command::AcCmdCRRequestGuildRankingInfoOK;
  }

  //! Writes the command to a provided sink stream.
  //! @param command Command.
  //! @param stream Sink stream.
  static void Write(
    const AcCmdCRRequestGuildRankingInfoOK& command,
    SinkStream& stream);

  //! Reader a command from a provided source stream.
  //! @param command Command.
  //! @param stream Source stream.
  static void Read(
    AcCmdCRRequestGuildRankingInfoOK& command,
    SourceStream& stream);
};

} // namespace server::protocol

#endif // LOBBY_MESSAGE_DEFINES_HPP
