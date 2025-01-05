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

#ifndef UTIL_HPP
#define UTIL_HPP

#define DECLARE_WRITER_READER(x)                                                                   \
  template <> struct StreamWriter<x>                                                               \
  {                                                                                                \
    void operator()(const x& value, SinkStream& buffer) const;                                     \
  };                                                                                               \
  template <> struct StreamReader<x>                                                               \
  {                                                                                                \
    void operator()(x& value, SourceStream& buffer) const;                                         \
  };

#define DEFINE_WRITER_READER(x, writer, reader)                                                    \
  void StreamWriter<x>::operator()(const x& value, SinkStream& buffer) const                       \
  {                                                                                                \
    return writer(value, buffer);                                                                  \
  }                                                                                                \
  void StreamReader<x>::operator()(x& value, SourceStream& buffer) const                           \
  {                                                                                                \
    return reader(value, buffer);                                                                  \
  }

#define COMMAND_WRITER_READER(x)                                                                   \
  DECLARE_WRITER_READER(x)                                                                         \
  DEFINE_WRITER_READER(x, x::Write, x::Read)

#include <boost/asio.hpp>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <span>

namespace alicia
{

//! Windows file-time represents number of 100 nanosecond intervals since January 1, 1601 (UTC).
struct WinFileTime
{
  uint32_t dwLowDateTime = 0;
  uint32_t dwHighDateTime = 0;
};

namespace asio = boost::asio;

//! Converts a time point to the Windows file time.
//! @param timePoint Point in time.
//! @return Windows file time representing specified point in time.
WinFileTime UnixTimeToFileTime(const std::chrono::system_clock::time_point& timePoint);

asio::ip::address_v4 ResolveHostName(const std::string& host);

template <typename StorageType> class StreamBase
{
public:
  //! Storage type.
  using Storage = StorageType;

  //! Default constructor.
  explicit StreamBase(Storage storage) noexcept
      : _storage(storage) {};

  //! Empty constructor.
  explicit StreamBase(nullptr_t) noexcept
      : _storage() {};

  //! Virtual destructor.
  virtual ~StreamBase() = default;

  //! Seeks to the cursor specified.
  //! @param cursor Cursor position.
  virtual void Seek(std::size_t cursor)
  {
    if (cursor > _storage.size())
    {
      throw std::overflow_error(std::format("Couldn't seek to {}. Not enough space.", cursor));
    }

    _cursor = cursor;
  }

  //! Gets the size of the underlying storage.
  //! @returns Size fo the underlying storage.
  [[nodiscard]] virtual std::size_t Size() const { return _storage.size(); }

  //! Gets the cursor of the storage.
  //! @returns Cursor position.
  [[nodiscard]] virtual std::size_t GetCursor() const { return _cursor; }

protected:
  Storage _storage;
  std::size_t _cursor{};
};

//! Forward declaration of a stream writer.
template <typename T> struct StreamWriter;

//! Buffered stream sink.
class SinkStream final : public StreamBase<std::span<std::byte>>
{
public:
  //! Default constructor
  //!
  //! @param buffer Underlying storage buffer.
  explicit SinkStream(Storage buffer) noexcept;
  //! Empty constructor
  explicit SinkStream(nullptr_t) noexcept;

  //! Move constructor.
  SinkStream(SinkStream&&) noexcept;
  //! Move assignment.
  SinkStream& operator=(SinkStream&&) noexcept;

  //! Deleted copy constructor.
  SinkStream(const SinkStream&) = delete;
  //! Deleted copy assignment.
  void operator=(const SinkStream&) = delete;

  //! Writes to the buffer storage.
  //! Fails if the operation can't be completed wholly.
  //!
  //! @param data Data.
  //! @param size Size of data.
  void Write(const void* data, std::size_t size);

  //! Write a value to the sink stream.
  //!
  //! @param value Value to write.
  //! @tparam T Type of value.
  //! @return Reference to this.
  template <typename T> SinkStream& Write(const T& value)
  {
    StreamWriter<T>{}(value, *this);
    return *this;
  }
};

//! General binary stream writer.
//! Writes a little-endian byte sequence to the provided sink buffer.
//!
//! @tparam T Type of value.
template <typename T> struct StreamWriter
{
  void operator()(const T& value, SinkStream& buffer)
  {
    const auto requiredByteCount = sizeof(value);

    // Write the value to the buffer.
    buffer.Write(reinterpret_cast<const void*>(&value), requiredByteCount);
  }
};

//! Forward declaration of a reader.
template <typename T> struct StreamReader;

//! Buffered stream source.
class SourceStream final : public StreamBase<std::span<const std::byte>>
{
public:
  //! Default constructor
  //!
  //! @param stream Source buffer.
  explicit SourceStream(Storage buffer);
  //! Empty constructor
  explicit SourceStream(nullptr_t);

  //! Move constructor
  SourceStream(SourceStream&& rhs) noexcept;
  //! Move assignment.
  SourceStream& operator=(SourceStream&&) noexcept;

  //! Deleted copy constructor.
  SourceStream(const SourceStream&) = delete;
  //! Deleted copy assignement.
  void operator=(const SourceStream&) = delete;

  //! Read from the buffer storage.
  //! Fails if the operation can't be completed wholly.
  //!
  //! @param data Data.
  //! @param size Size of data.
  void Read(void* data, std::size_t size);

  //! Read a value from the source stream.
  //!
  //! @param value Value to read.
  //! @tparam T Type of value.
  //! @return Reference to this.
  template <typename T> SourceStream& Read(T& value)
  {
    StreamReader<T>{}(value, *this);
    return *this;
  }
};

//! General binary reader.
//! Reads a little-endian byte sequence from the provided source buffer.
//!
//! @tparam T Type of value.
template <typename T> struct StreamReader
{
  void operator()(T& value, SourceStream& buffer)
  {
    const auto byteCount = sizeof(value);
    buffer.Read(reinterpret_cast<void*>(&value), byteCount);
  }
};

DECLARE_WRITER_READER(std::string)

//! Performs deferred call on destruction.
struct Deferred final
{
  //! Function.
  using Fnc = std::function<void(void)>;

  //! Construct deferred call that invokes
  //! the provided function on destruction of this object.
  explicit Deferred(Fnc func) noexcept
      : _func(std::move(func)) {};

  //! Deleted copy constructor.
  Deferred(const Deferred&) noexcept = delete;
  //! Deleted move constructor.
  Deferred(Deferred&&) noexcept = delete;

  //! Destructor.
  ~Deferred() { _func(); }

private:
  //! A function to invoke on destruction.
  Fnc _func;
};

} // namespace alicia

#endif // UTIL_HPP
