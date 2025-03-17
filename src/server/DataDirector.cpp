#include "server/DataDirector.hpp"

#include "libserver/command/proto/DataDefines.hpp"

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

std::future<data::User> DataDirector::GetUser(std::string const& name)
{
  const auto [iterator, inserted] = _users.try_emplace(name);
  if (inserted)
  {
    _taskLoop.Queue(
      [this, name]()
      {
        try
        {
          pqxx::work query(*_connection);

          // Query and find the user login and the token.
          const auto result = query.exec_prepared1(QueryUserRecordStatementId, name);

          _users[name].set_value(
            data::User{
              .name = name,
              .token = result["token"].as<std::string>({}), // ¯\_(ツ)_/¯
              .characterUid = result["characterUid"].as(InvalidDatumUid),
            });
        }
        catch (std::exception& x)
        {
          spdlog::error("DataDirector error: {}", x.what());
          // TODO: pass exception to future
        }
      });
  }

  return iterator->second.get_future();
}
std::future<data::Character> DataDirector::GetCharacter(DatumUid uid) {
  const auto [iterator, inserted] = _characters.try_emplace(uid);
  if (inserted)
  {
    _taskLoop.Queue(
      [this, uid]()
      {
        try
        {
          pqxx::work query(*_connection);

          // Query and find the user login and the token.
          const auto result = query.exec_prepared1(QueryUserRecordStatementId, uid);

          _characters[uid].set_value(
            data::Character{
              .name = result["name"].as<std::string>({}), // ¯\_(ツ)_/¯
              .status = result["status"].as<std::string>({}),
              .level = result["level"].as<uint16_t>(0),
              .carrots = result["carrots"].as<int32_t>(0),
              .cash = result["cash"].as<int32_t>(0),
              .ageGroup = result["ageGroup"].as<data::Character::AgeGroup>(
                data::Character::AgeGroup::Kid),
              .gender = result["gender"].as<data::Character::Gender>(
                data::Character::Gender::Unspecified),
              .appearance = {
                .charId = result["charId"].as<uint8_t>(0),
                .mouthSerialId = result["mouthSerialId"].as<uint8_t>(0),
                .faceSerialId = result["faceSerialId"].as<uint8_t>(0),
                .headVolume = result["headVolume"].as<uint16_t>(0),
                .height = result["height"].as<uint16_t>(0),
                .thighVolume = result["thighVolume"].as<uint16_t>(0),
                .legVolume = result["legVolume"].as<uint16_t>(0),
              },
              .mountUid = result["mountUid"].as<DatumUid>({0}),
              .ranchUid = result["ranchUid"].as<DatumUid>({0}),
            });
        }
        catch (std::exception& x)
        {
          spdlog::error("DataDirector error: {}", x.what());
          // TODO: pass exception to future
        }
      });
  }

  return iterator->second.get_future();
}
} // namespace alicia
