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

#ifndef STREAM_HPP
#define STREAM_HPP

#include <format>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace server
{

template <typename StorageType>
class StreamBase
{
public:
  //! Storage type.
  using Storage = StorageType;

  //! Default constructor.
  explicit StreamBase(Storage storage) noexcept
    : _storage(storage) {}

  //! Empty constructor.
  explicit StreamBase(std::nullptr_t) noexcept
    : _storage() {}

  //! Virtual destructor.
  virtual ~StreamBase() = default;

  //! Seeks to the cursor specified.
  //! @param cursor Cursor position.
  virtual void Seek(std::size_t cursor)
  {
    if (cursor > _storage.size())
    {
      throw std::overflow_error(
        std::format("Couldn't seek to {}. Not enough space.", cursor));
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

class SinkStream;
class SourceStream;

template <typename T>
concept Numeric = std::is_arithmetic_v<T> || std::is_enum_v<T>;

template <typename T>
concept Container = requires(T a) {
  typename T::value_type;
  typename T::iterator;
  { a.begin() } -> std::input_or_output_iterator;
  { a.end() } -> std::input_or_output_iterator;
};

template <typename T>
concept WritableStruct = requires(T value, SinkStream& stream) {
  { T::Write(value, stream) };
};

template <typename T>
concept ReadableStruct = requires(T value, SourceStream& stream) {
  { T::Read(value, stream) };
};

//! Buffered stream sink.
class SinkStream final
  : public StreamBase<std::span<std::byte>>
{
public:
  //! Default constructor
  //!
  //! @param buffer Underlying storage buffer.
  explicit SinkStream(Storage buffer) noexcept;
  //! Empty constructor
  explicit SinkStream(std::nullptr_t) noexcept;

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
  template <Numeric T>
  SinkStream& Write(const T& value)
  {
    const auto requiredByteCount = sizeof(value);
    Write(reinterpret_cast<const void*>(&value), requiredByteCount);
    return *this;
  }

  //! Write a string to the stream.
  //! Fails if the operation can'
  SinkStream& Write(const std::string& value);

  template <WritableStruct T>
  SinkStream& Write(const T& value)
  {
    T::Write(value, *this);
    return *this;
  }
};

//! Buffered stream source.
class SourceStream final
  : public StreamBase<std::span<const std::byte>>
{
public:
  //! Default constructor
  //!
  //! @param stream Source buffer.
  explicit SourceStream(Storage buffer);
  //! Empty constructor
  explicit SourceStream(std::nullptr_t);

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
  template <Numeric T>
  SourceStream& Read(T& value)
  {
    const auto byteCount = sizeof(value);
    Read(reinterpret_cast<void*>(&value), byteCount);
    return *this;
  }

  SourceStream& Read(std::string& value);

  template <ReadableStruct T>
  SourceStream& Read(T& value)
  {
    T::Read(value, *this);
    return *this;
  }
};

} // namespace server

#endif // STREAM_HPP
