//
// Created by rgnter on 24/08/2025.
//

#ifndef CHATTERMESSAGEDEFINITIONS_HPP
#define CHATTERMESSAGEDEFINITIONS_HPP

#include <libserver/network/chatter/ChatterProtocol.hpp>
#include <libserver/util/Stream.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace alicia::protocol
{

class ChatCmdLogin
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
    server::SinkStream& stream);

  static void Read(
    ChatCmdLogin& command,
    server::SourceStream& stream);
};

class ChatCmdLoginAckOK
{
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

  static ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckOK;
  }

  static void Write(
    const ChatCmdLoginAckOK& command,
    server::SinkStream& stream);

  static void Read(
    ChatCmdLoginAckOK& command,
    server::SourceStream& stream);
};

class ChatCmdLoginAckCancel
{
  uint32_t member1{};

  ChatterCommand GetCommand()
  {
    return ChatterCommand::ChatCmdLoginAckCancel;
  }

  void Write(
    const ChatCmdLoginAckCancel& command,
    server::SinkStream& stream);

  void Read(
    ChatCmdLoginAckCancel& command,
    server::SourceStream& stream);
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
  uint16_t port = htons(10034);
  uint32_t payload = 1;
};


} // namespace alicia::protocol

#endif //CHATTERMESSAGEDEFINITIONS_HPP
