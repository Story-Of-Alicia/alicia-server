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

// The base XOR scrambling constant, which seems to not roll.
constexpr std::array XorCode{
  static_cast<std::byte>(0x2B),
  static_cast<std::byte>(0xFE),
  static_cast<std::byte>(0xB8),
  static_cast<std::byte>(0x02)};

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

void ChatterServer::HandleNetworkTick()
{
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

  while (commandStream.GetCursor() != commandStream.Size())
  {
    const auto origin = commandStream.GetCursor();

    const auto bufferedDataSize = commandStream.Size() - commandStream.GetCursor();

    // If there's not enough buffered data to read the header,
    // break out of the loop.
    if (bufferedDataSize < sizeof(protocol::ChatterCommandHeader))
      break;

    // Read the header.
    protocol::ChatterCommandHeader header{};
    commandStream.Read(header.length)
      .Read(header.commandId);

    // Decrypt the header.
    header.length ^= *reinterpret_cast<const uint16_t*>(XorCode.data());
    header.commandId ^= *reinterpret_cast<const uint16_t*>(XorCode.data() + 2);

    // If the the length of the command is notat least the size of the header
    // or is more than 4KB discard the command.
    if (header.length < sizeof(protocol::ChatterCommandHeader) ||  header.length > 4092)
    {
      break;
    }

    // todo: verify length, verify command, consume the rest of data even if handler does not exist.

    // If there's not enough data to read the command
    // restore the read cursor so the command may be processed later when more data arrive.
    if (bufferedDataSize < header.length)
    {
      commandStream.Seek(origin);
      break;
    }

    const auto commandDataLength = header.length - sizeof(protocol::ChatterCommandHeader);
    std::vector<std::byte> commandData(commandDataLength);

    SinkStream commandDataSink({commandData.begin(), commandData.end()});

    // Read the command data from the command stream.
    for (uint64_t idx = 0; idx < commandDataLength; ++idx)
    {
      std::byte& val = commandData[idx];
      commandStream.Read(val);
      val ^= XorCode[(commandStream.GetCursor() - 1) % 4];
    }

    SourceStream commandDataSource({commandData.begin(), commandData.end()});

    switch (header.commandId)
    {
      case static_cast<uint16_t>(protocol::ChatterCommand::ChatCmdLogin):
      {
        // TODO: deserialization and handler call
        protocol::ChatCmdLogin command;
        commandDataSource.Read(command);
        _chatterCommandHandler.HandleChatterLogin(clientId, command);
        break;
      }
      case static_cast<uint16_t>(protocol::ChatterCommand::ChatCmdLetterList):
      {
        protocol::ChatCmdLetterList command;
        commandDataSource.Read(command);
        _chatterCommandHandler.HandleChatterLetterList(clientId, command);
        break;
      }
      case static_cast<uint16_t>(protocol::ChatterCommand::ChatCmdLetterSend):
      {
        protocol::ChatCmdLetterSend command;
        commandDataSource.Read(command);
        _chatterCommandHandler.HandleChatterLetterSend(clientId, command);
        break;
      }
      case static_cast<uint16_t>(protocol::ChatterCommand::ChatCmdLetterRead):
      {
        protocol::ChatCmdLetterRead command;
        commandDataSource.Read(command);
        _chatterCommandHandler.HandleChatterLetterRead(clientId, command);
        break;
      }
      case static_cast<uint16_t>(protocol::ChatterCommand::ChatCmdGuildLogin):
      {
        protocol::ChatCmdGuildLogin command;
        commandDataSource.Read(command);
        _chatterCommandHandler.HandleChatterGuildLogin(clientId, command);
        break;
      }
      default:
      {
        spdlog::warn("Unhandled chatter: {}", header.commandId);
        break;
      }
    }
  }

  return commandStream.GetCursor();
}

network::asio::ip::address_v4 ChatterServer::GetClientAddress(const network::ClientId clientId)
{
  return _server.GetClient(clientId)->GetAddress();
}

} // namespace server