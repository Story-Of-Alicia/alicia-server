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

#include "NetworkDefinitions.hpp"

#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <queue>
#include <span>
#include <unordered_map>

#include <boost/asio.hpp>

namespace server::network
{

namespace asio = boost::asio;

//! A write handler.
using WriteSupplier = std::function<size_t(asio::streambuf&)>;

//!
class EventHandlerInterface
{
public:
  virtual ~EventHandlerInterface() = default;

  //! Handler of a network tick.
  virtual void HandleNetworkTick() = 0;

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
class Client : public std::enable_shared_from_this<Client>
{
public:
  //! Default constructor.
  //! @param socket Underlying socket (remote address is read from it).
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
  //!
  asio::ip::address_v4 GetAddress() const noexcept;

private:
  void WriteLoop() noexcept;
  //! Read loop.
  void ReadLoop() noexcept;

  //! Indicates whether the client should process I/O.
  std::atomic<bool> _shouldRun = false;

  //! A mutex for write buffer.
  std::mutex _writeMutex;
  //! A queue of write suppliers.
  std::queue<WriteSupplier> _writeQueue{};
  std::condition_variable _writeCv{};
  //! A write buffer.
  asio::streambuf _writeBuffer{};
  std::atomic<bool> _isSending = false;

  //! A read buffer.
  asio::streambuf _readBuffer{};

  //! A unique-identifier of the client.
  ClientId _clientId;
  //! Remote address of the client.
  asio::ip::address_v4 _remoteAddress;
  //! A client socket.
  asio::ip::tcp::socket _socket;
  //! A network event handling interface
  EventHandlerInterface& _networkEventHandler;
};

//! Server with event-driven acceptor, reads and writes.
class Server : public EventHandlerInterface
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
  //! @throw std::runtime_error
  void Begin(
    const asio::ip::address& address,
    uint16_t port);

  //! Ends the server.
  void End();

  //! Get client.
  std::shared_ptr<Client> GetClient(ClientId clientId);

  void HandleNetworkTick() override;
  void OnClientConnected(ClientId clientId) override;
  void OnClientDisconnected(ClientId clientId) override;
  size_t OnClientData(ClientId clientId, const std::span<const std::byte>& data) override;

private:
  struct AddressState
  {
    std::size_t activeConnections = 0;
    std::deque<std::chrono::steady_clock::time_point> connectionTimestamps;
  };

  void AcceptLoop() noexcept;
  void TickLoop() noexcept;
  bool IsConnectionThrottled(const asio::ip::address_v4& address) noexcept;
  void OnThrottleDisconnect(const asio::ip::address_v4& address) noexcept;

  asio::io_context _io_ctx;
  asio::ip::tcp::acceptor _acceptor;
  asio::steady_timer _timer;

  //! Sequential client ID.
  ClientId _client_id = 0;
  //! Map of clients.
  std::unordered_map<ClientId, std::shared_ptr<Client>> _clients;
  //! Per-address state for connection throttling.
  std::unordered_map<asio::ip::address_v4, AddressState> _addressStates;

  //! A network event handler.
  EventHandlerInterface& _networkEventHandler;
};

} // namespace server::network

#endif // SERVER_HPP
