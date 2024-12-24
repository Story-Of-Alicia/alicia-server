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

#include "libserver/command/CommandServer.hpp"
#include "libserver/Util.hpp"

#include <spdlog/spdlog.h>

namespace alicia
{

namespace
{

//! Flag indicating whether to use the XOR algorithm on recieved data.
constexpr std::size_t UseXorAlgorithm = true;

//! Max size of the whole command payload.
//! That is command data size + size of the message magic.
constexpr std::size_t MaxCommandSize = MaxPayloadDataSize + sizeof(MessageMagic);

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


} // anon namespace

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

CommandServer::CommandServer(std::string name) : PacketServer(std::move(name)) {}

void CommandServer::SetCode(ClientId client, XorCode code)
{
  _clients[client].SetCode(code);
}

std::tuple<CommandId, size_t, std::array<std::byte, MaxPayloadDataSize>> CommandServer::PreprocessReceivedPacket(ClientId clientId, CommandClient& commandClient, SourceStream& commandStream)
{
  // Read the message magic.
  uint32_t magicValue{};
  commandStream.Read(magicValue);

  const MessageMagic magic = decode_message_magic(magicValue);

  // Command ID must be within the valid range.
  if (magic.id > static_cast<uint16_t>(CommandId::Count))
  {
    throw std::runtime_error(
      std::format(
        "Invalid command magic: Bad command ID '{}'.",
        magic.id).c_str());
  }

  const auto commandId = static_cast<CommandId>(magic.id);

  // The provided payload length must be at least the size
  // of the magic itself and smaller than the max command size.
  if (magic.length < sizeof(MessageMagic)
    || magic.length > MaxCommandSize)
  {
    throw std::runtime_error(
      std::format(
        "Invalid command magic: Bad command data size '{}'.",
        magic.length).c_str());
  }

  // Size of the data portion of the command.
  size_t commandDataSize = static_cast<size_t>(magic.length) - commandStream.GetCursor();

  if(!this->IsMuted(commandId))
  {
    spdlog::debug("Received command '{}', ID: 0x{:x}, Length: {},",
      GetCommandName(commandId),
      magic.id,
      magic.length);
  }

  // Buffer for the command payload.
  std::array<std::byte, MaxPayloadDataSize> commandDataBuffer{};

  // Read the command data.
  commandStream.Read(
    commandDataBuffer.data(), commandDataSize);

  SourceStream commandDataStream(nullptr);

  // Validate and process the command data.
  if (commandDataSize > 0)
  {
    if (UseXorAlgorithm)
    {
      commandClient.RollCode();

      const uint32_t code = commandClient.GetRollingCodeInt();

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
            padding).c_str());
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
        commandClient.GetRollingCode(),
        dataSourceStream,
        dataSinkStream);

      commandDataStream = std::move(SourceStream(
        {commandDataBuffer.begin(), actualCommandDataSize}));

      if(!this->IsMuted(commandId))
      {
        spdlog::debug("Data for command '{}' (0x{:X}), Code: {:#X}, Data Size: {} (padding: {}), Actual Data Size: {}",
          GetCommandName(commandId),
          magic.id,
          code,
          commandDataSize,
          padding,
          actualCommandDataSize);
      }

      commandDataSize = actualCommandDataSize;
    }
    else
    {
      commandDataStream = std::move(SourceStream(
        {commandDataBuffer.begin(), commandDataSize}));

      if(!this->IsMuted(commandId))
      {
        spdlog::debug("Data for command '{}' (0x{:X}), Data Size: {}",
          GetCommandName(commandId),
          magic.id,
          commandDataSize);
      }
    }
  }

  return {commandId, commandDataSize, commandDataBuffer};
}

size_t CommandServer::WriteOutgoingPacket(CommandId commandId, PacketSupplier supplier, SinkStream& packetSink)
{
    const auto streamOrigin = packetSink.GetCursor();
    packetSink.Seek(streamOrigin + 4);

    // Write the message data.
    supplier(packetSink);

    // Size of the packet, including magic and payload.
    const uint16_t totalSize = packetSink.GetCursor();

    // Traverse back the stream before the message data,
    // and write the message magic.
    packetSink.Seek(streamOrigin);

    // Write the message magic.
    const MessageMagic magic{
      .id = static_cast<uint16_t>(commandId),
      .length = totalSize};

    packetSink.Write(encode_message_magic(magic));

    return totalSize;
}

std::string_view CommandServer::GetPacketName(CommandId commandId)
{
  return GetCommandName(commandId);
}

bool CommandServer::IsMuted(CommandId id)
{
  return id == CommandId::LobbyHeartbeat
      || id == CommandId::RanchHeartbeat
      || id == CommandId::RanchSnapshot
      || id == CommandId::RanchSnapshotNotify;
}

} // namespace alicia
