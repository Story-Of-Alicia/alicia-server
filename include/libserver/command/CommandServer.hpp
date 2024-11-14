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
#include "libserver/base/Server.hpp"

#include <unordered_map>
#include <queue>

namespace alicia
{

//! A command handler.
using CommandHandler = std::function<void(ClientId, SourceStream&)>;
//! A command supplier.
using CommandSupplier = std::function<void(SinkStream&)>;

//! A command client.
class CommandClient
{
public:
  CommandClient();

  void ResetCode();
  void RollCode();
  std::array<std::byte, 4>& GetRollingCode();

private:
  std::queue<CommandSupplier> _commandQueue;
  std::array<std::byte, 4> _rollingCode;
};

//! A command server.
class CommandServer
{
public:
  //! Default constructor;
  CommandServer();

  //! Hosts the command server on the specified interface with the provided port.
  //! Runs the processing loop and blocks until exception or stopped.
  //! @param interface Interface address.
  //! @param port Port.
  void Host(const std::string& interface, uint16_t port);

  //! Registers a command handler.
  //! @param command Id of the command.
  //! @param handler Handler of the command.
  void RegisterCommandHandler(
    CommandId command,
    CommandHandler handler);

  void ResetCode(ClientId client);

  //!
  void QueueCommand(
    ClientId client,
    CommandId command,
    CommandSupplier supplier);

private:
  std::unordered_map<CommandId, CommandHandler> _handlers{};
  std::unordered_map<ClientId, CommandClient> _clients{};

  Server _server;
};

} // namespace alicia

#endif //COMMAND_SERVER_HPP
