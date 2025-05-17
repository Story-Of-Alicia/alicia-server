//
// Created by maros on 4/6/25.
//

#ifndef DATASTORAGE_HPP
#define DATASTORAGE_HPP

#include <atomic>
#include <functional>
#include <shared_mutex>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <spdlog/spdlog.h>

namespace soa
{

template<typename Data>
class Record
{
public:
  Record(Data& value, std::shared_mutex& mutex)
    : _mutex(mutex)
    , _exclusiveLock(_mutex, std::defer_lock)
    , _sharedLock(_mutex, std::defer_lock)
    , _value(value)
  {}

  ~Record()
  {
    if (_exclusiveLock.owns_lock())
      _exclusiveLock.unlock();
    if (_sharedLock.owns_lock())
      _sharedLock.unlock();
  }


  //! Deleted copy constructor.
  Record(const Record&) = delete;
  //! Deleted move constructor.
  void operator=(const Record&) = delete;

  //! Move constructor.
  Record(Record&& other)
    : _mutex(other._mutex)
    , _exclusiveLock(
      std::move(other._exclusiveLock))
    , _sharedLock(
      std::move(other._sharedLock))
    , _value(other._value)
  {}

  //! Move assignment.
  void operator=(Record&& other)
  {
    _mutex = other._mutex;
    _exclusiveLock.unlock();
    _exclusiveLock = std::move(other._exclusiveLock);
    _sharedLock.unlock();
    _sharedLock = std::move(other._sharedLock);
    _value = other._value;
  }

  //! Returns and exclusively locks the underlying data.
  //! @return Reference to the data.
  Data& operator()()
  {
    if (not _exclusiveLock.owns_lock())
      _exclusiveLock.lock();
    return _value;
  };

  //! Immutable shared access to the underlying data.
  //! @param consumer Consumer that receives the data.
  void Immutable(std::function<void(const Data&)> consumer)
  {
    _sharedLock.lock();
    consumer(_value);
    _sharedLock.unlock();
  }

  //! Access to the underlying data.
  //! @param consumer Consumer that receives the data.
  void Mutable(std::function<void(Data&)> consumer)
  {
    _exclusiveLock.lock();
    consumer(_value);
    _exclusiveLock.unlock();
  };

private:
  std::shared_mutex& _mutex;
  std::unique_lock<std::shared_mutex> _exclusiveLock;
  std::shared_lock<std::shared_mutex> _sharedLock;

  Data& _value;
};

template<typename Key, typename Data>
class DataStorage
{
public:
  using MultipleKeys = std::span<Key>;

  using DataSourceRetrieveListener = std::function<void(const Key& key, Data& data)>;
  using DataSourceStoreListener = std::function<void(const Key& key, Data& data)>;

  DataStorage(
    const DataSourceRetrieveListener& data_source_retrieve_listener,
    const DataSourceStoreListener& data_source_store_listener)
      : _dataSourceRetrieveListener(data_source_retrieve_listener)
      , _dataSourceStoreListener(data_source_store_listener)
  {
  }

  //! Whether data record is available.
  //! @param key Key of the datum.
  //! @returns `true` if datum is available, `false` otherwise.
  bool IsDatumAvailable(const Key& key)
  {
    const auto iterator = _entries.find(key);
    if (iterator == _entries.cend())
      return false;
    return iterator->second.available;
  }

  //! Whether data records are available.
  //! @param key Keys of the data.
  //! @returns `true` if data are available, `false` otherwise.
  bool AreDataAvailable(MultipleKeys keys)
  {
    for (const auto& key : keys)
    {
      if (not IsDatumAvailable(key))
        return false;
    }

    return true;
  }

  std::optional<Record<Data>> Create(const Key& key)
  {
    auto [recordIter, created] = _entries.try_emplace(key);
    auto& record = recordIter->second;

    if (not created)
    {
      return std::nullopt;
    }

    return  Record(record.value, record.mutex);
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
      return  Record(record.value, record.mutex);
    return std::nullopt;
  }

  std::optional<std::vector<Record<Data>>> Get(MultipleKeys keys)
  {
    if (not AreDataAvailable(keys))
      return std::nullopt;

    std::vector<Record<Data>> records;
    for (const auto & key : keys)
    {
      records.emplace_back(*Get(key));
    }

    return records;
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
    std::shared_mutex mutex;
    Data value;
  };

  std::unordered_set<Key> _retrieveQueue;
  std::unordered_set<Key> _storeQueue;
  std::unordered_map<Key, Entry> _entries{};

  DataSourceRetrieveListener _dataSourceRetrieveListener;
  DataSourceStoreListener _dataSourceStoreListener;
};

} // namespace soa

#endif //DATASTORAGE_HPP
