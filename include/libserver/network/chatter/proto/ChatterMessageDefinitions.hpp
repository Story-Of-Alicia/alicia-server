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

#ifndef CHATTERMESSAGEDEFINITIONS_HPP
#define CHATTERMESSAGEDEFINITIONS_HPP

#include <libserver/network/chatter/ChatterProtocol.hpp>
#include <libserver/util/Stream.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace server::protocol
{

struct ChatCmdLogin
{
  uint32_t val0{};
  std::string name{};
  uint32_t code{};
  uint32_t val1{};

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLogin;
  }

  static void Write(
    const ChatCmdLogin& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLogin& command,
    SourceStream& stream);
};

struct ChatCmdLoginAckOK
{
  uint32_t member1 = 0;

  struct MailAlarm
  {
    uint32_t mailUid{};
    bool hasMail = false;
  } mailAlarm;

  struct Group
  {
    uint32_t uid{};
    std::string name{};
  };
  std::vector<Group> groups;

  struct Friend
  {
    uint32_t uid{};
    uint32_t categoryUid{};
    std::string name{};

    enum class Status : uint8_t
    {
      Offline = 1,
      Online = 2
    } status = Status::Offline;

    uint8_t member5{};
    uint32_t roomUid{};
    uint32_t ranchUid{};
  };
  std::vector<Friend> friends;

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckOK;
  }

  static void Write(
    const ChatCmdLoginAckOK& command,
    SinkStream& stream);

  static void Read(
    ChatCmdLoginAckOK& command,
    SourceStream& stream);
};

struct ChatCmdLoginAckCancel
{
  uint32_t member1{};

  ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckCancel;
  }

  void Write(
    const ChatCmdLoginAckCancel& command,
    SinkStream& stream);

  void Read(
    ChatCmdLoginAckCancel& command,
    SourceStream& stream);
};

class ChatCmdUpdateState
{
  uint8_t member1;
  uint32_t member2;
  uint32_t member3;
};

class ChatCmdUpdateStateAckOK
{
  std::string hostname = "127.0.0.1";
  uint16_t port = /*htons(10034)*/ 0;
  uint32_t payload = 1;
};

} // namespace server::protocol

#endif // CHATTERMESSAGEDEFINITIONS_HPP
