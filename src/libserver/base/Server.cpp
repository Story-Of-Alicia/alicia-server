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

#include "libserver/base/Server.hpp"

namespace alicia
{

namespace
{

constexpr size_t MaxBufferSize = 4092;

} // anon namespace

Client::Client(asio::ip::tcp::socket&& socket) noexcept
    : _socket(std::move(socket))
{
}

void Client::SetReadHandler(ReadHandler readHandler)
{
  _readHandler = std::move(readHandler);
}

void Client::Begin()
{
  _shouldProcess = true;
  ReadLoop();
}

void Client::End()
{
  _shouldProcess = false;
}

void Client::QueueWrite(WriteSupplier writeSupplier)
{
  if (!_shouldProcess)
  {
    return;
  }

  // ToDo: Consider frame-based write loop instead of real-time writes.
  std::scoped_lock writeLock(_writeMutex);
  // Call the supplier.
  writeSupplier(_writeBuffer);

  // Send the whole buffer.
  asio::async_write(
    _socket,
    _writeBuffer.data(),
    [&](
      boost::system::error_code error, std::size_t size)
    {
      if (error)
      {
        printf("Failed to write\n");
        _socket.close();
        return;
      }

      // Consume the sent bytes.
      _writeBuffer.consume(size);
    });

  // ToDo: Write & send timing.
  // ToDo: Write & send batching.
}

void Client::ReadLoop() noexcept
{
  if (!_shouldProcess)
  {
    return;
  }

  // Chain the asynchronous functions.
  _socket.async_read_some(
    _readBuffer.prepare(MaxBufferSize),
    [&](boost::system::error_code error, std::size_t size)
    {
      if (error)
      {
        printf("Failed to read\n");
        _socket.close();
        return;
      }

      // Commit the received bytes, so they can be read by the handler.
      _readBuffer.commit(size);

      // ToDo: Read & handle timing.
      {
        // Call the read handler
        std::scoped_lock readLock(_readMutex);
        if (_readHandler)
        {
          _readHandler(_readBuffer);
        }
      }

      // Continue the read loop.
      ReadLoop();
    });
}

Server::Server() noexcept
  : _io_ctx()
  , _acceptor(_io_ctx)
{
}

void Server::Host(const std::string_view& interface, uint16_t port)
{
  const asio::ip::tcp::endpoint server_endpoint(
    asio::ip::make_address(interface.data()),
    port);

  _acceptor.open(server_endpoint.protocol());
  _acceptor.bind(server_endpoint);
  _acceptor.listen();

  AcceptLoop();

  _io_ctx.run();
}

void Server::SetOnConnectHandler(OnConnectHandler handler)
{
  _onConnectHandler = std::move(handler);
}
void Server::SetOnDisconnectHandler(OnDisconnectHandler handler)
{
  _onDisconnectHandler = std::move(handler);
}

Client& Server::GetClient(ClientId clientId)
{
  const auto clientItr = _clients.find(clientId);
  if (clientItr == _clients.end())
  {
    throw std::runtime_error("Invalid client Id");
  }

  return clientItr->second;
}

void Server::AcceptLoop() noexcept
{
  _acceptor.async_accept(
    [&](
      boost::system::error_code error,
      asio::ip::tcp::socket client_socket)
    {
      if (error)
      {
        printf("Failed to accept");
        return;
      }

      // Sequential Id.
      const ClientId clientId = _client_id++;

      // Create the client.
      const auto [itr, emplaced] = _clients.try_emplace(
        clientId, std::move(client_socket));

      // Id is sequential so emplacement should never fail.
      assert(emplaced);

      _onConnectHandler(clientId);
      itr->second.Begin();

      // Continue the accept loop.
      AcceptLoop();
    });
}

} // namespace alicia