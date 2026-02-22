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

#ifndef ALICIA_SERVER_RECORD_HPP
#define ALICIA_SERVER_RECORD_HPP

#include "libserver/event/Event.hpp"

#include <functional>
#include <shared_mutex>
#include <utility>

namespace server
{

//! Record holds a non-owning pointer to any value along with the access mutex of that value.
//! A record provies two access methods to the underlying value:
//! - An immutable (view) access which requests a shared lock of the value.
//! - A mutable (patch) access which requests an exclusive lock of the value.
template <typename Data>
class Record
{
public:
  //! A patch listener.
  using PatchListener = std::function<void()>;
  //! A consumer with immutable (view) access of the underlying value.
  using ImmutableConsumer = std::function<void(const Data&)>;
  //! A consumer with mutable (patch) access of the underlying value.
  using MutableConsumer = std::function<void(Data&)>;

  //! Constructor initializing an empty record.
  Record()
    : _mutex(nullptr)
    , _value(nullptr)
  {
  }

  //! Constructor initializing a record.
  //! @param value Pointer to value.
  //! @param mutex Pointer to value's mutex.
  //! @param patchListener A patch listener.
  Record(
    Data *const value,
    std::shared_mutex *const mutex,
    PatchListener  patchListener)
    : _mutex(mutex)
    , _lock(*_mutex, std::defer_lock)
    , _patchListener(std::move(patchListener))
    , _value(value)
  {
  }

  ~Record() = default;

  Record(const Record&) = delete;
  void operator=(const Record&) = delete;

  //! Move constructor.
  //! @param other Record to move from.
  Record(Record&& other) noexcept
    : _mutex(other._mutex)
    , _lock(std::move(other._lock))
    , _patchListener(std::move(other._patchListener))
    , _value(other._value)
  {
  }

  //! Move assignment operator.
  //! @param other Record to move from.
  Record& operator=(Record&& other) noexcept
  {
    _mutex = other._mutex;
    _lock = std::move(other._lock);
    _patchListener = std::move(_patchListener);
    _value = other._value;

    return *this;
  }

  //! Returns whether the record value is available.
  //! @retval `true` if the value is available
  //! @retval `false` if the value is not available.
  bool IsAvailable() const noexcept
  {
    return _value != nullptr && _mutex != nullptr;
  }

  //! An overload of the bool operator.
  //! Checks whether the record value is available.
  //! @retval `true` if the value is available
  //! @retval `false` if the value is not available.
  explicit operator bool() const noexcept
  {
    return IsAvailable();
  }

  //! Immutable shared access to the underlying data.
  //! @param consumer Consumer that receives the data.
  //! @throws std::runtime_error if the value is unavailable.
  void Immutable(ImmutableConsumer consumer) const
  {
    if (not IsAvailable())
      throw std::runtime_error("Value of the record is unavailable");

    // Lock the value for shared access.
    std::shared_lock lock(*_mutex);
    consumer(*_value);
  }

  //! Access to the underlying data.
  //! @param consumer Consumer that receives the data.
  //! @throws std::runtime_error if the value is unavailable.
  void Mutable(MutableConsumer consumer) const
  {
    if (not IsAvailable())
      throw std::runtime_error("Value of the record is unavailable");

    // Lock the value for exclusive access
    std::scoped_lock lock(*_mutex);
    consumer(*_value);
    _patchListener();
  }

private:
  //! An access mutex of the value.
  mutable std::shared_mutex* _mutex;
  //! A unique lock.
  mutable std::unique_lock<std::shared_mutex> _lock;
  //! A patch listener.
  PatchListener _patchListener;
  //! A value.
  Data* _value;
};

} // namespace servr

#endif // ALICIA_SERVER_RECORD_HPP
