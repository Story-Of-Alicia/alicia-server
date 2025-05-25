//
// Created by rgnter on 21/05/2025.
//

#include "libserver/util/Stream.hpp"

namespace alicia
{

SourceStream::SourceStream(Storage buffer)
  : StreamBase(buffer)
{
}

SourceStream::SourceStream(std::nullptr_t)
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

SinkStream::SinkStream(std::nullptr_t) noexcept
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
    throw std::overflow_error(std::format("Couldn't write {} bytes to the buffer (cursor: {}, available: {}). Not enough space.", size, _cursor, _storage.size()));
  }

  // Write the bytes.
  for (std::size_t byteIdx = 0; byteIdx < size; ++byteIdx)
  {
    _storage[_cursor++] = static_cast<const std::byte*>(data)[byteIdx];
  }
}

SinkStream& SinkStream::Write(const std::string& value)
{
  for (char b : value)
  {
    Write(b);
  }

  Write(static_cast<char>(0x00));
  return *this;
}

void SourceStream::Read(void* data, std::size_t size)
{
  if (_cursor + size > _storage.size())
  {
    throw std::overflow_error(std::format("Couldn't read {} bytes to the buffer (cursor: {}, available: {}). Not enough space.", size, _cursor, _storage.size()));
  }

  // Read the bytes.
  for (std::size_t byteIdx = 0; byteIdx < size; ++byteIdx)
  {
    static_cast<std::byte*>(data)[byteIdx] = _storage[_cursor++];
  }
}

SourceStream& SourceStream::Read(std::string& value)
{
  value.reserve(512);

  bool readNext = true;
  do
  {
    char read = '\0';
    Read(read);

    if (read == '\0')
    {
      readNext = false;
    }
    else
    {
      value += read;
    }
  } while (readNext);
  return *this;
}

} // namespace alicia
