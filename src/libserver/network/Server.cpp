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

#include "libserver/util/Deferred.hpp"

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
  if (_shouldRun.exchange(true, std::memory_order::acq_rel))
    return;

  _networkEventHandler.OnClientConnected(_clientId);

  ReadLoop();
}

void Client::End()
{
  if (not _shouldRun.exchange(false, std::memory_order::seq_cst))
    return;

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
    spdlog::error("Exception ending client: {}", x.what());
  }

  _networkEventHandler.OnClientDisconnected(_clientId);
}

void Client::QueueWrite(WriteSupplier writeSupplier)
{
  if (not _shouldRun.load(std::memory_order::acquire))
    return;

  {
    std::scoped_lock lock(_writeMutex);
    _writeQueue.emplace(writeSupplier);
  }

  WriteLoop();
}

void Client::WriteLoop() noexcept
{
  // todo: forgive me for this, its not clean, its not pretty and i'm pretty sure there some side effects
  //       i'll nuke it in the future

  if (not _shouldRun.load(std::memory_order::acquire))
    return;

  if (_isSending.load(std::memory_order::acquire))
    return;

  std::scoped_lock lock(_writeMutex);
  if (_writeQueue.empty())
  {
    return;
  }

  // Write the suppliers to the write buffer.
  while (not _writeQueue.empty())
  {
    const auto& supplier = _writeQueue.front();
    supplier(_writeBuffer);
    _writeQueue.pop();
  }

  _isSending.store(true, std::memory_order::release);

  // Asynchronously write the data to the socket.
  _socket.async_write_some(
    _writeBuffer.data(),
    [clientPtr = this->shared_from_this()](const boost::system::error_code& error, const std::size_t size)
    {
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
        {
          std::scoped_lock lock(clientPtr->_writeMutex);
          clientPtr->_writeBuffer.consume(size);
        }
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Exception in the write chain of client {}: {}",
          clientPtr->_clientId,
          x.what());

        clientPtr->End();
      }

      clientPtr->_isSending.store(false, std::memory_order::release);
      clientPtr->WriteLoop();
    });
}

void Client::ReadLoop() noexcept
{
  if (not _shouldRun.load(std::memory_order::acquire))
    return;

  _socket.async_read_some(
    _readBuffer.prepare(1024),
    [clientPtr = this->shared_from_this()](boost::system::error_code error, std::size_t size)
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

        clientPtr->_readBuffer.commit(size);

        const std::span receivedData{
          static_cast<const std::byte*>(clientPtr->_readBuffer.data().data()),
          clientPtr->_readBuffer.data().size()};

        const auto consumedBytes = clientPtr->_networkEventHandler.OnClientData(
          clientPtr->_clientId,
          receivedData);

        clientPtr->_readBuffer.consume(consumedBytes);

        // Continue the read loop.
        clientPtr->ReadLoop();
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Exception in the read chain of client {}: {}",
          clientPtr->_clientId,
          x.what());

        clientPtr->End();
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
  _acceptor.close();
  _io_ctx.stop();
}

std::shared_ptr<Client> Server::GetClient(ClientId clientId)
{
  const auto clientItr = _clients.find(clientId);
  if (clientItr == _clients.end())
  {
    throw std::runtime_error("Invalid client");
  }

  return clientItr->second->shared_from_this();
}

void Server::OnClientConnected(
  ClientId clientId)
{
  _networkEventHandler.OnClientConnected(clientId);
}

void Server::OnClientDisconnected(
  ClientId clientId)
{
  _networkEventHandler.OnClientDisconnected(clientId);
  _clients.erase(clientId);
}

size_t Server::OnClientData(
  ClientId clientId,
  const std::span<const std::byte>& data)
{
  return _networkEventHandler.OnClientData(clientId, data);
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
          std::make_shared<Client>(clientId,
            std::move(client_socket),
            *this));

        // Id is sequential so emplacement should never fail.
        assert(emplaced);

        itr->second->Begin();

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
