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

void server::ChatterServer::OnClientConnected(
  network::ClientId clientId)
{
}

void server::ChatterServer::OnClientDisconnected(
  network::ClientId clientId)
{
}

size_t server::ChatterServer::OnClientData(
  network::ClientId clientId,
  const std::span<const std::byte>& data)
{
  SourceStream stream(data);

  std::array<std::byte, 4092> dataBuffer;
  SinkStream dataSinkStream(
    {dataBuffer.begin(), dataBuffer.end()});
  SourceStream dataSourceStream(
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

  struct Header
  {
    uint16_t size{};
    uint16_t id{};
  } header;

  dataSourceStream.Read(header.size)
    .Read(header.id);

  spdlog::debug(
    "Chatter message:\n\n"
    "Size: {}\n"
    "Dump: \n\n{}\n\n",
    dataSinkStream.GetCursor(),
    util::GenerateByteDump({dataBuffer.begin(), dataSinkStream.GetCursor()}));

  std::vector<std::byte> response;
  response.resize(0xFFFF);

  switch (header.id)
  {
    case 1:
    {
      struct Response
      {
        Header header {
          .id = 2};

        uint32_t member1 = 0;
        // something with letter alarm
        struct Something0
        {
          uint32_t member1 = 0;
          uint8_t member2 = 0;
        } member2;

        struct Category
        {
          uint32_t member1 = 0;
          std::string member2 = "";
        };

        uint16_t smth;

        std::vector<Category> categories = {
          {100, "category 1"},
          {200, "category 2"},
          {300, "category 2"},
          {0, "default"},};

        struct Ranch
        {
          uint32_t uid = 1;
          uint32_t categoryUid = 1;
          std::string name = "default";
          uint8_t member4 = 2;
          uint8_t member5 = 1;
          uint32_t member6 = 0;
          uint32_t otherUid = 1;
        };
        std::vector<Ranch> ranches = {
          {},
          {.uid = 2, .categoryUid = 100, .name = "ranch 2", .otherUid = 2},
          {.uid = 3, .categoryUid = 200, .name = "ranch 3", .otherUid = 3}};

      } chatter;

      SinkStream stream({response.begin(), response.end()});
      stream.Write(0);

      stream.Write(chatter.member1)
        .Write(chatter.member2.member1)
        .Write(chatter.member2.member2);

      stream.Write(static_cast<uint32_t>(chatter.categories.size()));

      for (const auto& category : chatter.categories)
      {
        stream.Write(category.member1)
          .Write(category.member2);
      }

      stream.Write(static_cast<uint32_t>(chatter.ranches.size()));
      for (const auto& ranch : chatter.ranches)
      {
        stream.Write(ranch.uid)
          .Write(ranch.categoryUid)
          .Write(ranch.name)
          .Write(ranch.member4)
          .Write(ranch.member5)
          .Write(ranch.member6)
          .Write(ranch.otherUid);
      }

      chatter.header.size = stream.GetCursor();

      stream.Seek(0);
      stream.Write(chatter.header.size)
        .Write(chatter.header.id);

      response.resize(chatter.header.size);

      break;
    }
    case 37:
    {
      struct Response
      {
        Header header {
          .id = 57};
        std::string hostname = "127.0.0.1";
        uint16_t port = htons(10034);
        uint32_t payload = 0;
      } chatter;

      SinkStream stream({response.begin(), response.end()});
      stream.Write(0);
      stream.Write(chatter.hostname)
        .Write(chatter.port)
        .Write(chatter.payload);

      chatter.header.size = stream.GetCursor();
      response.resize(chatter.header.size);

      stream.Seek(0);
      stream.Write(chatter.header.size)
        .Write(chatter.header.id);
    }
  }

  _server.GetClient(clientId).QueueWrite([response](boost::asio::streambuf& buf)
  {
    const auto mutableBuffer = buf.prepare(response.size());
    for (int i = 0; i < response.size(); ++i)
    {
      static_cast<std::byte*>(mutableBuffer.data())[i] = response[i] ^ XorCode[i % 4];
    }

    buf.commit(response.size());
    return response.size();
  });

  return data.size();
}
