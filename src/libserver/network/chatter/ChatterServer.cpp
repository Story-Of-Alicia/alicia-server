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
  uint32_t val0{};
  std::string characterName{};
  uint32_t code{};
  uint32_t val1{};
};

alicia::ChatterServer::ChatterServer()
  : _server(
      [](ClientId clientId) {},
      [](ClientId clientId) {},
      [](ClientId clientId, asio::streambuf& readBuffer)
      {
        const auto buffer = readBuffer.data();

        alicia::SourceStream stream({static_cast<const std::byte*>(buffer.data()), buffer.size()});

        std::array<std::byte, 4092> dataBuffer;
        SinkStream dataSinkStream(
          {dataBuffer.begin(), dataBuffer.end()});

        constexpr std::array XorCode{
          static_cast<std::byte>(0x2B),
          static_cast<std::byte>(0xFE),
          static_cast<std::byte>(0xB8),
          static_cast<std::byte>(0x02)};
        while (stream.GetCursor() != stream.Size())
        {
          std::byte val;
          stream.Read(val);
          val ^= XorCode[(stream.GetCursor() - 1) % 4];
          dataSinkStream.Write(val);
        }

        spdlog::debug(
          "Chatter message:\n\n"
          "Size: {}\n"
          "Dump: \n\n{}\n\n",
          buffer.size(),
          GenerateByteDump({dataBuffer.begin(), stream.GetCursor()}));

        readBuffer.consume(buffer.size());
      },
      [](ClientId clientId, asio::streambuf& writeBuffer) {})
{
}

alicia::ChatterServer::~ChatterServer()
{
  if (_serverThread.joinable())
    _serverThread.join();
}

void alicia::ChatterServer::Host()
{
  _serverThread = std::thread([this]()
                              { _server.Begin(boost::asio::ip::address_v4::any(), 10032); });
}
