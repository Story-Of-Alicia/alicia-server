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

#include "libserver/chatter/ChatterServer.hpp"

namespace alicia
{

namespace
{

}


ChatterServer::ChatterServer(std::string name) : PacketServer(std::move(name)) {}

std::tuple<ChatterId, size_t, std::array<std::byte, MaxPayloadDataSize>> ChatterServer::PreprocessReceivedPacket(ClientId clientId, ChatterClient& chatterClient, SourceStream& commandStream)
{
  // TODO: Implement
  return {ChatterId::TODO, 0, std::array<std::byte, MaxPayloadDataSize>{}};
}

size_t ChatterServer::WriteOutgoingPacket(ChatterId ChatterId, PacketSupplier supplier, SinkStream& packetSink)
{
    // TODO: Implement
    return 0;
}

std::string_view ChatterServer::GetPacketName(ChatterId ChatterId)
{
  return "TODO";
}

bool ChatterServer::IsMuted(ChatterId id)
{
  return false;
}

}