//
// Created by rgnter on 25/11/2024.
//

#include "server/DataDirector.hpp"

#include <spdlog/spdlog.h>

namespace
{

//!
template<typename T>
void ProvideLockedDatumAccess(
  alicia::DataDirector::Datum<T>& datum,
  const alicia::DatumConsumer<T&>& immutableConsumer)
{
  std::scoped_lock lock(datum.lock);
  try
  {
    immutableConsumer(datum.value);
  }
  catch (const std::exception& x)
  {
    spdlog::error("Unhandled exception in immutable datum consumer:", x.what());
  }
}

} // anon namespace

namespace alicia
{

DataDirector::DataDirector()
{
  _users["rgnter"].value = {
    .characterUid = 1
  };
  _users["laith"].value = {
    .characterUid = 2
  };

  _characters[1].value = {
    .nickName = "rgnt",
    .gender = Gender::Unspecified,
    .level = 60,
    .carrots = 5000,
    .looks = alicia::Character{},
    .characterEquipment = {Item{.uid = 100, .tid = 30035, .val = 0, .count = 1}},
    .mountUid = 3,
    .ranchUid = 100,
    .horses = {3}
  };
  _characters[2].value = {
    .nickName = "laith",
    .gender = Gender::Unspecified,
    .level = 60,
    .carrots = 5000,
    .characterEquipment = {},
    .mountUid = 4,
    .ranchUid = 100,
    .horses = {4}
  };

  _mounts[3].value = {
    .tid = 0x4E21, .name = "idontunderstand"
  };
  _mounts[4].value = {
    .tid = 0x4E21, .name = "Ramon"
  };

  _ranches[100].value = {
    .ranchName = "SoA Ranch"
  };
}


void DataDirector::GetUser(const std::string& name, DatumConsumer<User&> consumer)
{
  consumer(_users[name].value);
}

DataDirector::DatumAccess<User> DataDirector::GetUser(
  const std::string& name)
{
  auto& datum = _users[name];
  return DatumAccess(datum);
}

void DataDirector::GetCharacter(
  DatumUid characterUid,
  DatumConsumer<User::Character&> consumer)
{
  ProvideLockedDatumAccess(_characters[characterUid], consumer);
}

DataDirector::DatumAccess<User::Character> DataDirector::GetCharacter(
  DatumUid characterUid)
{
  auto& datum = _characters[characterUid];
  return DatumAccess(datum);
}

void DataDirector::GetMount(
  DatumUid mountUid,
  DatumConsumer<User::Mount&> consumer)
{
  ProvideLockedDatumAccess(_mounts[mountUid], consumer);
}

DataDirector::DatumAccess<User::Mount> DataDirector::GetMount(
  DatumUid mountUid)
{
  auto& datum = _mounts[mountUid];
  return DatumAccess(datum);
}

void DataDirector::GetRanch(
  DatumUid ranchUid,
  DatumConsumer<User::Ranch&> consumer)
{
  ProvideLockedDatumAccess(_ranches[ranchUid], consumer);
}

DataDirector::DatumAccess<User::Ranch> DataDirector::GetRanch(
  DatumUid ranchUid)
{
  auto& datum = _ranches[ranchUid];
  return DatumAccess(datum);
}

} // namespace alicia
