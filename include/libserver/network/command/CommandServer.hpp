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

  //! Hosts the command server on the specified interface with the provided port.
  //! Runs the processing loop and blocks until exception or stopped.
  //! @param interface Interface address.
  //! @param port Port.
  void Host(const asio::ip::address& address, uint16_t port);

  //! Registers a command handler.
  //!
  //! @param commandId ID of the command to register the handler for.
  //! @param handler Handler function.
  void RegisterCommandHandler(
    CommandId commandId,
    RawCommandHandler handler);

  //! Registers a command handler.
  //!
  //! @param commandId ID of the command to register the handler for.
  //! @param handler Handler function.
  template <typename T>
  void RegisterCommandHandler(
    CommandId commandId,
    std::function<void(ClientId clientId, const T&)> handler)
  {
    RegisterCommandHandler(
      commandId,
      [=](ClientId clientId, SourceStream& source)
      {
        T command;
        T::Read(command, source);
        handler(clientId, command);
      });
  }

  void SetCode(ClientId client, XorCode code);

  template <typename T>
  void QueueCommand(
    ClientId clientId,
    CommandId commandId,
    std::function<T()> supplier)
  {
    QueueCommand(clientId, commandId, [supplier](auto& sink)
                 {
      const auto command = supplier();
      sink.Write(supplier()); });
  }

  //!
  void QueueCommand(
    ClientId client,
    CommandId command,
    CommandSupplier supplier);

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

  std::unordered_map<CommandId, RawCommandHandler> _handlers{};
  std::unordered_map<ClientId, CommandClient> _clients{};

  Server _server;
  std::thread _serverThread;
};

} // namespace alicia

#endif // COMMAND_SERVER_HPP
