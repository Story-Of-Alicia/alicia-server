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

#ifndef DATASTORAGE_HPP
#define DATASTORAGE_HPP

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <ranges>
#include <shared_mutex>
#include <span>
#include <unordered_map>
#include <unordered_set>

namespace server
{

//! Record holds a non-owning pointer to any value along with the access mutex of that value.
//! A record provies two access methods to the underlying value:
//! - A mutable access which requests an exclusive lock of the value.
//! - An immutable access which requests a shared lock of the value.
template <typename Data>
class Record
{
public:
  //! A consumer with immutable access to the underlying value.
  using ImmutableAccessConsumer = std::function<void(const Data&)>;
  //! A consumer with mutable access to the underlying value.
  using MutableAccessConsumer = std::function<void(Data&)>;

  //! Constructor initializing an empty record.
  Record()
    : _mutex(nullptr)
    , _value(nullptr)
  {
  }
  //! Constructor initializing a record.
  //! @param value Pointer to value.
  //! @param mutex Pointer to value's mutex.
  Record(Data* const value, std::shared_mutex* const mutex)
    : _mutex(mutex)
    , _lock(*_mutex, std::defer_lock)
    , _value(value)
  {
  }

  //! Default destructor.
  ~Record() = default;

  //! Deleted copy constructor.
  Record(const Record&) = delete;
  //! Deleted copy assignement operator.
  void operator=(const Record&) = delete;

  //! Move constructor.
  Record(Record&& other) noexcept
    : _mutex(other._mutex)
    , _lock(std::move(other._lock))
    , _value(other._value)
  {
  }
  //! Move assignment operator.
  //! @param other Record to move from.
  Record& operator=(Record&& other) noexcept
  {
    _mutex = other._mutex;
    _lock = std::move(other._lock);
    _value = other._value;

    return *this;
  }

  //! Returns whether the record value is available.
  //! @returns `true` if the value is available, otherwise `false`.
  bool IsAvailable() const noexcept
  {
    return _value != nullptr && _mutex != nullptr;
  }

  //! An overload for bool operator. Checks whether the record value is available.
  //! @returns `true` if the value is available, otherwise `false`.
  operator bool() const noexcept
  {
    return IsAvailable();
  }

  //! Immutable shared access to the underlying data.
  //! @param consumer Consumer that receives the data.
  //! @throws std::runtime_error if the value is unavailable.
  void Immutable(ImmutableAccessConsumer consumer) const
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
  void Mutable(MutableAccessConsumer consumer) const
  {
    if (not IsAvailable())
      throw std::runtime_error("Value of the record is unavailable");

    // Lock the value for exclusive access
    std::scoped_lock lock(*_mutex);
    consumer(*_value);
  }

  const Data& Immutable() const
  {
    if (not IsAvailable())
      throw std::runtime_error("Value of the record is unavailable");

    _lock.lock();
    return *_value;
  }

private:
  //! An access mutex of the value.
  mutable std::shared_mutex* _mutex;
  //! A unique lock.
  mutable std::unique_lock<std::shared_mutex> _lock;
  //! A value.
  Data* _value;
};

template <typename Key, typename Data>
class DataStorage
{
public:
  using KeySpan = std::span<const Key>;

  using DataSourceRetrieveListener = std::function<bool(const Key& key, Data& data)>;
  using DataSourceStoreListener = std::function<bool(const Key& key, Data& data)>;

  DataStorage(
    const DataSourceRetrieveListener& data_source_retrieve_listener,
    const DataSourceStoreListener& data_source_store_listener)
    : _dataSourceRetrieveListener(data_source_retrieve_listener)
    , _dataSourceStoreListener(data_source_store_listener)
  {
  }

  void Initialize()
  {
  }

  void Terminate()
  {
    _storeQueue.clear();
    _retrieveQueue.clear();

    for (auto& entry : _entries)
    {
      if (entry.second.available)
        _dataSourceStoreListener(entry.first, entry.second.value);
    }

    _entries.clear();
  }

  //! Whether data record is available.
  //! @param key Key of the datum.
  //! @returns `true` if datum is available, `false` otherwise.
  bool IsAvailable(const Key& key)
  {
    const auto iterator = _entries.find(key);
    if (iterator == _entries.cend())
      return false;
    return iterator->second.available;
  }

  //! Whether data records are available.
  //! @param keys Keys of the data.
  //! @returns `true` if data are available, `false` otherwise.
  bool IsAvailable(KeySpan keys)
  {
    for (const auto& key : keys)
    {
      if (not IsAvailable(key))
        return false;
    }

    return true;
  }

  Record<Data> Create(std::function<std::pair<Key, Data>()> supplier)
  {
    auto [key, data] = supplier();
    auto [it, created] = _entries.try_emplace(key);
    if (not created)
      throw std::runtime_error("Entry already exists");

    auto& entry = it->second;
    entry.value = std::move(data);
    entry.available = true;

    return Record(&entry.value, &entry.mutex);
  }

  std::optional<Record<Data>> Get(const Key& key, bool retrieve = true)
  {
    auto [recordIter, created] = _entries.try_emplace(key);
    auto& record = recordIter->second;

    if (created && retrieve)
    {
      RequestRetrieve(key);
      return std::nullopt;
    }

    if (record.available)
      return Record(&record.value, &record.mutex);
    return std::nullopt;
  }

  std::optional<std::vector<Record<Data>>> Get(const KeySpan keys)
  {
    bool isComplete = true;

    std::vector<Record<Data>> records;
    for (const auto& key : keys)
    {
      auto record = Get(key);
      if (not record)
      {
        isComplete = false;
        continue;
      }

      records.emplace_back() = std::move(*record);
    }

    if (isComplete)
      return records;
    return std::nullopt;
  }

  std::vector<Key> GetKeys()
  {
    std::vector<Key> keys;
    for (const auto& key : std::ranges::views::keys(_entries))
    {
      keys.emplace_back(key);
    }
    return keys;
  }

  void Save(const Key& key)
  {
    RequestStore(key);
  }

  void Tick()
  {
    // Perform retrieve operations.
    for (const auto& key : _retrieveQueue)
    {
      auto& entry = _entries[key];

      if (_dataSourceRetrieveListener(key, entry.value))
        entry.available.store(true, std::memory_order::relaxed);
    }
    _retrieveQueue.clear();

    // Perform store operations.
    for (const auto& key : _storeQueue)
    {
      auto& entry = _entries[key];

      if (entry.available)
        if (_dataSourceStoreListener(key, entry.value))
          entry.available.store(false, std::memory_order::relaxed);
    }
    _storeQueue.clear();
  }

private:
  void RequestRetrieve(const Key& key)
  {
    _retrieveQueue.insert(key);
  }

  void RequestStore(const Key& key)
  {
    _storeQueue.insert(key);
  }

  struct Entry
  {
    std::atomic_bool available{false};
    std::atomic_bool dirty{false};
    std::shared_mutex mutex{};
    Data value;
  };

  std::unordered_set<Key> _retrieveQueue;
  std::unordered_set<Key> _storeQueue;
  std::unordered_map<Key, Entry> _entries{};

  DataSourceRetrieveListener _dataSourceRetrieveListener;
  DataSourceStoreListener _dataSourceStoreListener;
};

} // namespace server

#endif // DATASTORAGE_HPP
