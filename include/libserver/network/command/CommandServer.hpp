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
#include "libserver/Constants.hpp"
#include "libserver/network/Server.hpp"
#include "libserver/util/Stream.hpp"

#include <queue>
#include <unordered_map>

namespace server
{

namespace asio = network::asio;
using ClientId = network::ClientId;

//! A command handler.
using RawCommandHandler = std::function<void(ClientId, SourceStream&)>;

//! A command supplier.
using CommandSupplier = std::function<void(SinkStream&)>;

//! A command client.
class CommandClient
{
public:
  void SetCode(protocol::XorCode code);
  void RollCode();

  [[nodiscard]] const protocol::XorCode& GetRollingCode() const;
  [[nodiscard]] int32_t GetRollingCodeInt() const;

private:
  std::queue<CommandSupplier> _commandQueue;
  protocol::XorCode _rollingCode{};
};

template <typename T>
concept ReadableCommandStruct = ReadableStruct<T> and requires
{
  {T::GetCommand()};
};

template <typename T>
concept WritableCommandStruct = WritableStruct<T> and requires
{
  {T::GetCommand()};
};

//! A command server.
class CommandServer final
{
public:
  class EventHandlerInterface
  {
  public:
    virtual ~EventHandlerInterface() = default;
    virtual void HandleClientConnected(ClientId clientId) = 0;
    virtual void HandleClientDisconnected(ClientId clientId) = 0;
  };

  //! Default constructor;
  explicit CommandServer(
    EventHandlerInterface& events);
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
  template <ReadableCommandStruct C>
  void RegisterCommandHandler(
    std::function<void(ClientId clientId, const C& command)> handler)
  {
    _handlers[C::GetCommand()] = [handler](ClientId clientId, SourceStream& source)
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
    std::function<C()> supplier)
  {
    SendCommand(clientId, C::GetCommand(), [supplier](SinkStream& sink){
      C::Write(supplier(), sink);
    });
  }

  void SetCode(ClientId client, protocol::XorCode code);

private:
  class NetworkEventHandler
    : public network::EventHandlerInterface
  {
  public:
    NetworkEventHandler(CommandServer& commandServer);

    void OnClientConnected(network::ClientId clientId) override;
    void OnClientDisconnected(network::ClientId clientId) override;
    size_t OnClientData(network::ClientId clientId, const std::span<const std::byte>& data) override;

  private:
    CommandServer& _commandServer;
  };

  //!
  void SendCommand(
    ClientId clientId,
    protocol::Command commandId,
    CommandSupplier supplier);

  bool debugIncomingCommandData = constants::DebugCommands;
  bool debugOutgoingCommandData = constants::DebugCommands;
  bool debugCommands = constants::DebugCommands;

  std::unordered_map<protocol::Command, RawCommandHandler> _handlers{};
  std::unordered_map<ClientId, CommandClient> _clients{};

  EventHandlerInterface& _eventHandler;
  NetworkEventHandler _serverNetworkEventHandler;

  network::Server _server;
  std::thread _serverThread;
};

} // namespace server

#endif // COMMAND_SERVER_HPP
