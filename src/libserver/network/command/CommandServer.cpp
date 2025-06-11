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

#include "libserver/network/command/CommandServer.hpp"
#include "libserver/util/Deferred.hpp"
#include "libserver/util/Util.hpp"

#include <ranges>

#include <spdlog/spdlog.h>

namespace alicia
{

namespace
{

//! Max size of the command data.
constexpr std::size_t MaxCommandDataSize = 4092;

//! Max size of the whole command payload.
//! That is command data size + size of the message magic.
constexpr std::size_t MaxCommandSize = MaxCommandDataSize + sizeof(MessageMagic);

//! Reads every specified byte of the source stream,
//! performs XOR operation on that byte with the specified sliding key
//! and writes the result to the sink stream.
//!
//! @param key Xor Key
//! @param source Source stream.
//! @param sink Sink stream.
void XorAlgorithm(
  const XorCode& key,
  SourceStream& source,
  SinkStream& sink)
{
  for (std::size_t idx = 0; idx < source.Size(); idx++)
  {
    const auto shift = idx % 4;

    // Read a byte.
    std::byte v;
    source.Read(v);

    // Xor it with the key.
    sink.Write(v ^ key[shift]);
  }
}

bool IsMuted(CommandId id)
{
  return id == CommandId::LobbyHeartbeat
    || id == CommandId::RanchHeartbeat
    || id == CommandId::RanchSnapshot
    || id == CommandId::RanchSnapshotNotify;
}

} // namespace

void CommandClient::SetCode(XorCode code)
{
  this->_rollingCode = code;
}

void CommandClient::RollCode()
{
  auto& rollingCode = *reinterpret_cast<int32_t*>(
    _rollingCode.data());

  rollingCode = rollingCode * XorMultiplier;
  rollingCode = XorControl - rollingCode;
}

const XorCode& CommandClient::GetRollingCode() const
{
  return _rollingCode;
}

int32_t CommandClient::GetRollingCodeInt() const
{
  return *reinterpret_cast<const int32_t*>(_rollingCode.data());
}

CommandServer::CommandServer()
  : _server(
      [this](ClientId clientId)
      {
        HandleClientConnect(clientId);
      },
      [this](ClientId clientId)
      {
        HandleClientDisconnect(clientId);
      },
      [this](ClientId clientId, asio::streambuf& readBuffer)
      {
        HandleClientRead(clientId, readBuffer);
      },
      [this](ClientId clientId, asio::streambuf& writeBuffer)
      {
        HandleClientWrite(clientId, writeBuffer);
      })
{
}

CommandServer::~CommandServer()
{
  EndHost();
}

void CommandServer::BeginHost(const asio::ip::address& address, uint16_t port)
{
  _serverThread = std::thread(
    [this, address, port]()
    {
      _server.Begin(address, port);
    });
}

void CommandServer::EndHost()
{
  _server.End();
  if (_serverThread.joinable())
  {
    _serverThread.join();
  }
}

void CommandServer::SetCode(ClientId client, XorCode code)
{
  _clients[client].SetCode(code);
}

void CommandServer::SendCommand(ClientId client, CommandId command, CommandSupplier supplier)
{
  // ToDo: Actual queue.
  _server.GetClient(client).QueueWrite(
    [client, command, supplier = std::move(supplier)](asio::streambuf& writeBuffer)
    {
      const auto mutableBuffer = writeBuffer.prepare(MaxCommandSize);
      const auto writeBufferView =
        std::span(static_cast<std::byte*>(mutableBuffer.data()), mutableBuffer.size());

      SinkStream commandSink(writeBufferView);

      const auto streamOrigin = commandSink.GetCursor();
      commandSink.Seek(streamOrigin + 4);

      // Write the message data.
      supplier(commandSink);

      // Payload is the message data size
      // with the size of the message magic.
      const uint16_t payloadSize = commandSink.GetCursor();

      // Traverse back the stream before the message data,
      // and write the message magic.
      commandSink.Seek(streamOrigin);

      // Write the message magic.
      const MessageMagic magic{
        .id = static_cast<uint16_t>(
          static_cast<uint16_t>(command)),
        .length = payloadSize};

      commandSink.Write(encode_message_magic(magic));
      writeBuffer.commit(magic.length);

      if (not IsMuted(command))
      {
        spdlog::debug("Sent command message '{}' (0x{:X})",
        GetCommandName(command),
        static_cast<uint32_t>(command));
      }
    });
}

void CommandServer::HandleClientConnect(ClientId clientId)
{
}

void CommandServer::HandleClientDisconnect(ClientId clientId)
{
}

void CommandServer::HandleClientRead(
  ClientId clientId,
  asio::streambuf& streamBuf)
{
  const auto buffer = streamBuf.data();
  SourceStream stream({static_cast<const std::byte*>(buffer.data()), buffer.size()});

  const Deferred deferredConsume([&]()
  {
    // Consume the amount of bytes that were
    // read from the command stream.
    streamBuf.consume(stream.GetCursor());
  });

  while (stream.GetCursor() != stream.Size())
  {
    const auto streamOrigin = stream.GetCursor();
    bool isCommandBufferedWhole = true;

    const Deferred resetStreamCursor([streamOrigin, &stream, &isCommandBufferedWhole]()
    {
      // If the command was not buffered whole,
      // reset the stream to the cursor before the command was read,
      // so that it may be read when more data arrive.
      if (not isCommandBufferedWhole)
        stream.Seek(streamOrigin);
    });

    // Read the message magic.
    uint32_t magicValue{};
    stream.Read(magicValue);

    const MessageMagic magic = decode_message_magic(
      magicValue);

    // Command ID must be within the valid range.
    if (magic.id > static_cast<uint16_t>(CommandId::Count))
    {
      throw std::runtime_error(
        std::format(
          "Invalid command magic: Bad command ID '{}'.",
          magic.id)
          .c_str());
    }

    const auto commandId = static_cast<CommandId>(magic.id);

    // The provided payload length must be at least the size
    // of the magic itself and smaller than the max command size.
    if (magic.length < sizeof(MessageMagic) || magic.length > MaxCommandSize)
    {
      throw std::runtime_error(
        std::format(
          "Invalid command magic: Bad command data size '{}'.",
          magic.length)
          .c_str());
    }

    // Size of the data portion of the command.
    const size_t commandDataSize = static_cast<size_t>(magic.length) - sizeof(MessageMagic);
    // The size of data that is available and was not yet read.
    const size_t bufferedDataSize = streamBuf.in_avail() - stream.GetCursor();

    // If all the required command data are not buffered,
    // wait for them to arrive.
    if (commandDataSize > bufferedDataSize)
    {
      isCommandBufferedWhole = false;
      return;
    }

    // Buffer for the command data.
    std::array<std::byte, MaxCommandDataSize> commandDataBuffer{};

    // Read the command data.
    stream.Read(
      commandDataBuffer.data(),
      commandDataSize);

    SourceStream commandDataStream(nullptr);

    auto& client = _clients[clientId];

    // Validate and process the command data.
    if (commandDataSize > 0)
    {
      client.RollCode();

      const uint32_t code = client.GetRollingCodeInt();

      // Extract the padding from the code.
      const auto padding = code & 7;

      // The command is malformed, if the provided command data
      // size is smaller or equal to the generated padding.
      if (padding >= commandDataSize)
      {
        throw std::runtime_error(
          std::format(
            "Malformed command: Bad command data size '{}', padding is {}.",
            magic.length,
            padding)
            .c_str());
      }

      const auto actualCommandDataSize = commandDataSize - padding;

      // Source stream of the command data.
      SourceStream dataSourceStream(
        {commandDataBuffer.begin(), commandDataSize});
      // Sink stream of the command data.
      // Correctly points to the same buffer.
      SinkStream dataSinkStream(
        {commandDataBuffer.begin(), commandDataBuffer.end()});

      // Apply XOR algorithm to the data.
      XorAlgorithm(
        client.GetRollingCode(),
        dataSourceStream,
        dataSinkStream);

      commandDataStream = std::move(SourceStream(
        {commandDataBuffer.begin(), actualCommandDataSize}));

      if (!IsMuted(commandId))
      {
        spdlog::debug("Processed data for command message '{}' (0x{:X}),\n\n"
          "XOR code: {:#X},\n"
          "Command data size: {} (padding: {}),\n"
          "Actual command data size: {}\n"
          "Processed data dump: \n\n{}\n",
          GetCommandName(commandId),
          magic.id,
          code,
          commandDataSize,
          padding,
          actualCommandDataSize,
          GenerateByteDump({commandDataBuffer.data(), commandDataSize}));
      }
    }
    else
    {
    }

    // Find the handler of the command.
    const auto handlerIter = _handlers.find(commandId);
    if (handlerIter == _handlers.cend())
    {
      if (!IsMuted(commandId))
      {
        spdlog::warn("Unhandled command message '{}', ID: 0x{:x}, Length: {}", GetCommandName(commandId), magic.id, magic.length);
      }
    }
    else
    {
      const auto& handler = handlerIter->second;
      // Handler validity is checked when registering.
      assert(handler);

      // Call the handler.
      handler(clientId, commandDataStream);

      // There shouldn't be any left-over data in the stream.
      assert(commandDataStream.GetCursor() == commandDataStream.Size());

      if (!IsMuted(commandId))
      {
        spdlog::debug("Handled command messsage '{}', ID: 0x{:x}", GetCommandName(commandId), magic.id, magic.length);
      }
    }
  }
}

void CommandServer::HandleClientWrite(
  ClientId clientId,
  asio::streambuf& writeBuffer)
{
}

} // namespace alicia
