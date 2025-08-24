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

#include "libserver/network/chatter/ChatterServer.hpp"
#include "libserver/util/Stream.hpp"
#include "libserver/util/Util.hpp"

#include <spdlog/spdlog.h>

constexpr bool Debug = false;

struct ChatterCommandLogin
{

};

server::ChatterServer::ChatterServer()
  : _server(*this)
{
}

server::ChatterServer::~ChatterServer()
{
  _server.End();
  if (_serverThread.joinable())
    _serverThread.join();
}

void server::ChatterServer::BeginHost()
{
  _serverThread = std::thread([this]()
    {
      _server.Begin(boost::asio::ip::address_v4::any(), 10033);
    });

  // todo: remove
  _serverThread.join();
}

void server::ChatterServer::EndHost()
{
  if (_serverThread.joinable())
  {
    _server.End();
    _serverThread.join();
  }
}

void server::ChatterServer::OnClientConnected(network::ClientId clientId)
{
}

void server::ChatterServer::OnClientDisconnected(network::ClientId clientId)
{
}

size_t server::ChatterServer::OnClientData(
  network::ClientId clientId,
  const std::span<const std::byte>& data)
{
  return 0;
}
