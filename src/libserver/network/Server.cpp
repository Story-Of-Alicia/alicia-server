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

#include "libserver/network/Server.hpp"

#include <spdlog/spdlog.h>

namespace alicia
{

namespace
{

constexpr size_t MaxBufferSize = 4092;

} // namespace

Client::Client(
  asio::ip::tcp::socket&& socket,
  BeginHandler beginHandler,
  EndHandler endHandler,
  ReadHandler readHandler,
  WriteHandler writeHandler) noexcept
  : _beginHandler(std::move(beginHandler))
  , _endHandler(std::move(endHandler))
  , _readHandler(std::move(readHandler))
  , _writeHandler(std::move(writeHandler))
  , _socket(std::move(socket))
{
}

void Client::Begin()
{
  _shouldRun = true;
  _beginHandler();

  ReadLoop();
}

void Client::End()
{
  _shouldRun = false;

  try
  {
    _socket.shutdown(asio::socket_base::shutdown_both);
    _socket.close();
  }
  catch (const std::exception& x)
  {
    spdlog::error("Couldn't end connection", x.what());
  }

  _endHandler();
}

void Client::QueueWrite(WriteSupplier writeSupplier)
{
  // ToDo: Write & send timing.
  // ToDo: Write & send batching.
  if (not _shouldRun)
  {
    return;
  }

  // ToDo: Consider frame-based write loop instead of real-time writes.
  // Call the supplier.
  writeSupplier(_writeBuffer);
  _writeHandler(_writeBuffer);

  // Send the whole buffer.
  asio::async_write(
    _socket,
    _writeBuffer.data(),
    [&](boost::system::error_code error, std::size_t size)
    {
      try
      {
        if (error)
        {
          throw std::runtime_error(
            fmt::format("Network error (0x{}): {}", error.value(), error.what()));
        }

        // Consume the sent bytes.
        _writeBuffer.consume(size);
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Error in the client write loop: {}",
          x.what());

        End();
      }
    });
}

void Client::ReadLoop() noexcept
{
  // ToDo: Read & receive timing.
  // ToDo: Read & receive batching.
  if (!_shouldRun)
  {
    return;
  }

  // Chain the asynchronous functions.
  _socket.async_read_some(
    _readBuffer.prepare(MaxBufferSize),
    [&](boost::system::error_code error, std::size_t size)
    {
      try
      {
        if (error)
        {
          throw std::runtime_error(
            fmt::format("Network error (0x{}): {}", error.value(), error.what()));
        }

        // Commit the received bytes, so they can be read by the handler.
        _readBuffer.commit(size);
        _readHandler(_readBuffer);

        // Continue the read loop.
        ReadLoop();
      }
      catch (const std::exception& x)
      {
        End();

        spdlog::error("Error in the client read loop: {}", x.what());
        _socket.close();
      }
    });
}

Server::Server(
  ClientConnectHandler clientConnectHandler,
  ClientDisconnectHandler clientDisconnectHandler,
  ClientReadHandler clientReadHandler,
  ClientWriteHandler clientWriteHandler) noexcept
  : _clientConnectHandler(std::move(clientConnectHandler))
  , _clientDisconnectHandler(std::move(clientDisconnectHandler))
  , _clientReadHandler(std::move(clientReadHandler))
  , _clientWriteHandler(std::move(clientWriteHandler))
  , _io_ctx()
  , _acceptor(_io_ctx)
{
}

void Server::Begin(const asio::ip::address& address, uint16_t port)
{
  const asio::ip::tcp::endpoint server_endpoint(address, port);

  _acceptor.open(server_endpoint.protocol());
  _acceptor.bind(server_endpoint);
  _acceptor.listen();

  // Run the accept loop.
  AcceptLoop();

  _io_ctx.run();
}

void Server::End()
{
  // Disconnect all the clients.
  for (auto& client : _clients)
  {
    client.second.End();
  }

  _acceptor.close();
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
    [&](boost::system::error_code error, asio::ip::tcp::socket client_socket)
    {
      try
      {
        if (error)
        {
          throw std::runtime_error(
            fmt::format("Network exception (0x{}): {}", error.value(), error.what()));
        }

        // Sequential Id.
        const ClientId clientId = _client_id++;

        // Create the client.
        const auto [itr, emplaced] = _clients.try_emplace(
          clientId,
          std::move(client_socket),
          [this, clientId]()
          {
            // Invoke the connect handler.
            _clientConnectHandler(clientId);
          },
          [this, clientId]()
          {
            // Invoke the disconnect handler.
            _clientDisconnectHandler(clientId);
          },
          [this, clientId](asio::streambuf& readBuffer)
          {
            // Invoke the read handler.
            _clientReadHandler(clientId, readBuffer);
          },
          [this, clientId](asio::streambuf& readBuffer)
          {
            // Invoke the write handler.
            _clientWriteHandler(clientId, readBuffer);
          });

        // Id is sequential so emplacement should never fail.
        assert(emplaced);

        itr->second.Begin();

        // Continue the accept loop.
        AcceptLoop();
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Error in the server accept loop: {}",
          x.what());
      }
    });
}

} // namespace alicia
