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

#ifndef COMMAND_SERVER_HPP
#define COMMAND_SERVER_HPP

#include "CommandProtocol.hpp"

#include "libserver/base/PacketServer.hpp"
#include "libserver/base/Server.hpp"

#include <unordered_map>
#include <queue>

namespace alicia
{

//! A command client.
class CommandClient : PacketClient
{
public:
  void SetCode(XorCode code);
  void RollCode();

  [[nodiscard]] const XorCode& GetRollingCode() const;
  [[nodiscard]] int32_t GetRollingCodeInt() const;

private:
  XorCode _rollingCode{};
};

//! A command server.
class CommandServer : public PacketServer<CommandId, CommandClient>
{
public:
  CommandServer(std::string name);

  void SetCode(ClientId client, XorCode code);

protected:
  std::tuple<CommandId, size_t, std::array<std::byte, MaxPayloadDataSize>> PreprocessReceivedPacket(ClientId clientId, CommandClient& client, SourceStream& packetStream) override;
  size_t WriteOutgoingPacket(CommandId packetId, PacketSupplier supplier, SinkStream& packetSink) override;
  std::string_view GetPacketName(CommandId packetId) override;
  bool IsMuted(CommandId id) override;
};

} // namespace alicia

#endif //COMMAND_SERVER_HPP
