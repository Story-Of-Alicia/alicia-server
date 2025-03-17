#include "server/DataDirector.hpp"

#include <spdlog/spdlog.h>

namespace
{

//!
const std::string QueryUserTokenRecordStatementId = "queryUserTokenRecord";
//!
const std::string QueryUserRecordStatementId = "queryUserRecord";
//!
const std::string QueryCharacterRecordStatementId = "queryCharacterRecord";
//!
const std::string QueryHorseRecordStatementId = "queryHorseRecord";
//!
const std::string QueryItemRecordStatementId = "queryItemRecord";
//!
const std::string QueryRanchRecordStatementId = "queryRanchRecord";

} // anon namespace


namespace alicia
{

DataDirector::DataDirector(Settings::DataSource settings)
  : _settings(std::move(settings))
{
  _taskLoop.Begin();
}

void DataDirector::EstablishConnection()
{
  try
  {
    _connection = std::make_unique<pqxx::connection>(_settings.connectionString.c_str());

    _connection->prepare(
      QueryUserTokenRecordStatementId, "SELECT token, user_uid FROM data.token WHERE login=$1");
    _connection->prepare(QueryUserRecordStatementId, "SELECT * FROM data.user WHERE username=$1");
    _connection->prepare(
      QueryCharacterRecordStatementId, "SELECT * FROM data.character WHERE uid=$1");
    _connection->prepare(QueryHorseRecordStatementId, "SELECT * FROM data.horse WHERE uid=$1");
    _connection->prepare(QueryRanchRecordStatementId, "SELECT * FROM data.ranch WHERE uid=$1");
    _connection->prepare(QueryItemRecordStatementId, "SELECT * FROM data.item WHERE uid=$1");

    spdlog::info(
      "Initialized the data source with the connection string '{}'", _settings.connectionString);
  }
  catch (const std::exception& x)
  {
    spdlog::error(
      "Failed to establish the data source connection with connection string '{}' because: {}",
      _settings.connectionString,
      x.what());
  }
}

std::future<data::User> DataDirector::GetUser(std::string const& username)
{
  const auto [iterator, inserted] = _users.try_emplace(username);
  if (inserted)
  {
    _taskLoop.Queue(
     [this, username]()
     {
       try
       {
         pqxx::work query(*_connection);

         // Query and find the user login and the token.
         const auto result = query.exec_prepared1(
           QueryUserRecordStatementId, username
         );

         _users[username].set_value(data::User{
           .username = username,
           .token = result["token"].as<std::string>({}), // ¯\_(ツ)_/¯
           .characterUid = result["characterUid"].as(InvalidDatumUid),
         });
       }
       catch (std::exception& x)
       {
         spdlog::error("DataDirector error: {}", x.what());
         //TODO: pass exception to future
       }
     });
  }

  return iterator->second.get_future();
}
} // namespace alicia
