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

#include <ranges>
#include <spdlog/spdlog.h>

namespace server::network
{

Client::Client(
  ClientId clientId,
  asio::ip::tcp::socket&& socket,
  EventHandlerInterface& networkEventHandler) noexcept
  : _clientId(clientId)
  , _socket(std::move(socket))
  , _networkEventHandler(networkEventHandler)
{
}

void Client::Begin()
{
  _shouldRun = true;

  _networkEventHandler.OnClientConnected(_clientId);
  ReadLoop();
}

void Client::End()
{
  _shouldRun = false;

  try
  {
    if (_socket.is_open())
    {
      _socket.shutdown(asio::socket_base::shutdown_both);
      _socket.close();
    }
  }
  catch (const std::exception& x)
  {
    spdlog::error("Client ", x.what());
  }

  _networkEventHandler.OnClientDisconnected(_clientId);
}

void Client::QueueWrite(WriteSupplier writeSupplier)
{
  if (not _shouldRun.load(std::memory_order::relaxed))
    return;

  // Write the bytes to the write buffer that is being sent to the client.
  {
    std::scoped_lock lock(_send_mutex);
    writeSupplier(_writeBuffer);
  }

  // Asynchronously write the data to the socket.
  _socket.async_write_some(
    _writeBuffer.data(),
    [this](const boost::system::error_code& error, const std::size_t size)
    {
      std::scoped_lock lock(_send_mutex);

      try
      {
        if (error)
        {
          switch (error.value())
          {
            case asio::error::operation_aborted:
              throw std::runtime_error("Connection aborted by the server");
            case asio::error::connection_reset:
              throw std::runtime_error("Connection reset by the client");
            default:
              throw std::runtime_error(
                std::format("Generic network error {}", error.message()));
          }
        }

        // Consume the sent bytes.
        _writeBuffer.consume(size);
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Exception in the write chain of client {}: {}",
          _clientId,
          x.what());
        End();
      }
    });
}

void Client::ReadLoop() noexcept
{
  if (!_shouldRun)
    return;

  _socket.async_read_some(
    _readBuffer.prepare(1024),
    [this](boost::system::error_code error, std::size_t size)
    {
      try
      {
        if (error)
        {
          switch (error.value())
          {
            case asio::error::operation_aborted:
              throw std::runtime_error("Connection aborted by the server");
            case asio::error::misc_errors::eof:
            case asio::error::connection_reset:
              throw std::runtime_error("Connection reset by the client");
            default:
              throw std::runtime_error(
                std::format("Generic network error {}", error.message()));
          }
        }

        _readBuffer.commit(size);

        const std::span receivedData{
          static_cast<const std::byte*>(_readBuffer.data().data()),
          _readBuffer.data().size()};

        const auto consumedBytes = _networkEventHandler.OnClientData(
          _clientId,
          receivedData);

        _readBuffer.consume(consumedBytes);

        // Continue the read loop.
        ReadLoop();
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Exception in the read chain of client {}: {}",
          _clientId,
          x.what());

        End();
      }
    });
}

Server::Server(EventHandlerInterface& networkEventHandler) noexcept
  : _acceptor(_io_ctx)
  , _networkEventHandler(networkEventHandler)
{
}

void Server::Begin(const asio::ip::address& address, uint16_t port)
{
  const asio::ip::tcp::endpoint server_endpoint(address, port);

  _acceptor.open(server_endpoint.protocol());

  try
  {
    _acceptor.bind(server_endpoint);
  }
  catch (const std::exception& x)
  {
    spdlog::error(
      "Failed to host server on {}:{}: {}",
      address.to_string(),
      port,
      x.what());
  }

  _acceptor.listen();

  // Run the accept loop.
  AcceptLoop();

  _io_ctx.run();
}

void Server::End()
{
  // Disconnect all the clients.
  for (auto& client : _clients | std::views::values)
  {
    client.End();
  }

  _acceptor.close();
  _io_ctx.stop();
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
    [&](const boost::system::error_code& error, asio::ip::tcp::socket client_socket)
    {
      try
      {
        if (error)
        {
          throw std::runtime_error(
            fmt::format("Network exception 0x{}", error.value()));
        }

        // Sequential Id.
        const ClientId clientId = _client_id++;

        // Create the client.
        const auto [itr, emplaced] = _clients.try_emplace(
          clientId,
          clientId,
          std::move(client_socket),
          _networkEventHandler);

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

} // namespace server::network
