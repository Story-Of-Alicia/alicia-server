/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#include "libserver/data/pq/PqDataSource.hpp"

#include <chrono>
#include <cstdint>
#include <format>
#include <set>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

namespace server
{

namespace
{

pqxx::connection& RequireConnection(
  const std::unique_ptr<pqxx::connection>& connection)
{
  if (not connection or not connection->is_open())
  {
    throw std::runtime_error("Connection to data source is not available");
  }

  return *connection;
}

template <typename T>
auto WriteField(
  const T& value)
{
  if constexpr (std::is_enum_v<T>)
  {
    return static_cast<std::underlying_type_t<T>>(value);
  }
  else if constexpr (std::is_same_v<T, data::Clock::time_point>)
  {
    return std::chrono::duration_cast<std::chrono::seconds>(
      value.time_since_epoch())
      .count();
  }
  else if constexpr (std::is_same_v<T, std::chrono::seconds>)
  {
    return value.count();
  }
  else
  {
    return value;
  }
}

template <typename T>
void ReadField(
  const pqxx::row& row,
  const char* column,
  dao::Field<T>& field)
{
  if constexpr (std::is_enum_v<T>)
  {
    field = static_cast<T>(row[column].as<std::underlying_type_t<T>>());
  }
  else if constexpr (std::is_same_v<T, data::Clock::time_point>)
  {
    field = data::Clock::time_point(
      std::chrono::duration_cast<data::Clock::duration>(
        std::chrono::seconds(row[column].as<int64_t>())));
  }
  else if constexpr (std::is_same_v<T, std::chrono::seconds>)
  {
    field = std::chrono::seconds(
      row[column].as<int64_t>());
  }
  else
  {
    field = row[column].as<T>();
  }
}

std::string QualifiedTable(
  const pqxx::work& query,
  const std::string_view table)
{
  return query.quote_name("data") + "." + query.quote_name(table);
}

template <typename Key>
void WriteUidList(
  pqxx::work& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey,
  const std::vector<data::Uid>& values)
{
  const auto tableName = QualifiedTable(query, table);
  const auto ownerName = query.quote_name(ownerColumn);
  query.exec(
    std::format("delete from {} where {} = $1", tableName, ownerName),
    pqxx::params(ownerKey));
  const auto insertSql = std::format(
    "insert into {} ({}, \"position\", \"valueUid\") values ($1, $2, $3)",
    tableName,
    ownerName);
  int64_t position = 0;
  for (const auto value : values)
  {
    query.exec(insertSql, pqxx::params(ownerKey, position, value));
    ++position;
  }
}

template <typename Key>
std::vector<data::Uid> ReadUidList(
  pqxx::work& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey)
{
  const auto result = query.exec(
    std::format(
      "select \"valueUid\" from {} where {} = $1 order by \"position\"",
      QualifiedTable(query, table),
      query.quote_name(ownerColumn)),
    pqxx::params(ownerKey));
  std::vector<data::Uid> values;
  values.reserve(result.size());
  for (const auto& row : result)
  {
    values.emplace_back(row["valueUid"].as<data::Uid>());
  }
  return values;
}

template <typename Key>
void WriteUidSet(
  pqxx::work& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey,
  const std::set<data::Uid>& values)
{
  const auto tableName = QualifiedTable(query, table);
  const auto ownerName = query.quote_name(ownerColumn);
  query.exec(
    std::format("delete from {} where {} = $1", tableName, ownerName),
    pqxx::params(ownerKey));
  const auto insertSql = std::format(
    "insert into {} ({}, \"valueUid\") values ($1, $2)",
    tableName,
    ownerName);
  for (const auto value : values)
  {
    query.exec(insertSql, pqxx::params(ownerKey, value));
  }
}

template <typename Key>
std::set<data::Uid> ReadUidSet(
  pqxx::work& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey)
{
  const auto result = query.exec(
    std::format(
      "select \"valueUid\" from {} where {} = $1 order by \"valueUid\"",
      QualifiedTable(query, table),
      query.quote_name(ownerColumn)),
    pqxx::params(ownerKey));
  std::set<data::Uid> values;
  for (const auto& row : result)
  {
    values.emplace(row["valueUid"].as<data::Uid>());
  }
  return values;
}

void WriteCharacterSkills(
  pqxx::work& query,
  const data::Uid uid,
  const std::string_view mode,
  const data::Character::Skills::Sets& skills)
{
  query.exec("insert into \"data\".\"characterSkills\" "
             "(\"characterUid\", \"mode\", \"activeSetId\", \"set1Slot1\", "
             "\"set1Slot2\", \"set2Slot1\", \"set2Slot2\") "
             "values ($1, $2, $3, $4, $5, $6, $7)",
    pqxx::params(
      uid,
      mode,
      skills.activeSetId,
      skills.set1.slot1,
      skills.set1.slot2,
      skills.set2.slot1,
      skills.set2.slot2));
}

void WriteCharacterRelations(
  pqxx::work& query,
  const data::Uid uid,
  const data::Character& character)
{
  WriteUidSet(query, "characterContactsPending", "characterUid", uid, character.contacts.pending());
  WriteUidList(query, "characterGifts", "characterUid", uid, character.gifts());
  WriteUidList(query, "characterPurchases", "characterUid", uid, character.purchases());
  WriteUidList(query, "characterInventory", "characterUid", uid, character.inventory());
  WriteUidList(query, "characterEquipment", "characterUid", uid, character.characterEquipment());
  WriteUidList(query, "characterHorses", "characterUid", uid, character.horses());
  WriteUidSet(query, "characterBreedingWishlist", "characterUid", uid, character.breedingWishlist());
  WriteUidList(query, "characterPets", "characterUid", uid, character.pets());
  WriteUidList(query, "characterEggs", "characterUid", uid, character.eggs());
  WriteUidList(query, "characterHousing", "characterUid", uid, character.housing());
  WriteUidList(query, "characterMailboxInbox", "characterUid", uid, character.mailbox.inbox());
  WriteUidList(query, "characterMailboxSent", "characterUid", uid, character.mailbox.sent());
  WriteUidList(query, "characterQuests", "characterUid", uid, character.quests());

  query.exec("delete from \"data\".\"characterContactGroups\" where "
             "\"characterUid\" = $1",
    pqxx::params(uid));
  for (const auto& [key, group] : character.contacts.groups())
  {
    query.exec(
      "insert into \"data\".\"characterContactGroups\" "
      "(\"characterUid\", \"groupKey\", \"uid\", \"name\", \"createdAt\") "
      "values ($1, $2, $3, $4, $5)",
      pqxx::params(
        uid,
        key,
        group.uid,
        group.name,
        WriteField(group.createdAt)));
    for (const auto member : group.members)
    {
      query.exec(
        "insert into \"data\".\"characterContactGroupMembers\" "
        "(\"characterUid\", \"groupKey\", \"memberUid\") values ($1, $2, $3)",
        pqxx::params(
          uid,
          key,
          member));
    }
  }

  query.exec(
    "delete from \"data\".\"characterSkills\" where \"characterUid\" = $1",
    pqxx::params(uid));
  WriteCharacterSkills(query, uid, "speed", character.skills.speed());
  WriteCharacterSkills(query, uid, "magic", character.skills.magic());

  query.exec(
    "delete from \"data\".\"characterMissions\" where \"characterUid\" = $1",
    pqxx::params(uid));
  for (const auto& [key, mission] : character.missions())
  {
    query.exec("insert into \"data\".\"characterMissions\" "
               "(\"characterUid\", \"missionKey\", \"id\") values ($1, $2, $3)",
      pqxx::params(
        uid,
        key,
        mission.id));
    int64_t position = 0;
    for (const auto& progress : mission.progress)
    {
      query.exec(
        "insert into \"data\".\"characterMissionProgress\" "
        "(\"characterUid\", \"missionKey\", \"position\", \"id\", \"value\") "
        "values ($1, $2, $3, $4, $5)",
        pqxx::params(
          uid,
          key,
          position,
          progress.id,
          progress.value));
      ++position;
    }
  }
}

void ReadCharacterRelations(
  pqxx::work& query,
  const data::Uid uid,
  data::Character& character)
{
  character.contacts.pending =
    ReadUidSet(query, "characterContactsPending", "characterUid", uid);
  character.gifts = ReadUidList(query, "characterGifts", "characterUid", uid);
  character.purchases =
    ReadUidList(query, "characterPurchases", "characterUid", uid);
  character.inventory =
    ReadUidList(query, "characterInventory", "characterUid", uid);
  character.characterEquipment =
    ReadUidList(query, "characterEquipment", "characterUid", uid);
  character.horses = ReadUidList(query, "characterHorses", "characterUid", uid);
  character.breedingWishlist =
    ReadUidSet(query, "characterBreedingWishlist", "characterUid", uid);
  character.pets = ReadUidList(query, "characterPets", "characterUid", uid);
  character.eggs = ReadUidList(query, "characterEggs", "characterUid", uid);
  character.housing =
    ReadUidList(query, "characterHousing", "characterUid", uid);
  character.mailbox.inbox =
    ReadUidList(query, "characterMailboxInbox", "characterUid", uid);
  character.mailbox.sent =
    ReadUidList(query, "characterMailboxSent", "characterUid", uid);
  character.quests = ReadUidList(query, "characterQuests", "characterUid", uid);

  std::map<data::Uid, data::Character::Contacts::Group> groups;
  const auto groupResult =
    query.exec("select \"groupKey\", \"uid\", \"name\", \"createdAt\" "
               "from \"data\".\"characterContactGroups\" where "
               "\"characterUid\" = $1 order by \"groupKey\"",
      pqxx::params(uid));
  for (const auto& row : groupResult)
  {
    data::Character::Contacts::Group group;
    group.uid = row["uid"].as<data::Uid>();
    group.name = row["name"].as<std::string>();
    group.createdAt = data::Clock::time_point(
      std::chrono::duration_cast<data::Clock::duration>(
        std::chrono::seconds(row["createdAt"].as<int64_t>())));
    groups.emplace(row["groupKey"].as<data::Uid>(), std::move(group));
  }
  const auto memberResult = query.exec(
    "select \"groupKey\", \"memberUid\" "
    "from \"data\".\"characterContactGroupMembers\" "
    "where \"characterUid\" = $1 order by \"groupKey\", \"memberUid\"",
    pqxx::params(uid));
  for (const auto& row : memberResult)
  {
    groups.at(row["groupKey"].as<data::Uid>())
      .members.insert(row["memberUid"].as<data::Uid>());
  }
  character.contacts.groups = std::move(groups);

  data::Character::Skills::Sets speedSkills;
  data::Character::Skills::Sets magicSkills;
  const auto skillResult =
    query.exec("select \"mode\", \"activeSetId\", \"set1Slot1\", "
               "\"set1Slot2\", \"set2Slot1\", \"set2Slot2\" "
               "from \"data\".\"characterSkills\" where \"characterUid\" = "
               "$1 order by \"mode\"",
      pqxx::params(uid));
  for (const auto& row : skillResult)
  {
    data::Character::Skills::Sets skills;
    skills.activeSetId = row["activeSetId"].as<uint32_t>();
    skills.set1.slot1 = row["set1Slot1"].as<uint32_t>();
    skills.set1.slot2 = row["set1Slot2"].as<uint32_t>();
    skills.set2.slot1 = row["set2Slot1"].as<uint32_t>();
    skills.set2.slot2 = row["set2Slot2"].as<uint32_t>();
    const auto mode = row["mode"].as<std::string>();
    if (mode == "speed")
    {
      speedSkills = skills;
    }
    else if (mode == "magic")
    {
      magicSkills = skills;
    }
    else
    {
      throw std::runtime_error("Character skill mode is invalid");
    }
  }
  character.skills.speed = std::move(speedSkills);
  character.skills.magic = std::move(magicSkills);

  std::map<uint32_t, data::Character::Mission> missions;
  const auto missionResult = query.exec(
    "select \"missionKey\", \"id\" from \"data\".\"characterMissions\" "
    "where \"characterUid\" = $1 order by \"missionKey\"",
    pqxx::params(uid));
  for (const auto& row : missionResult)
  {
    data::Character::Mission mission;
    mission.id = row["id"].as<uint32_t>();
    missions.emplace(row["missionKey"].as<uint32_t>(), std::move(mission));
  }
  const auto progressResult = query.exec(
    "select \"missionKey\", \"id\", \"value\" from "
    "\"data\".\"characterMissionProgress\" "
    "where \"characterUid\" = $1 order by \"missionKey\", \"position\"",
    pqxx::params(uid));
  for (const auto& row : progressResult)
  {
    missions.at(row["missionKey"].as<uint32_t>())
      .progress.push_back({.id = row["id"].as<uint32_t>(),
        .value = row["value"].as<uint32_t>()});
  }
  character.missions = std::move(missions);
}

void WriteStorageItemContents(
  pqxx::work& query,
  const data::Uid uid,
  const std::vector<data::StorageItem::Item>& items)
{
  query.exec(
    "delete from \"data\".\"storageItemContents\" where \"storageItemUid\" = $1",
    pqxx::params(
      uid));
  for (std::size_t position = 0; position < items.size(); ++position)
  {
    const auto& item = items[position];
    query.exec(
      "insert into \"data\".\"storageItemContents\" (\"storageItemUid\", \"position\", \"tid\", \"count\", \"duration\") values ($1, $2, $3, $4, $5)",
      pqxx::params(
        uid,
        position,
        item.tid,
        item.count,
        WriteField(item.duration)));
  }
}

std::vector<data::StorageItem::Item> ReadStorageItemContents(
  pqxx::work& query,
  const data::Uid uid)
{
  const auto result = query.exec(
    "select \"position\", \"tid\", \"count\", \"duration\" from \"data\".\"storageItemContents\" where \"storageItemUid\" = $1 order by \"position\"",
    pqxx::params(
      uid));
  std::vector<data::StorageItem::Item> items;
  items.reserve(result.size());
  for (const auto& row : result)
  {
    if (row["position"].as<std::size_t>() != items.size())
    {
      throw std::runtime_error("Storage item contents have an invalid position");
    }

    items.push_back({.tid = row["tid"].as<data::Tid>(),
      .count = row["count"].as<uint32_t>(),
      .duration = std::chrono::seconds(row["duration"].as<int64_t>())});
  }
  return items;
}

void WriteSettingsKeyboardBindings(
  pqxx::work& query,
  const data::Uid uid,
  const std::optional<std::vector<data::Settings::Option>>& bindings)
{
  query.exec(
    "delete from \"data\".\"settingsKeyboardBindings\" where \"settingsUid\" = $1",
    pqxx::params(
      uid));
  if (not bindings)
  {
    return;
  }

  for (std::size_t position = 0; position < bindings->size(); ++position)
  {
    const auto& binding = bindings->at(position);
    query.exec(
      "insert into \"data\".\"settingsKeyboardBindings\" (\"settingsUid\", \"position\", \"primaryKey\", \"type\", \"secondaryKey\") values ($1, $2, $3, $4, $5)",
      pqxx::params(
        uid,
        position,
        binding.primaryKey,
        binding.type,
        binding.secondaryKey));
  }
}

std::optional<std::vector<data::Settings::Option>> ReadSettingsKeyboardBindings(
  pqxx::work& query,
  const data::Uid uid,
  const bool present)
{
  if (not present)
  {
    return std::nullopt;
  }

  const auto result = query.exec(
    "select \"position\", \"primaryKey\", \"type\", \"secondaryKey\" from \"data\".\"settingsKeyboardBindings\" where \"settingsUid\" = $1 order by \"position\"",
    pqxx::params(
      uid));
  std::vector<data::Settings::Option> bindings;
  bindings.reserve(result.size());
  for (const auto& row : result)
  {
    if (row["position"].as<std::size_t>() != bindings.size())
    {
      throw std::runtime_error("Settings bindings have an invalid position");
    }

    bindings.push_back({.primaryKey = row["primaryKey"].as<uint32_t>(),
      .type = row["type"].as<uint32_t>(),
      .secondaryKey = row["secondaryKey"].as<uint32_t>()});
  }
  return bindings;
}

void WriteSettingsGamepadBindings(
  pqxx::work& query,
  const data::Uid uid,
  const std::optional<std::vector<data::Settings::Option>>& bindings)
{
  query.exec(
    "delete from \"data\".\"settingsGamepadBindings\" where \"settingsUid\" = $1",
    pqxx::params(
      uid));
  if (not bindings)
  {
    return;
  }

  for (std::size_t position = 0; position < bindings->size(); ++position)
  {
    const auto& binding = bindings->at(position);
    query.exec(
      "insert into \"data\".\"settingsGamepadBindings\" (\"settingsUid\", \"position\", \"primaryKey\", \"type\", \"secondaryKey\") values ($1, $2, $3, $4, $5)",
      pqxx::params(
        uid,
        position,
        binding.primaryKey,
        binding.type,
        binding.secondaryKey));
  }
}

std::optional<std::vector<data::Settings::Option>> ReadSettingsGamepadBindings(
  pqxx::work& query,
  const data::Uid uid,
  const bool present)
{
  if (not present)
  {
    return std::nullopt;
  }

  const auto result = query.exec(
    "select \"position\", \"primaryKey\", \"type\", \"secondaryKey\" from \"data\".\"settingsGamepadBindings\" where \"settingsUid\" = $1 order by \"position\"",
    pqxx::params(
      uid));
  std::vector<data::Settings::Option> bindings;
  bindings.reserve(result.size());
  for (const auto& row : result)
  {
    if (row["position"].as<std::size_t>() != bindings.size())
    {
      throw std::runtime_error("Settings bindings have an invalid position");
    }

    bindings.push_back({.primaryKey = row["primaryKey"].as<uint32_t>(),
      .type = row["type"].as<uint32_t>(),
      .secondaryKey = row["secondaryKey"].as<uint32_t>()});
  }
  return bindings;
}

void WriteSettingsMacros(
  pqxx::work& query,
  const data::Uid uid,
  const std::optional<std::array<std::string, 8>>& macros)
{
  query.exec(
    "delete from \"data\".\"settingsMacros\" where \"settingsUid\" = $1",
    pqxx::params(
      uid));
  if (not macros)
  {
    return;
  }

  for (std::size_t position = 0; position < macros->size(); ++position)
  {
    query.exec(
      "insert into \"data\".\"settingsMacros\" (\"settingsUid\", \"position\", \"value\") values ($1, $2, $3)",
      pqxx::params(
        uid,
        position,
        macros->at(position)));
  }
}

std::optional<std::array<std::string, 8>> ReadSettingsMacros(
  pqxx::work& query,
  const data::Uid uid,
  const bool present)
{
  if (not present)
  {
    return std::nullopt;
  }

  const auto result = query.exec(
    "select \"position\", \"value\" from \"data\".\"settingsMacros\" where \"settingsUid\" = $1 order by \"position\"",
    pqxx::params(
      uid));
  std::array<std::string, 8> macros{};
  if (result.size() != macros.size())
  {
    throw std::runtime_error("Settings macros have an invalid entry count");
  }

  for (const auto& row : result)
  {
    macros.at(row["position"].as<std::size_t>()) = row["value"].as<std::string>();
  }
  return macros;
}

void WriteDailyQuestEntries(
  pqxx::work& query,
  const data::Uid uid,
  const std::array<data::DailyQuestEntry, 3>& entries)
{
  query.exec(
    "delete from \"data\".\"dailyQuestEntries\" where \"dailyQuestGroupUid\" = $1",
    pqxx::params(
      uid));
  for (std::size_t position = 0; position < entries.size(); ++position)
  {
    const auto& entry = entries.at(position);
    query.exec(
      "insert into \"data\".\"dailyQuestEntries\" (\"dailyQuestGroupUid\", \"position\", \"questId\", \"progress\") values ($1, $2, $3, $4)",
      pqxx::params(
        uid,
        position,
        entry.questId,
        entry.progress));
  }
}

std::array<data::DailyQuestEntry, 3> ReadDailyQuestEntries(
  pqxx::work& query,
  const data::Uid uid)
{
  const auto result = query.exec(
    "select \"position\", \"questId\", \"progress\" from \"data\".\"dailyQuestEntries\" where \"dailyQuestGroupUid\" = $1 order by \"position\"",
    pqxx::params(
      uid));
  std::array<data::DailyQuestEntry, 3> entries{};
  if (result.size() != entries.size())
  {
    throw std::runtime_error("Daily quest group has an invalid entry count");
  }

  for (const auto& row : result)
  {
    entries.at(row["position"].as<std::size_t>()) = {
      .questId = row["questId"].as<uint16_t>(),
      .progress = row["progress"].as<uint32_t>()};
  }
  return entries;
}

} // namespace

void PqDataSource::Establish(
  const std::string& url)
{
  _connection = std::make_unique<pqxx::connection>(url);
}

bool PqDataSource::IsConnectionFine()
{
  return _connection and _connection->is_open();
}

PqDataSource::~PqDataSource() = default;

void PqDataSource::CreateUser(
  data::User& user)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "insert into \"data\".\"users\" (\"name\", \"token\", \"characterUid\", \"lastSeenOnline\") values ($1, $2, $3, $4)",
    pqxx::params(
      WriteField(user.name()),
      WriteField(user.token()),
      WriteField(user.characterUid()),
      WriteField(user.lastSeenOnline())));
  WriteUidList(query, "userInfractions", "userName", user.name(), user.infractions());
  query.commit();
}

void PqDataSource::RetrieveUser(
  const std::string_view& name,
  data::User& user)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("set transaction isolation level repeatable read read only");
  const auto result = query.exec(
    "select * from \"data\".\"users\" where \"name\" = $1",
    pqxx::params(name));
  if (result.empty())
  {
    throw std::runtime_error("User does not exist");
  }
  const auto row = result.one_row();
  ReadField(row, "name", user.name);
  ReadField(row, "token", user.token);
  ReadField(row, "characterUid", user.characterUid);
  ReadField(row, "lastSeenOnline", user.lastSeenOnline);
  user.infractions = ReadUidList(query, "userInfractions", "userName", name);
}

void PqDataSource::StoreUser(
  const std::string_view& name,
  const data::User& user)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"users\" set \"token\" = $2, \"characterUid\" = $3, \"lastSeenOnline\" = $4 where \"name\" = $1",
    pqxx::params(
      name,
      WriteField(user.token()),
      WriteField(user.characterUid()),
      WriteField(user.lastSeenOnline())));
  WriteUidList(query, "userInfractions", "userName", name, user.infractions());
  query.commit();
}

bool PqDataSource::IsUserNameUnique(
  const std::string_view& name)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select 1 from \"data\".\"users\" where lower(\"name\") = lower($1)",
    pqxx::params(name));
  return result.empty();
}

void PqDataSource::CreateInfraction(
  data::Infraction& infraction)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"infractions\" (\"description\", \"punishment\", \"duration\", \"createdAt\") "
    "values ($1, $2, $3, $4) returning \"uid\"",
    pqxx::params(
      WriteField(infraction.description()),
      WriteField(infraction.punishment()),
      WriteField(infraction.duration()),
      WriteField(infraction.createdAt())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  infraction.uid = uid;
}

void PqDataSource::RetrieveInfraction(
  const data::Uid uid,
  data::Infraction& infraction)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"infractions\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Infraction does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", infraction.uid);
  ReadField(row, "description", infraction.description);
  ReadField(row, "punishment", infraction.punishment);
  ReadField(row, "duration", infraction.duration);
  ReadField(row, "createdAt", infraction.createdAt);
}

void PqDataSource::StoreInfraction(
  const data::Uid uid,
  const data::Infraction& infraction)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"infractions\" set \"description\" = $2, \"punishment\" = $3, \"duration\" = $4, \"createdAt\" = $5 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(infraction.description()),
      WriteField(infraction.punishment()),
      WriteField(infraction.duration()),
      WriteField(infraction.createdAt())));
  query.commit();
}

void PqDataSource::DeleteInfraction(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"infractions\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreateCharacter(
  data::Character& character)
{
  pqxx::work query(RequireConnection(_connection));

  const auto result = query.exec(
    "insert into \"data\".\"characters\" ("
    "\"name\", \"introduction\", \"level\", \"experience\", "
    "\"carrots\", \"cash\", \"role\", \"staffRank\", "
    "\"partsModelId\", \"partsMouthId\", \"partsFaceId\", \"appearanceVoiceId\", "
    "\"appearanceHeadSize\", \"appearanceHeight\", \"appearanceThighVolume\", \"appearanceLegVolume\", "
    "\"appearanceEmblemId\", \"guildUid\", \"horseSlotCount\", \"mountUid\", "
    "\"petUid\", \"ranchExperience\", \"totalRaces\", \"isRanchLocked\", "
    "\"isIntroCompleted\", \"settingsUid\", \"dailyQuestGroupUid\", \"mailboxHasNewMail\") "
    "values ("
    "$1, $2, $3, $4, $5, $6, $7, $8, "
    "$9, $10, $11, $12, $13, $14, $15, $16, "
    "$17, $18, $19, $20, $21, $22, $23, $24, "
    "$25, $26, $27, $28) returning \"uid\"",
    pqxx::params(
      WriteField(character.name()),
      WriteField(character.introduction()),
      WriteField(character.level()),
      WriteField(character.experience()),
      WriteField(character.carrots()),
      WriteField(character.cash()),
      WriteField(character.role()),
      WriteField(character.staffRank()),
      WriteField(character.parts.modelId()),
      WriteField(character.parts.mouthId()),
      WriteField(character.parts.faceId()),
      WriteField(character.appearance.voiceId()),
      WriteField(character.appearance.headSize()),
      WriteField(character.appearance.height()),
      WriteField(character.appearance.thighVolume()),
      WriteField(character.appearance.legVolume()),
      WriteField(character.appearance.emblemId()),
      WriteField(character.guildUid()),
      WriteField(character.horseSlotCount()),
      WriteField(character.mountUid()),
      WriteField(character.petUid()),
      WriteField(character.ranchManagement.ranchExperience()),
      WriteField(character.ranchManagement.totalRaces()),
      WriteField(character.isRanchLocked()),
      WriteField(character.isIntroCompleted()),
      WriteField(character.settingsUid()),
      WriteField(character.dailyQuestGroupUid()),
      WriteField(character.mailbox.hasNewMail())));

  const auto uid = result.one_row()["uid"].as<data::Uid>();
  WriteCharacterRelations(query, uid, character);
  query.commit();
  character.uid = uid;
}

void PqDataSource::RetrieveCharacter(
  const data::Uid uid,
  data::Character& character)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("set transaction isolation level repeatable read read only");

  const auto result = query.exec(
    "select * from \"data\".\"characters\" where \"uid\" = $1",
    pqxx::params(uid));

  if (result.empty())
  {
    throw std::runtime_error("Character does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", character.uid);
  ReadField(row, "name", character.name);
  ReadField(row, "introduction", character.introduction);
  ReadField(row, "level", character.level);
  ReadField(row, "experience", character.experience);
  ReadField(row, "carrots", character.carrots);
  ReadField(row, "cash", character.cash);
  ReadField(row, "role", character.role);
  ReadField(row, "staffRank", character.staffRank);
  ReadField(row, "partsModelId", character.parts.modelId);
  ReadField(row, "partsMouthId", character.parts.mouthId);
  ReadField(row, "partsFaceId", character.parts.faceId);
  ReadField(row, "appearanceVoiceId", character.appearance.voiceId);
  ReadField(row, "appearanceHeadSize", character.appearance.headSize);
  ReadField(row, "appearanceHeight", character.appearance.height);
  ReadField(row, "appearanceThighVolume", character.appearance.thighVolume);
  ReadField(row, "appearanceLegVolume", character.appearance.legVolume);
  ReadField(row, "appearanceEmblemId", character.appearance.emblemId);
  ReadField(row, "guildUid", character.guildUid);
  ReadField(row, "horseSlotCount", character.horseSlotCount);
  ReadField(row, "mountUid", character.mountUid);
  ReadField(row, "petUid", character.petUid);
  ReadField(row, "ranchExperience", character.ranchManagement.ranchExperience);
  ReadField(row, "totalRaces", character.ranchManagement.totalRaces);
  ReadField(row, "isRanchLocked", character.isRanchLocked);
  ReadField(row, "isIntroCompleted", character.isIntroCompleted);
  ReadField(row, "settingsUid", character.settingsUid);
  ReadField(row, "dailyQuestGroupUid", character.dailyQuestGroupUid);
  ReadField(row, "mailboxHasNewMail", character.mailbox.hasNewMail);
  ReadCharacterRelations(query, uid, character);
}

void PqDataSource::StoreCharacter(
  const data::Uid uid,
  const data::Character& character)
{
  pqxx::work query(RequireConnection(_connection));

  query.exec(
    "update \"data\".\"characters\" set "
    "\"name\" = $2, \"introduction\" = $3, \"level\" = $4, "
    "\"experience\" = $5, \"carrots\" = $6, \"cash\" = $7, "
    "\"role\" = $8, \"staffRank\" = $9, \"partsModelId\" = $10, "
    "\"partsMouthId\" = $11, \"partsFaceId\" = $12, \"appearanceVoiceId\" = $13, "
    "\"appearanceHeadSize\" = $14, \"appearanceHeight\" = $15, \"appearanceThighVolume\" = $16, "
    "\"appearanceLegVolume\" = $17, \"appearanceEmblemId\" = $18, \"guildUid\" = $19, "
    "\"horseSlotCount\" = $20, \"mountUid\" = $21, \"petUid\" = $22, "
    "\"ranchExperience\" = $23, \"totalRaces\" = $24, \"isRanchLocked\" = $25, "
    "\"isIntroCompleted\" = $26, \"settingsUid\" = $27, \"dailyQuestGroupUid\" = $28, "
    "\"mailboxHasNewMail\" = $29 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(character.name()),
      WriteField(character.introduction()),
      WriteField(character.level()),
      WriteField(character.experience()),
      WriteField(character.carrots()),
      WriteField(character.cash()),
      WriteField(character.role()),
      WriteField(character.staffRank()),
      WriteField(character.parts.modelId()),
      WriteField(character.parts.mouthId()),
      WriteField(character.parts.faceId()),
      WriteField(character.appearance.voiceId()),
      WriteField(character.appearance.headSize()),
      WriteField(character.appearance.height()),
      WriteField(character.appearance.thighVolume()),
      WriteField(character.appearance.legVolume()),
      WriteField(character.appearance.emblemId()),
      WriteField(character.guildUid()),
      WriteField(character.horseSlotCount()),
      WriteField(character.mountUid()),
      WriteField(character.petUid()),
      WriteField(character.ranchManagement.ranchExperience()),
      WriteField(character.ranchManagement.totalRaces()),
      WriteField(character.isRanchLocked()),
      WriteField(character.isIntroCompleted()),
      WriteField(character.settingsUid()),
      WriteField(character.dailyQuestGroupUid()),
      WriteField(character.mailbox.hasNewMail())));

  WriteCharacterRelations(query, uid, character);
  query.commit();
}

void PqDataSource::DeleteCharacter(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "delete from \"data\".\"characters\" where \"uid\" = $1",
    pqxx::params(uid));
  query.commit();
}

data::Uid PqDataSource::RetrieveCharacterUidByName(
  const std::string_view& name)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select \"uid\" from \"data\".\"characters\" where lower(\"name\") = lower($1)",
    pqxx::params(name));
  return result.empty() ? data::InvalidUid : result.front()["uid"].as<data::Uid>();
}

bool PqDataSource::IsCharacterNameUnique(
  const std::string_view& name)
{
  return RetrieveCharacterUidByName(name) == data::InvalidUid;
}

void PqDataSource::CreateHorse(
  data::Horse& horse)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"horses\" (\"tid\", \"name\", \"partsSkinTid\", \"partsFaceTid\", \"partsManeTid\", \"partsTailTid\", "
    "\"appearanceScale\", \"appearanceLegLength\", \"appearanceLegVolume\", \"appearanceBodyLength\", "
    "\"appearanceBodyVolume\", \"statsAgility\", \"statsCourage\", \"statsRush\", \"statsEndurance\", \"statsAmbition\", "
    "\"masterySpurMagicCount\", \"masteryJumpCount\", \"masterySlidingTime\", \"masteryGlidingDistance\", "
    "\"rating\", \"clazz\", \"clazzProgress\", \"grade\", \"growthPoints\", \"breedingCount\", \"breedingCombo\", \"type\", "
    "\"dateOfBirth\", \"tendency\", \"spirit\", \"potentialType\", \"potentialLevel\", \"potentialValue\", \"luckState\", "
    "\"fatigue\", \"emblemUid\", \"mountConditionStamina\", \"mountConditionCharm\", \"mountConditionFriendliness\", "
    "\"mountConditionInjury\", \"mountConditionPlenitude\", \"mountConditionBodyDirtiness\", "
    "\"mountConditionManeDirtiness\", \"mountConditionTailDirtiness\", \"mountConditionBodyPolish\", "
    "\"mountConditionManePolish\", \"mountConditionTailPolish\", \"mountConditionAttachment\", "
    "\"mountConditionBoredom\", \"mountConditionLastDailyCareTick\", \"mountConditionStopAmendsPoint\", "
    "\"mountInfoBoostsInARow\", \"mountInfoWinsSpeedSingle\", \"mountInfoWinsSpeedTeam\", "
    "\"mountInfoWinsMagicSingle\", \"mountInfoWinsMagicTeam\", \"mountInfoTotalDistance\", "
    "\"mountInfoTopSpeed\", \"mountInfoLongestGlideDistance\", \"mountInfoParticipated\", "
    "\"mountInfoCumulativePrize\", \"mountInfoBiggestPrize\", \"ancestorsFather\", \"ancestorsMother\", \"lineage\") "
    "values ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $20, "
    "$21, $22, $23, $24, $25, $26, $27, $28, $29, $30, $31, $32, $33, $34, $35, $36, $37, $38, $39, $40, "
    "$41, $42, $43, $44, $45, $46, $47, $48, $49, $50, $51, $52, $53, $54, $55, $56, $57, $58, $59, $60, "
    "$61, $62, $63, $64, $65, $66) returning \"uid\"",
    pqxx::params(
      WriteField(horse.tid()),
      WriteField(horse.name()),
      WriteField(horse.parts.skinTid()),
      WriteField(horse.parts.faceTid()),
      WriteField(horse.parts.maneTid()),
      WriteField(horse.parts.tailTid()),
      WriteField(horse.appearance.scale()),
      WriteField(horse.appearance.legLength()),
      WriteField(horse.appearance.legVolume()),
      WriteField(horse.appearance.bodyLength()),
      WriteField(horse.appearance.bodyVolume()),
      WriteField(horse.stats.agility()),
      WriteField(horse.stats.courage()),
      WriteField(horse.stats.rush()),
      WriteField(horse.stats.endurance()),
      WriteField(horse.stats.ambition()),
      WriteField(horse.mastery.spurMagicCount()),
      WriteField(horse.mastery.jumpCount()),
      WriteField(horse.mastery.slidingTime()),
      WriteField(horse.mastery.glidingDistance()),
      WriteField(horse.rating()),
      WriteField(horse.clazz()),
      WriteField(horse.clazzProgress()),
      WriteField(horse.grade()),
      WriteField(horse.growthPoints()),
      WriteField(horse.breedingCount()),
      WriteField(horse.breedingCombo()),
      WriteField(horse.type()),
      WriteField(horse.dateOfBirth()),
      WriteField(horse.tendency()),
      WriteField(horse.spirit()),
      WriteField(horse.potential.type()),
      WriteField(horse.potential.level()),
      WriteField(horse.potential.value()),
      WriteField(horse.luckState()),
      WriteField(horse.fatigue()),
      WriteField(horse.emblemUid()),
      WriteField(horse.mountCondition.stamina()),
      WriteField(horse.mountCondition.charm()),
      WriteField(horse.mountCondition.friendliness()),
      WriteField(horse.mountCondition.injury()),
      WriteField(horse.mountCondition.plenitude()),
      WriteField(horse.mountCondition.bodyDirtiness()),
      WriteField(horse.mountCondition.maneDirtiness()),
      WriteField(horse.mountCondition.tailDirtiness()),
      WriteField(horse.mountCondition.bodyPolish()),
      WriteField(horse.mountCondition.manePolish()),
      WriteField(horse.mountCondition.tailPolish()),
      WriteField(horse.mountCondition.attachment()),
      WriteField(horse.mountCondition.boredom()),
      WriteField(horse.mountCondition.lastDailyCareTick()),
      WriteField(horse.mountCondition.stopAmendsPoint()),
      WriteField(horse.mountInfo.boostsInARow()),
      WriteField(horse.mountInfo.winsSpeedSingle()),
      WriteField(horse.mountInfo.winsSpeedTeam()),
      WriteField(horse.mountInfo.winsMagicSingle()),
      WriteField(horse.mountInfo.winsMagicTeam()),
      WriteField(horse.mountInfo.totalDistance()),
      WriteField(horse.mountInfo.topSpeed()),
      WriteField(horse.mountInfo.longestGlideDistance()),
      WriteField(horse.mountInfo.participated()),
      WriteField(horse.mountInfo.cumulativePrize()),
      WriteField(horse.mountInfo.biggestPrize()),
      WriteField(horse.ancestors.father),
      WriteField(horse.ancestors.mother),
      WriteField(horse.lineage())));

  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  horse.uid = uid;
}

void PqDataSource::RetrieveHorse(
  const data::Uid uid,
  data::Horse& horse)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"horses\" where \"uid\" = $1",
    pqxx::params(uid));

  if (result.empty())
  {
    throw std::runtime_error("Horse does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", horse.uid);
  ReadField(row, "tid", horse.tid);
  ReadField(row, "name", horse.name);
  ReadField(row, "partsSkinTid", horse.parts.skinTid);
  ReadField(row, "partsFaceTid", horse.parts.faceTid);
  ReadField(row, "partsManeTid", horse.parts.maneTid);
  ReadField(row, "partsTailTid", horse.parts.tailTid);
  ReadField(row, "appearanceScale", horse.appearance.scale);
  ReadField(row, "appearanceLegLength", horse.appearance.legLength);
  ReadField(row, "appearanceLegVolume", horse.appearance.legVolume);
  ReadField(row, "appearanceBodyLength", horse.appearance.bodyLength);
  ReadField(row, "appearanceBodyVolume", horse.appearance.bodyVolume);
  ReadField(row, "statsAgility", horse.stats.agility);
  ReadField(row, "statsCourage", horse.stats.courage);
  ReadField(row, "statsRush", horse.stats.rush);
  ReadField(row, "statsEndurance", horse.stats.endurance);
  ReadField(row, "statsAmbition", horse.stats.ambition);
  ReadField(row, "masterySpurMagicCount", horse.mastery.spurMagicCount);
  ReadField(row, "masteryJumpCount", horse.mastery.jumpCount);
  ReadField(row, "masterySlidingTime", horse.mastery.slidingTime);
  ReadField(row, "masteryGlidingDistance", horse.mastery.glidingDistance);
  ReadField(row, "rating", horse.rating);
  ReadField(row, "clazz", horse.clazz);
  ReadField(row, "clazzProgress", horse.clazzProgress);
  ReadField(row, "grade", horse.grade);
  ReadField(row, "growthPoints", horse.growthPoints);
  ReadField(row, "breedingCount", horse.breedingCount);
  ReadField(row, "breedingCombo", horse.breedingCombo);
  ReadField(row, "type", horse.type);
  ReadField(row, "dateOfBirth", horse.dateOfBirth);
  ReadField(row, "tendency", horse.tendency);
  ReadField(row, "spirit", horse.spirit);
  ReadField(row, "potentialType", horse.potential.type);
  ReadField(row, "potentialLevel", horse.potential.level);
  ReadField(row, "potentialValue", horse.potential.value);
  ReadField(row, "luckState", horse.luckState);
  ReadField(row, "fatigue", horse.fatigue);
  ReadField(row, "emblemUid", horse.emblemUid);
  ReadField(row, "mountConditionStamina", horse.mountCondition.stamina);
  ReadField(row, "mountConditionCharm", horse.mountCondition.charm);
  ReadField(row, "mountConditionFriendliness", horse.mountCondition.friendliness);
  ReadField(row, "mountConditionInjury", horse.mountCondition.injury);
  ReadField(row, "mountConditionPlenitude", horse.mountCondition.plenitude);
  ReadField(row, "mountConditionBodyDirtiness", horse.mountCondition.bodyDirtiness);
  ReadField(row, "mountConditionManeDirtiness", horse.mountCondition.maneDirtiness);
  ReadField(row, "mountConditionTailDirtiness", horse.mountCondition.tailDirtiness);
  ReadField(row, "mountConditionBodyPolish", horse.mountCondition.bodyPolish);
  ReadField(row, "mountConditionManePolish", horse.mountCondition.manePolish);
  ReadField(row, "mountConditionTailPolish", horse.mountCondition.tailPolish);
  ReadField(row, "mountConditionAttachment", horse.mountCondition.attachment);
  ReadField(row, "mountConditionBoredom", horse.mountCondition.boredom);
  ReadField(row, "mountConditionLastDailyCareTick", horse.mountCondition.lastDailyCareTick);
  ReadField(row, "mountConditionStopAmendsPoint", horse.mountCondition.stopAmendsPoint);
  ReadField(row, "mountInfoBoostsInARow", horse.mountInfo.boostsInARow);
  ReadField(row, "mountInfoWinsSpeedSingle", horse.mountInfo.winsSpeedSingle);
  ReadField(row, "mountInfoWinsSpeedTeam", horse.mountInfo.winsSpeedTeam);
  ReadField(row, "mountInfoWinsMagicSingle", horse.mountInfo.winsMagicSingle);
  ReadField(row, "mountInfoWinsMagicTeam", horse.mountInfo.winsMagicTeam);
  ReadField(row, "mountInfoTotalDistance", horse.mountInfo.totalDistance);
  ReadField(row, "mountInfoTopSpeed", horse.mountInfo.topSpeed);
  ReadField(row, "mountInfoLongestGlideDistance", horse.mountInfo.longestGlideDistance);
  ReadField(row, "mountInfoParticipated", horse.mountInfo.participated);
  ReadField(row, "mountInfoCumulativePrize", horse.mountInfo.cumulativePrize);
  ReadField(row, "mountInfoBiggestPrize", horse.mountInfo.biggestPrize);
  horse.ancestors.father = row["ancestorsFather"].as<data::Uid>();
  horse.ancestors.mother = row["ancestorsMother"].as<data::Uid>();
  ReadField(row, "lineage", horse.lineage);
}

void PqDataSource::StoreHorse(
  const data::Uid uid,
  const data::Horse& horse)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"horses\" set \"tid\" = $2, \"name\" = $3, \"partsSkinTid\" = $4, \"partsFaceTid\" = $5, "
    "\"partsManeTid\" = $6, \"partsTailTid\" = $7, \"appearanceScale\" = $8, \"appearanceLegLength\" = $9, "
    "\"appearanceLegVolume\" = $10, \"appearanceBodyLength\" = $11, \"appearanceBodyVolume\" = $12, "
    "\"statsAgility\" = $13, \"statsCourage\" = $14, \"statsRush\" = $15, \"statsEndurance\" = $16, "
    "\"statsAmbition\" = $17, \"masterySpurMagicCount\" = $18, \"masteryJumpCount\" = $19, "
    "\"masterySlidingTime\" = $20, \"masteryGlidingDistance\" = $21, \"rating\" = $22, \"clazz\" = $23, "
    "\"clazzProgress\" = $24, \"grade\" = $25, \"growthPoints\" = $26, \"breedingCount\" = $27, \"breedingCombo\" = $28, "
    "\"type\" = $29, \"dateOfBirth\" = $30, \"tendency\" = $31, \"spirit\" = $32, \"potentialType\" = $33, "
    "\"potentialLevel\" = $34, \"potentialValue\" = $35, \"luckState\" = $36, \"fatigue\" = $37, \"emblemUid\" = $38, "
    "\"mountConditionStamina\" = $39, \"mountConditionCharm\" = $40, \"mountConditionFriendliness\" = $41, "
    "\"mountConditionInjury\" = $42, \"mountConditionPlenitude\" = $43, "
    "\"mountConditionBodyDirtiness\" = $44, \"mountConditionManeDirtiness\" = $45, "
    "\"mountConditionTailDirtiness\" = $46, \"mountConditionBodyPolish\" = $47, "
    "\"mountConditionManePolish\" = $48, \"mountConditionTailPolish\" = $49, "
    "\"mountConditionAttachment\" = $50, \"mountConditionBoredom\" = $51, "
    "\"mountConditionLastDailyCareTick\" = $52, \"mountConditionStopAmendsPoint\" = $53, "
    "\"mountInfoBoostsInARow\" = $54, \"mountInfoWinsSpeedSingle\" = $55, "
    "\"mountInfoWinsSpeedTeam\" = $56, \"mountInfoWinsMagicSingle\" = $57, "
    "\"mountInfoWinsMagicTeam\" = $58, \"mountInfoTotalDistance\" = $59, \"mountInfoTopSpeed\" = $60, "
    "\"mountInfoLongestGlideDistance\" = $61, \"mountInfoParticipated\" = $62, "
    "\"mountInfoCumulativePrize\" = $63, \"mountInfoBiggestPrize\" = $64, \"ancestorsFather\" = $65, "
    "\"ancestorsMother\" = $66, \"lineage\" = $67 where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(horse.tid()),
      WriteField(horse.name()),
      WriteField(horse.parts.skinTid()),
      WriteField(horse.parts.faceTid()),
      WriteField(horse.parts.maneTid()),
      WriteField(horse.parts.tailTid()),
      WriteField(horse.appearance.scale()),
      WriteField(horse.appearance.legLength()),
      WriteField(horse.appearance.legVolume()),
      WriteField(horse.appearance.bodyLength()),
      WriteField(horse.appearance.bodyVolume()),
      WriteField(horse.stats.agility()),
      WriteField(horse.stats.courage()),
      WriteField(horse.stats.rush()),
      WriteField(horse.stats.endurance()),
      WriteField(horse.stats.ambition()),
      WriteField(horse.mastery.spurMagicCount()),
      WriteField(horse.mastery.jumpCount()),
      WriteField(horse.mastery.slidingTime()),
      WriteField(horse.mastery.glidingDistance()),
      WriteField(horse.rating()),
      WriteField(horse.clazz()),
      WriteField(horse.clazzProgress()),
      WriteField(horse.grade()),
      WriteField(horse.growthPoints()),
      WriteField(horse.breedingCount()),
      WriteField(horse.breedingCombo()),
      WriteField(horse.type()),
      WriteField(horse.dateOfBirth()),
      WriteField(horse.tendency()),
      WriteField(horse.spirit()),
      WriteField(horse.potential.type()),
      WriteField(horse.potential.level()),
      WriteField(horse.potential.value()),
      WriteField(horse.luckState()),
      WriteField(horse.fatigue()),
      WriteField(horse.emblemUid()),
      WriteField(horse.mountCondition.stamina()),
      WriteField(horse.mountCondition.charm()),
      WriteField(horse.mountCondition.friendliness()),
      WriteField(horse.mountCondition.injury()),
      WriteField(horse.mountCondition.plenitude()),
      WriteField(horse.mountCondition.bodyDirtiness()),
      WriteField(horse.mountCondition.maneDirtiness()),
      WriteField(horse.mountCondition.tailDirtiness()),
      WriteField(horse.mountCondition.bodyPolish()),
      WriteField(horse.mountCondition.manePolish()),
      WriteField(horse.mountCondition.tailPolish()),
      WriteField(horse.mountCondition.attachment()),
      WriteField(horse.mountCondition.boredom()),
      WriteField(horse.mountCondition.lastDailyCareTick()),
      WriteField(horse.mountCondition.stopAmendsPoint()),
      WriteField(horse.mountInfo.boostsInARow()),
      WriteField(horse.mountInfo.winsSpeedSingle()),
      WriteField(horse.mountInfo.winsSpeedTeam()),
      WriteField(horse.mountInfo.winsMagicSingle()),
      WriteField(horse.mountInfo.winsMagicTeam()),
      WriteField(horse.mountInfo.totalDistance()),
      WriteField(horse.mountInfo.topSpeed()),
      WriteField(horse.mountInfo.longestGlideDistance()),
      WriteField(horse.mountInfo.participated()),
      WriteField(horse.mountInfo.cumulativePrize()),
      WriteField(horse.mountInfo.biggestPrize()),
      WriteField(horse.ancestors.father),
      WriteField(horse.ancestors.mother),
      WriteField(horse.lineage())));
  query.commit();
}

void PqDataSource::DeleteHorse(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "delete from \"data\".\"horses\" where \"uid\" = $1",
    pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreateItem(
  data::Item& item)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"items\" (\"tid\", \"count\", \"duration\", \"createdAt\") "
    "values ($1, $2, $3, $4) returning \"uid\"",
    pqxx::params(
      WriteField(item.tid()),
      WriteField(item.count()),
      WriteField(item.duration()),
      WriteField(item.createdAt())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  item.uid = uid;
}

void PqDataSource::RetrieveItem(
  const data::Uid uid,
  data::Item& item)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"items\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Item does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", item.uid);
  ReadField(row, "tid", item.tid);
  ReadField(row, "count", item.count);
  ReadField(row, "duration", item.duration);
  ReadField(row, "createdAt", item.createdAt);
}

void PqDataSource::StoreItem(
  const data::Uid uid,
  const data::Item& item)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"items\" set \"tid\" = $2, \"count\" = $3, \"duration\" = $4, \"createdAt\" = $5 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(item.tid()),
      WriteField(item.count()),
      WriteField(item.duration()),
      WriteField(item.createdAt())));
  query.commit();
}

void PqDataSource::DeleteItem(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"items\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreateStorageItem(
  data::StorageItem& storageItem)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"storageItems\" (\"sender\", \"message\", \"carrots\", \"checked\", \"createdAt\", \"duration\", "
    "\"goodsSq\", \"priceId\") values ($1, $2, $3, $4, $5, $6, $7, $8) returning \"uid\"",
    pqxx::params(
      WriteField(storageItem.sender()),
      WriteField(storageItem.message()),
      WriteField(storageItem.carrots()),
      WriteField(storageItem.checked()),
      WriteField(storageItem.createdAt()),
      WriteField(storageItem.duration()),
      WriteField(storageItem.goodsSq()),
      WriteField(storageItem.priceId())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  WriteStorageItemContents(query, uid, storageItem.items());
  query.commit();
  storageItem.uid = uid;
}

void PqDataSource::RetrieveStorageItem(
  const data::Uid uid,
  data::StorageItem& storageItem)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("set transaction isolation level repeatable read read only");
  const auto result = query.exec(
    "select * from \"data\".\"storageItems\" where \"uid\" = $1",
    pqxx::params(
      uid));
  if (result.empty())
  {
    throw std::runtime_error("StorageItem does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", storageItem.uid);
  ReadField(row, "sender", storageItem.sender);
  ReadField(row, "message", storageItem.message);
  ReadField(row, "carrots", storageItem.carrots);
  ReadField(row, "checked", storageItem.checked);
  ReadField(row, "createdAt", storageItem.createdAt);
  ReadField(row, "duration", storageItem.duration);
  ReadField(row, "goodsSq", storageItem.goodsSq);
  ReadField(row, "priceId", storageItem.priceId);
  storageItem.items = ReadStorageItemContents(query, uid);
}

void PqDataSource::StoreStorageItem(
  const data::Uid uid,
  const data::StorageItem& storageItem)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"storageItems\" set \"sender\" = $2, \"message\" = $3, \"carrots\" = $4, \"checked\" = $5, \"createdAt\" = "
    "$6, \"duration\" = $7, \"goodsSq\" = $8, \"priceId\" = $9 where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(storageItem.sender()),
      WriteField(storageItem.message()),
      WriteField(storageItem.carrots()),
      WriteField(storageItem.checked()),
      WriteField(storageItem.createdAt()),
      WriteField(storageItem.duration()),
      WriteField(storageItem.goodsSq()),
      WriteField(storageItem.priceId())));
  WriteStorageItemContents(query, uid, storageItem.items());
  query.commit();
}

void PqDataSource::DeleteStorageItem(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "delete from \"data\".\"storageItems\" where \"uid\" = $1",
    pqxx::params(
      uid));
  query.commit();
}

void PqDataSource::CreateEgg(
  data::Egg& egg)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"eggs\" (\"itemUid\", \"itemTid\", \"incubatedAt\", \"incubatorSlot\", \"boostsUsed\") "
    "values ($1, $2, $3, $4, $5) returning \"uid\"",
    pqxx::params(
      WriteField(egg.itemUid()),
      WriteField(egg.itemTid()),
      WriteField(egg.incubatedAt()),
      WriteField(egg.incubatorSlot()),
      WriteField(egg.boostsUsed())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  egg.uid = uid;
}

void PqDataSource::RetrieveEgg(
  const data::Uid uid,
  data::Egg& egg)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"eggs\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Egg does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", egg.uid);
  ReadField(row, "itemUid", egg.itemUid);
  ReadField(row, "itemTid", egg.itemTid);
  ReadField(row, "incubatedAt", egg.incubatedAt);
  ReadField(row, "incubatorSlot", egg.incubatorSlot);
  ReadField(row, "boostsUsed", egg.boostsUsed);
}

void PqDataSource::StoreEgg(
  const data::Uid uid,
  const data::Egg& egg)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"eggs\" set \"itemUid\" = $2, \"itemTid\" = $3, \"incubatedAt\" = $4, \"incubatorSlot\" = $5, \"boostsUsed\" = $6 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(egg.itemUid()),
      WriteField(egg.itemTid()),
      WriteField(egg.incubatedAt()),
      WriteField(egg.incubatorSlot()),
      WriteField(egg.boostsUsed())));
  query.commit();
}

void PqDataSource::DeleteEgg(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"eggs\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreatePet(
  data::Pet& pet)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"pets\" (\"itemUid\", \"petId\", \"name\", \"birthDate\") "
    "values ($1, $2, $3, $4) returning \"uid\"",
    pqxx::params(
      WriteField(pet.itemUid()),
      WriteField(pet.petId()),
      WriteField(pet.name()),
      WriteField(pet.birthDate())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  pet.uid = uid;
}

void PqDataSource::RetrievePet(
  const data::Uid uid,
  data::Pet& pet)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"pets\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Pet does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", pet.uid);
  ReadField(row, "itemUid", pet.itemUid);
  ReadField(row, "petId", pet.petId);
  ReadField(row, "name", pet.name);
  ReadField(row, "birthDate", pet.birthDate);
}

void PqDataSource::StorePet(
  const data::Uid uid,
  const data::Pet& pet)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"pets\" set \"itemUid\" = $2, \"petId\" = $3, \"name\" = $4, \"birthDate\" = $5 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(pet.itemUid()),
      WriteField(pet.petId()),
      WriteField(pet.name()),
      WriteField(pet.birthDate())));
  query.commit();
}

void PqDataSource::DeletePet(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"pets\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreateHousing(
  data::Housing& housing)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"housing\" (\"housingId\", \"expiresAt\", \"durability\") "
    "values ($1, $2, $3) returning \"uid\"",
    pqxx::params(
      WriteField(housing.housingId()),
      WriteField(housing.expiresAt()),
      WriteField(housing.durability())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  housing.uid = uid;
}

void PqDataSource::RetrieveHousing(
  const data::Uid uid,
  data::Housing& housing)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"housing\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Housing does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", housing.uid);
  ReadField(row, "housingId", housing.housingId);
  ReadField(row, "expiresAt", housing.expiresAt);
  ReadField(row, "durability", housing.durability);
}

void PqDataSource::StoreHousing(
  const data::Uid uid,
  const data::Housing& housing)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"housing\" set \"housingId\" = $2, \"expiresAt\" = $3, \"durability\" = $4 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(housing.housingId()),
      WriteField(housing.expiresAt()),
      WriteField(housing.durability())));
  query.commit();
}

void PqDataSource::DeleteHousing(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"housing\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreateGuild(
  data::Guild& guild)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"guilds\" (\"name\", \"description\", \"owner\", \"rank\", \"totalWins\", \"totalLosses\", \"seasonalWins\", \"seasonalLosses\") values ($1, $2, $3, $4, $5, $6, $7, $8) returning \"uid\"",
    pqxx::params(
      WriteField(guild.name()),
      WriteField(guild.description()),
      WriteField(guild.owner()),
      WriteField(guild.rank()),
      WriteField(guild.totalWins()),
      WriteField(guild.totalLosses()),
      WriteField(guild.seasonalWins()),
      WriteField(guild.seasonalLosses())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  WriteUidList(query, "guildOfficers", "guildUid", uid, guild.officers());
  WriteUidList(query, "guildMembers", "guildUid", uid, guild.members());
  query.commit();
  guild.uid = uid;
}

void PqDataSource::RetrieveGuild(
  const data::Uid uid,
  data::Guild& guild)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("set transaction isolation level repeatable read read only");
  const auto result = query.exec(
    "select * from \"data\".\"guilds\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Guild does not exist");
  }
  const auto row = result.one_row();
  ReadField(row, "name", guild.name);
  ReadField(row, "description", guild.description);
  ReadField(row, "owner", guild.owner);
  ReadField(row, "rank", guild.rank);
  ReadField(row, "totalWins", guild.totalWins);
  ReadField(row, "totalLosses", guild.totalLosses);
  ReadField(row, "seasonalWins", guild.seasonalWins);
  ReadField(row, "seasonalLosses", guild.seasonalLosses);
  ReadField(row, "uid", guild.uid);
  guild.officers = ReadUidList(query, "guildOfficers", "guildUid", uid);
  guild.members = ReadUidList(query, "guildMembers", "guildUid", uid);
}

void PqDataSource::StoreGuild(
  const data::Uid uid,
  const data::Guild& guild)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"guilds\" set \"name\" = $2, \"description\" = $3, \"owner\" = $4, \"rank\" = $5, \"totalWins\" = $6, \"totalLosses\" = $7, \"seasonalWins\" = $8, \"seasonalLosses\" = $9 where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(guild.name()),
      WriteField(guild.description()),
      WriteField(guild.owner()),
      WriteField(guild.rank()),
      WriteField(guild.totalWins()),
      WriteField(guild.totalLosses()),
      WriteField(guild.seasonalWins()),
      WriteField(guild.seasonalLosses())));
  WriteUidList(query, "guildOfficers", "guildUid", uid, guild.officers());
  WriteUidList(query, "guildMembers", "guildUid", uid, guild.members());
  query.commit();
}

void PqDataSource::DeleteGuild(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"guilds\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

bool PqDataSource::IsGuildNameUnique(
  const std::string_view& name)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select 1 from \"data\".\"guilds\" where lower(\"name\") = lower($1)",
    pqxx::params(name));
  return result.empty();
}

void PqDataSource::CreateSettings(
  data::Settings& settings)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"settings\" (\"hasKeyboardBindings\", \"hasMacros\", \"hasGamepadBindings\", \"age\", \"hideAge\") "
    "values ($1, $2, $3, $4, $5) returning \"uid\"",
    pqxx::params(
      settings.keyboardBindings().has_value(),
      settings.macros().has_value(),
      settings.gamepadBindings().has_value(),
      WriteField(settings.age()),
      WriteField(settings.hideAge())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  WriteSettingsKeyboardBindings(query, uid, settings.keyboardBindings());
  WriteSettingsMacros(query, uid, settings.macros());
  WriteSettingsGamepadBindings(query, uid, settings.gamepadBindings());
  query.commit();
  settings.uid = uid;
}

void PqDataSource::RetrieveSettings(
  const data::Uid uid,
  data::Settings& settings)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("set transaction isolation level repeatable read read only");
  const auto result = query.exec(
    "select * from \"data\".\"settings\" where \"uid\" = $1",
    pqxx::params(
      uid));
  if (result.empty())
  {
    throw std::runtime_error("Settings does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", settings.uid);
  ReadField(row, "age", settings.age);
  ReadField(row, "hideAge", settings.hideAge);
  settings.keyboardBindings = ReadSettingsKeyboardBindings(query, uid, row["hasKeyboardBindings"].as<bool>());
  settings.macros = ReadSettingsMacros(query, uid, row["hasMacros"].as<bool>());
  settings.gamepadBindings = ReadSettingsGamepadBindings(query, uid, row["hasGamepadBindings"].as<bool>());
}

void PqDataSource::StoreSettings(
  const data::Uid uid,
  const data::Settings& settings)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"settings\" set \"hasKeyboardBindings\" = $2, \"hasMacros\" = $3, \"hasGamepadBindings\" = $4, \"age\" = "
    "$5, \"hideAge\" = $6 where \"uid\" = $1",
    pqxx::params(
      uid,
      settings.keyboardBindings().has_value(),
      settings.macros().has_value(),
      settings.gamepadBindings().has_value(),
      WriteField(settings.age()),
      WriteField(settings.hideAge())));
  WriteSettingsKeyboardBindings(query, uid, settings.keyboardBindings());
  WriteSettingsMacros(query, uid, settings.macros());
  WriteSettingsGamepadBindings(query, uid, settings.gamepadBindings());
  query.commit();
}

void PqDataSource::DeleteSettings(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "delete from \"data\".\"settings\" where \"uid\" = $1",
    pqxx::params(
      uid));
  query.commit();
}

void PqDataSource::CreateDailyQuestGroup(
  data::DailyQuestGroup& group)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"dailyQuestGroups\" (\"rewardId\", \"rewardType\", \"rewardPoints\", \"carrotsClaimed\") values ($1, "
    "$2, $3, $4) returning \"uid\"",
    pqxx::params(
      WriteField(group.rewardId()),
      WriteField(group.rewardType()),
      WriteField(group.rewardPoints()),
      WriteField(group.carrotsClaimed())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  WriteDailyQuestEntries(query, uid, group.quests());
  query.commit();
  group.uid = uid;
}

void PqDataSource::RetrieveDailyQuestGroup(
  const data::Uid uid,
  data::DailyQuestGroup& group)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("set transaction isolation level repeatable read read only");
  const auto result = query.exec(
    "select * from \"data\".\"dailyQuestGroups\" where \"uid\" = $1",
    pqxx::params(
      uid));
  if (result.empty())
  {
    throw std::runtime_error("DailyQuestGroup does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", group.uid);
  ReadField(row, "rewardId", group.rewardId);
  ReadField(row, "rewardType", group.rewardType);
  ReadField(row, "rewardPoints", group.rewardPoints);
  ReadField(row, "carrotsClaimed", group.carrotsClaimed);
  group.quests = ReadDailyQuestEntries(query, uid);
}

void PqDataSource::StoreDailyQuestGroup(
  const data::Uid uid,
  const data::DailyQuestGroup& group)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"dailyQuestGroups\" set \"rewardId\" = $2, \"rewardType\" = $3, \"rewardPoints\" = $4, \"carrotsClaimed\" "
    "= $5 where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(group.rewardId()),
      WriteField(group.rewardType()),
      WriteField(group.rewardPoints()),
      WriteField(group.carrotsClaimed())));
  WriteDailyQuestEntries(query, uid, group.quests());
  query.commit();
}

void PqDataSource::DeleteDailyQuestGroup(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "delete from \"data\".\"dailyQuestGroups\" where \"uid\" = $1",
    pqxx::params(
      uid));
  query.commit();
}

void PqDataSource::CreateMail(
  data::Mail& mail)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"mails\" (\"senderUid\", \"recipientUid\", \"isRead\", \"isDeleted\", \"type\", \"claimUid\", \"createdAt\", \"body\") "
    "values ($1, $2, $3, $4, $5, $6, $7, $8) returning \"uid\"",
    pqxx::params(
      WriteField(mail.from()),
      WriteField(mail.to()),
      WriteField(mail.isRead()),
      WriteField(mail.isDeleted()),
      WriteField(mail.type()),
      WriteField(mail.claimUid()),
      WriteField(mail.createdAt()),
      WriteField(mail.body())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  mail.uid = uid;
}

void PqDataSource::RetrieveMail(
  const data::Uid uid,
  data::Mail& mail)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"mails\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Mail does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", mail.uid);
  ReadField(row, "senderUid", mail.from);
  ReadField(row, "recipientUid", mail.to);
  ReadField(row, "isRead", mail.isRead);
  ReadField(row, "isDeleted", mail.isDeleted);
  ReadField(row, "type", mail.type);
  ReadField(row, "claimUid", mail.claimUid);
  ReadField(row, "createdAt", mail.createdAt);
  ReadField(row, "body", mail.body);
}

void PqDataSource::StoreMail(
  const data::Uid uid,
  const data::Mail& mail)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"mails\" set \"senderUid\" = $2, \"recipientUid\" = $3, \"isRead\" = $4, \"isDeleted\" = $5, \"type\" = $6, \"claimUid\" = $7, \"createdAt\" = $8, \"body\" = $9 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(mail.from()),
      WriteField(mail.to()),
      WriteField(mail.isRead()),
      WriteField(mail.isDeleted()),
      WriteField(mail.type()),
      WriteField(mail.claimUid()),
      WriteField(mail.createdAt()),
      WriteField(mail.body())));
  query.commit();
}

void PqDataSource::DeleteMail(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"mails\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreateQuest(
  data::Quest& quest)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"quests\" (\"questId\", \"isCompleted\", \"progress\") "
    "values ($1, $2, $3) returning \"uid\"",
    pqxx::params(
      WriteField(quest.questId()),
      WriteField(quest.isCompleted()),
      WriteField(quest.progress())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  quest.uid = uid;
}

void PqDataSource::RetrieveQuest(
  const data::Uid uid,
  data::Quest& quest)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"quests\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Quest does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", quest.uid);
  ReadField(row, "questId", quest.questId);
  ReadField(row, "isCompleted", quest.isCompleted);
  ReadField(row, "progress", quest.progress);
}

void PqDataSource::StoreQuest(
  const data::Uid uid,
  const data::Quest& quest)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"quests\" set \"questId\" = $2, \"isCompleted\" = $3, \"progress\" = $4 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(quest.questId()),
      WriteField(quest.isCompleted()),
      WriteField(quest.progress())));
  query.commit();
}

void PqDataSource::DeleteQuest(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"quests\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

void PqDataSource::CreateStallion(
  data::Stallion& stallion)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"stallions\" (\"horseUid\", \"ownerUid\", \"breedingCharge\", \"timesMated\", \"registeredAt\", \"expiresAt\") "
    "values ($1, $2, $3, $4, $5, $6) returning \"uid\"",
    pqxx::params(
      WriteField(stallion.horseUid()),
      WriteField(stallion.ownerUid()),
      WriteField(stallion.breedingCharge()),
      WriteField(stallion.timesMated()),
      WriteField(stallion.registeredAt()),
      WriteField(stallion.expiresAt())));
  const auto uid = result.one_row()["uid"].as<data::Uid>();
  query.commit();
  stallion.uid = uid;
}

void PqDataSource::RetrieveStallion(
  const data::Uid uid,
  data::Stallion& stallion)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"stallions\" where \"uid\" = $1",
    pqxx::params(uid));
  if (result.empty())
  {
    throw std::runtime_error("Stallion does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "uid", stallion.uid);
  ReadField(row, "horseUid", stallion.horseUid);
  ReadField(row, "ownerUid", stallion.ownerUid);
  ReadField(row, "breedingCharge", stallion.breedingCharge);
  ReadField(row, "timesMated", stallion.timesMated);
  ReadField(row, "registeredAt", stallion.registeredAt);
  ReadField(row, "expiresAt", stallion.expiresAt);
}

void PqDataSource::StoreStallion(
  const data::Uid uid,
  const data::Stallion& stallion)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"stallions\" set \"horseUid\" = $2, \"ownerUid\" = $3, \"breedingCharge\" = $4, \"timesMated\" = $5, \"registeredAt\" = $6, \"expiresAt\" = $7 "
    "where \"uid\" = $1",
    pqxx::params(
      uid,
      WriteField(stallion.horseUid()),
      WriteField(stallion.ownerUid()),
      WriteField(stallion.breedingCharge()),
      WriteField(stallion.timesMated()),
      WriteField(stallion.registeredAt()),
      WriteField(stallion.expiresAt())));
  query.commit();
}

void PqDataSource::DeleteStallion(
  const data::Uid uid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"stallions\" where \"uid\" = $1", pqxx::params(uid));
  query.commit();
}

std::vector<data::Uid> PqDataSource::ListRegisteredStallions()
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec("select \"uid\" from \"data\".\"stallions\" order by \"uid\"");
  std::vector<data::Uid> uids;
  uids.reserve(result.size());
  for (const auto& row : result)
  {
    uids.emplace_back(row["uid"].as<data::Uid>());
  }
  return uids;
}

void PqDataSource::CreateReward(
  data::Reward& reward)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "insert into \"data\".\"rewards\" (\"characterUid\", \"type\", \"carrots\", \"isClaimed\", \"createdAt\", \"claimedAt\") "
    "values ($1, $2, $3, $4, $5, $6) returning \"claimUid\"",
    pqxx::params(
      WriteField(reward.characterUid()),
      WriteField(reward.type()),
      WriteField(reward.carrots()),
      WriteField(reward.isClaimed()),
      WriteField(reward.createdAt()),
      WriteField(reward.claimedAt())));
  const auto uid = result.one_row()["claimUid"].as<data::Uid>();
  query.commit();
  reward.claimUid = uid;
}

void PqDataSource::RetrieveReward(
  const data::Uid claimUid,
  data::Reward& reward)
{
  pqxx::work query(RequireConnection(_connection));
  const auto result = query.exec(
    "select * from \"data\".\"rewards\" where \"claimUid\" = $1",
    pqxx::params(claimUid));
  if (result.empty())
  {
    throw std::runtime_error("Reward does not exist");
  }

  const auto row = result.one_row();
  ReadField(row, "claimUid", reward.claimUid);
  ReadField(row, "characterUid", reward.characterUid);
  ReadField(row, "type", reward.type);
  ReadField(row, "carrots", reward.carrots);
  ReadField(row, "isClaimed", reward.isClaimed);
  ReadField(row, "createdAt", reward.createdAt);
  ReadField(row, "claimedAt", reward.claimedAt);
}

void PqDataSource::StoreReward(
  const data::Uid claimUid,
  const data::Reward& reward)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec(
    "update \"data\".\"rewards\" set \"characterUid\" = $2, \"type\" = $3, \"carrots\" = $4, \"isClaimed\" = $5, \"createdAt\" = $6, \"claimedAt\" = $7 "
    "where \"claimUid\" = $1",
    pqxx::params(
      claimUid,
      WriteField(reward.characterUid()),
      WriteField(reward.type()),
      WriteField(reward.carrots()),
      WriteField(reward.isClaimed()),
      WriteField(reward.createdAt()),
      WriteField(reward.claimedAt())));
  query.commit();
}

void PqDataSource::DeleteReward(
  const data::Uid claimUid)
{
  pqxx::work query(RequireConnection(_connection));
  query.exec("delete from \"data\".\"rewards\" where \"claimUid\" = $1", pqxx::params(claimUid));
  query.commit();
}

} // namespace server
