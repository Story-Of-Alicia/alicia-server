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
#include <shared_mutex>
#include <span>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <unordered_set>
#include <ranges>

namespace soa
{

template <typename Data>
class Record
{
public:
  using DataPtr = Data*;
  using Mutex = std::shared_mutex;
  using MutexPtr = Mutex*;

  Record()
    : _mutex(nullptr)
    , _value(nullptr)
  {

  }

  Record(DataPtr value, MutexPtr mutex)
    : _mutex(mutex)
    , _exclusiveLock(*_mutex, std::defer_lock)
    , _sharedLock(*_mutex, std::defer_lock)
    , _value(value)
  {
  }

  ~Record()
  {
  }

  //! Deleted copy constructor.
  Record(const Record&) = delete;
  //! Deleted move constructor.
  void operator=(const Record&) = delete;

  //! Move constructor.
  Record(Record&& other) noexcept
    : _mutex(other._mutex)
    , _exclusiveLock(std::move(other._exclusiveLock))
    , _sharedLock(std::move(other._sharedLock))
    , _value(other._value)
  {
  }

  //! Move assignment.
  void operator=(Record&& other) noexcept
  {
    _mutex = other._mutex;
    _exclusiveLock = std::move(other._exclusiveLock);
    _sharedLock = std::move(other._sharedLock);
    _value = other._value;
  }

  //! Returns and exclusively locks the underlying data.
  //! @return Reference to the data.
  Data& operator()()
  {
    if (not _exclusiveLock.owns_lock())
      _exclusiveLock.lock();
    return *_value;
  };

  //! Immutable shared access to the underlying data.
  //! @param consumer Consumer that receives the data.
  void Immutable(std::function<void(const Data&)> consumer) const
  {
    _sharedLock.lock();
    consumer(*_value);
    _sharedLock.unlock();
  }

  //! Access to the underlying data.
  //! @param consumer Consumer that receives the data.
  void Mutable(std::function<void(Data&)> consumer) const
  {
    _exclusiveLock.lock();
    consumer(*_value);
    _exclusiveLock.unlock();
  };

private:
  mutable MutexPtr _mutex;
  mutable std::unique_lock<std::shared_mutex> _exclusiveLock;
  mutable std::shared_lock<std::shared_mutex> _sharedLock;

  DataPtr _value;
};

template <typename Key, typename Data>
class DataStorage
{
public:
  using KeySpan = std::span<const Key>;

  using DataSourceRetrieveListener = std::function<void(const Key& key, Data& data)>;
  using DataSourceStoreListener = std::function<void(const Key& key, Data& data)>;

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
  //! @param key Keys of the data.
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

  std::optional<Record<Data>> Get(const Key& key)
  {
    auto [recordIter, created] = _entries.try_emplace(key);
    auto& record = recordIter->second;

    if (created)
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
    for (const auto & key : std::ranges::views::keys(_entries))
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
      _dataSourceRetrieveListener(key, entry.value);
      entry.available = true;
    }
    _retrieveQueue.clear();

    // Perform store operations.
    for (const auto& key : _storeQueue)
    {
      auto& entry = _entries[key];
      _dataSourceStoreListener(key, entry.value);
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

} // namespace soa

#endif // DATASTORAGE_HPP
