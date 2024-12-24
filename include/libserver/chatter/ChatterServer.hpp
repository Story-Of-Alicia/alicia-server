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

#ifndef CHATTER_SERVER_HPP
#define CHATTER_SERVER_HPP

#include "libserver/base/PacketServer.hpp"
#include "libserver/base/Server.hpp"

namespace alicia
{

// TODO: Define ChatterId
enum class ChatterId
  : uint16_t
{
    TODO = 0
};

//! A Chatter client.
class ChatterClient : PacketClient
{
};

//! A Chatter server.
class ChatterServer : public PacketServer<ChatterId, ChatterClient>
{
public:
  ChatterServer(std::string name);

protected:
  std::tuple<ChatterId, size_t, std::array<std::byte, MaxPayloadDataSize>> PreprocessReceivedPacket(ClientId clientId, ChatterClient& client, SourceStream& packetStream) override;
  size_t WriteOutgoingPacket(ChatterId packetId, PacketSupplier supplier, SinkStream& packetSink) override;
  std::string_view GetPacketName(ChatterId packetId) override;
  bool IsMuted(ChatterId id) override;
};

}

#endif //CHATTER_SERVER_HPP
