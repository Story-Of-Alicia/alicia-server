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

#include "libserver/data/pq/PqDataSource.hpp"

#include "PqSchema.hpp"

#include <format>
#include <ranges>
#include <span>
#include <unordered_set>

#include <spdlog/spdlog.h>

namespace server
{

namespace
{

//! The Postgres type of a column.
enum class SqlType
{
  SmallInt,
  Integer,
  BigInt,
  Text,
  Boolean,
  //! A `timestamptz`, converted to and from the count of seconds since the
  //! epoch which is used on the C++ side.
  Timestamp
};

//! A column of a record table.
struct Column
{
  std::string_view name;
  SqlType type{SqlType::Integer};
  //! Overrides the zero default of the type, for the few columns which do not
  //! default to a zero value. Only reached when the column is created.
  std::string_view defaultValue{};
};

//! A record table, described well enough to be created from the description.
struct Table
{
  //! The qualified name, quoted where the name is a reserved word.
  std::string_view qualifiedName;
  //! The bare name, used to look the table up in `information_schema`.
  std::string_view name;
  //! The columns, the primary key first.
  std::span<const Column> columns;
};

//! @param type Type of a column.
//! @returns The Postgres name of the type.
constexpr std::string_view SqlTypeName(const SqlType type)
{
  switch (type)
  {
    case SqlType::SmallInt: return "smallint";
    case SqlType::Integer: return "integer";
    case SqlType::BigInt: return "bigint";
    case SqlType::Text: return "text";
    case SqlType::Boolean: return "boolean";
    case SqlType::Timestamp: return "timestamptz";
  }
  return "integer";
}

//! @param type Type of a column.
//! @returns The zero default of the type.
constexpr std::string_view SqlTypeDefault(const SqlType type)
{
  switch (type)
  {
    case SqlType::SmallInt:
    case SqlType::Integer:
    case SqlType::BigInt: return "0";
    case SqlType::Text: return "''";
    case SqlType::Boolean: return "false";
    case SqlType::Timestamp: return "'epoch'";
  }
  return "0";
}

//! Builds the select list of a record table, converting timestamps to the
//! count of seconds since the epoch while keeping the column name.
//! @param columns Columns of the table.
//! @returns The select list.
std::string BuildSelectList(const std::span<const Column> columns)
{
  std::string list;
  for (const auto& column : columns)
  {
    if (not list.empty())
      list += ", ";

    if (column.type == SqlType::Timestamp)
      list += std::format("extract(epoch from {})::bigint AS {}", column.name, column.name);
    else
      list += column.name;
  }
  return list;
}

//! Builds an upsert of a whole record. The first column is the primary key,
//! which is inserted but never updated. Parameters are numbered in the order of
//! the columns, so the parameters have to be appended in that same order.
//! @param table Name of the table.
//! @param columns Columns of the table, the primary key first.
//! @returns The upsert statement.
std::string BuildUpsert(
  const std::string_view table,
  const std::span<const Column> columns)
{
  std::string names;
  std::string values;
  std::string assignments;

  for (size_t index = 0; index < columns.size(); ++index)
  {
    const auto& column = columns[index];
    const auto parameter = std::format("${}", index + 1);

    if (not names.empty())
    {
      names += ", ";
      values += ", ";
    }

    names += column.name;
    values += column.type == SqlType::Timestamp
      ? std::format("to_timestamp({})", parameter)
      : parameter;

    // The primary key is what the conflict is detected on, so it is never
    // part of the update.
    if (index == 0)
      continue;

    if (not assignments.empty())
      assignments += ", ";
    assignments += std::format("{} = excluded.{}", column.name, column.name);
  }

  return std::format(
    "INSERT INTO {} ({}) VALUES ({}) ON CONFLICT ({}) DO UPDATE SET {}",
    table,
    names,
    values,
    columns.front().name,
    assignments);
}

//! Converts a time point to the count of seconds since the epoch.
//! Rounds up to match the representation the file data source writes.
//! @param timePoint Time point to convert.
//! @returns The count of seconds since the epoch.
int64_t ToEpochSeconds(const data::Clock::time_point& timePoint)
{
  return std::chrono::ceil<std::chrono::seconds>(
    timePoint.time_since_epoch()).count();
}

//! Converts a count of seconds since the epoch to a time point.
//! @param seconds Count of seconds since the epoch.
//! @returns The time point.
data::Clock::time_point FromEpochSeconds(const int64_t seconds)
{
  return data::Clock::time_point(std::chrono::seconds(seconds));
}

//! Draws the next value of a sequence. No row is inserted, because the record
//! is only written later, when the store queue is drained.
//! @param transaction Transaction to draw in.
//! @param sequence Name of the sequence.
//! @returns The drawn UID.
data::Uid NextUid(
  pqxx::transaction_base& transaction,
  const std::string_view sequence)
{
  return transaction.exec(
    std::format("SELECT nextval('{}')", sequence)).one_row()[0].as<data::Uid>();
}

//! Retrieves a single record row.
//! @param transaction Transaction to retrieve in.
//! @param table Name of the table.
//! @param keyColumn Name of the primary key column.
//! @param columns Columns of the table.
//! @param key Value of the primary key.
//! @returns The retrieved row.
//! @throws std::runtime_error if the record does not exist. The data director
//!         relies on a throw to count a retrieval as failed.
template <typename Key>
pqxx::row RetrieveRow(
  pqxx::transaction_base& transaction,
  const std::string_view table,
  const std::string_view keyColumn,
  const std::span<const Column> columns,
  const Key& key)
{
  const auto result = transaction.exec(
    std::format(
      "SELECT {} FROM {} WHERE {} = $1",
      BuildSelectList(columns),
      table,
      keyColumn),
    pqxx::params{key});

  if (result.empty())
  {
    throw std::runtime_error(
      std::format("Record of '{}' does not exist", table));
  }

  return result.one_row();
}

//! Deletes a record.
//! @param transaction Transaction to delete in.
//! @param table Name of the table.
//! @param keyColumn Name of the primary key column.
//! @param key Value of the primary key.
template <typename Key>
void DeleteRow(
  pqxx::transaction_base& transaction,
  const std::string_view table,
  const std::string_view keyColumn,
  const Key& key)
{
  transaction.exec(
    std::format("DELETE FROM {} WHERE {} = $1", table, keyColumn),
    pqxx::params{key});
}

//! Retrieves an ordered list of UIDs owned by a record.
//! @param transaction Transaction to retrieve in.
//! @param table Name of the child table.
//! @param ownerColumn Name of the column referencing the owning record.
//! @param valueColumn Name of the column holding the UID.
//! @param ownerUid UID of the owning record.
//! @returns The list of UIDs, in their stored order.
std::vector<data::Uid> RetrieveUidList(
  pqxx::transaction_base& transaction,
  const std::string_view table,
  const std::string_view ownerColumn,
  const std::string_view valueColumn,
  const data::Uid ownerUid)
{
  const auto result = transaction.exec(
    std::format(
      "SELECT {} FROM {} WHERE {} = $1 ORDER BY ordinal",
      valueColumn,
      table,
      ownerColumn),
    pqxx::params{ownerUid});

  std::vector<data::Uid> uids;
  uids.reserve(result.size());
  for (const auto& row : result)
    uids.emplace_back(row[0].as<data::Uid>());

  return uids;
}

//! Replaces the ordered list of UIDs owned by a record.
//! @param transaction Transaction to store in.
//! @param table Name of the child table.
//! @param ownerColumn Name of the column referencing the owning record.
//! @param valueColumn Name of the column holding the UID.
//! @param ownerUid UID of the owning record.
//! @param uids UIDs to store.
void StoreUidList(
  pqxx::transaction_base& transaction,
  const std::string_view table,
  const std::string_view ownerColumn,
  const std::string_view valueColumn,
  const data::Uid ownerUid,
  const std::span<const data::Uid> uids)
{
  transaction.exec(
    std::format("DELETE FROM {} WHERE {} = $1", table, ownerColumn),
    pqxx::params{ownerUid});

  const auto statement = std::format(
    "INSERT INTO {} ({}, ordinal, {}) VALUES ($1, $2, $3)",
    table,
    ownerColumn,
    valueColumn);

  for (size_t index = 0; index < uids.size(); ++index)
  {
    transaction.exec(
      statement,
      pqxx::params{ownerUid, static_cast<int32_t>(index), uids[index]});
  }
}

//! Retrieves an unordered set of UIDs owned by a record.
//! @param transaction Transaction to retrieve in.
//! @param table Name of the child table.
//! @param ownerColumn Name of the column referencing the owning record.
//! @param valueColumn Name of the column holding the UID.
//! @param ownerUid UID of the owning record.
//! @returns The set of UIDs.
std::set<data::Uid> RetrieveUidSet(
  pqxx::transaction_base& transaction,
  const std::string_view table,
  const std::string_view ownerColumn,
  const std::string_view valueColumn,
  const data::Uid ownerUid)
{
  const auto result = transaction.exec(
    std::format("SELECT {} FROM {} WHERE {} = $1", valueColumn, table, ownerColumn),
    pqxx::params{ownerUid});

  std::set<data::Uid> uids;
  for (const auto& row : result)
    uids.emplace(row[0].as<data::Uid>());

  return uids;
}

//! Replaces the unordered set of UIDs owned by a record.
//! @param transaction Transaction to store in.
//! @param table Name of the child table.
//! @param ownerColumn Name of the column referencing the owning record.
//! @param valueColumn Name of the column holding the UID.
//! @param ownerUid UID of the owning record.
//! @param uids UIDs to store.
void StoreUidSet(
  pqxx::transaction_base& transaction,
  const std::string_view table,
  const std::string_view ownerColumn,
  const std::string_view valueColumn,
  const data::Uid ownerUid,
  const std::set<data::Uid>& uids)
{
  transaction.exec(
    std::format("DELETE FROM {} WHERE {} = $1", table, ownerColumn),
    pqxx::params{ownerUid});

  const auto statement = std::format(
    "INSERT INTO {} ({}, {}) VALUES ($1, $2)",
    table,
    ownerColumn,
    valueColumn);

  for (const auto uid : uids)
    transaction.exec(statement, pqxx::params{ownerUid, uid});
}

// ---------------------------------------------------------------------------
// Column definitions, mirroring scripts/schema/001_initial.sql. The order has
// to match the order the parameters are appended in.
// ---------------------------------------------------------------------------

constexpr Column UserColumns[]{
  {"name", SqlType::Text},
  {"character_uid", SqlType::Integer},
  {"last_seen_online", SqlType::Timestamp}};

constexpr Column InfractionColumns[]{
  {"uid", SqlType::Integer},
  {"description", SqlType::Text},
  {"punishment", SqlType::SmallInt},
  {"duration", SqlType::BigInt},
  {"created_at", SqlType::Timestamp}};

constexpr Column ItemColumns[]{
  {"uid", SqlType::Integer},
  {"tid", SqlType::Integer},
  {"count", SqlType::Integer},
  {"duration", SqlType::BigInt},
  {"created_at", SqlType::Timestamp}};

constexpr Column PetColumns[]{
  {"uid", SqlType::Integer},
  {"item_uid", SqlType::Integer},
  {"pet_id", SqlType::Integer},
  {"name", SqlType::Text},
  {"birth_date", SqlType::Timestamp}};

constexpr Column EggColumns[]{
  {"uid", SqlType::Integer},
  {"item_uid", SqlType::Integer},
  {"item_tid", SqlType::Integer},
  {"incubated_at", SqlType::Timestamp},
  {"incubator_slot", SqlType::Integer},
  {"boosts_used", SqlType::Integer}};

constexpr Column HousingColumns[]{
  {"uid", SqlType::Integer},
  {"housing_id", SqlType::Integer},
  {"expires_at", SqlType::Timestamp},
  {"durability", SqlType::Integer}};

constexpr Column QuestColumns[]{
  {"uid", SqlType::Integer},
  {"quest_id", SqlType::Integer},
  {"is_completed", SqlType::SmallInt},
  {"progress", SqlType::BigInt}};

constexpr Column MailColumns[]{
  {"uid", SqlType::Integer},
  {"from_uid", SqlType::Integer},
  {"to_uid", SqlType::Integer},
  {"is_read", SqlType::Boolean},
  {"is_deleted", SqlType::Boolean},
  {"type", SqlType::SmallInt},
  {"claim_uid", SqlType::Integer},
  {"created_at", SqlType::Timestamp},
  {"body", SqlType::Text}};

constexpr Column StallionColumns[]{
  {"uid", SqlType::Integer},
  {"horse_uid", SqlType::Integer},
  {"owner_uid", SqlType::Integer},
  {"breeding_charge", SqlType::Integer},
  {"times_mated", SqlType::Integer},
  {"registered_at", SqlType::Timestamp},
  {"expires_at", SqlType::Timestamp}};

constexpr Column RewardColumns[]{
  {"claim_uid", SqlType::Integer},
  {"character_uid", SqlType::Integer},
  {"type", SqlType::SmallInt},
  {"carrots", SqlType::Integer},
  {"is_claimed", SqlType::Boolean},
  {"created_at", SqlType::Timestamp},
  {"claimed_at", SqlType::Timestamp}};

constexpr Column StorageItemColumns[]{
  {"uid", SqlType::Integer},
  {"sender", SqlType::Text},
  {"message", SqlType::Text},
  {"carrots", SqlType::Integer},
  {"checked", SqlType::Boolean},
  {"created_at", SqlType::Timestamp},
  {"duration", SqlType::BigInt},
  {"goods_sq", SqlType::Integer},
  {"price_id", SqlType::Integer}};

constexpr Column DailyQuestGroupColumns[]{
  {"uid", SqlType::Integer},
  {"reward_id", SqlType::SmallInt},
  {"reward_type", SqlType::SmallInt},
  {"reward_points", SqlType::BigInt},
  {"carrots_claimed", SqlType::Boolean}};

constexpr Column SettingsColumns[]{
  {"uid", SqlType::Integer},
  {"age", SqlType::Integer},
  {"hide_age", SqlType::Boolean, "true"},
  {"has_keyboard_bindings", SqlType::Boolean},
  {"has_gamepad_bindings", SqlType::Boolean},
  {"has_macros", SqlType::Boolean}};

constexpr Column GuildColumns[]{
  {"uid", SqlType::Integer},
  {"name", SqlType::Text},
  {"description", SqlType::Text},
  {"owner", SqlType::Integer},
  {"rank", SqlType::Integer},
  {"total_wins", SqlType::Integer},
  {"total_losses", SqlType::Integer},
  {"seasonal_wins", SqlType::Integer},
  {"seasonal_losses", SqlType::Integer}};

constexpr Column HorseColumns[]{
  {"uid", SqlType::Integer},
  {"tid", SqlType::Integer},
  {"name", SqlType::Text},
  {"parts_skin_tid", SqlType::Integer},
  {"parts_face_tid", SqlType::Integer},
  {"parts_mane_tid", SqlType::Integer},
  {"parts_tail_tid", SqlType::Integer},
  {"appearance_scale", SqlType::Integer},
  {"appearance_leg_length", SqlType::Integer},
  {"appearance_leg_volume", SqlType::Integer},
  {"appearance_body_length", SqlType::Integer},
  {"appearance_body_volume", SqlType::Integer},
  {"stats_agility", SqlType::Integer},
  {"stats_courage", SqlType::Integer},
  {"stats_rush", SqlType::Integer},
  {"stats_endurance", SqlType::Integer},
  {"stats_ambition", SqlType::Integer},
  {"mastery_spur_magic_count", SqlType::BigInt},
  {"mastery_jump_count", SqlType::BigInt},
  {"mastery_sliding_time", SqlType::BigInt},
  {"mastery_gliding_distance", SqlType::BigInt},
  {"rating", SqlType::Integer},
  {"clazz", SqlType::Integer},
  {"clazz_progress", SqlType::BigInt},
  {"grade", SqlType::Integer},
  {"growth_points", SqlType::BigInt},
  {"breeding_count", SqlType::Integer},
  {"breeding_combo", SqlType::Integer},
  {"type", SqlType::SmallInt},
  {"date_of_birth", SqlType::Timestamp},
  {"tendency", SqlType::Integer},
  {"spirit", SqlType::Integer},
  {"potential_type", SqlType::Integer},
  {"potential_level", SqlType::Integer},
  {"potential_value", SqlType::Integer},
  {"luck_state", SqlType::Integer},
  {"fatigue", SqlType::Integer},
  {"emblem_uid", SqlType::Integer},
  {"condition_stamina", SqlType::Integer},
  {"condition_charm", SqlType::Integer},
  {"condition_friendliness", SqlType::Integer},
  {"condition_injury", SqlType::Integer},
  {"condition_plenitude", SqlType::Integer},
  {"condition_body_dirtiness", SqlType::Integer},
  {"condition_mane_dirtiness", SqlType::Integer},
  {"condition_tail_dirtiness", SqlType::Integer},
  {"condition_body_polish", SqlType::Integer},
  {"condition_mane_polish", SqlType::Integer},
  {"condition_tail_polish", SqlType::Integer},
  {"condition_attachment", SqlType::Integer},
  {"condition_boredom", SqlType::Integer},
  {"condition_stop_amends_point", SqlType::Integer},
  {"info_boosts_in_a_row", SqlType::Integer},
  {"info_wins_speed_single", SqlType::Integer},
  {"info_wins_speed_team", SqlType::Integer},
  {"info_wins_magic_single", SqlType::Integer},
  {"info_wins_magic_team", SqlType::Integer},
  {"info_total_distance", SqlType::BigInt},
  {"info_top_speed", SqlType::Integer},
  {"info_longest_glide_distance", SqlType::BigInt},
  {"info_participated", SqlType::Integer},
  {"info_cumulative_prize", SqlType::BigInt},
  {"info_biggest_prize", SqlType::BigInt},
  {"ancestors_father", SqlType::Integer},
  {"ancestors_mother", SqlType::Integer},
  {"lineage", SqlType::Integer, "1"}};

constexpr Column CharacterColumns[]{
  {"uid", SqlType::Integer},
  {"name", SqlType::Text},
  {"introduction", SqlType::Text},
  {"level", SqlType::Integer},
  {"experience", SqlType::BigInt},
  {"carrots", SqlType::Integer},
  {"cash", SqlType::Integer},
  {"role", SqlType::SmallInt},
  {"role_rank", SqlType::SmallInt},
  {"parts_model_id", SqlType::Integer},
  {"parts_mouth_id", SqlType::Integer},
  {"parts_face_id", SqlType::Integer},
  {"appearance_voice_id", SqlType::Integer},
  {"appearance_head_size", SqlType::Integer},
  {"appearance_height", SqlType::Integer},
  {"appearance_thigh_volume", SqlType::Integer},
  {"appearance_leg_volume", SqlType::Integer},
  {"appearance_emblem_id", SqlType::Integer},
  {"guild_uid", SqlType::Integer},
  {"horse_slot_count", SqlType::SmallInt},
  {"mount_uid", SqlType::Integer},
  {"pet_uid", SqlType::Integer},
  {"is_ranch_locked", SqlType::Boolean},
  {"settings_uid", SqlType::Integer},
  {"daily_quest_group_uid", SqlType::Integer},
  {"skills_speed_set1_slot1", SqlType::Integer},
  {"skills_speed_set1_slot2", SqlType::Integer},
  {"skills_speed_set2_slot1", SqlType::Integer},
  {"skills_speed_set2_slot2", SqlType::Integer},
  {"skills_speed_active_set_id", SqlType::Integer},
  {"skills_magic_set1_slot1", SqlType::Integer},
  {"skills_magic_set1_slot2", SqlType::Integer},
  {"skills_magic_set2_slot1", SqlType::Integer},
  {"skills_magic_set2_slot2", SqlType::Integer},
  {"skills_magic_active_set_id", SqlType::Integer},
  {"mailbox_has_new_mail", SqlType::Boolean}};

//! Every record table, in the order they are created. The child tables of the
//! schema file reference these, so they all have to exist first.
constexpr Table RecordTables[]{
  {"data.\"user\"", "user", UserColumns},
  {"data.infraction", "infraction", InfractionColumns},
  {"data.character", "character", CharacterColumns},
  {"data.horse", "horse", HorseColumns},
  {"data.item", "item", ItemColumns},
  {"data.storage_item", "storage_item", StorageItemColumns},
  {"data.egg", "egg", EggColumns},
  {"data.pet", "pet", PetColumns},
  {"data.housing", "housing", HousingColumns},
  {"data.guild", "guild", GuildColumns},
  {"data.settings", "settings", SettingsColumns},
  {"data.daily_quest_group", "daily_quest_group", DailyQuestGroupColumns},
  {"data.mail", "mail", MailColumns},
  {"data.quest", "quest", QuestColumns},
  {"data.stallion", "stallion", StallionColumns},
  {"data.reward", "reward", RewardColumns}};

//! @param column Column.
//! @returns The default of the column, which is the default of its type unless
//!          the column overrides it.
constexpr std::string_view ColumnDefault(const Column& column)
{
  return column.defaultValue.empty()
    ? SqlTypeDefault(column.type)
    : column.defaultValue;
}

//! Builds the statement creating a record table. Every column is NOT NULL with
//! a default, so that a column added later can be filled in for existing rows.
//! @param table Table to create.
//! @returns The create statement.
std::string BuildCreateTable(const Table& table)
{
  std::string columns;
  for (const auto& column : table.columns)
  {
    if (not columns.empty())
      columns += ",\n";

    columns += std::format(
      "  {} {} NOT NULL DEFAULT {}",
      column.name,
      SqlTypeName(column.type),
      ColumnDefault(column));
  }

  return std::format(
    "CREATE TABLE IF NOT EXISTS {} (\n{},\n  PRIMARY KEY ({}))",
    table.qualifiedName,
    columns,
    table.columns.front().name);
}

//! Adds every column the table does not carry yet, which is how a field added
//! to a record reaches a database created before it existed.
//!
//! Columns are only ever added. A column which the table carries but the record
//! no longer describes is left alone, because dropping it would destroy data
//! and cannot be undone.
//! @param transaction Transaction to alter in.
//! @param table Table to bring up to date.
//! @returns The count of columns added.
size_t AddMissingColumns(pqxx::transaction_base& transaction, const Table& table)
{
  const auto result = transaction.exec(
    "SELECT column_name FROM information_schema.columns "
    "WHERE table_schema = 'data' AND table_name = $1",
    pqxx::params{table.name});

  std::unordered_set<std::string> present;
  for (const auto& row : result)
    present.emplace(row[0].as<std::string>());

  size_t added = 0;
  for (const auto& column : table.columns)
  {
    if (present.contains(std::string(column.name)))
      continue;

    transaction.exec(std::format(
      "ALTER TABLE {} ADD COLUMN {} {} NOT NULL DEFAULT {}",
      table.qualifiedName,
      column.name,
      SqlTypeName(column.type),
      ColumnDefault(column)));

    spdlog::info(
      "Added the column '{}' missing from '{}'", column.name, table.qualifiedName);
    ++added;
  }

  return added;
}

//! Kinds of the character item lists, matching data.character_item.kind.
enum class CharacterItemKind : int16_t
{
  Inventory = 0,
  CharacterEquipment = 1,
  // 2 was expiredEquipment, which the character no longer carries. Rows of that
  // kind are simply never read again. The value must not be handed to something
  // else, or those leftover rows would come back as that something else.
  Gifts = 3,
  Purchases = 4
};

//! Boxes of the character mailbox, matching data.character_mail.box.
enum class CharacterMailBox : int16_t
{
  Inbox = 0,
  Sent = 1
};

//! Devices of the settings bindings, matching data.settings_binding.device.
enum class SettingsBindingDevice : int16_t
{
  Keyboard = 0,
  Gamepad = 1
};

//! Retrieves one of the character item lists.
std::vector<data::Uid> RetrieveCharacterItems(
  pqxx::transaction_base& transaction,
  const data::Uid characterUid,
  const CharacterItemKind kind)
{
  const auto result = transaction.exec(
    "SELECT item_uid FROM data.character_item "
    "WHERE character_uid = $1 AND kind = $2 ORDER BY ordinal",
    pqxx::params{characterUid, static_cast<int16_t>(kind)});

  std::vector<data::Uid> uids;
  uids.reserve(result.size());
  for (const auto& row : result)
    uids.emplace_back(row[0].as<data::Uid>());

  return uids;
}

//! Stores one of the character item lists. The whole list is deleted first, so
//! this has to be called for every kind whenever a character is stored.
void StoreCharacterItems(
  pqxx::transaction_base& transaction,
  const data::Uid characterUid,
  const CharacterItemKind kind,
  const std::span<const data::Uid> uids)
{
  transaction.exec(
    "DELETE FROM data.character_item WHERE character_uid = $1 AND kind = $2",
    pqxx::params{characterUid, static_cast<int16_t>(kind)});

  for (size_t index = 0; index < uids.size(); ++index)
  {
    transaction.exec(
      "INSERT INTO data.character_item (character_uid, kind, ordinal, item_uid) "
      "VALUES ($1, $2, $3, $4)",
      pqxx::params{
        characterUid,
        static_cast<int16_t>(kind),
        static_cast<int32_t>(index),
        uids[index]});
  }
}

//! Retrieves one of the character mailbox lists.
std::vector<data::Uid> RetrieveCharacterMail(
  pqxx::transaction_base& transaction,
  const data::Uid characterUid,
  const CharacterMailBox box)
{
  const auto result = transaction.exec(
    "SELECT mail_uid FROM data.character_mail "
    "WHERE character_uid = $1 AND box = $2 ORDER BY ordinal",
    pqxx::params{characterUid, static_cast<int16_t>(box)});

  std::vector<data::Uid> uids;
  uids.reserve(result.size());
  for (const auto& row : result)
    uids.emplace_back(row[0].as<data::Uid>());

  return uids;
}

//! Stores one of the character mailbox lists.
void StoreCharacterMail(
  pqxx::transaction_base& transaction,
  const data::Uid characterUid,
  const CharacterMailBox box,
  const std::span<const data::Uid> uids)
{
  transaction.exec(
    "DELETE FROM data.character_mail WHERE character_uid = $1 AND box = $2",
    pqxx::params{characterUid, static_cast<int16_t>(box)});

  for (size_t index = 0; index < uids.size(); ++index)
  {
    transaction.exec(
      "INSERT INTO data.character_mail (character_uid, box, ordinal, mail_uid) "
      "VALUES ($1, $2, $3, $4)",
      pqxx::params{
        characterUid,
        static_cast<int16_t>(box),
        static_cast<int32_t>(index),
        uids[index]});
  }
}

} // anon namespace

void PqDataSource::Initialize(const std::string& uri)
{
  const auto timerBegin = std::chrono::steady_clock::now();

  _uri = uri;

  std::scoped_lock lock(_connectionMutex);
  _connection.emplace(_uri);

  const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::steady_clock::now() - timerBegin);
  spdlog::info("Connected to the primary data source in {}ms", time.count());

  EnsureSchema();
}

void PqDataSource::Terminate()
{
  std::scoped_lock lock(_connectionMutex);
  _connection.reset();
}

pqxx::connection& PqDataSource::Connection()
{
  if (not _connection)
    throw std::runtime_error("The data source is not connected");

  if (not _connection->is_open())
  {
    spdlog::warn("Reconnecting a broken connection to the primary data source");
    _connection.emplace(_uri);
  }

  return *_connection;
}

void PqDataSource::EnsureSchema()
{
  // Everything here is idempotent, so it runs on every start-up rather than
  // only against an empty database. That is what lets a column added to a
  // record reach a database which already exists. It all goes in as one
  // transaction, so a failure part-way through changes nothing.
  pqxx::work transaction(*_connection);

  transaction.exec("CREATE SCHEMA IF NOT EXISTS data");

  // The record tables are generated from their column tables, which already
  // have to list every column in parameter order. Describing them a second time
  // in the schema file is what would let the two drift apart.
  for (const auto& table : RecordTables)
    transaction.exec(BuildCreateTable(table));

  // The sequences, the child tables backing the collection fields and the
  // indexes cannot be derived from the column tables, so they come from the
  // schema file. It references the record tables, hence the ordering.
  transaction.exec(GetDataSchemaSql());

  size_t addedColumns = 0;
  for (const auto& table : RecordTables)
    addedColumns += AddMissingColumns(transaction, table);

  transaction.commit();

  if (addedColumns > 0)
  {
    spdlog::info(
      "Brought the primary data source up to date with {} added column(s)", addedColumns);
  }
}

// ---------------------------------------------------------------------------
// User
// ---------------------------------------------------------------------------

void PqDataSource::CreateUser(data::User&)
{
  // A user is keyed by its name, so there is no UID to draw.
}

void PqDataSource::RetrieveUser(const std::string_view& name, data::User& user)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(
    transaction, "data.\"user\"", "name", UserColumns, name);

  user.name = row["name"].as<std::string>();
  user.characterUid = row["character_uid"].as<data::Uid>();
  user.lastSeenOnline = FromEpochSeconds(row["last_seen_online"].as<int64_t>());

  // The infraction list is keyed by the user name rather than a UID, so it
  // cannot go through the shared helper.
  const auto infractions = transaction.exec(
    "SELECT infraction_uid FROM data.user_infraction "
    "WHERE user_name = $1 ORDER BY ordinal",
    pqxx::params{name});

  std::vector<data::Uid> infractionUids;
  infractionUids.reserve(infractions.size());
  for (const auto& infractionRow : infractions)
    infractionUids.emplace_back(infractionRow[0].as<data::Uid>());
  user.infractions = std::move(infractionUids);

  transaction.commit();
}

void PqDataSource::StoreUser(const std::string_view&, const data::User& user)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.\"user\"", UserColumns),
    pqxx::params{
      user.name(),
      user.characterUid(),
      ToEpochSeconds(user.lastSeenOnline())});

  transaction.exec(
    "DELETE FROM data.user_infraction WHERE user_name = $1",
    pqxx::params{user.name()});

  const auto& infractions = user.infractions();
  for (size_t index = 0; index < infractions.size(); ++index)
  {
    transaction.exec(
      "INSERT INTO data.user_infraction (user_name, ordinal, infraction_uid) "
      "VALUES ($1, $2, $3)",
      pqxx::params{user.name(), static_cast<int32_t>(index), infractions[index]});
  }

  transaction.commit();
}

bool PqDataSource::IsUserNameUnique(const std::string_view& name)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto result = transaction.exec(
    "SELECT 1 FROM data.\"user\" WHERE lower(name) = lower($1)",
    pqxx::params{name});

  transaction.commit();
  return result.empty();
}

// ---------------------------------------------------------------------------
// Infraction
// ---------------------------------------------------------------------------

void PqDataSource::CreateInfraction(data::Infraction& infraction)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  infraction.uid = NextUid(transaction, "data.infraction_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveInfraction(const data::Uid uid, data::Infraction& infraction)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(
    transaction, "data.infraction", "uid", InfractionColumns, uid);

  infraction.uid = row["uid"].as<data::Uid>();
  infraction.description = row["description"].as<std::string>();
  infraction.punishment = static_cast<data::Infraction::Punishment>(
    row["punishment"].as<int16_t>());
  infraction.duration = std::chrono::seconds(row["duration"].as<int64_t>());
  infraction.createdAt = FromEpochSeconds(row["created_at"].as<int64_t>());

  transaction.commit();
}

void PqDataSource::StoreInfraction(const data::Uid uid, const data::Infraction& infraction)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.infraction", InfractionColumns),
    pqxx::params{
      uid,
      infraction.description(),
      static_cast<int16_t>(infraction.punishment()),
      static_cast<int64_t>(infraction.duration().count()),
      ToEpochSeconds(infraction.createdAt())});

  transaction.commit();
}

void PqDataSource::DeleteInfraction(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.infraction", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Item
// ---------------------------------------------------------------------------

void PqDataSource::CreateItem(data::Item& item)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  // Items and horses draw from a single sequence, so that their UIDs never
  // collide, matching what the file data source did with one shared counter.
  item.uid = NextUid(transaction, "data.equipment_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveItem(const data::Uid uid, data::Item& item)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.item", "uid", ItemColumns, uid);

  item.uid = row["uid"].as<data::Uid>();
  item.tid = row["tid"].as<data::Tid>();
  item.count = row["count"].as<uint32_t>();
  item.duration = std::chrono::seconds(row["duration"].as<int64_t>());
  item.createdAt = FromEpochSeconds(row["created_at"].as<int64_t>());

  transaction.commit();
}

void PqDataSource::StoreItem(const data::Uid uid, const data::Item& item)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.item", ItemColumns),
    pqxx::params{
      uid,
      item.tid(),
      item.count(),
      static_cast<int64_t>(item.duration().count()),
      ToEpochSeconds(item.createdAt())});

  transaction.commit();
}

void PqDataSource::DeleteItem(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.item", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Pet
// ---------------------------------------------------------------------------

void PqDataSource::CreatePet(data::Pet& pet)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  pet.uid = NextUid(transaction, "data.pet_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrievePet(const data::Uid uid, data::Pet& pet)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.pet", "uid", PetColumns, uid);

  pet.uid = row["uid"].as<data::Uid>();
  pet.itemUid = row["item_uid"].as<data::Uid>();
  pet.petId = row["pet_id"].as<data::Uid>();
  pet.name = row["name"].as<std::string>();
  pet.birthDate = FromEpochSeconds(row["birth_date"].as<int64_t>());

  transaction.commit();
}

void PqDataSource::StorePet(const data::Uid uid, const data::Pet& pet)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.pet", PetColumns),
    pqxx::params{
      uid,
      pet.itemUid(),
      pet.petId(),
      pet.name(),
      ToEpochSeconds(pet.birthDate())});

  transaction.commit();
}

void PqDataSource::DeletePet(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.pet", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Egg
// ---------------------------------------------------------------------------

void PqDataSource::CreateEgg(data::Egg& egg)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  egg.uid = NextUid(transaction, "data.egg_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveEgg(const data::Uid uid, data::Egg& egg)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.egg", "uid", EggColumns, uid);

  egg.uid = row["uid"].as<data::Uid>();
  egg.itemUid = row["item_uid"].as<data::Uid>();
  egg.itemTid = row["item_tid"].as<data::Tid>();
  egg.incubatedAt = FromEpochSeconds(row["incubated_at"].as<int64_t>());
  egg.incubatorSlot = row["incubator_slot"].as<uint32_t>();
  egg.boostsUsed = row["boosts_used"].as<uint32_t>();

  transaction.commit();
}

void PqDataSource::StoreEgg(const data::Uid uid, const data::Egg& egg)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.egg", EggColumns),
    pqxx::params{
      uid,
      egg.itemUid(),
      egg.itemTid(),
      ToEpochSeconds(egg.incubatedAt()),
      egg.incubatorSlot(),
      egg.boostsUsed()});

  transaction.commit();
}

void PqDataSource::DeleteEgg(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.egg", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Housing
// ---------------------------------------------------------------------------

void PqDataSource::CreateHousing(data::Housing& housing)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  housing.uid = NextUid(transaction, "data.housing_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveHousing(const data::Uid uid, data::Housing& housing)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.housing", "uid", HousingColumns, uid);

  housing.uid = row["uid"].as<data::Uid>();
  housing.housingId = row["housing_id"].as<uint32_t>();
  housing.expiresAt = FromEpochSeconds(row["expires_at"].as<int64_t>());
  housing.durability = row["durability"].as<uint32_t>();

  transaction.commit();
}

void PqDataSource::StoreHousing(const data::Uid uid, const data::Housing& housing)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.housing", HousingColumns),
    pqxx::params{
      uid,
      housing.housingId(),
      ToEpochSeconds(housing.expiresAt()),
      housing.durability()});

  transaction.commit();
}

void PqDataSource::DeleteHousing(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.housing", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Quest
// ---------------------------------------------------------------------------

void PqDataSource::CreateQuest(data::Quest& quest)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  quest.uid = NextUid(transaction, "data.quest_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveQuest(const data::Uid uid, data::Quest& quest)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.quest", "uid", QuestColumns, uid);

  quest.uid = row["uid"].as<data::Uid>();
  quest.questId = row["quest_id"].as<uint32_t>();
  quest.isCompleted = static_cast<data::Quest::Status>(row["is_completed"].as<int16_t>());
  quest.progress = row["progress"].as<uint32_t>();

  transaction.commit();
}

void PqDataSource::StoreQuest(const data::Uid uid, const data::Quest& quest)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.quest", QuestColumns),
    pqxx::params{
      uid,
      quest.questId(),
      static_cast<int16_t>(quest.isCompleted()),
      quest.progress()});

  transaction.commit();
}

void PqDataSource::DeleteQuest(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.quest", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Mail
// ---------------------------------------------------------------------------

void PqDataSource::CreateMail(data::Mail& mail)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  mail.uid = NextUid(transaction, "data.mail_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveMail(const data::Uid uid, data::Mail& mail)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.mail", "uid", MailColumns, uid);

  mail.uid = row["uid"].as<data::Uid>();
  mail.from = row["from_uid"].as<data::Uid>();
  mail.to = row["to_uid"].as<data::Uid>();
  mail.isRead = row["is_read"].as<bool>();
  mail.isDeleted = row["is_deleted"].as<bool>();
  mail.type = static_cast<data::Mail::MailType>(row["type"].as<int16_t>());
  mail.claimUid = row["claim_uid"].as<uint32_t>();
  mail.createdAt = FromEpochSeconds(row["created_at"].as<int64_t>());
  mail.body = row["body"].as<std::string>();

  transaction.commit();
}

void PqDataSource::StoreMail(const data::Uid uid, const data::Mail& mail)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.mail", MailColumns),
    pqxx::params{
      uid,
      mail.from(),
      mail.to(),
      mail.isRead(),
      mail.isDeleted(),
      static_cast<int16_t>(mail.type()),
      mail.claimUid(),
      ToEpochSeconds(mail.createdAt()),
      mail.body()});

  transaction.commit();
}

void PqDataSource::DeleteMail(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.mail", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Stallion
// ---------------------------------------------------------------------------

void PqDataSource::CreateStallion(data::Stallion& stallion)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  stallion.uid = NextUid(transaction, "data.stallion_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveStallion(const data::Uid uid, data::Stallion& stallion)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.stallion", "uid", StallionColumns, uid);

  stallion.uid = row["uid"].as<data::Uid>();
  stallion.horseUid = row["horse_uid"].as<data::Uid>();
  stallion.ownerUid = row["owner_uid"].as<data::Uid>();
  stallion.breedingCharge = row["breeding_charge"].as<uint32_t>();
  stallion.timesMated = row["times_mated"].as<uint32_t>();
  stallion.registeredAt = FromEpochSeconds(row["registered_at"].as<int64_t>());
  stallion.expiresAt = FromEpochSeconds(row["expires_at"].as<int64_t>());

  transaction.commit();
}

void PqDataSource::StoreStallion(const data::Uid uid, const data::Stallion& stallion)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.stallion", StallionColumns),
    pqxx::params{
      uid,
      stallion.horseUid(),
      stallion.ownerUid(),
      stallion.breedingCharge(),
      stallion.timesMated(),
      ToEpochSeconds(stallion.registeredAt()),
      ToEpochSeconds(stallion.expiresAt())});

  transaction.commit();
}

void PqDataSource::DeleteStallion(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.stallion", "uid", uid);
  transaction.commit();
}

std::vector<data::Uid> PqDataSource::ListRegisteredStallions()
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto result = transaction.exec("SELECT uid FROM data.stallion ORDER BY uid");

  std::vector<data::Uid> uids;
  uids.reserve(result.size());
  for (const auto& row : result)
    uids.emplace_back(row[0].as<data::Uid>());

  transaction.commit();
  return uids;
}

// ---------------------------------------------------------------------------
// Reward
// ---------------------------------------------------------------------------

void PqDataSource::CreateReward(data::Reward& reward)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  reward.claimUid = NextUid(transaction, "data.reward_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveReward(const data::Uid claimUid, data::Reward& reward)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(
    transaction, "data.reward", "claim_uid", RewardColumns, claimUid);

  reward.claimUid = row["claim_uid"].as<data::Uid>();
  reward.characterUid = row["character_uid"].as<data::Uid>();
  reward.type = static_cast<data::Reward::Type>(row["type"].as<int16_t>());
  reward.carrots = row["carrots"].as<uint32_t>();
  reward.isClaimed = row["is_claimed"].as<bool>();
  reward.createdAt = FromEpochSeconds(row["created_at"].as<int64_t>());
  reward.claimedAt = FromEpochSeconds(row["claimed_at"].as<int64_t>());

  transaction.commit();
}

void PqDataSource::StoreReward(const data::Uid claimUid, const data::Reward& reward)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.reward", RewardColumns),
    pqxx::params{
      claimUid,
      reward.characterUid(),
      static_cast<int16_t>(reward.type()),
      reward.carrots(),
      reward.isClaimed(),
      ToEpochSeconds(reward.createdAt()),
      ToEpochSeconds(reward.claimedAt())});

  transaction.commit();
}

void PqDataSource::DeleteReward(const data::Uid claimUid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.reward", "claim_uid", claimUid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Storage item
// ---------------------------------------------------------------------------

void PqDataSource::CreateStorageItem(data::StorageItem& storageItem)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  storageItem.uid = NextUid(transaction, "data.storage_item_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveStorageItem(const data::Uid uid, data::StorageItem& storageItem)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(
    transaction, "data.storage_item", "uid", StorageItemColumns, uid);

  storageItem.uid = row["uid"].as<data::Uid>();
  storageItem.sender = row["sender"].as<std::string>();
  storageItem.message = row["message"].as<std::string>();
  storageItem.carrots = row["carrots"].as<int32_t>();
  storageItem.checked = row["checked"].as<bool>();
  storageItem.createdAt = FromEpochSeconds(row["created_at"].as<int64_t>());
  storageItem.duration = std::chrono::seconds(row["duration"].as<int64_t>());
  storageItem.goodsSq = row["goods_sq"].as<uint32_t>();
  storageItem.priceId = row["price_id"].as<uint32_t>();

  const auto entries = transaction.exec(
    "SELECT tid, count, duration FROM data.storage_item_entry "
    "WHERE storage_item_uid = $1 ORDER BY ordinal",
    pqxx::params{uid});

  std::vector<data::StorageItem::Item> items;
  items.reserve(entries.size());
  for (const auto& entry : entries)
  {
    items.emplace_back(data::StorageItem::Item{
      .tid = entry["tid"].as<data::Tid>(),
      .count = entry["count"].as<uint32_t>(),
      .duration = std::chrono::seconds(entry["duration"].as<int64_t>())});
  }
  storageItem.items = std::move(items);

  transaction.commit();
}

void PqDataSource::StoreStorageItem(const data::Uid uid, const data::StorageItem& storageItem)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.storage_item", StorageItemColumns),
    pqxx::params{
      uid,
      storageItem.sender(),
      storageItem.message(),
      storageItem.carrots(),
      storageItem.checked(),
      ToEpochSeconds(storageItem.createdAt()),
      static_cast<int64_t>(storageItem.duration().count()),
      storageItem.goodsSq(),
      storageItem.priceId()});

  transaction.exec(
    "DELETE FROM data.storage_item_entry WHERE storage_item_uid = $1",
    pqxx::params{uid});

  const auto& items = storageItem.items();
  for (size_t index = 0; index < items.size(); ++index)
  {
    const auto& item = items[index];
    transaction.exec(
      "INSERT INTO data.storage_item_entry "
      "(storage_item_uid, ordinal, tid, count, duration) VALUES ($1, $2, $3, $4, $5)",
      pqxx::params{
        uid,
        static_cast<int32_t>(index),
        item.tid,
        item.count,
        static_cast<int64_t>(item.duration.count())});
  }

  transaction.commit();
}

void PqDataSource::DeleteStorageItem(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.storage_item", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Daily quest group
// ---------------------------------------------------------------------------

void PqDataSource::CreateDailyQuestGroup(data::DailyQuestGroup& group)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  group.uid = NextUid(transaction, "data.daily_quest_group_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveDailyQuestGroup(const data::Uid uid, data::DailyQuestGroup& group)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(
    transaction, "data.daily_quest_group", "uid", DailyQuestGroupColumns, uid);

  group.uid = row["uid"].as<data::Uid>();
  group.rewardId = static_cast<uint8_t>(row["reward_id"].as<int16_t>());
  group.rewardType = static_cast<uint8_t>(row["reward_type"].as<int16_t>());
  group.rewardPoints = row["reward_points"].as<uint32_t>();
  group.carrotsClaimed = row["carrots_claimed"].as<bool>();

  const auto entries = transaction.exec(
    "SELECT slot, quest_id, progress FROM data.daily_quest_entry WHERE group_uid = $1",
    pqxx::params{uid});

  std::array<data::DailyQuestEntry, 3> quests{};
  for (const auto& entry : entries)
  {
    const auto slot = entry["slot"].as<int16_t>();
    if (slot < 0 || slot >= static_cast<int16_t>(quests.size()))
      continue;

    quests[static_cast<size_t>(slot)] = data::DailyQuestEntry{
      .questId = entry["quest_id"].as<uint16_t>(),
      .progress = entry["progress"].as<uint32_t>()};
  }
  group.quests = quests;

  transaction.commit();
}

void PqDataSource::StoreDailyQuestGroup(const data::Uid uid, const data::DailyQuestGroup& group)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.daily_quest_group", DailyQuestGroupColumns),
    pqxx::params{
      uid,
      static_cast<int16_t>(group.rewardId()),
      static_cast<int16_t>(group.rewardType()),
      group.rewardPoints(),
      group.carrotsClaimed()});

  transaction.exec(
    "DELETE FROM data.daily_quest_entry WHERE group_uid = $1",
    pqxx::params{uid});

  const auto& quests = group.quests();
  for (size_t slot = 0; slot < quests.size(); ++slot)
  {
    transaction.exec(
      "INSERT INTO data.daily_quest_entry (group_uid, slot, quest_id, progress) "
      "VALUES ($1, $2, $3, $4)",
      pqxx::params{
        uid,
        static_cast<int16_t>(slot),
        quests[slot].questId,
        quests[slot].progress});
  }

  transaction.commit();
}

void PqDataSource::DeleteDailyQuestGroup(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.daily_quest_group", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

void PqDataSource::CreateSettings(data::Settings& settings)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  settings.uid = NextUid(transaction, "data.settings_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveSettings(const data::Uid uid, data::Settings& settings)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(
    transaction, "data.settings", "uid", SettingsColumns, uid);

  settings.uid = row["uid"].as<data::Uid>();
  settings.age = row["age"].as<uint32_t>();
  settings.hideAge = row["hide_age"].as<bool>();

  // An absent optional and a present but empty one are distinct, so the
  // presence flag decides whether the optional is engaged at all.
  const auto retrieveBindings = [&](
    const SettingsBindingDevice device,
    std::optional<std::vector<data::Settings::Option>>& target)
  {
    auto& bindings = target.emplace();

    const auto result = transaction.exec(
      "SELECT primary_key, type, secondary_key FROM data.settings_binding "
      "WHERE settings_uid = $1 AND device = $2 ORDER BY ordinal",
      pqxx::params{uid, static_cast<int16_t>(device)});

    bindings.reserve(result.size());
    for (const auto& binding : result)
    {
      bindings.emplace_back(data::Settings::Option{
        .primaryKey = binding["primary_key"].as<uint32_t>(),
        .type = binding["type"].as<uint32_t>(),
        .secondaryKey = binding["secondary_key"].as<uint32_t>()});
    }
  };

  if (row["has_keyboard_bindings"].as<bool>())
    retrieveBindings(SettingsBindingDevice::Keyboard, settings.keyboardBindings());
  else
    settings.keyboardBindings().reset();

  if (row["has_gamepad_bindings"].as<bool>())
    retrieveBindings(SettingsBindingDevice::Gamepad, settings.gamepadBindings());
  else
    settings.gamepadBindings().reset();

  if (row["has_macros"].as<bool>())
  {
    auto& macros = settings.macros().emplace();

    const auto result = transaction.exec(
      "SELECT slot, body FROM data.settings_macro WHERE settings_uid = $1",
      pqxx::params{uid});

    for (const auto& macro : result)
    {
      const auto slot = macro["slot"].as<int16_t>();
      if (slot < 0 || slot >= static_cast<int16_t>(macros.size()))
        continue;
      macros[static_cast<size_t>(slot)] = macro["body"].as<std::string>();
    }
  }
  else
  {
    settings.macros().reset();
  }

  transaction.commit();
}

void PqDataSource::StoreSettings(const data::Uid uid, const data::Settings& settings)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.settings", SettingsColumns),
    pqxx::params{
      uid,
      settings.age(),
      settings.hideAge(),
      settings.keyboardBindings().has_value(),
      settings.gamepadBindings().has_value(),
      settings.macros().has_value()});

  transaction.exec(
    "DELETE FROM data.settings_binding WHERE settings_uid = $1",
    pqxx::params{uid});

  const auto storeBindings = [&](
    const SettingsBindingDevice device,
    const std::optional<std::vector<data::Settings::Option>>& bindings)
  {
    if (not bindings)
      return;

    for (size_t index = 0; index < bindings->size(); ++index)
    {
      const auto& binding = (*bindings)[index];
      transaction.exec(
        "INSERT INTO data.settings_binding "
        "(settings_uid, device, ordinal, primary_key, type, secondary_key) "
        "VALUES ($1, $2, $3, $4, $5, $6)",
        pqxx::params{
          uid,
          static_cast<int16_t>(device),
          static_cast<int32_t>(index),
          binding.primaryKey,
          binding.type,
          binding.secondaryKey});
    }
  };

  storeBindings(SettingsBindingDevice::Keyboard, settings.keyboardBindings());
  storeBindings(SettingsBindingDevice::Gamepad, settings.gamepadBindings());

  transaction.exec(
    "DELETE FROM data.settings_macro WHERE settings_uid = $1",
    pqxx::params{uid});

  if (settings.macros())
  {
    const auto& macros = *settings.macros();
    for (size_t slot = 0; slot < macros.size(); ++slot)
    {
      transaction.exec(
        "INSERT INTO data.settings_macro (settings_uid, slot, body) VALUES ($1, $2, $3)",
        pqxx::params{uid, static_cast<int16_t>(slot), macros[slot]});
    }
  }

  transaction.commit();
}

void PqDataSource::DeleteSettings(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.settings", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Guild
// ---------------------------------------------------------------------------

void PqDataSource::CreateGuild(data::Guild& guild)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  guild.uid = NextUid(transaction, "data.guild_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveGuild(const data::Uid uid, data::Guild& guild)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.guild", "uid", GuildColumns, uid);

  guild.uid = row["uid"].as<data::Uid>();
  guild.name = row["name"].as<std::string>();
  guild.description = row["description"].as<std::string>();
  guild.owner = row["owner"].as<data::Uid>();
  guild.rank = row["rank"].as<uint32_t>();
  guild.totalWins = row["total_wins"].as<uint32_t>();
  guild.totalLosses = row["total_losses"].as<uint32_t>();
  guild.seasonalWins = row["seasonal_wins"].as<uint32_t>();
  guild.seasonalLosses = row["seasonal_losses"].as<uint32_t>();

  guild.officers = RetrieveUidList(
    transaction, "data.guild_officer", "guild_uid", "character_uid", uid);
  guild.members = RetrieveUidList(
    transaction, "data.guild_member", "guild_uid", "character_uid", uid);

  transaction.commit();
}

void PqDataSource::StoreGuild(const data::Uid uid, const data::Guild& guild)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  transaction.exec(
    BuildUpsert("data.guild", GuildColumns),
    pqxx::params{
      uid,
      guild.name(),
      guild.description(),
      guild.owner(),
      guild.rank(),
      guild.totalWins(),
      guild.totalLosses(),
      guild.seasonalWins(),
      guild.seasonalLosses()});

  StoreUidList(
    transaction, "data.guild_officer", "guild_uid", "character_uid", uid, guild.officers());
  StoreUidList(
    transaction, "data.guild_member", "guild_uid", "character_uid", uid, guild.members());

  transaction.commit();
}

void PqDataSource::DeleteGuild(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.guild", "uid", uid);
  transaction.commit();
}

bool PqDataSource::IsGuildNameUnique(const std::string_view& name)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto result = transaction.exec(
    "SELECT 1 FROM data.guild WHERE lower(name) = lower($1)",
    pqxx::params{name});

  transaction.commit();
  return result.empty();
}

// ---------------------------------------------------------------------------
// Horse
// ---------------------------------------------------------------------------

void PqDataSource::CreateHorse(data::Horse& horse)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  // Shared with items, see CreateItem.
  horse.uid = NextUid(transaction, "data.equipment_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveHorse(const data::Uid uid, data::Horse& horse)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(transaction, "data.horse", "uid", HorseColumns, uid);

  horse.uid = row["uid"].as<data::Uid>();
  horse.tid = row["tid"].as<data::Tid>();
  horse.name = row["name"].as<std::string>();

  horse.parts.skinTid = row["parts_skin_tid"].as<data::Tid>();
  horse.parts.faceTid = row["parts_face_tid"].as<data::Tid>();
  horse.parts.maneTid = row["parts_mane_tid"].as<data::Tid>();
  horse.parts.tailTid = row["parts_tail_tid"].as<data::Tid>();

  horse.appearance.scale = row["appearance_scale"].as<uint32_t>();
  horse.appearance.legLength = row["appearance_leg_length"].as<uint32_t>();
  horse.appearance.legVolume = row["appearance_leg_volume"].as<uint32_t>();
  horse.appearance.bodyLength = row["appearance_body_length"].as<uint32_t>();
  horse.appearance.bodyVolume = row["appearance_body_volume"].as<uint32_t>();

  horse.stats.agility = row["stats_agility"].as<uint32_t>();
  horse.stats.courage = row["stats_courage"].as<uint32_t>();
  horse.stats.rush = row["stats_rush"].as<uint32_t>();
  horse.stats.endurance = row["stats_endurance"].as<uint32_t>();
  horse.stats.ambition = row["stats_ambition"].as<uint32_t>();

  horse.mastery.spurMagicCount = row["mastery_spur_magic_count"].as<uint32_t>();
  horse.mastery.jumpCount = row["mastery_jump_count"].as<uint32_t>();
  horse.mastery.slidingTime = row["mastery_sliding_time"].as<uint32_t>();
  horse.mastery.glidingDistance = row["mastery_gliding_distance"].as<uint32_t>();

  horse.rating = row["rating"].as<uint32_t>();
  horse.clazz = row["clazz"].as<uint32_t>();
  horse.clazzProgress = row["clazz_progress"].as<uint32_t>();
  horse.grade = row["grade"].as<uint32_t>();
  horse.growthPoints = row["growth_points"].as<uint32_t>();

  horse.breedingCount = row["breeding_count"].as<uint32_t>();
  horse.breedingCombo = row["breeding_combo"].as<uint32_t>();

  horse.type = static_cast<data::Horse::Type>(row["type"].as<int16_t>());
  horse.dateOfBirth = FromEpochSeconds(row["date_of_birth"].as<int64_t>());

  horse.tendency = row["tendency"].as<uint32_t>();
  horse.spirit = row["spirit"].as<uint32_t>();

  horse.potential.type = row["potential_type"].as<uint32_t>();
  horse.potential.level = row["potential_level"].as<uint32_t>();
  horse.potential.value = row["potential_value"].as<uint32_t>();

  horse.luckState = row["luck_state"].as<uint32_t>();
  horse.fatigue = row["fatigue"].as<uint32_t>();
  horse.emblemUid = row["emblem_uid"].as<uint32_t>();

  horse.mountCondition.stamina = row["condition_stamina"].as<uint32_t>();
  horse.mountCondition.charm = row["condition_charm"].as<uint32_t>();
  horse.mountCondition.friendliness = row["condition_friendliness"].as<uint32_t>();
  horse.mountCondition.injury = row["condition_injury"].as<uint32_t>();
  horse.mountCondition.plenitude = row["condition_plenitude"].as<uint32_t>();
  horse.mountCondition.bodyDirtiness = row["condition_body_dirtiness"].as<uint32_t>();
  horse.mountCondition.maneDirtiness = row["condition_mane_dirtiness"].as<uint32_t>();
  horse.mountCondition.tailDirtiness = row["condition_tail_dirtiness"].as<uint32_t>();
  horse.mountCondition.bodyPolish = row["condition_body_polish"].as<uint32_t>();
  horse.mountCondition.manePolish = row["condition_mane_polish"].as<uint32_t>();
  horse.mountCondition.tailPolish = row["condition_tail_polish"].as<uint32_t>();
  horse.mountCondition.attachment = row["condition_attachment"].as<uint32_t>();
  horse.mountCondition.boredom = row["condition_boredom"].as<uint32_t>();
  horse.mountCondition.stopAmendsPoint = row["condition_stop_amends_point"].as<uint32_t>();

  horse.mountInfo.boostsInARow = row["info_boosts_in_a_row"].as<uint32_t>();
  horse.mountInfo.winsSpeedSingle = row["info_wins_speed_single"].as<uint32_t>();
  horse.mountInfo.winsSpeedTeam = row["info_wins_speed_team"].as<uint32_t>();
  horse.mountInfo.winsMagicSingle = row["info_wins_magic_single"].as<uint32_t>();
  horse.mountInfo.winsMagicTeam = row["info_wins_magic_team"].as<uint32_t>();
  horse.mountInfo.totalDistance = row["info_total_distance"].as<uint32_t>();
  horse.mountInfo.topSpeed = row["info_top_speed"].as<uint32_t>();
  horse.mountInfo.longestGlideDistance = row["info_longest_glide_distance"].as<uint32_t>();
  horse.mountInfo.participated = row["info_participated"].as<uint32_t>();
  horse.mountInfo.cumulativePrize = row["info_cumulative_prize"].as<uint32_t>();
  horse.mountInfo.biggestPrize = row["info_biggest_prize"].as<uint32_t>();

  horse.ancestors.father = row["ancestors_father"].as<data::Uid>();
  horse.ancestors.mother = row["ancestors_mother"].as<data::Uid>();

  horse.lineage = row["lineage"].as<uint32_t>();

  transaction.commit();
}

void PqDataSource::StoreHorse(const data::Uid uid, const data::Horse& horse)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  pqxx::params parameters;
  parameters.append(uid);
  parameters.append(horse.tid());
  parameters.append(horse.name());

  parameters.append(horse.parts.skinTid());
  parameters.append(horse.parts.faceTid());
  parameters.append(horse.parts.maneTid());
  parameters.append(horse.parts.tailTid());

  parameters.append(horse.appearance.scale());
  parameters.append(horse.appearance.legLength());
  parameters.append(horse.appearance.legVolume());
  parameters.append(horse.appearance.bodyLength());
  parameters.append(horse.appearance.bodyVolume());

  parameters.append(horse.stats.agility());
  parameters.append(horse.stats.courage());
  parameters.append(horse.stats.rush());
  parameters.append(horse.stats.endurance());
  parameters.append(horse.stats.ambition());

  parameters.append(horse.mastery.spurMagicCount());
  parameters.append(horse.mastery.jumpCount());
  parameters.append(horse.mastery.slidingTime());
  parameters.append(horse.mastery.glidingDistance());

  parameters.append(horse.rating());
  parameters.append(horse.clazz());
  parameters.append(horse.clazzProgress());
  parameters.append(horse.grade());
  parameters.append(horse.growthPoints());

  parameters.append(horse.breedingCount());
  parameters.append(horse.breedingCombo());

  parameters.append(static_cast<int16_t>(horse.type()));
  parameters.append(ToEpochSeconds(horse.dateOfBirth()));

  parameters.append(horse.tendency());
  parameters.append(horse.spirit());

  parameters.append(horse.potential.type());
  parameters.append(horse.potential.level());
  parameters.append(horse.potential.value());

  parameters.append(horse.luckState());
  parameters.append(horse.fatigue());
  parameters.append(horse.emblemUid());

  parameters.append(horse.mountCondition.stamina());
  parameters.append(horse.mountCondition.charm());
  parameters.append(horse.mountCondition.friendliness());
  parameters.append(horse.mountCondition.injury());
  parameters.append(horse.mountCondition.plenitude());
  parameters.append(horse.mountCondition.bodyDirtiness());
  parameters.append(horse.mountCondition.maneDirtiness());
  parameters.append(horse.mountCondition.tailDirtiness());
  parameters.append(horse.mountCondition.bodyPolish());
  parameters.append(horse.mountCondition.manePolish());
  parameters.append(horse.mountCondition.tailPolish());
  parameters.append(horse.mountCondition.attachment());
  parameters.append(horse.mountCondition.boredom());
  parameters.append(horse.mountCondition.stopAmendsPoint());

  parameters.append(horse.mountInfo.boostsInARow());
  parameters.append(horse.mountInfo.winsSpeedSingle());
  parameters.append(horse.mountInfo.winsSpeedTeam());
  parameters.append(horse.mountInfo.winsMagicSingle());
  parameters.append(horse.mountInfo.winsMagicTeam());
  parameters.append(horse.mountInfo.totalDistance());
  parameters.append(horse.mountInfo.topSpeed());
  parameters.append(horse.mountInfo.longestGlideDistance());
  parameters.append(horse.mountInfo.participated());
  parameters.append(horse.mountInfo.cumulativePrize());
  parameters.append(horse.mountInfo.biggestPrize());

  parameters.append(horse.ancestors.father);
  parameters.append(horse.ancestors.mother);

  parameters.append(horse.lineage());

  transaction.exec(BuildUpsert("data.horse", HorseColumns), std::move(parameters));
  transaction.commit();
}

void PqDataSource::DeleteHorse(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  DeleteRow(transaction, "data.horse", "uid", uid);
  transaction.commit();
}

// ---------------------------------------------------------------------------
// Character
// ---------------------------------------------------------------------------

void PqDataSource::CreateCharacter(data::Character& character)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  character.uid = NextUid(transaction, "data.character_uid_seq");
  transaction.commit();
}

void PqDataSource::RetrieveCharacter(const data::Uid uid, data::Character& character)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto row = RetrieveRow(
    transaction, "data.character", "uid", CharacterColumns, uid);

  character.uid = row["uid"].as<data::Uid>();
  character.name = row["name"].as<std::string>();
  character.introduction = row["introduction"].as<std::string>();

  character.level = row["level"].as<uint32_t>();
  character.experience = row["experience"].as<uint32_t>();
  character.carrots = row["carrots"].as<int32_t>();
  character.cash = row["cash"].as<int32_t>();

  character.role = static_cast<data::Character::Role>(row["role"].as<int16_t>());
  character.roleRank = static_cast<data::Character::RoleRank>(row["role_rank"].as<int16_t>());

  character.parts.modelId = row["parts_model_id"].as<uint32_t>();
  character.parts.mouthId = row["parts_mouth_id"].as<uint32_t>();
  character.parts.faceId = row["parts_face_id"].as<uint32_t>();

  character.appearance.voiceId = row["appearance_voice_id"].as<uint32_t>();
  character.appearance.headSize = row["appearance_head_size"].as<uint32_t>();
  character.appearance.height = row["appearance_height"].as<uint32_t>();
  character.appearance.thighVolume = row["appearance_thigh_volume"].as<uint32_t>();
  character.appearance.legVolume = row["appearance_leg_volume"].as<uint32_t>();
  character.appearance.emblemId = row["appearance_emblem_id"].as<uint32_t>();

  character.guildUid = row["guild_uid"].as<data::Uid>();
  character.horseSlotCount = static_cast<uint8_t>(row["horse_slot_count"].as<int16_t>());
  character.mountUid = row["mount_uid"].as<data::Uid>();
  character.petUid = row["pet_uid"].as<data::Uid>();
  character.isRanchLocked = row["is_ranch_locked"].as<bool>();
  character.settingsUid = row["settings_uid"].as<data::Uid>();
  character.dailyQuestGroupUid = row["daily_quest_group_uid"].as<data::Uid>();

  character.skills.speed = data::Character::Skills::Sets{
    .set1 = {
      .slot1 = row["skills_speed_set1_slot1"].as<uint32_t>(),
      .slot2 = row["skills_speed_set1_slot2"].as<uint32_t>()},
    .set2 = {
      .slot1 = row["skills_speed_set2_slot1"].as<uint32_t>(),
      .slot2 = row["skills_speed_set2_slot2"].as<uint32_t>()},
    .activeSetId = row["skills_speed_active_set_id"].as<uint32_t>()};
  character.skills.magic = data::Character::Skills::Sets{
    .set1 = {
      .slot1 = row["skills_magic_set1_slot1"].as<uint32_t>(),
      .slot2 = row["skills_magic_set1_slot2"].as<uint32_t>()},
    .set2 = {
      .slot1 = row["skills_magic_set2_slot1"].as<uint32_t>(),
      .slot2 = row["skills_magic_set2_slot2"].as<uint32_t>()},
    .activeSetId = row["skills_magic_active_set_id"].as<uint32_t>()};

  character.mailbox.hasNewMail = row["mailbox_has_new_mail"].as<bool>();

  character.inventory = RetrieveCharacterItems(
    transaction, uid, CharacterItemKind::Inventory);
  character.characterEquipment = RetrieveCharacterItems(
    transaction, uid, CharacterItemKind::CharacterEquipment);
  character.gifts = RetrieveCharacterItems(
    transaction, uid, CharacterItemKind::Gifts);
  character.purchases = RetrieveCharacterItems(
    transaction, uid, CharacterItemKind::Purchases);

  character.horses = RetrieveUidList(
    transaction, "data.character_horse", "character_uid", "horse_uid", uid);
  character.pets = RetrieveUidList(
    transaction, "data.character_pet", "character_uid", "pet_uid", uid);
  character.eggs = RetrieveUidList(
    transaction, "data.character_egg", "character_uid", "egg_uid", uid);
  character.housing = RetrieveUidList(
    transaction, "data.character_housing", "character_uid", "housing_uid", uid);
  character.quests = RetrieveUidList(
    transaction, "data.character_quest", "character_uid", "quest_uid", uid);

  character.mailbox.inbox = RetrieveCharacterMail(
    transaction, uid, CharacterMailBox::Inbox);
  character.mailbox.sent = RetrieveCharacterMail(
    transaction, uid, CharacterMailBox::Sent);

  character.breedingWishlist = RetrieveUidSet(
    transaction, "data.character_breeding_wishlist", "character_uid", "horse_uid", uid);
  character.contacts.pending = RetrieveUidSet(
    transaction, "data.character_contact_pending", "character_uid", "contact_uid", uid);

  const auto groups = transaction.exec(
    "SELECT group_uid, name, extract(epoch from created_at)::bigint AS created_at "
    "FROM data.character_contact_group WHERE character_uid = $1",
    pqxx::params{uid});

  std::map<data::Uid, data::Character::Contacts::Group> contactGroups;
  for (const auto& group : groups)
  {
    const auto groupUid = group["group_uid"].as<data::Uid>();

    const auto members = transaction.exec(
      "SELECT member_uid FROM data.character_contact_group_member "
      "WHERE character_uid = $1 AND group_uid = $2",
      pqxx::params{uid, groupUid});

    std::set<data::Uid> memberUids;
    for (const auto& member : members)
      memberUids.emplace(member[0].as<data::Uid>());

    contactGroups.try_emplace(
      groupUid,
      data::Character::Contacts::Group{
        .uid = groupUid,
        .name = group["name"].as<std::string>(),
        .members = std::move(memberUids),
        .createdAt = FromEpochSeconds(group["created_at"].as<int64_t>())});
  }
  character.contacts.groups = std::move(contactGroups);

  transaction.commit();
}

void PqDataSource::StoreCharacter(const data::Uid uid, const data::Character& character)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  pqxx::params parameters;
  parameters.append(uid);
  parameters.append(character.name());
  parameters.append(character.introduction());

  parameters.append(character.level());
  parameters.append(character.experience());
  parameters.append(character.carrots());
  parameters.append(character.cash());

  parameters.append(static_cast<int16_t>(character.role()));
  parameters.append(static_cast<int16_t>(character.roleRank()));

  parameters.append(character.parts.modelId());
  parameters.append(character.parts.mouthId());
  parameters.append(character.parts.faceId());

  parameters.append(character.appearance.voiceId());
  parameters.append(character.appearance.headSize());
  parameters.append(character.appearance.height());
  parameters.append(character.appearance.thighVolume());
  parameters.append(character.appearance.legVolume());
  parameters.append(character.appearance.emblemId());

  parameters.append(character.guildUid());
  parameters.append(static_cast<int16_t>(character.horseSlotCount()));
  parameters.append(character.mountUid());
  parameters.append(character.petUid());
  parameters.append(character.isRanchLocked());
  parameters.append(character.settingsUid());
  parameters.append(character.dailyQuestGroupUid());

  parameters.append(character.skills.speed().set1.slot1);
  parameters.append(character.skills.speed().set1.slot2);
  parameters.append(character.skills.speed().set2.slot1);
  parameters.append(character.skills.speed().set2.slot2);
  parameters.append(character.skills.speed().activeSetId);
  parameters.append(character.skills.magic().set1.slot1);
  parameters.append(character.skills.magic().set1.slot2);
  parameters.append(character.skills.magic().set2.slot1);
  parameters.append(character.skills.magic().set2.slot2);
  parameters.append(character.skills.magic().activeSetId);

  parameters.append(character.mailbox.hasNewMail());

  transaction.exec(BuildUpsert("data.character", CharacterColumns), std::move(parameters));

  StoreCharacterItems(
    transaction, uid, CharacterItemKind::Inventory, character.inventory());
  StoreCharacterItems(
    transaction, uid, CharacterItemKind::CharacterEquipment, character.characterEquipment());
  StoreCharacterItems(
    transaction, uid, CharacterItemKind::Gifts, character.gifts());
  StoreCharacterItems(
    transaction, uid, CharacterItemKind::Purchases, character.purchases());

  StoreUidList(
    transaction, "data.character_horse", "character_uid", "horse_uid", uid, character.horses());
  StoreUidList(
    transaction, "data.character_pet", "character_uid", "pet_uid", uid, character.pets());
  StoreUidList(
    transaction, "data.character_egg", "character_uid", "egg_uid", uid, character.eggs());
  StoreUidList(
    transaction, "data.character_housing", "character_uid", "housing_uid", uid, character.housing());
  StoreUidList(
    transaction, "data.character_quest", "character_uid", "quest_uid", uid, character.quests());

  StoreCharacterMail(
    transaction, uid, CharacterMailBox::Inbox, character.mailbox.inbox());
  StoreCharacterMail(
    transaction, uid, CharacterMailBox::Sent, character.mailbox.sent());

  StoreUidSet(
    transaction,
    "data.character_breeding_wishlist",
    "character_uid",
    "horse_uid",
    uid,
    character.breedingWishlist());
  StoreUidSet(
    transaction,
    "data.character_contact_pending",
    "character_uid",
    "contact_uid",
    uid,
    character.contacts.pending());

  // The members cascade from the groups, so deleting the groups is enough.
  transaction.exec(
    "DELETE FROM data.character_contact_group WHERE character_uid = $1",
    pqxx::params{uid});

  for (const auto& group : character.contacts.groups() | std::views::values)
  {
    transaction.exec(
      "INSERT INTO data.character_contact_group "
      "(character_uid, group_uid, name, created_at) VALUES ($1, $2, $3, to_timestamp($4))",
      pqxx::params{uid, group.uid, group.name, ToEpochSeconds(group.createdAt)});

    for (const auto memberUid : group.members)
    {
      transaction.exec(
        "INSERT INTO data.character_contact_group_member "
        "(character_uid, group_uid, member_uid) VALUES ($1, $2, $3)",
        pqxx::params{uid, group.uid, memberUid});
    }
  }

  transaction.commit();
}

void PqDataSource::DeleteCharacter(const data::Uid uid)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  // Every child table cascades from the character row.
  DeleteRow(transaction, "data.character", "uid", uid);
  transaction.commit();
}

data::Uid PqDataSource::RetrieveCharacterUidByName(const std::string_view& name)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto result = transaction.exec(
    "SELECT uid FROM data.character WHERE lower(name) = lower($1)",
    pqxx::params{name});

  transaction.commit();

  if (result.empty())
    return data::InvalidUid;
  return result.one_row()[0].as<data::Uid>();
}

bool PqDataSource::IsCharacterNameUnique(const std::string_view& name)
{
  std::scoped_lock lock(_connectionMutex);
  pqxx::work transaction(Connection());

  const auto result = transaction.exec(
    "SELECT 1 FROM data.character WHERE lower(name) = lower($1)",
    pqxx::params{name});

  transaction.commit();
  return result.empty();
}

} // namespace server
