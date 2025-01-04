//
// Created by rgnter on 25/11/2024.
//

#include "server/DataDirector.hpp"

#include <spdlog/spdlog.h>

namespace
{

//!
const std::string QueryUserTokenStatementId = "queryUserToken";

} // anon namespace


namespace alicia
{

DataDirector::DataDirector(Settings::DataSource settings)
  : _settings(std::move(settings))
{
  try
  {
    _connection = pqxx::connection(
      _settings.connectionString);

    _connection.prepare(
      QueryUserTokenStatementId,
      "SELECT token, user_uid FROM token WHERE login=$1");
  }
  catch (std::exception& x)
  {
    spdlog::error(
      "Failed to initialize the data source with connection string '{}' because: {}",
      settings.connectionString,
      x.what());

    throw;
  }
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
