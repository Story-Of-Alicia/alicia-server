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

#include "libserver/network/Server.hpp"

#include "proto/ChatterMessageDefinitions.hpp"

#include <functional>

namespace server
{

//! An interface for handler of server events.
class IChatterServerEventsHandler
{
public:
  virtual ~IChatterServerEventsHandler() = default;

  virtual void HandleClientConnected(network::ClientId clientId) = 0;
  virtual void HandleClientDisconnected(network::ClientId clientId) = 0;
};

//! An interface for handler of chatter commands.
class IChatterCommandHandler
{
public:
  virtual ~IChatterCommandHandler() = default;

  virtual void HandleChatterLogin(
    network::ClientId clientId,
    const protocol::ChatCmdLogin& command) = 0;
};

class ChatterServer final
  : public network::EventHandlerInterface
{
public:
  ChatterServer(
    IChatterServerEventsHandler& chatterServerEventsHandler,
    IChatterCommandHandler& chatterCommandHandler);
  ~ChatterServer();

  void BeginHost(network::asio::ip::address_v4 address, uint16_t port);
  void EndHost();

  template<typename T>
  void QueueCommand(network::ClientId clientId, std::function<T()> commandSupplier)
  {
    _server.GetClient(clientId).QueueWrite([commandSupplier = std::move(commandSupplier)](
      network::asio::streambuf& buf)
    {
      // todo: this templated function should just write the bytes to the buffer,
      //       rest of the logic should be moved to non-templated function which deals with buffer directly.

      const auto buffer = buf.prepare(4092);
      SinkStream bufferSink({
        static_cast<std::byte*>(buffer.data()),
        buffer.size()});

      // reserve the space for the header
      bufferSink.Write(0);

      // write the command data
      T command = commandSupplier();
      bufferSink.Write(command);

      const protocol::ChatterCommandHeader header {
        .length = static_cast<uint16_t>(bufferSink.GetCursor()),
        .commandId = static_cast<uint16_t>(T::GetCommand()),};

      bufferSink.Seek(0);
      bufferSink.Write(header.length)
        .Write(header.commandId);

      // scramble the message
      SourceStream bufferSource({
        static_cast<std::byte*>(buffer.data()),
        header.length});

      bufferSink.Seek(0);

      constexpr std::array XorCode{
        static_cast<std::byte>(0x2B),
        static_cast<std::byte>(0xFE),
        static_cast<std::byte>(0xB8),
        static_cast<std::byte>(0x02)};

      while (bufferSource.GetCursor() != bufferSource.Size())
      {
        std::byte val;
        bufferSource.Read(val);
        val ^= XorCode[(bufferSource.GetCursor() - 1) % 4];
        bufferSink.Write(val);
      }

      buf.commit(bufferSource.GetCursor());
      return bufferSource.GetCursor();
    });
  }

private:
  void OnClientConnected(network::ClientId clientId) override;
  void OnClientDisconnected(network::ClientId clientId) override;
  size_t OnClientData(network::ClientId clientId, const std::span<const std::byte>& data) override;

  IChatterServerEventsHandler& _chatterServerEventsHandler;
  IChatterCommandHandler& _chatterCommandHandler;

  network::Server _server;
  std::thread _serverThread;
};

} // namespace server

#endif // CHATTER_SERVER_HPP
