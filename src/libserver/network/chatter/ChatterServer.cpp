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

#include "libserver/network/chatter/ChatterServer.hpp"
#include "libserver/util/Stream.hpp"
#include "libserver/util/Util.hpp"

#include <spdlog/spdlog.h>

namespace server
{

namespace
{

// todo: de/serializer map, handler map

} // anon namespace

ChatterServer::ChatterServer(
  IChatterServerEventsHandler& chatterServerEventsHandler,
  IChatterCommandHandler& chatterCommandHandler)
  : _chatterServerEventsHandler(chatterServerEventsHandler)
  , _chatterCommandHandler(chatterCommandHandler)
  , _server(*this)
{
}

ChatterServer::~ChatterServer()
{
  _server.End();
  if (_serverThread.joinable())
    _serverThread.join();
}

void ChatterServer::BeginHost(network::asio::ip::address_v4 address, uint16_t port)
{
  _serverThread = std::thread([this, address, port]()
  {
    _server.Begin(address, port);
  });
}

void ChatterServer::EndHost()
{
  if (_serverThread.joinable())
  {
    _server.End();
    _serverThread.join();
  }
}

void ChatterServer::OnClientConnected(network::ClientId clientId)
{
  _chatterServerEventsHandler.HandleClientConnected(clientId);
}

void ChatterServer::OnClientDisconnected(network::ClientId clientId)
{
  _chatterServerEventsHandler.HandleClientDisconnected(clientId);
}

size_t ChatterServer::OnClientData(
  network::ClientId clientId,
  const std::span<const std::byte>& data)
{
  SourceStream commandStream{data};

  // The base XOR scrambling constant, which seems to not roll.
  constexpr std::array XorCode{
    static_cast<std::byte>(0x2B),
    static_cast<std::byte>(0xFE),
    static_cast<std::byte>(0xB8),
    static_cast<std::byte>(0x02)};

  protocol::ChatterCommandHeader header;
  commandStream.Read(header.length)
    .Read(header.commandId);
  header.length ^= *reinterpret_cast<const uint16_t*>(XorCode.data());
  header.commandId ^= *reinterpret_cast<const uint16_t*>(XorCode.data() + 2);

  if (header.commandId == static_cast<uint16_t>(
    protocol::ChatterCommand::ChatCmdLogin))
  {
    std::vector<std::byte> commandData(header.length);
    SinkStream commandDataSink({commandData.begin(), commandData.end()});

    for (uint16_t idx = 0; idx < header.length - sizeof(protocol::ChatterCommandHeader); ++idx)
    {
      std::byte& val = commandData[idx];
      commandStream.Read(val);
      val ^= XorCode[(commandStream.GetCursor() - 1) % 4];
    }

    SourceStream commandDataSource({commandData.begin(), commandData.end()});

    // todo: deserialization and handler call
    protocol::ChatCmdLogin command;
    commandDataSource.Read(command);
    _chatterCommandHandler.HandleChatterLogin(clientId, command);
  }

  return commandStream.GetCursor();
}

} // namespace server