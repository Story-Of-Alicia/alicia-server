//
// Created by rgnter on 25/11/2024.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "spdlog/spdlog.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include <libserver/command/proto/DataDefines.hpp>

namespace alicia
{

//! Unique datum identifier.
using DatumUid = uint32_t;
//! Invalid datum identifier.
constexpr DatumUid InvalidDatumUid = 0;

//! Datum consumer is a callback which provides access to a value,
//! to which access is guaranteed while in the consumer function.
template<typename T>
using DatumConsumer = std::function<void(T value)>;

//! User.
struct User
{
  std::string _token;

  DatumUid characterUid;

  //! Character.
  struct Character
  {
    std::string nickName;
    Gender gender = Gender::Unspecified;
    uint16_t level{};
    int32_t carrots{};
    AgeGroup ageGroup = AgeGroup::Kid;
    std::optional<alicia::Character> looks{};

    std::string status;

    std::vector<Item> characterEquipment;
    std::vector<Item> horseEquipment;

    DatumUid mountUid{};
    DatumUid ranchUid{};

    std::vector<DatumUid> horses{};
  };

  //! Mount.
  struct Mount
  {
    uint32_t tid{};
    std::string name;
  };

  //! Ranch
  struct Ranch
  {
    std::string ranchName;
  };
};

class DataDirector
{
public:
  //!
  template<typename Val>
  struct Datum
  {
    Val value;
    std::mutex lock;
  };

  template<typename Val>
  struct DatumAccess
  {
    //!
    explicit DatumAccess(Datum<Val>& datum)
      : _datum(datum)
      , _accessLock(_datum.lock)
    {
    }

    //! Deleted copy constructor.
    DatumAccess(const DatumAccess&) noexcept = delete;
    //! Move constructor.
    DatumAccess(DatumAccess&& other) noexcept
      : _datum(other._datum)
    {
      other._accessLock.unlock();
      other._accessLock.release();

      _accessLock = _datum.lock;
    }

    ~DatumAccess()
    {
      _accessLock.unlock();
    }

    [[nodiscard]] Val* operator->() const noexcept {
      return &_datum.value;
    }

  private:
    Datum<Val>& _datum;
    std::unique_lock<std::mutex> _accessLock;
  };

  DataDirector();

  //!
  void GetUser(
    const std::string& name,
    DatumConsumer<User&> consumer);
  //!
  DatumAccess<User> GetUser(
    const std::string& name);

  //!
  void GetCharacter(
    DatumUid characterUid,
    DatumConsumer<User::Character&> consumer);
  //!
  DatumAccess<User::Character> GetCharacter(
    DatumUid characterUid);

  //!
  void GetMount(
    DatumUid mountUid,
    DatumConsumer<User::Mount&> consumer);
  DatumAccess<User::Mount> GetMount(
    DatumUid mountUid);

  //!
  void GetRanch(
    DatumUid ranchUid,
    DatumConsumer<User::Ranch&> consumer);
  DatumAccess<User::Ranch> GetRanch(
    DatumUid ranchUid);

private:
  //!
  std::unordered_map<std::string, Datum<User>> _users;
  //!
  std::unordered_map<DatumUid, Datum<User::Character>> _characters;
  //!
  std::unordered_map<DatumUid, Datum<User::Mount>> _mounts;
  //!
  std::unordered_map<DatumUid, Datum<User::Ranch>> _ranches;
};

}

#endif //DATADIRECTOR_HPP