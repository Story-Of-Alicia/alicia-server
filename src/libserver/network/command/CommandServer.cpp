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

namespace server
{

namespace
{

//! Max size of the command data.
constexpr std::size_t MaxCommandDataSize = 4092;

//! Max size of the whole command payload.
//! That is command data size + size of the message magic.
constexpr std::size_t MaxCommandSize = MaxCommandDataSize + sizeof(protocol::MessageMagic);

//! Reads every specified byte of the source stream,
//! performs XOR operation on that byte with the specified sliding key
//! and writes the result to the sink stream.
//!
//! @param key Xor Key
//! @param source Source stream.
//! @param sink Sink stream.
void XorAlgorithm(
  const protocol::XorCode& key,
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

bool IsMuted(protocol::Command id)
{
  return id == protocol::Command::AcCmdCLHeartbeat
    || id == protocol::Command::AcCmdCRHeartbeat
    || id == protocol::Command::AcCmdCRRanchSnapshot
    || id == protocol::Command::AcCmdCRRanchSnapshotNotify;
}

} // namespace

void CommandClient::SetCode(protocol::XorCode code)
{
  this->_rollingCode = code;
}

void CommandClient::RollCode()
{
  auto& rollingCode = *reinterpret_cast<int32_t*>(
    _rollingCode.data());

  rollingCode = rollingCode * protocol::XorMultiplier;
  rollingCode = protocol::XorControl - rollingCode;
}

const protocol::XorCode& CommandClient::GetRollingCode() const
{
  return _rollingCode;
}

int32_t CommandClient::GetRollingCodeInt() const
{
  return *reinterpret_cast<const int32_t*>(_rollingCode.data());
}

CommandServer::CommandServer(
  EventHandlerInterface& networkEventHandler)
  : _eventHandler(networkEventHandler)
  , _serverNetworkEventHandler(*this)
  , _server(_serverNetworkEventHandler)
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
  if (not _serverThread.joinable())
    return;

  _server.End();
  _serverThread.join();
}

void CommandServer::SetCode(ClientId client, protocol::XorCode code)
{
  _clients[client].SetCode(code);
}

CommandServer::NetworkEventHandler::NetworkEventHandler(
  CommandServer& commandServer)
  : _commandServer(commandServer)
{
}

void CommandServer::NetworkEventHandler::OnClientConnected(
  network::ClientId clientId)
{
  _commandServer._eventHandler.HandleClientConnected(clientId);
}

void CommandServer::NetworkEventHandler::OnClientDisconnected(
  network::ClientId clientId)
{
  _commandServer._eventHandler.HandleClientDisconnected(clientId);
}

size_t CommandServer::NetworkEventHandler::OnClientData(
  network::ClientId clientId,
  const std::span<const std::byte>& data)
{
  SourceStream commandStream(data);

  while (commandStream.GetCursor() != commandStream.Size())
  {
    // Store the origin cursor position before reading the command.
    // This is so we can return to it if the command is not completely buffered.
    const auto streamOrigin = commandStream.GetCursor();
    bool isCommandBufferedWhole = true;

    const Deferred deferredResetCommandStreamCursor(
      [streamOrigin, &commandStream, &isCommandBufferedWhole]()
      {
        // If the command was not buffered whole,
        // reset the stream to the cursor before the command was read,
        // so that it may be read when more data arrive.
        if (not isCommandBufferedWhole)
          commandStream.Seek(streamOrigin);
      });

    // Read the message magic.
    uint32_t magicValue{};
    commandStream.Read(magicValue);

    const auto magic = protocol::decode_message_magic(magicValue);

    // Command ID must be within the valid range.
    if (magic.id > static_cast<uint16_t>(protocol::Command::Count))
    {
      throw std::runtime_error(
        std::format(
          "Invalid command magic: Bad command ID '{}'",
          magic.id)
          .c_str());
    }

    // The provided payload length must be at least the size
    // of the magic itself and smaller than the max command size.
    if (magic.length < sizeof(protocol::MessageMagic) || magic.length > MaxCommandSize)
    {
      throw std::runtime_error(
        std::format(
          "Invalid command magic: Bad command data size '{}'",
          magic.length)
          .c_str());
    }

    // Size of the data portion of the command.
    const size_t commandDataSize = static_cast<size_t>(magic.length) - sizeof(protocol::MessageMagic);
    // The size of data that is available and was not yet read.
    const size_t bufferedDataSize = commandStream.Size() - commandStream.GetCursor();

    // If all the required command data are not buffered,
    // wait for them to arrive.
    if (commandDataSize > bufferedDataSize)
    {
      isCommandBufferedWhole = false;
      break;
    }

    // Buffer for the command data.
    std::array<std::byte, MaxCommandDataSize> commandDataBuffer{};

    // Read the command data.
    commandStream.Read(
      commandDataBuffer.data(),
      commandDataSize);

    SourceStream commandDataStream(nullptr);

    auto& client = _commandServer._clients[clientId];

    const auto commandId = static_cast<protocol::Command>(magic.id);

    // Validate and process the command data.
    if (commandDataSize > 0)
    {
      client.RollCode();

      const uint32_t code = client.GetRollingCodeInt();

      // Extract the padding from the code.
      const auto padding = code & 7;

      // The command is malformed if the provided command data
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

      if (_commandServer.debugIncomingCommandData
        && not IsMuted(commandId))
      {
        spdlog::debug("Read data for command '{}' (0x{:X}),\n\n"
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
          util::GenerateByteDump({commandDataBuffer.data(), commandDataSize}));
      }
    }

    // Find the handler of the command.
    const auto handlerIter = _commandServer._handlers.find(commandId);
    if (handlerIter == _commandServer._handlers.cend())
    {
      if (_commandServer.debugCommands
        && not IsMuted(commandId))
      {
        spdlog::warn(
          "Unhandled command '{}' (0x{:x})",
          GetCommandName(commandId),
          magic.id);
      }
    }
    else
    {
      const auto& handler = handlerIter->second;
      // Handler validity is checked when registering.
      assert(handler);

      try
      {
        // Call the handler.
        handler(clientId, commandDataStream);
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Unhandled exception handling command '{}' (0x{:x}): {}",
          protocol::GetCommandName(commandId),
          magic.id,
          x.what());
      }

      // There shouldn't be any left-over data in the stream.
      assert(commandDataStream.GetCursor() == commandDataStream.Size());

      if (_commandServer.debugCommands
        && not IsMuted(commandId))
      {
        spdlog::debug(
          "Handled command '{}' (0x{:x})",
          GetCommandName(commandId),
          magic.id);
      }
    }
  }

  return commandStream.GetCursor();
}

void CommandServer::SendCommand(
  ClientId clientId,
  protocol::Command commandId,
  CommandSupplier supplier)
{
  try
  {
    _server.GetClient(clientId).QueueWrite(
      [this, commandId, supplier = std::move(supplier)](asio::streambuf& writeBuffer)
      {
        const auto mutableBuffer = writeBuffer.prepare(MaxCommandSize);
        const auto writeBufferView = std::span(
          static_cast<std::byte*>(mutableBuffer.data()),
          mutableBuffer.size());

        SinkStream commandSink(writeBufferView);

        const auto streamOrigin = commandSink.GetCursor();
        commandSink.Seek(streamOrigin + sizeof(protocol::MessageMagic));

        // Write the message data.
        supplier(commandSink);

        // Command size is the size of the whole command.
        const uint16_t commandSize = commandSink.GetCursor();

        if (debugOutgoingCommandData
          && not IsMuted(commandId))
        {
          spdlog::debug("Write data for command '{}' (0x{:X}),\n\n"
            "Command data size: {} \n"
            "Data dump: \n\n{}\n",
            GetCommandName(commandId),
            static_cast<uint32_t>(commandId),
            commandSize,
            util::GenerateByteDump(
              std::span(
                static_cast<std::byte*>(mutableBuffer.data()) + sizeof(protocol::MessageMagic),
                commandSize - sizeof(protocol::MessageMagic))));
        }

        // Traverse back the stream before the message data,
        // and write the message magic.
        commandSink.Seek(streamOrigin);

        // Write the message magic.
        const protocol::MessageMagic magic{
          .id = static_cast<uint16_t>(commandId),
          .length = commandSize};

        commandSink.Write(encode_message_magic(magic));
        writeBuffer.commit(magic.length);

        if (debugCommands
          && not IsMuted(commandId))
        {
          spdlog::debug("Sent command message '{}' (0x{:X})",
          GetCommandName(commandId),
          static_cast<uint32_t>(commandId));
        }

        return commandSize;
      });
  }
  catch (std::exception& x)
  {
    // the client disconnected, todo dont use client ids, or dont
  }
}

} // namespace server
