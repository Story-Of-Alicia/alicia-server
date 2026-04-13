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

#include "libserver/data/Record.hpp"

#include <atomic>
#include <functional>
#include <ranges>
#include <shared_mutex>
#include <span>
#include <unordered_map>
#include <unordered_set>

namespace server
{

template <typename Key, typename Data>
class DataStorage
{
public:
  using KeySpan = std::span<const Key>;

  using DataSourceRetrieveListener = std::function<bool(const Key& key, Data& data)>;
  using DataSourceStoreListener = std::function<bool(const Key& key, Data& data)>;
  using DataSourceDeleteListener = std::function<bool(const Key& key)>;

  using DataSupplier = std::function<std::pair<Key, Data>()>;

  DataStorage(
    const DataSourceRetrieveListener& retrieveListener,
    const DataSourceStoreListener& storeListener,
    const DataSourceDeleteListener& deleteListener)
    : _dataSourceRetrieveListener(retrieveListener)
    , _dataSourceStoreListener(storeListener)
    , _dataSourceDeleteListener(deleteListener)
  {
  }

  void Initialize()
  {
  }

  void Terminate()
  {
    {
      std::scoped_lock lock(_entriesMutex);
      for (auto& entry : _entries)
      {
        if (not entry.second.available)
          continue;
        RequestStore(entry.first);
      }
    }

    // Process the queued operations.
    ProcessRetrieveQueue();
    ProcessStoreQueue();
    ProcessDeleteQueue();

    {
      std::scoped_lock lock(_entriesMutex);
      _entries.clear();
    }
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

  Record<Data> Create(DataSupplier supplier)
  {
    auto [key, data] = supplier();

    std::unique_lock lock(_entriesMutex);
    auto [it, created] = _entries.try_emplace(key);
    lock.unlock();

    if (not created)
      throw std::runtime_error(std::format("Entry with key {} already exists", key));

    auto& entry = it->second;
    entry.value = std::move(data);
    entry.available = true;

    RequestStore(key);

    return Record(&entry.value, &entry.mutex, [this, key]()
      {
        RequestStore(key);
      });
  }

  Record<Data> GetOrCreate(DataSupplier supplier)
  {
    auto [key, data] = supplier();

    std::unique_lock lock(_entriesMutex);
    auto [it, created] = _entries.try_emplace(key);
    lock.unlock();

    if (not created)
      return Record(&it->second.value, &it->second.mutex, [this, key]()
      {
        RequestStore(key);
      });

    auto& entry = it->second;
    entry.value = std::move(data);
    entry.available = true;

    RequestStore(key);

    return Record(&entry.value, &entry.mutex, [this, key]()
      {
        RequestStore(key);
      });
  }

  std::optional<Record<Data>> Get(const Key& key, bool retrieve = true)
  {
    std::unique_lock lock(_entriesMutex);
    auto [recordIter, created] = _entries.try_emplace(key);
    lock.unlock();

    auto& record = recordIter->second;

    if (created && retrieve)
    {
      RequestRetrieve(key);
      return std::nullopt;
    }

    if (record.available)
      return Record(&record.value, &record.mutex, [this, key]()
      {
        RequestStore(key);
      });
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

  void Delete(const Key& key)
  {
    RequestDelete(key);
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
    ProcessRetrieveQueue();
    ProcessStoreQueue();
    ProcessDeleteQueue();
  }

private:
  void RequestRetrieve(const Key& key)
  {
    std::scoped_lock lock(_retrieveQueue.mutex);
    _retrieveQueue.data.insert(key);
    _retrieveQueue.dataFlag.store(true, std::memory_order::relaxed);
  }

  void RequestStore(const Key& key)
  {
    std::scoped_lock lock(_storeQueue.mutex);
    _storeQueue.data.insert(key);
    _storeQueue.dataFlag.store(true, std::memory_order::relaxed);
  }

  void RequestDelete(const Key& key)
  {
    std::scoped_lock lock(_deleteQueue.mutex);
    _deleteQueue.data.insert(key);
    _deleteQueue.dataFlag.store(true, std::memory_order::relaxed);
  }

  void ProcessRetrieveQueue()
  {
    if (not _retrieveQueue.dataFlag.exchange(false, std::memory_order::relaxed))
      return;

    std::scoped_lock queueLock(_retrieveQueue.mutex);
    for (const auto& key : _retrieveQueue.data)
    {
      std::unique_lock lock(_entriesMutex);
      auto& entry = _entries[key];
      lock.unlock();

      if (_dataSourceRetrieveListener(key, entry.value))
        entry.available.store(true, std::memory_order::relaxed);
    }
    _retrieveQueue.data.clear();
  }

  void ProcessStoreQueue()
  {
    if (not _storeQueue.dataFlag.exchange(false, std::memory_order::relaxed))
      return;

    std::scoped_lock queueLock(_storeQueue.mutex);
    for (const auto& key : _storeQueue.data)
    {
      std::unique_lock lock(_entriesMutex);
      auto& entry = _entries[key];
      lock.unlock();

      if (entry.available)
        _dataSourceStoreListener(key, entry.value);
    }
    _storeQueue.data.clear();
  }

  void ProcessDeleteQueue()
  {
    if (not _deleteQueue.dataFlag.exchange(false, std::memory_order::relaxed))
      return;

    std::scoped_lock queueLock(_deleteQueue.mutex);
    for (const auto& key : _deleteQueue.data)
    {
      std::unique_lock lock(_entriesMutex);
      auto& entry = _entries[key];
      lock.unlock();

      if (entry.available)
        if (_dataSourceDeleteListener(key))
          entry.available.store(false, std::memory_order::relaxed);
    }
    _deleteQueue.data.clear();
  }

  struct Entry
  {
    std::atomic_bool available{false};
    std::atomic_bool dirty{false};
    Record<Data>::PatchListener listener;
    std::shared_mutex mutex{};
    Data value;
  };

  struct Queue
  {
    std::mutex mutex;
    std::atomic_bool dataFlag;
    std::unordered_set<Key> data;
  };

  Queue _retrieveQueue;
  Queue _storeQueue;
  Queue _deleteQueue;

  std::mutex _entriesMutex;
  std::unordered_map<Key, Entry> _entries{};

  DataSourceRetrieveListener _dataSourceRetrieveListener;
  DataSourceStoreListener _dataSourceStoreListener;
  DataSourceDeleteListener _dataSourceDeleteListener;
};

} // namespace server

#endif // DATASTORAGE_HPP
