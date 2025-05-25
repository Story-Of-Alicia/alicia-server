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

#include <boost/asio.hpp>

namespace alicia
{

namespace asio = boost::asio;

//! Client Id.
using ClientId = std::size_t;

//! A write handler.
using WriteSupplier = std::function<void(asio::streambuf&)>;

//! Client with event driven reads and writes
//! to the underlying socket connection.
class Client
{
public:
  //! Client IO begin handler.
  using BeginHandler = std::function<void()>;
  //! Client IO end handler.
  using EndHandler = std::function<void()>;

  //! Client write handler.
  using WriteHandler = std::function<void(asio::streambuf&)>;
  //! Client read handler.
  using ReadHandler = std::function<void(asio::streambuf&)>;

  //! Default constructor.
  //! @param socket Underlying socket.
  explicit Client(
    asio::ip::tcp::socket&& socket,
    BeginHandler beginHandler,
    EndHandler endHandler,
    ReadHandler readHandler,
    WriteHandler writeHandler) noexcept;

  //!
  void Begin();
  //!
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
  //! A write buffer.
  asio::streambuf _writeBuffer{};

  //! A begin handler.
  BeginHandler _beginHandler;
  //! An end handler.
  EndHandler _endHandler;
  //! A read handler.
  ReadHandler _readHandler;
  //! A write handler.
  WriteHandler _writeHandler;

  //! A client socket.
  asio::ip::tcp::socket _socket;
};

//! Server with event-driven acceptor, reads and writes.
class Server
{
public:
  //! Client write handler.
  using ClientWriteHandler = std::function<void(ClientId, asio::streambuf&)>;
  //! Client read handler.
  using ClientReadHandler = std::function<void(ClientId, asio::streambuf&)>;

  //! Client connect handler.
  using ClientConnectHandler = std::function<void(ClientId)>;
  //! Client disconnect handler.
  using ClientDisconnectHandler = std::function<void(ClientId)>;

  //! Default constructor.
  explicit Server(
    ClientConnectHandler clientConnectHandler,
    ClientDisconnectHandler clientDisconnectHandler,
    ClientReadHandler clientReadHandler,
    ClientWriteHandler clientWriteHandler) noexcept;

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

private:
  void AcceptLoop() noexcept;

  //! A client connect handler.
  ClientConnectHandler _clientConnectHandler;
  //! A client disconnect handler.
  ClientDisconnectHandler _clientDisconnectHandler;
  //! A client read handler.
  ClientReadHandler _clientReadHandler;
  //! A client write handler.
  ClientWriteHandler _clientWriteHandler;

  //! A flag indicating whether the server should run.
  std::atomic<bool> _shouldRun = false;

  asio::io_context _io_ctx;
  asio::ip::tcp::acceptor _acceptor;

  //! Sequential client ID.
  ClientId _client_id = 0;
  //! Map of clients.
  std::unordered_map<ClientId, Client> _clients;
};

} // namespace alicia

#endif // SERVER_HPP
