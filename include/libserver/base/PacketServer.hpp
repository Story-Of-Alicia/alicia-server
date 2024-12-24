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

#ifndef PACKET_SERVER_HPP
#define PACKET_SERVER_HPP

#include "libserver/Util.hpp"

#include "libserver/base/Server.hpp"

#include <spdlog/spdlog.h>

#include <unordered_map>
#include <utility>
#include <queue>

namespace alicia
{

//! Max size of the packet's payload.
constexpr std::size_t MaxPayloadDataSize = 4092;

//! A packet handler.
using RawPacketHandler = std::function<void(ClientId, SourceStream&)>;

//! A packet supplier.
using PacketSupplier = std::function<void(SinkStream&)>;

//! A packet client.
class PacketClient
{
private:
  std::queue<PacketSupplier> _packetQueue;
};

//! A packet server.
template<typename PacketId, class PacketClientClass>
class PacketServer
{
public:
  //! Default constructor;
  PacketServer(std::string name);

  //! Hosts the packet server on the specified interface with the provided port.
  //! Runs the processing loop and blocks until exception or stopped.
  //! @param interface Interface address.
  //! @param port Port.
  void Host(const asio::ip::address& address, uint16_t port);

  //! Registers a packet handler.
  //!
  //! @param packetId ID of the packet to register the handler for.
  //! @param handler Handler function.
  void RegisterPacketHandler(
    PacketId packetId,
    RawPacketHandler handler);

  //! Registers a packet handler.
  //!
  //! @param packetId ID of the packet to register the handler for.
  //! @param handler Handler function.
  template<typename T>
  void RegisterPacketHandler(
    PacketId packetId,
    std::function<void(ClientId clientId, const T&)> handler)
  {
    RegisterPacketHandler(
      packetId,
      [=](ClientId clientId, SourceStream& source)
      {
        T packet;
        T::Read(packet, source);
        handler(clientId, packet);
      });
  }

  //!
  void QueuePacket(
    ClientId client,
    PacketId packetId,
    PacketSupplier supplier);

protected:
  void LogBytes(std::span<std::byte> data);

  //!
  void HandleClientConnect(ClientId clientId);

  //!
  void HandleClientDisconnect(ClientId clientId);

  //! Splits the incoming raw packet data into its packet ID, size and payload.
  //!
  //! @param clientId Client ID.
  //! @param client Client.
  //! @param packetStream Stream containing the raw packet data.
  //! @return Packet ID, payload size and its data.
  virtual std::tuple<PacketId, size_t, std::array<std::byte, MaxPayloadDataSize>> PreprocessReceivedPacket(ClientId clientId, PacketClientClass& client, SourceStream& packetStream);

  //! Writes an outgoing packet's header and through the supplier, it's payload, to the packetSink.
  //!
  //! @param packet Packet ID.
  //! @param supplier Supplier of the packet's payload.
  //! @param packetSink Sink stream to write the packet to.
  //! @return Size of the written packet.
  virtual size_t WriteOutgoingPacket(PacketId packetId, PacketSupplier supplier, SinkStream& packetSink);

  //! Translates a packet ID to its name.
  //!
  //! @param packetId Packet ID.
  //! @return Packet name.
  virtual std::string_view GetPacketName(PacketId packetId);
  virtual bool IsMuted(PacketId id);
  
    
  std::string _name;

  std::unordered_map<PacketId, RawPacketHandler> _handlers{};
  std::unordered_map<ClientId, PacketClientClass> _clients{};


private:
  //!
  void HandleClientRead(
    ClientId clientId,
    asio::streambuf& readBuffer);

  //!
  void HandleClientWrite(
    ClientId clientId,
    asio::streambuf& writeBuffer);


  Server _server;
};

template<typename PacketId, class PacketClientClass>
PacketServer<PacketId, PacketClientClass>::PacketServer(std::string name) : _server(
  [this](ClientId clientId)
  {
    HandleClientConnect(clientId);
  },
  [this](ClientId clientId)
  {
    HandleClientDisconnect(clientId);
  },
  [this](
    ClientId clientId,
    asio::streambuf& readBuffer)
  {
    HandleClientRead(clientId, readBuffer);
  },
  [this](
    ClientId clientId,
    asio::streambuf& writeBuffer)
  {
    HandleClientWrite(clientId, writeBuffer);
  })
{
  _name = std::move(name);
}

template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::Host(
  const asio::ip::address& address,
  uint16_t port)
{
  _server.Host(address, port);
  spdlog::debug("{} server hosted on %s:%d", this->_name, address.to_string(), port);
}

template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::RegisterPacketHandler(
  PacketId packet,
  RawPacketHandler handler)
{
  if (!handler)
  {
    return;
  }

  _handlers[packet] = std::move(handler);
}

template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::QueuePacket(
  ClientId client,
  PacketId packetId,
  PacketSupplier supplier)
{
  // ToDo: Actual queue.
  _server.GetClient(client).QueueWrite(
    [this, client, packetId, supplier = std::move(supplier)](asio::streambuf& writeBuffer)
    {
      const auto mutableBuffer = writeBuffer.prepare(MaxPayloadDataSize);
      const auto writeBufferView = std::span(
        static_cast<std::byte*>(mutableBuffer.data()),
        mutableBuffer.size());

      SinkStream packetSink(writeBufferView);

      size_t writtenDataSize = this->WriteOutgoingPacket(packetId, supplier, packetSink);

      writeBuffer.commit(writtenDataSize);

      if(!this->IsMuted(packetId))
      {
        spdlog::debug("Sent to client {} packet '{}'",
          client,
          this->GetPacketName(packetId));
        this->LogBytes({ (std::byte *) mutableBuffer.data(), writtenDataSize});
      }
    });
}

template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::LogBytes(std::span<std::byte> data)
{
  if(data.size() == 0) {
    return;
  }

  char rowString[17];
  memset(rowString, 0, 17);

  int column = 0;
  for (int i = 0; i < data.size(); ++i) {
    column = i%16;

    if(i > 0)
    {
      switch(column)
      {
        case 0:
          printf("\t%s\n", rowString);
          memset(rowString, 0, 17);
          break;
        case 8:
          printf(" ");
          break;
      }
    }

    std::byte datum = data[i];
    if(datum >= std::byte(32) && datum <= std::byte(126)) {
      rowString[column] = (char)datum;
    } else {
      rowString[column] = '.';
    }

    printf(" %02X", static_cast<unsigned int>(datum));
  }
  printf("%*s\t%s\n\n", (16-column)*3, "", rowString);
}


//!
template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::HandleClientConnect(ClientId clientId)
{
  spdlog::info("Client {} connected to {}", clientId, _name);
}

//!
template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::HandleClientDisconnect(ClientId clientId)
{
  spdlog::info("Client {} disconnected from {}", clientId, _name);
}

template<typename PacketId, class PacketClientClass>
std::tuple<PacketId, size_t, std::array<std::byte, MaxPayloadDataSize>> PacketServer<PacketId, PacketClientClass>::PreprocessReceivedPacket(ClientId clientId, PacketClientClass& client, SourceStream& packetStream)
{
  // Subclasses must implement this method.
  throw std::logic_error("Not implemented.");
}

template<typename PacketId, class PacketClientClass>
size_t PacketServer<PacketId, PacketClientClass>::WriteOutgoingPacket(PacketId packetId, PacketSupplier supplier, SinkStream& packetSink)
{
  // Subclasses must implement this method.
  throw std::logic_error("Not implemented.");
}

template<typename PacketId, class PacketClientClass>
std::string_view PacketServer<PacketId, PacketClientClass>::GetPacketName(PacketId packetId)
{
  // Subclasses must implement this method.
  throw std::logic_error("Not implemented.");
}

template<typename PacketId, class PacketClientClass>
bool PacketServer<PacketId, PacketClientClass>::IsMuted(PacketId id)
{
  // Subclasses must implement this method.
  throw std::logic_error("Not implemented.");
}

//!
template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::HandleClientRead(
  ClientId clientId,
  asio::streambuf& readBuffer)
{
  auto& client = _server.GetClient(clientId);
  auto& packetClient = _clients[clientId];

  SourceStream packetStream({
    static_cast<const std::byte*>(readBuffer.data().data()),
    readBuffer.data().size()
  });

  // Flag indicates whether to consume the bytes
  // when the read handler exits.
  bool consumeBytesOnExit = true;

  // Defer the consumption of the bytes from the read buffer,
  // until the end of the function.
  const Deferred deferredConsume([&]()
  {
    if (!consumeBytesOnExit)
      return;

    // Consume the amount of bytes that were
    // read from the packet stream.
    readBuffer.consume(packetStream.GetCursor());
  });

  PacketId packetId;
  size_t payloadDataSize;
  std::array<std::byte, MaxPayloadDataSize> payloadDataBuffer;
  try
  {
    std::tie(packetId, payloadDataSize, payloadDataBuffer) = this->PreprocessReceivedPacket(clientId, packetClient, packetStream);
  }
  catch(const std::overflow_error& e)
  {
    // If all the required packet data are not buffered,
    // wait for them to arrive.
    // Indicate that the bytes read until now
    // shouldn't be consumed, as we expect more data to arrive.
    consumeBytesOnExit = false;
    return;
  }

  if(!this->IsMuted(packetId))
  {
    spdlog::debug("Received packet '{}', Length: {},",
      this->GetPacketName(packetId),
      payloadDataSize);
    this->LogBytes({payloadDataBuffer.data(), payloadDataSize});
  }

  // Find the handler of the packet.
  const auto handlerIter = _handlers.find(packetId);
  if (handlerIter == _handlers.cend())
  {
    if(!this->IsMuted(packetId))
    {
      spdlog::warn("Unhandled packet '{}', Length: {}",
        this->GetPacketName(packetId),
        payloadDataSize);
    }
  }
  else
  {
    const auto& handler = handlerIter->second;
    // Handler validity is checked when registering.
    assert(handler);

    // Call the handler.
    SourceStream packetDataStream = SourceStream({payloadDataBuffer.begin(), payloadDataSize});
    handler(clientId, packetDataStream);
  
    // There shouldn't be any left-over data in the stream.
    assert(packetDataStream.GetCursor() == packetDataStream.Size());

    if(!this->IsMuted(packetId))
    {
      spdlog::debug("Handled packet '{}', Length: {}",
          GetPacketName(packetId),
          payloadDataSize);
    }
  }
}

//!
template<typename PacketId, class PacketClientClass>
void PacketServer<PacketId, PacketClientClass>::HandleClientWrite(
  ClientId clientId,
  asio::streambuf& writeBuffer) {}

} // namespace alicia

#endif //PACKET_SERVER_HPP
