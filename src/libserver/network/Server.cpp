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

#include <cassert>
#include <ranges>
#include <spdlog/spdlog.h>
#include <stacktrace>

namespace server::network
{

namespace
{

constexpr std::size_t MaxConnectionsPerAddress = 3;
constexpr std::size_t MaxTotalConnections = 1024;
constexpr std::size_t MaxConnectRatePerAddress = 10;
constexpr auto RateWindow = std::chrono::seconds(30);

} // namespace

Client::Client(
  ClientId clientId,
  asio::ip::tcp::socket&& socket,
  EventHandlerInterface& networkEventHandler) noexcept
  : _clientId(clientId)
  , _socket(std::move(socket))
  , _networkEventHandler(networkEventHandler)
{
  _remoteAddress = _socket.remote_endpoint().address().to_v4();
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
  catch (const std::exception&)
  {
    // Ignore
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

asio::ip::address_v4 Client::GetAddress() const noexcept
{
  return _remoteAddress;
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
    try
    {
      const auto& supplier = _writeQueue.front();
      supplier(_writeBuffer);
      _writeQueue.pop();
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unhandled exception in supplier of write data for client {}: {}",
        _clientId,
        e.what());
      End();
      return;
    }
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
        spdlog::debug(
          "Client {} is disconnecting because of read loop exception: {}",
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
        spdlog::debug(
          "Client {} is disconnecting because of read loop exception: {}",
          clientPtr->_clientId,
          x.what());

        clientPtr->End();
      }
    });
}

Server::Server(EventHandlerInterface& networkEventHandler) noexcept
  : _acceptor(_io_ctx)
  , _timer(_io_ctx)
  , _networkEventHandler(networkEventHandler)
{
}

void Server::Begin(const asio::ip::address& address, uint16_t port)
{
  const asio::ip::tcp::endpoint server_endpoint(address, port);

  try
  {
    _acceptor.open(server_endpoint.protocol());
    _acceptor.bind(server_endpoint);
    _acceptor.listen();
  }
  catch (const std::exception& x)
  {
    throw std::runtime_error(
      std::format(
        "Exception while trying to host server on {}:{}: {}",
        address.to_string(),
        port,
        x.what()));
  }

  // Run the accept loop.
  AcceptLoop();

  try
  {
    // Run the tick loop.
    TickLoop();
  }
  catch (const std::exception& x)
  {
    throw std::runtime_error(
      std::format(
        "Exception in network tick loop: {}",
        x.what()));
  }

  try
  {
    _io_ctx.run();
  }
  catch (const std::exception& x)
  {
    throw std::runtime_error(
      std::format(
        "Exception in asio IO context: {}",
        x.what()));
  }
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

void Server::HandleNetworkTick()
{
}

void Server::OnClientConnected(
  ClientId clientId)
{
  _networkEventHandler.OnClientConnected(clientId);
}

void Server::OnClientDisconnected(
  ClientId clientId)
{
  const auto clientIt = _clients.find(clientId);
  assert(clientIt != _clients.end());

  const auto address = clientIt->second->GetAddress();
  OnThrottleDisconnect(address);

  _networkEventHandler.OnClientDisconnected(clientId);

  _clients.erase(clientIt);
}

size_t Server::OnClientData(
  ClientId clientId,
  const std::span<const std::byte>& data)
{
  return _networkEventHandler.OnClientData(clientId, data);
}

bool Server::IsConnectionThrottled(const asio::ip::address_v4& address) noexcept
{
  auto& state = _addressStates[address];
  // If there are more active connections than allowed by `MaxConnectionsPerAddress`
  // throttle the connection from the address.
  if (state.activeConnections >= MaxConnectionsPerAddress)
    return true;

  const auto now = std::chrono::steady_clock::now();

  // Pop the connection timestamps which are over the rate window and have expired.
  while (not state.connectionTimestamps.empty())
  {
    const auto timeSinceConnection = now - state.connectionTimestamps.front();
    if (timeSinceConnection < RateWindow)
      break;

    state.connectionTimestamps.pop_front();
  }

  // If there are more connection attempts than allowed by `MaxConnectRatePerAddress`
  // throttle the connection from the address.
  if (state.connectionTimestamps.size() >= MaxConnectRatePerAddress)
    return true;

  state.activeConnections++;
  state.connectionTimestamps.push_back(now);

  return false;
}

void Server::OnThrottleDisconnect(const asio::ip::address_v4& address) noexcept
{
  const auto it = _addressStates.find(address);
  if (it == _addressStates.end())
  {
    return;
  }

  if (it->second.activeConnections > 0)
  {
    --it->second.activeConnections;
  }

  if (it->second.activeConnections == 0 &&
      it->second.connectionTimestamps.empty())
  {
    _addressStates.erase(it);
  }
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

        const auto remoteAddr = client_socket.remote_endpoint().address().to_v4();

        if (IsConnectionThrottled(remoteAddr))
        {
          spdlog::warn(
            "Connection rejected from {} (throttled)",
            remoteAddr.to_string());
          client_socket.close();
          AcceptLoop();
          return;
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

void Server::TickLoop() noexcept
{
  _networkEventHandler.HandleNetworkTick();

  _timer.expires_after(std::chrono::seconds(1));
  _timer.async_wait([this](const boost::system::error_code& error)
    {
      if (error)
        return;

      TickLoop();
    });
}

} // namespace server::network
