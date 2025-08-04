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

#ifndef SERVER_HPP
#define SERVER_HPP

#include <functional>
#include <unordered_map>
#include <span>

#include <boost/asio.hpp>

namespace server::network
{

namespace asio = boost::asio;

//! Client Id.
using ClientId = std::size_t;

//! A write handler.
using WriteSupplier = std::function<void(asio::streambuf&)>;

//!
class EventHandlerInterface
{
public:
  virtual ~EventHandlerInterface() = default;

  //! Handler of client connection event.
  //! @param clientId ID of the client connected.
  virtual void OnClientConnected(ClientId clientId) = 0;

  //! Handler of client disconnection event.
  //! @param clientId ID of the client disconnected.
  virtual void OnClientDisconnected(ClientId clientId) = 0;

  //! Handler of client data event.
  //! @param clientId ID of the client that sent the data.
  //! @param data Byte buffer of the data sent.
  //! @returns Count of bytes consumed from the byte buffer.
  virtual size_t OnClientData(
    ClientId clientId,
    const std::span<const std::byte>& data) = 0;
};

//! Client with event driven reads and writes
//! to the underlying socket connection.
class Client
{
public:
  //! Default constructor.
  //! @param socket Underlying socket.
  explicit Client(
    ClientId clientId,
    asio::ip::tcp::socket&& socket,
    EventHandlerInterface& networkEventHandler) noexcept;

  //! Begins the client's asynchronous read loop.
  void Begin();
  //! Ends the client's asynchronous read loop.
  void End();
  //! Queues a write.
  void QueueWrite(WriteSupplier writeSupplier);

private:
  //! Read loop.
  void ReadLoop() noexcept;

  //! Indicates whether the client should process I/O.
  std::atomic<bool> _shouldRun = false;

  //! A read buffer.
  asio::streambuf _readBuffer{};
  //! A send mutex.
  std::mutex _send_mutex;
  //! A write buffer.
  asio::streambuf _writeBuffer{};

  //! A unique-identifier of the client.
  ClientId _clientId;
  //! A client socket.
  asio::ip::tcp::socket _socket;
  //! A network event handling interface
  EventHandlerInterface& _networkEventHandler;
};

//! Server with event-driven acceptor, reads and writes.
class Server :
  public EventHandlerInterface
{
public:
  //! Default constructor.
  explicit Server(
    EventHandlerInterface& networkEventHandler) noexcept;

  //! Begins the server on the current thread.
  //! Blocks the current thread until stopped.
  //!
  //! @param address Address of the interface to bind to.
  //! @param port Port to bind to.
  void Begin(
    const asio::ip::address& address,
    uint16_t port);

  //! Ends the server thread.
  void End();

  //! Get client.
  Client& GetClient(ClientId clientId);

  void OnClientConnected(ClientId clientId) override;
  void OnClientDisconnected(ClientId clientId) override;
  size_t OnClientData(ClientId clientId, const std::span<const std::byte>& data) override;

private:
  void AcceptLoop() noexcept;

  asio::io_context _io_ctx;
  asio::ip::tcp::acceptor _acceptor;

  //! Sequential client ID.
  ClientId _client_id = 0;
  //! Map of clients.
  std::unordered_map<ClientId, Client> _clients;

  //! A network event handler.
  EventHandlerInterface& _networkEventHandler;
};

} // namespace server

#endif // SERVER_HPP
