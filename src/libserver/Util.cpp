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

#include "libserver/Util.hpp"

#define Int32x32To64(a, b) ((uint16_t)(((uint64_t)((long)(a))) * ((long)(b))))

namespace
{

void WriteCString(const std::string& value, alicia::SinkStream& buffer)
{
  for (char b : value)
  {
    buffer.Write(b);
  }

  buffer.Write(static_cast<char>(0x00));
}

void ReadCString(std::string& value, alicia::SourceStream& buffer)
{
  value.reserve(512);

  bool readNext = true;
  do
  {
    char read = '\0';
    buffer.Read(read);

    if (read == '\0')
    {
      readNext = false;
    }
    else
    {
      value += read;
    }
  } while (readNext);
}

} // namespace

namespace alicia
{

// Sources:
//  https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-int32x32to64
//  https://gist.github.com/JamesMenetrey/d3f494262bcab48af1d617c3d39f34cf#file-winnt-h-L944
WinFileTime UnixTimeToFileTime(const std::chrono::system_clock::time_point& timePoint)
{
  const uint64_t unixTime = timePoint.time_since_epoch().count();
  const uint64_t convertedUnixTime = Int32x32To64(unixTime, 10000000) + 116444736000000000;
  return WinFileTime{
    .dwLowDateTime = static_cast<uint32_t>(convertedUnixTime),
    .dwHighDateTime = static_cast<uint32_t>(convertedUnixTime >> 32)};
}

asio::ip::address_v4 ResolveHostName(const std::string& host)
{
  const auto address = asio::ip::make_address(host);
  if (!address.is_unspecified() && address.is_v4())
  {
    return address.to_v4();
  }

  asio::io_context ioContext;
  asio::ip::tcp::resolver resolver(ioContext);
  auto endpoints = resolver.resolve(host);

  for (const auto& endpoint : endpoints)
  {
    const auto& addr = endpoint.endpoint().address();
    if (addr.is_v4())
    {
      return addr.to_v4();
    }
  }

  return {};
}

DEFINE_WRITER_READER(std::string, WriteCString, ReadCString)

SourceStream::SourceStream(Storage buffer)
    : StreamBase(buffer)
{
}

SourceStream::SourceStream(nullptr_t)
    : StreamBase(nullptr)
{
}

SourceStream::SourceStream(SourceStream&& rhs) noexcept
    : StreamBase(rhs._storage)
{
  _cursor = rhs._cursor;
}

SourceStream& SourceStream::operator=(SourceStream&& rhs) noexcept
{
  this->_cursor = rhs._cursor;
  this->_storage = rhs._storage;
  return *this;
}

SinkStream::SinkStream(Storage buffer) noexcept
    : StreamBase(buffer)
{
}

SinkStream::SinkStream(nullptr_t) noexcept
    : StreamBase(nullptr)
{
}

SinkStream::SinkStream(SinkStream&& rhs) noexcept
    : StreamBase(rhs._storage)
{
  _cursor = rhs._cursor;
}

SinkStream& SinkStream::operator=(SinkStream&& rhs) noexcept
{
  this->_cursor = rhs._cursor;
  this->_storage = rhs._storage;
  return *this;
}

void SinkStream::Write(const void* data, std::size_t size)
{
  if (_cursor + size > _storage.size())
  {
    throw std::overflow_error(std::format(
      "Couldn't write {} bytes to the buffer (cursor: {}, available: {}). Not enough space.",
      size,
      _cursor,
      _storage.size()));
  }

  // Write the bytes.
  for (std::size_t byteIdx = 0; byteIdx < size; ++byteIdx)
  {
    _storage[_cursor++] = static_cast<const std::byte*>(data)[byteIdx];
  }
}

void SourceStream::Read(void* data, std::size_t size)
{
  if (_cursor + size > _storage.size())
  {
    throw std::overflow_error(std::format(
      "Couldn't read {} bytes to the buffer (cursor: {}, available: {}). Not enough space.",
      size,
      _cursor,
      _storage.size()));
  }

  // Read the bytes.
  for (std::size_t byteIdx = 0; byteIdx < size; ++byteIdx)
  {
    static_cast<std::byte*>(data)[byteIdx] = _storage[_cursor++];
  }
}

} // namespace alicia
