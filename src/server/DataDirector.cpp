//
// Created by rgnter on 25/11/2024.
//

#include "server/DataDirector.hpp"

#include <spdlog/spdlog.h>

namespace alicia
{

DataDirector::DataDirector()
{
  // _users["rgnter"].value = {
  //   .characterUid = 1
  // };
  // _users["laith"].value = {
  //   .characterUid = 2
  // };
  //
  // _characters[1].value = {
  //   .nickName = "rgnt",
  //   .gender = Gender::Unspecified,
  //   .level = 60,
  //   .carrots = 5000,
  //   .characterEquipment = {Item{.uid = 100, .tid = 30035, .val = 0, .count = 1}},
  //   .mountUid = 3,
  //   .ranchUid = 100,
  //   .horses = {3}
  // };
  // _characters[2].value = {
  //   .nickName = "laith",
  //   .gender = Gender::Unspecified,
  //   .level = 60,
  //   .carrots = 5000,
  //   .characterEquipment = {},
  //   .mountUid = 4,
  //   .ranchUid = 100,
  //   .horses = {4}
  // };
  //
  // _horses[3].value = {
  //   .tid = 0x4E21, .name = "idontunderstand"
  // };
  // _horses[4].value = {
  //   .tid = 0x4E21, .name = "Ramon"
  // };
  //
  // _ranches[100].value = {
  //   .ranchName = "SoA Ranch"
  // };
}


DataDirector::DatumAccess<data::User> DataDirector::GetUser(
  const std::string& name)
{
  auto& datum = _users[name];
  return DatumAccess(datum);
}

DataDirector::DatumAccess<data::Character> DataDirector::GetCharacter(
  DatumUid characterUid)
{
  auto& datum = _characters[characterUid];
  return DatumAccess(datum);
}

DataDirector::DatumAccess<data::Horse> DataDirector::GetHorse(
  DatumUid mountUid)
{
  auto& datum = _horses[mountUid];
  return DatumAccess(datum);
}

DataDirector::DatumAccess<data::Ranch> DataDirector::GetRanch(
  DatumUid ranchUid)
{
  auto& datum = _ranches[ranchUid];
  return DatumAccess(datum);
}

} // namespace alicia
