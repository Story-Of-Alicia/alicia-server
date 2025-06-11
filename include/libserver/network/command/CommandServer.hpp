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
#include "libserver/network/Server.hpp"
#include "libserver/util/Stream.hpp"

#include <queue>
#include <unordered_map>

namespace alicia
{

//! A command handler.
using RawCommandHandler = std::function<void(ClientId, SourceStream&)>;

//! A command supplier.
using CommandSupplier = std::function<void(SinkStream&)>;

//! A command client.
class CommandClient
{
public:
  void SetCode(XorCode code);
  void RollCode();

  [[nodiscard]] const XorCode& GetRollingCode() const;
  [[nodiscard]] int32_t GetRollingCodeInt() const;

private:
  std::queue<CommandSupplier> _commandQueue;
  XorCode _rollingCode{};
};

//! A command server.
class CommandServer
{
public:
  //! Default constructor;
  explicit CommandServer();
  ~CommandServer();

  //! Begins the server.
  //! @param address Address.
  //! @param port Port.
  void BeginHost(const asio::ip::address& address, uint16_t port);

  //! Ends the server.
  void EndHost();

  //! Registers a command handler.
  //! @param commandId ID of the command to register the handler for.
  //! @param handler Handler of the command.
  template <ReadableStruct C>
  void RegisterCommandHandler(
    CommandId commandId,
    std::function<void(ClientId clientId, const C& command)> handler)
  {
    _handlers[commandId] = [handler](ClientId clientId, SourceStream& source)
    {
      C command;
      C::Read(command, source);
      handler(clientId, command);
    };
  }

  //! Queues a command for sending.
  //! @param clientId ID of the client to send the command to.
  //! @param commandId ID of the command.
  //! @param supplier Supplier of the command.
  template <WritableStruct C>
  void QueueCommand(
    ClientId clientId,
    CommandId commandId,
    std::function<C()> supplier)
  {
    SendCommand(clientId, commandId, [supplier](SinkStream& sink){
      C::Write(supplier(), sink);
    });
  }

  void SetCode(ClientId client, XorCode code);

private:
  //!
  void HandleClientConnect(ClientId clientId);
  //!
  void HandleClientDisconnect(ClientId clientId);
  //!
  void HandleClientRead(
    ClientId clientId,
    asio::streambuf& streamBuf);
  //!
  void HandleClientWrite(
    ClientId clientId,
    asio::streambuf& writeBuffer);

  //!
  void SendCommand(
    ClientId client,
    CommandId command,
    CommandSupplier supplier);

  std::unordered_map<CommandId, RawCommandHandler> _handlers{};
  std::unordered_map<ClientId, CommandClient> _clients{};

  Server _server;
  std::thread _serverThread;
};

} // namespace alicia

#endif // COMMAND_SERVER_HPP
