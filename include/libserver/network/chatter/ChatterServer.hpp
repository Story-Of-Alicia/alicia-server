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
#include "libserver/util/Stream.hpp"
#include "libserver/Constants.hpp"
#include "libserver/util/Util.hpp"

#include "proto/ChatterMessageDefinitions.hpp"

#include <spdlog/spdlog.h>

#include <functional>
#include <unordered_map>

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

//! A raw command handler.
using RawChatterCommandHandler = std::function<void(network::ClientId, SourceStream&)>;

//! Concept for readable command structs.
template <typename T>
concept ReadableChatterCommandStruct = ReadableStruct<T> and requires
{
  {T::GetCommand()} -> std::convertible_to<protocol::ChatterCommand>;
};

class ChatterServer final
  : public network::EventHandlerInterface
{
public:
  explicit ChatterServer(IChatterServerEventsHandler& chatterServerEventsHandler);
  ~ChatterServer();

  void BeginHost(network::asio::ip::address_v4 address, uint16_t port);
  void EndHost();

  network::asio::ip::address_v4 GetClientAddress(const network::ClientId clientId);
  void DisconnectClient(network::ClientId clientId);

  //! Registers a command handler.
  template <ReadableChatterCommandStruct C>
  void RegisterCommandHandler(
    std::function<void(network::ClientId clientId, const C& command)> handler)
  {
    _handlers[static_cast<uint16_t>(C::GetCommand())] = 
      [handler](network::ClientId clientId, SourceStream& source)
      {
        C command;
        C::Read(command, source);
        handler(clientId, command);
      };
  }

  template<typename T>
  void QueueCommand(network::ClientId clientId, std::function<T()> commandSupplier)
  {
    _server.GetClient(clientId)->QueueWrite([this, commandSupplier = std::move(commandSupplier)](
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

      if (debugOutgoingCommandData)
      {
        spdlog::debug("Write data for command '{}' (0x{:X}),\n\n"
          "Command data size: {} \n"
          "Data dump: \n\n{}\n",
          GetChatterCommandName(T::GetCommand()),
          static_cast<uint16_t>(T::GetCommand()),
          header.length,
          util::GenerateByteDump(
            std::span(
              static_cast<std::byte*>(buffer.data()) + sizeof(protocol::ChatterCommandHeader),
              header.length - sizeof(protocol::ChatterCommandHeader))));
      }

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
      
      if (debugCommands)
      {
        spdlog::debug("Sent chatter command message '{}' (0x{:X})",
          GetChatterCommandName(T::GetCommand()),
          static_cast<uint16_t>(T::GetCommand()));
      }

      buf.commit(bufferSource.GetCursor());
      return bufferSource.GetCursor();
    });
  }

private:
  void HandleNetworkTick() override;
  void OnClientConnected(network::ClientId clientId) override;
  void OnClientDisconnected(network::ClientId clientId) override;
  size_t OnClientData(network::ClientId clientId, const std::span<const std::byte>& data) override;

  IChatterServerEventsHandler& _chatterServerEventsHandler;
  std::unordered_map<uint16_t, RawChatterCommandHandler> _handlers{};

  network::Server _server;
  std::thread _serverThread;

  // Debug flags for logging command handling
  bool debugIncomingCommandData = constants::DebugCommands;
  bool debugOutgoingCommandData = constants::DebugCommands;
  bool debugCommands = constants::DebugCommands;
};

} // namespace server

#endif // CHATTER_SERVER_HPP
