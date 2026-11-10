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

#include <array>
#include <chrono>
#include <cstdint>
#include <format>
#include <map>
#include <set>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

#include <spdlog/spdlog.h>

namespace server
{

namespace
{

//! A transaction reading a consistent snapshot of multiple tables.
using SnapshotTransaction = pqxx::transaction<
  pqxx::isolation_level::repeatable_read,
  pqxx::write_policy::read_only>;

//! A column of a record table.
struct Column
{
  //! Name of the column.
  std::string_view name;
  //! Table referenced by the column, if the column is a foreign key.
  std::string_view referencedTable{};
  //! Key column of the referenced table.
  std::string_view referencedKey{"uid"};
  //! Whether the referenced record must be owned by the record of the column.
  bool isOwned{false};
};

//! Returns the field of the column in the row.
//! The column name is quoted, as libpq folds unquoted column names to lower case.
pqxx::field FieldOf(
  const pqxx::row& row,
  const std::string_view column)
{
  return row[std::format("\"{}\"", column)];
}

data::Clock::time_point ToTimePoint(
  const int64_t seconds)
{
  return data::Clock::time_point(
    std::chrono::duration_cast<data::Clock::duration>(
      std::chrono::seconds(seconds)));
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
    return std::chrono::ceil<std::chrono::seconds>(
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

//! Reads the column to the field, a null value is read as the zero value of the field.
//! Null values are present in the foreign key columns, where they stand for an invalid UID.
template <typename T>
void ReadField(
  const pqxx::row& row,
  const char* column,
  dao::Field<T>& field)
{
  if constexpr (std::is_enum_v<T>)
  {
    using Underlying = std::underlying_type_t<T>;
    field = static_cast<T>(FieldOf(row, column).as<Underlying>(Underlying{}));
  }
  else if constexpr (std::is_same_v<T, data::Clock::time_point>)
  {
    field = ToTimePoint(FieldOf(row, column).as<int64_t>(int64_t{}));
  }
  else if constexpr (std::is_same_v<T, std::chrono::seconds>)
  {
    field = std::chrono::seconds(FieldOf(row, column).as<int64_t>(int64_t{}));
  }
  else
  {
    field = FieldOf(row, column).as<T>(T{});
  }
}

std::string QualifiedTable(
  const pqxx::transaction_base& query,
  const std::string_view table)
{
  return query.quote_name("data") + "." + query.quote_name(table);
}

//! Produces the expression binding the parameter to the column.
//! A reference to a record which does not exist, including the invalid UID, is bound as null
//! so that a dangling reference never fails the store of the whole record.
//! @param keyIndex Index of the key parameter of the record, required by the owned columns.
std::string ParameterExpression(
  const pqxx::transaction_base& query,
  const Column& column,
  const std::size_t index,
  const std::optional<std::size_t> keyIndex)
{
  if (column.referencedTable.empty())
  {
    return std::format("${}", index);
  }

  std::string ownerCondition;
  if (column.isOwned)
  {
    if (not keyIndex)
    {
      throw std::logic_error("Owned column requires the key of the record");
    }

    // A reference to a record owned by another owner is bound as null too.
    ownerCondition = std::format(" and \"ownerUid\" = ${}", *keyIndex);
  }

  return std::format(
    "(select {0} from {1} where {0} = ${2}{3})",
    query.quote_name(column.referencedKey),
    QualifiedTable(query, column.referencedTable),
    index,
    ownerCondition);
}

//! Assigns the record without an owner to the owner, the ownership of a record is permanent.
template <typename Key>
void ClaimOwnership(
  pqxx::transaction_base& query,
  const std::string_view table,
  const Key& ownerKey,
  const data::Uid uid)
{
  query.exec(
    std::format(
      "update {} "
      "set \"ownerUid\" = $1 "
      "where \"uid\" = $2 and \"ownerUid\" is null",
      QualifiedTable(query, table)),
    pqxx::params(ownerKey, uid));
}

//! Produces a list of columns with the column prepended.
std::vector<Column> PrependColumn(
  const Column& column,
  const std::span<const Column> columns)
{
  std::vector<Column> result;
  result.reserve(columns.size() + 1);
  result.emplace_back(column);
  result.insert(result.end(), columns.begin(), columns.end());
  return result;
}

//! Produces an insert statement of the columns returning the key column.
//! The columns are bound to the parameters in order.
std::string InsertSql(
  const pqxx::transaction_base& query,
  const std::string_view table,
  const std::span<const Column> columns,
  const std::string_view keyColumn)
{
  std::string columnList;
  std::string valueList;
  for (std::size_t index = 0; index < columns.size(); ++index)
  {
    if (index > 0)
    {
      columnList += ", ";
      valueList += ", ";
    }
    columnList += query.quote_name(columns[index].name);
    valueList += ParameterExpression(query, columns[index], index + 1, std::nullopt);
  }

  return std::format(
    "insert into {} ({}) values ({}) returning {}",
    QualifiedTable(query, table),
    columnList,
    valueList,
    query.quote_name(keyColumn));
}

//! Produces an update statement of the columns.
//! The columns are bound to the parameters in order, the key is bound to the last parameter.
std::string UpdateSql(
  const pqxx::transaction_base& query,
  const std::string_view table,
  const std::span<const Column> columns,
  const std::string_view keyColumn)
{
  const auto keyIndex = columns.size() + 1;

  std::string assignmentList;
  for (std::size_t index = 0; index < columns.size(); ++index)
  {
    if (index > 0)
    {
      assignmentList += ", ";
    }
    assignmentList += std::format(
      "{} = {}",
      query.quote_name(columns[index].name),
      ParameterExpression(query, columns[index], index + 1, keyIndex));
  }

  return std::format(
    "update {} set {} where {} = ${}",
    QualifiedTable(query, table),
    assignmentList,
    query.quote_name(keyColumn),
    keyIndex);
}

//! Inserts the record and returns its generated UID.
data::Uid InsertRecord(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::span<const Column> columns,
  const std::string_view keyColumn,
  const pqxx::params& params)
{
  return query.exec(InsertSql(query, table, columns, keyColumn), params)
    .one_row()[0]
    .as<data::Uid>();
}

//! Updates the record by its key.
template <typename Key>
void UpdateRecord(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::span<const Column> columns,
  const std::string_view keyColumn,
  const Key& key,
  pqxx::params params)
{
  params.append(key);
  query.exec(UpdateSql(query, table, columns, keyColumn), params);
}

//! Selects the record by its key.
//! @throws std::runtime_error if the record does not exist.
template <typename Key>
pqxx::row SelectRecord(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::string_view keyColumn,
  const Key& key)
{
  const auto result = query.exec(
    std::format(
      "select * from {} where {} = $1",
      QualifiedTable(query, table),
      query.quote_name(keyColumn)),
    pqxx::params(key));
  if (result.empty())
  {
    throw std::runtime_error(std::format("Record of '{}' does not exist", table));
  }

  return result.one_row();
}

//! Deletes the records by their key.
template <typename Key>
void DeleteRecord(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::string_view keyColumn,
  const Key& key)
{
  query.exec(
    std::format(
      "delete from {} where {} = $1",
      QualifiedTable(query, table),
      query.quote_name(keyColumn)),
    pqxx::params(key));
}

//! Returns the SQL type of the owner key.
//! The parameters selected by `insert ... select` are not typed by the target columns.
template <typename Key>
constexpr std::string_view OwnerKeyType()
{
  if constexpr (std::is_integral_v<Key>)
  {
    return "\"data\".\"uint32\"";
  }
  else
  {
    return "text";
  }
}

//! Whether the list owns the referenced records.
enum class Ownership
{
  //! The referenced records are not owned by the list owner.
  None,
  //! The referenced records are permanently owned by the list owner.
  //! A record without an owner is assigned to the list owner,
  //! a record owned by another owner can not be held by the list.
  Permanent,
};

//! Writes the references held by the record, the order of the references is not kept.
//! References to records which do not exist, or which are owned by another owner, are skipped
//! so that an invalid reference never fails the store of the whole record.
template <typename Key, typename Values>
void WriteUids(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey,
  const std::string_view valueColumn,
  const std::string_view referencedTable,
  const Values& values,
  const Ownership ownership = Ownership::None)
{
  DeleteRecord(query, table, ownerColumn, ownerKey);

  const bool isOwning = ownership == Ownership::Permanent;
  // A reference held more than once is stored once.
  const auto insertSql = std::format(
    "insert into {} ({}, {}) "
    "select $1::{}, \"uid\" "
    "from {} "
    "where \"uid\" = $2{} "
    "on conflict do nothing",
    QualifiedTable(query, table),
    query.quote_name(ownerColumn),
    query.quote_name(valueColumn),
    OwnerKeyType<Key>(),
    QualifiedTable(query, referencedTable),
    isOwning ? " and \"ownerUid\" = $1" : "");

  for (const auto value : values)
  {
    if (isOwning)
    {
      ClaimOwnership(query, referencedTable, ownerKey, value);
    }

    const auto result = query.exec(insertSql, pqxx::params(ownerKey, value));
    if (result.affected_rows() == 0)
    {
      spdlog::warn(
        "Skipped the reference to '{}' {} held by '{}' of {}, "
        "the record does not exist, is owned by another owner or is held more than once",
        referencedTable,
        value,
        table,
        ownerKey);
    }
  }
}

//! Writes the ordered list of references held by the record.
//! References to records which do not exist are skipped.
template <typename Key>
void WriteUidList(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey,
  const std::string_view valueColumn,
  const std::string_view referencedTable,
  const std::vector<data::Uid>& values)
{
  DeleteRecord(query, table, ownerColumn, ownerKey);

  const auto insertSql = std::format(
    "insert into {} ({}, \"position\", {}) "
    "select $1::{}, $2::\"data\".\"uint32\", \"uid\" "
    "from {} "
    "where \"uid\" = $3",
    QualifiedTable(query, table),
    query.quote_name(ownerColumn),
    query.quote_name(valueColumn),
    OwnerKeyType<Key>(),
    QualifiedTable(query, referencedTable));

  for (uint32_t position = 0; position < values.size(); ++position)
  {
    const auto value = values[position];
    const auto result = query.exec(insertSql, pqxx::params(ownerKey, position, value));
    if (result.affected_rows() == 0)
    {
      spdlog::warn(
        "Skipped the reference to '{}' {} held by '{}' of {}, the record does not exist",
        referencedTable,
        value,
        table,
        ownerKey);
    }
  }
}

//! Reads the ordered list of references held by the record.
template <typename Key>
std::vector<data::Uid> ReadUidList(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey,
  const std::string_view valueColumn)
{
  const auto result = query.exec(
    std::format(
      "select {} from {} where {} = $1 order by \"position\"",
      query.quote_name(valueColumn),
      QualifiedTable(query, table),
      query.quote_name(ownerColumn)),
    pqxx::params(ownerKey));

  std::vector<data::Uid> values;
  values.reserve(result.size());
  for (const auto& row : result)
  {
    values.emplace_back(row[0].as<data::Uid>());
  }
  return values;
}

//! Reads the references held by the record, ordered by their UID.
template <typename Values, typename Key>
Values ReadUids(
  pqxx::transaction_base& query,
  const std::string_view table,
  const std::string_view ownerColumn,
  const Key& ownerKey,
  const std::string_view valueColumn)
{
  const auto result = query.exec(
    std::format(
      "select {0} from {1} where {2} = $1 order by {0}",
      query.quote_name(valueColumn),
      QualifiedTable(query, table),
      query.quote_name(ownerColumn)),
    pqxx::params(ownerKey));

  Values values;
  for (const auto& row : result)
  {
    values.insert(values.end(), row[0].as<data::Uid>());
  }
  return values;
}

//! Verifies the position of a row read from an ordered child table.
void VerifyPosition(
  const pqxx::row& row,
  const std::size_t expectedPosition)
{
  if (FieldOf(row, "position").as<uint32_t>() != expectedPosition)
  {
    throw std::runtime_error("Child record has an invalid position");
  }
}

constexpr auto UserColumns = std::to_array<Column>({
  {"token"},
  {"characterUid", "character"},
  {"lastSeenOnline"}});

pqxx::params UserParams(
  const data::User& user)
{
  return pqxx::params(
    WriteField(user.token()),
    WriteField(user.characterUid()),
    WriteField(user.lastSeenOnline()));
}

constexpr auto InfractionColumns = std::to_array<Column>({
  {"description"},
  {"punishment"},
  {"duration"},
  {"createdAt"}});

pqxx::params InfractionParams(
  const data::Infraction& infraction)
{
  return pqxx::params(
    WriteField(infraction.description()),
    WriteField(infraction.punishment()),
    WriteField(infraction.duration()),
    WriteField(infraction.createdAt()));
}

//! Columns of the character without the name, which is assigned only by `ClaimCharacterName`.
constexpr auto CharacterColumns = std::to_array<Column>({
  {"introduction"},
  {"level"},
  {"experience"},
  {"carrots"},
  {"cash"},
  {"role"},
  {"staffRank"},
  {"partsModelId"},
  {"partsMouthId"},
  {"partsFaceId"},
  {"appearanceVoiceId"},
  {"appearanceHeadSize"},
  {"appearanceHeight"},
  {"appearanceThighVolume"},
  {"appearanceLegVolume"},
  {"appearanceEmblemId"},
  {"guildUid", "guild"},
  {"horseSlotCount"},
  {.name = "mountUid", .referencedTable = "horse", .isOwned = true},
  {"petUid", "pet"},
  {"ranchExperience"},
  {"totalRaces"},
  {"isRanchLocked"},
  {"isIntroCompleted"},
  {"settingsUid", "settings"},
  {"dailyQuestGroupUid", "dailyQuestGroup"},
  {"mailboxHasNewMail"}});

pqxx::params CharacterParams(
  const data::Character& character)
{
  return pqxx::params(
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
    WriteField(character.mailbox.hasNewMail()));
}

constexpr auto HorseColumns = std::to_array<Column>({
  {"tid"},
  {"name"},
  {"partsSkinTid"},
  {"partsFaceTid"},
  {"partsManeTid"},
  {"partsTailTid"},
  {"appearanceScale"},
  {"appearanceLegLength"},
  {"appearanceLegVolume"},
  {"appearanceBodyLength"},
  {"appearanceBodyVolume"},
  {"statsAgility"},
  {"statsCourage"},
  {"statsRush"},
  {"statsEndurance"},
  {"statsAmbition"},
  {"masterySpurMagicCount"},
  {"masteryJumpCount"},
  {"masterySlidingTime"},
  {"masteryGlidingDistance"},
  {"rating"},
  {"clazz"},
  {"clazzProgress"},
  {"grade"},
  {"growthPoints"},
  {"breedingCount"},
  {"breedingCombo"},
  {"type"},
  {"dateOfBirth"},
  {"tendency"},
  {"spirit"},
  {"potentialType"},
  {"potentialLevel"},
  {"potentialValue"},
  {"luckState"},
  {"fatigue"},
  {"emblemUid"},
  {"mountConditionStamina"},
  {"mountConditionCharm"},
  {"mountConditionFriendliness"},
  {"mountConditionInjury"},
  {"mountConditionPlenitude"},
  {"mountConditionBodyDirtiness"},
  {"mountConditionManeDirtiness"},
  {"mountConditionTailDirtiness"},
  {"mountConditionBodyPolish"},
  {"mountConditionManePolish"},
  {"mountConditionTailPolish"},
  {"mountConditionAttachment"},
  {"mountConditionBoredom"},
  {"mountConditionLastDailyCareTick"},
  {"mountConditionStopAmendsPoint"},
  {"mountInfoBoostsInARow"},
  {"mountInfoWinsSpeedSingle"},
  {"mountInfoWinsSpeedTeam"},
  {"mountInfoWinsMagicSingle"},
  {"mountInfoWinsMagicTeam"},
  {"mountInfoTotalDistance"},
  {"mountInfoTopSpeed"},
  {"mountInfoLongestGlideDistance"},
  {"mountInfoParticipated"},
  {"mountInfoCumulativePrize"},
  {"mountInfoBiggestPrize"},
  {"ancestorsFather", "horse"},
  {"ancestorsMother", "horse"},
  {"lineage"}});

pqxx::params HorseParams(
  const data::Horse& horse)
{
  return pqxx::params(
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
    WriteField(horse.lineage()));
}

constexpr auto ItemColumns = std::to_array<Column>({
  {"tid"},
  {"count"},
  {"duration"},
  {"createdAt"}});

pqxx::params ItemParams(
  const data::Item& item)
{
  return pqxx::params(
    WriteField(item.tid()),
    WriteField(item.count()),
    WriteField(item.duration()),
    WriteField(item.createdAt()));
}

constexpr auto StorageItemColumns = std::to_array<Column>({
  {"sender"},
  {"message"},
  {"carrots"},
  {"checked"},
  {"createdAt"},
  {"duration"},
  {"goodsSq"},
  {"priceId"}});

pqxx::params StorageItemParams(
  const data::StorageItem& storageItem)
{
  return pqxx::params(
    WriteField(storageItem.sender()),
    WriteField(storageItem.message()),
    WriteField(storageItem.carrots()),
    WriteField(storageItem.checked()),
    WriteField(storageItem.createdAt()),
    WriteField(storageItem.duration()),
    WriteField(storageItem.goodsSq()),
    WriteField(storageItem.priceId()));
}

constexpr auto EggColumns = std::to_array<Column>({
  {"itemUid", "item"},
  {"itemTid"},
  {"incubatedAt"},
  {"incubatorSlot"},
  {"boostsUsed"}});

pqxx::params EggParams(
  const data::Egg& egg)
{
  return pqxx::params(
    WriteField(egg.itemUid()),
    WriteField(egg.itemTid()),
    WriteField(egg.incubatedAt()),
    WriteField(egg.incubatorSlot()),
    WriteField(egg.boostsUsed()));
}

constexpr auto PetColumns = std::to_array<Column>({
  {"itemUid", "item"},
  {"petId"},
  {"name"},
  {"birthDate"}});

pqxx::params PetParams(
  const data::Pet& pet)
{
  return pqxx::params(
    WriteField(pet.itemUid()),
    WriteField(pet.petId()),
    WriteField(pet.name()),
    WriteField(pet.birthDate()));
}

constexpr auto HousingColumns = std::to_array<Column>({
  {"housingId"},
  {"expiresAt"},
  {"durability"}});

pqxx::params HousingParams(
  const data::Housing& housing)
{
  return pqxx::params(
    WriteField(housing.housingId()),
    WriteField(housing.expiresAt()),
    WriteField(housing.durability()));
}

constexpr auto GuildColumns = std::to_array<Column>({
  {"name"},
  {"description"},
  {"owner", "character"},
  {"rank"},
  {"totalWins"},
  {"totalLosses"},
  {"seasonalWins"},
  {"seasonalLosses"}});

pqxx::params GuildParams(
  const data::Guild& guild)
{
  return pqxx::params(
    WriteField(guild.name()),
    WriteField(guild.description()),
    WriteField(guild.owner()),
    WriteField(guild.rank()),
    WriteField(guild.totalWins()),
    WriteField(guild.totalLosses()),
    WriteField(guild.seasonalWins()),
    WriteField(guild.seasonalLosses()));
}

constexpr auto SettingsColumns = std::to_array<Column>({
  {"hasKeyboardBindings"},
  {"hasMacros"},
  {"hasGamepadBindings"},
  {"age"},
  {"hideAge"}});

pqxx::params SettingsParams(
  const data::Settings& settings)
{
  return pqxx::params(
    settings.keyboardBindings().has_value(),
    settings.macros().has_value(),
    settings.gamepadBindings().has_value(),
    WriteField(settings.age()),
    WriteField(settings.hideAge()));
}

constexpr auto DailyQuestGroupColumns = std::to_array<Column>({
  {"rewardId"},
  {"rewardType"},
  {"rewardPoints"},
  {"carrotsClaimed"}});

pqxx::params DailyQuestGroupParams(
  const data::DailyQuestGroup& group)
{
  return pqxx::params(
    WriteField(group.rewardId()),
    WriteField(group.rewardType()),
    WriteField(group.rewardPoints()),
    WriteField(group.carrotsClaimed()));
}

constexpr auto MailColumns = std::to_array<Column>({
  {"senderUid", "character"},
  {"recipientUid", "character"},
  {"isRead"},
  {"isDeleted"},
  {"type"},
  {"claimUid", "reward", "claimUid"},
  {"createdAt"},
  {"body"}});

pqxx::params MailParams(
  const data::Mail& mail)
{
  return pqxx::params(
    WriteField(mail.from()),
    WriteField(mail.to()),
    WriteField(mail.isRead()),
    WriteField(mail.isDeleted()),
    WriteField(mail.type()),
    WriteField(mail.claimUid()),
    WriteField(mail.createdAt()),
    WriteField(mail.body()));
}

constexpr auto QuestColumns = std::to_array<Column>({
  {"questId"},
  {"isCompleted"},
  {"progress"}});

pqxx::params QuestParams(
  const data::Quest& quest)
{
  return pqxx::params(
    WriteField(quest.questId()),
    WriteField(quest.isCompleted()),
    WriteField(quest.progress()));
}

constexpr auto StallionColumns = std::to_array<Column>({
  {"horseUid", "horse"},
  {"ownerUid", "character"},
  {"breedingCharge"},
  {"timesMated"},
  {"registeredAt"},
  {"expiresAt"}});

pqxx::params StallionParams(
  const data::Stallion& stallion)
{
  return pqxx::params(
    WriteField(stallion.horseUid()),
    WriteField(stallion.ownerUid()),
    WriteField(stallion.breedingCharge()),
    WriteField(stallion.timesMated()),
    WriteField(stallion.registeredAt()),
    WriteField(stallion.expiresAt()));
}

constexpr auto RewardColumns = std::to_array<Column>({
  {"characterUid", "character"},
  {"type"},
  {"carrots"},
  {"isClaimed"},
  {"createdAt"},
  {"claimedAt"}});

pqxx::params RewardParams(
  const data::Reward& reward)
{
  return pqxx::params(
    WriteField(reward.characterUid()),
    WriteField(reward.type()),
    WriteField(reward.carrots()),
    WriteField(reward.isClaimed()),
    WriteField(reward.createdAt()),
    WriteField(reward.claimedAt()));
}

void WriteCharacterSkillSet(
  pqxx::transaction_base& query,
  const data::Uid uid,
  const std::string_view mode,
  const data::Character::Skills::Sets& skills)
{
  query.exec(
    "insert into \"data\".\"characterSkillSet\" "
    "(\"characterUid\", \"mode\", \"activeSetId\", \"set1Slot1\", \"set1Slot2\", \"set2Slot1\", \"set2Slot2\") "
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
  pqxx::transaction_base& query,
  const data::Uid uid,
  const data::Character& character)
{
  WriteUids(query, "characterContactPending", "characterUid", uid,
    "contactUid", "character", character.contacts.pending());
  WriteUids(query, "characterGift", "characterUid", uid,
    "storageItemUid", "storageItem", character.gifts());
  WriteUids(query, "characterPurchase", "characterUid", uid,
    "storageItemUid", "storageItem", character.purchases());
  WriteUids(query, "characterInventory", "characterUid", uid,
    "itemUid", "item", character.inventory(), Ownership::Permanent);
  WriteUids(query, "characterEquipment", "characterUid", uid,
    "itemUid", "item", character.characterEquipment(), Ownership::Permanent);
  WriteUids(query, "characterHorse", "characterUid", uid,
    "horseUid", "horse", character.horses(), Ownership::Permanent);
  WriteUids(query, "characterBreedingWishlist", "characterUid", uid,
    "horseUid", "horse", character.breedingWishlist());
  WriteUids(query, "characterPet", "characterUid", uid,
    "petUid", "pet", character.pets());
  WriteUids(query, "characterEgg", "characterUid", uid,
    "eggUid", "egg", character.eggs());
  WriteUids(query, "characterHousing", "characterUid", uid,
    "housingUid", "housing", character.housing());
  WriteUidList(query, "characterMailboxInbox", "characterUid", uid,
    "mailUid", "mail", character.mailbox.inbox());
  WriteUidList(query, "characterMailboxSent", "characterUid", uid,
    "mailUid", "mail", character.mailbox.sent());
  WriteUids(query, "characterQuest", "characterUid", uid,
    "questUid", "quest", character.quests());

  // Deleting the groups cascades to their members.
  DeleteRecord(query, "characterContactGroup", "characterUid", uid);
  for (const auto& [groupUid, group] : character.contacts.groups())
  {
    query.exec(
      "insert into \"data\".\"characterContactGroup\" "
      "(\"characterUid\", \"groupUid\", \"name\", \"createdAt\") "
      "values ($1, $2, $3, $4)",
      pqxx::params(
        uid,
        groupUid,
        group.name,
        WriteField(group.createdAt)));

    // Members which do not exist are skipped.
    for (const auto member : group.members)
    {
      query.exec(
        "insert into \"data\".\"characterContactGroupMember\" "
        "(\"characterUid\", \"groupUid\", \"memberUid\") "
        "select $1::\"data\".\"uint32\", $2::\"data\".\"uint32\", \"uid\" "
        "from \"data\".\"character\" "
        "where \"uid\" = $3",
        pqxx::params(
          uid,
          groupUid,
          member));
    }
  }

  DeleteRecord(query, "characterSkillSet", "characterUid", uid);
  WriteCharacterSkillSet(query, uid, "speed", character.skills.speed());
  WriteCharacterSkillSet(query, uid, "magic", character.skills.magic());

  // Deleting the missions cascades to their progress.
  DeleteRecord(query, "characterMission", "characterUid", uid);
  for (const auto& [key, mission] : character.missions())
  {
    query.exec(
      "insert into \"data\".\"characterMission\" "
      "(\"characterUid\", \"missionKey\", \"id\") "
      "values ($1, $2, $3)",
      pqxx::params(
        uid,
        key,
        mission.id));

    for (uint32_t position = 0; position < mission.progress.size(); ++position)
    {
      const auto& progress = mission.progress[position];
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
    }
  }
}

//! Writes the character, except for its name, and its relations.
void WriteCharacter(
  pqxx::transaction_base& query,
  const data::Uid uid,
  const data::Character& character)
{
  // The mount is claimed first, only a mount owned by the character is stored.
  ClaimOwnership(query, "horse", uid, character.mountUid());
  // The name is not stored, it is assigned only by `ClaimCharacterName`
  // so that a name conflict can never fail the store of the whole character.
  UpdateRecord(query, "character", CharacterColumns, "uid", uid, CharacterParams(character));
  WriteCharacterRelations(query, uid, character);
}

void ReadCharacterRelations(
  pqxx::transaction_base& query,
  const data::Uid uid,
  data::Character& character)
{
  character.contacts.pending = ReadUids<std::set<data::Uid>>(
    query, "characterContactPending", "characterUid", uid, "contactUid");
  character.gifts = ReadUids<std::vector<data::Uid>>(
    query, "characterGift", "characterUid", uid, "storageItemUid");
  character.purchases = ReadUids<std::vector<data::Uid>>(
    query, "characterPurchase", "characterUid", uid, "storageItemUid");
  character.inventory = ReadUids<std::vector<data::Uid>>(
    query, "characterInventory", "characterUid", uid, "itemUid");
  character.characterEquipment = ReadUids<std::vector<data::Uid>>(
    query, "characterEquipment", "characterUid", uid, "itemUid");
  character.horses = ReadUids<std::vector<data::Uid>>(
    query, "characterHorse", "characterUid", uid, "horseUid");
  character.breedingWishlist = ReadUids<std::set<data::Uid>>(
    query, "characterBreedingWishlist", "characterUid", uid, "horseUid");
  character.pets = ReadUids<std::vector<data::Uid>>(
    query, "characterPet", "characterUid", uid, "petUid");
  character.eggs = ReadUids<std::vector<data::Uid>>(
    query, "characterEgg", "characterUid", uid, "eggUid");
  character.housing = ReadUids<std::vector<data::Uid>>(
    query, "characterHousing", "characterUid", uid, "housingUid");
  character.mailbox.inbox = ReadUidList(
    query, "characterMailboxInbox", "characterUid", uid, "mailUid");
  character.mailbox.sent = ReadUidList(
    query, "characterMailboxSent", "characterUid", uid, "mailUid");
  character.quests = ReadUids<std::vector<data::Uid>>(
    query, "characterQuest", "characterUid", uid, "questUid");

  std::map<data::Uid, data::Character::Contacts::Group> groups;
  const auto groupResult = query.exec(
    "select \"groupUid\", \"name\", \"createdAt\" "
    "from \"data\".\"characterContactGroup\" "
    "where \"characterUid\" = $1",
    pqxx::params(uid));
  for (const auto& row : groupResult)
  {
    data::Character::Contacts::Group group;
    group.uid = FieldOf(row, "groupUid").as<data::Uid>();
    group.name = FieldOf(row, "name").as<std::string>();
    group.createdAt = ToTimePoint(FieldOf(row, "createdAt").as<int64_t>());
    groups.emplace(group.uid, std::move(group));
  }

  const auto memberResult = query.exec(
    "select \"groupUid\", \"memberUid\" "
    "from \"data\".\"characterContactGroupMember\" "
    "where \"characterUid\" = $1",
    pqxx::params(uid));
  for (const auto& row : memberResult)
  {
    groups.at(FieldOf(row, "groupUid").as<data::Uid>())
      .members.insert(FieldOf(row, "memberUid").as<data::Uid>());
  }
  character.contacts.groups = std::move(groups);

  data::Character::Skills::Sets speedSkills;
  data::Character::Skills::Sets magicSkills;
  const auto skillSetResult = query.exec(
    "select \"mode\", \"activeSetId\", \"set1Slot1\", \"set1Slot2\", \"set2Slot1\", \"set2Slot2\" "
    "from \"data\".\"characterSkillSet\" "
    "where \"characterUid\" = $1",
    pqxx::params(uid));
  for (const auto& row : skillSetResult)
  {
    data::Character::Skills::Sets skills;
    skills.activeSetId = FieldOf(row, "activeSetId").as<uint32_t>();
    skills.set1.slot1 = FieldOf(row, "set1Slot1").as<uint32_t>();
    skills.set1.slot2 = FieldOf(row, "set1Slot2").as<uint32_t>();
    skills.set2.slot1 = FieldOf(row, "set2Slot1").as<uint32_t>();
    skills.set2.slot2 = FieldOf(row, "set2Slot2").as<uint32_t>();

    const auto mode = FieldOf(row, "mode").as<std::string>();
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
    "select \"missionKey\", \"id\" "
    "from \"data\".\"characterMission\" "
    "where \"characterUid\" = $1",
    pqxx::params(uid));
  for (const auto& row : missionResult)
  {
    data::Character::Mission mission;
    mission.id = FieldOf(row, "id").as<uint32_t>();
    missions.emplace(FieldOf(row, "missionKey").as<uint32_t>(), std::move(mission));
  }

  const auto progressResult = query.exec(
    "select \"missionKey\", \"id\", \"value\" "
    "from \"data\".\"characterMissionProgress\" "
    "where \"characterUid\" = $1 "
    "order by \"missionKey\", \"position\"",
    pqxx::params(uid));
  for (const auto& row : progressResult)
  {
    missions.at(FieldOf(row, "missionKey").as<uint32_t>())
      .progress.push_back({
        .id = FieldOf(row, "id").as<uint32_t>(),
        .value = FieldOf(row, "value").as<uint32_t>()});
  }
  character.missions = std::move(missions);
}

void WriteStorageItemContents(
  pqxx::transaction_base& query,
  const data::Uid uid,
  const std::vector<data::StorageItem::Item>& items)
{
  DeleteRecord(query, "storageItemContent", "storageItemUid", uid);
  for (uint32_t position = 0; position < items.size(); ++position)
  {
    const auto& item = items[position];
    query.exec(
      "insert into \"data\".\"storageItemContent\" "
      "(\"storageItemUid\", \"position\", \"tid\", \"count\", \"duration\") "
      "values ($1, $2, $3, $4, $5)",
      pqxx::params(
        uid,
        position,
        item.tid,
        item.count,
        WriteField(item.duration)));
  }
}

std::vector<data::StorageItem::Item> ReadStorageItemContents(
  pqxx::transaction_base& query,
  const data::Uid uid)
{
  const auto result = query.exec(
    "select \"position\", \"tid\", \"count\", \"duration\" "
    "from \"data\".\"storageItemContent\" "
    "where \"storageItemUid\" = $1 "
    "order by \"position\"",
    pqxx::params(uid));

  std::vector<data::StorageItem::Item> items;
  items.reserve(result.size());
  for (const auto& row : result)
  {
    VerifyPosition(row, items.size());
    items.push_back({
      .tid = FieldOf(row, "tid").as<data::Tid>(),
      .count = FieldOf(row, "count").as<uint32_t>(),
      .duration = std::chrono::seconds(FieldOf(row, "duration").as<int64_t>())});
  }
  return items;
}

//! Writes the keyboard or gamepad bindings to their table.
void WriteSettingsBindings(
  pqxx::transaction_base& query,
  const std::string_view table,
  const data::Uid uid,
  const std::optional<std::vector<data::Settings::Option>>& bindings)
{
  DeleteRecord(query, table, "settingsUid", uid);
  if (not bindings)
  {
    return;
  }

  const auto insertSql = std::format(
    "insert into {} "
    "(\"settingsUid\", \"position\", \"primaryKey\", \"type\", \"secondaryKey\") "
    "values ($1, $2, $3, $4, $5)",
    QualifiedTable(query, table));
  for (uint32_t position = 0; position < bindings->size(); ++position)
  {
    const auto& binding = bindings->at(position);
    query.exec(
      insertSql,
      pqxx::params(
        uid,
        position,
        binding.primaryKey,
        binding.type,
        binding.secondaryKey));
  }
}

//! Reads the keyboard or gamepad bindings from their table.
std::optional<std::vector<data::Settings::Option>> ReadSettingsBindings(
  pqxx::transaction_base& query,
  const std::string_view table,
  const data::Uid uid,
  const bool present)
{
  if (not present)
  {
    return std::nullopt;
  }

  const auto result = query.exec(
    std::format(
      "select \"position\", \"primaryKey\", \"type\", \"secondaryKey\" "
      "from {} "
      "where \"settingsUid\" = $1 "
      "order by \"position\"",
      QualifiedTable(query, table)),
    pqxx::params(uid));

  std::vector<data::Settings::Option> bindings;
  bindings.reserve(result.size());
  for (const auto& row : result)
  {
    VerifyPosition(row, bindings.size());
    bindings.push_back({
      .primaryKey = FieldOf(row, "primaryKey").as<uint32_t>(),
      .type = FieldOf(row, "type").as<uint32_t>(),
      .secondaryKey = FieldOf(row, "secondaryKey").as<uint32_t>()});
  }
  return bindings;
}

void WriteSettingsMacros(
  pqxx::transaction_base& query,
  const data::Uid uid,
  const std::optional<std::array<std::string, 8>>& macros)
{
  DeleteRecord(query, "settingsMacro", "settingsUid", uid);
  if (not macros)
  {
    return;
  }

  for (uint32_t position = 0; position < macros->size(); ++position)
  {
    query.exec(
      "insert into \"data\".\"settingsMacro\" "
      "(\"settingsUid\", \"position\", \"value\") "
      "values ($1, $2, $3)",
      pqxx::params(
        uid,
        position,
        macros->at(position)));
  }
}

std::optional<std::array<std::string, 8>> ReadSettingsMacros(
  pqxx::transaction_base& query,
  const data::Uid uid,
  const bool present)
{
  if (not present)
  {
    return std::nullopt;
  }

  const auto result = query.exec(
    "select \"position\", \"value\" "
    "from \"data\".\"settingsMacro\" "
    "where \"settingsUid\" = $1 "
    "order by \"position\"",
    pqxx::params(uid));

  std::array<std::string, 8> macros{};
  if (result.size() != macros.size())
  {
    throw std::runtime_error("Settings macros have an invalid entry count");
  }

  std::size_t position = 0;
  for (const auto& row : result)
  {
    VerifyPosition(row, position);
    macros[position] = FieldOf(row, "value").as<std::string>();
    ++position;
  }
  return macros;
}

void WriteDailyQuestEntries(
  pqxx::transaction_base& query,
  const data::Uid uid,
  const std::array<data::DailyQuestEntry, 3>& entries)
{
  DeleteRecord(query, "dailyQuestEntry", "dailyQuestGroupUid", uid);
  for (uint32_t position = 0; position < entries.size(); ++position)
  {
    const auto& entry = entries[position];
    query.exec(
      "insert into \"data\".\"dailyQuestEntry\" "
      "(\"dailyQuestGroupUid\", \"position\", \"questId\", \"progress\") "
      "values ($1, $2, $3, $4)",
      pqxx::params(
        uid,
        position,
        entry.questId,
        entry.progress));
  }
}

std::array<data::DailyQuestEntry, 3> ReadDailyQuestEntries(
  pqxx::transaction_base& query,
  const data::Uid uid)
{
  const auto result = query.exec(
    "select \"position\", \"questId\", \"progress\" "
    "from \"data\".\"dailyQuestEntry\" "
    "where \"dailyQuestGroupUid\" = $1 "
    "order by \"position\"",
    pqxx::params(uid));

  std::array<data::DailyQuestEntry, 3> entries{};
  if (result.size() != entries.size())
  {
    throw std::runtime_error("Daily quest group has an invalid entry count");
  }

  std::size_t position = 0;
  for (const auto& row : result)
  {
    VerifyPosition(row, position);
    entries[position] = {
      .questId = FieldOf(row, "questId").as<uint32_t>(),
      .progress = FieldOf(row, "progress").as<uint32_t>()};
    ++position;
  }
  return entries;
}

} // namespace

PqDataSource::ConnectionLease::ConnectionLease(
  PqDataSource& dataSource,
  std::unique_ptr<pqxx::connection> connection)
  : _dataSource(dataSource)
  , _connection(std::move(connection))
{
}

PqDataSource::ConnectionLease::~ConnectionLease()
{
  _dataSource.ReleaseConnection(std::move(_connection));
}

pqxx::connection& PqDataSource::ConnectionLease::operator*() const
{
  return *_connection;
}

void PqDataSource::Establish(
  const std::string& url,
  const std::size_t connectionCount)
{
  if (connectionCount == 0)
  {
    throw std::invalid_argument("Connection pool must have at least one connection");
  }

  std::vector<std::unique_ptr<pqxx::connection>> connections;
  connections.reserve(connectionCount);
  for (std::size_t index = 0; index < connectionCount; ++index)
  {
    connections.emplace_back(std::make_unique<pqxx::connection>(url));
  }

  std::scoped_lock lock(_poolMutex);
  _url = url;
  _idleConnections = std::move(connections);
}

bool PqDataSource::IsConnectionFine()
{
  try
  {
    const auto connection = AcquireConnection();
    return (*connection).is_open();
  }
  catch (const std::exception&)
  {
    return false;
  }
}

PqDataSource::ConnectionLease PqDataSource::AcquireConnection()
{
  std::unique_ptr<pqxx::connection> connection;
  std::string url;
  {
    std::unique_lock lock(_poolMutex);
    if (_url.empty())
    {
      throw std::runtime_error("Connection to data source is not established");
    }

    _poolCondition.wait(lock, [this]()
    {
      return not _idleConnections.empty();
    });

    connection = std::move(_idleConnections.back());
    _idleConnections.pop_back();
    url = _url;
  }

  if (not connection or not connection->is_open())
  {
    try
    {
      connection = std::make_unique<pqxx::connection>(url);
    }
    catch (const std::exception&)
    {
      // Return the connection to the pool so that the pool does not shrink,
      // the next lease will attempt to re-establish it again.
      ReleaseConnection(std::move(connection));
      throw;
    }
  }

  return ConnectionLease(*this, std::move(connection));
}

void PqDataSource::ReleaseConnection(
  std::unique_ptr<pqxx::connection> connection)
{
  {
    std::scoped_lock lock(_poolMutex);
    _idleConnections.emplace_back(std::move(connection));
  }
  _poolCondition.notify_one();
}

PqDataSource::~PqDataSource() = default;

void PqDataSource::CreateUser(
  data::User& user)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);

  pqxx::params params(WriteField(user.name()));
  params.append(UserParams(user));
  query.exec(InsertSql(query, "user", PrependColumn({"name"}, UserColumns), "name"), params);
  WriteUids(query, "userInfraction", "userName", user.name(),
    "infractionUid", "infraction", user.infractions());
  query.commit();
}

std::optional<bool> PqDataSource::RetrieveUser(
  const std::string_view& name,
  data::User& user)
{
  try
  {
    const auto connection = AcquireConnection();
    SnapshotTransaction query(*connection);

    const auto result = query.exec(
      "select * "
      "from \"data\".\"user\" "
      "where \"name\" = $1",
      pqxx::params(name));
    if (result.empty())
    {
      return false;
    }

    const auto row = result.one_row();
    ReadField(row, "name", user.name);
    ReadField(row, "token", user.token);
    ReadField(row, "characterUid", user.characterUid);
    ReadField(row, "lastSeenOnline", user.lastSeenOnline);
    user.infractions = ReadUids<std::vector<data::Uid>>(query, "userInfraction", "userName", name, "infractionUid");
  }
  catch (const std::exception& x)
  {
    spdlog::error("Exception retrieving user '{}' from the postgres data source: {}", name, x.what());
    return std::nullopt;
  }

  return true;
}

void PqDataSource::StoreUser(
  const std::string_view& name,
  const data::User& user)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "user", UserColumns, "name", name, UserParams(user));
  WriteUids(query, "userInfraction", "userName", name,
    "infractionUid", "infraction", user.infractions());
  query.commit();
}

bool PqDataSource::IsUserNameUnique(
  const std::string_view& name)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto result = query.exec(
    "select 1 "
    "from \"data\".\"user\" "
    "where \"name\" = $1",
    pqxx::params(name));
  return result.empty();
}

void PqDataSource::CreateInfraction(
  data::Infraction& infraction)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "infraction", InfractionColumns, "uid", InfractionParams(infraction));
  query.commit();
  infraction.uid = uid;
}

void PqDataSource::RetrieveInfraction(
  const data::Uid uid,
  data::Infraction& infraction)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "infraction", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "infraction", InfractionColumns, "uid", uid, InfractionParams(infraction));
  query.commit();
}

void PqDataSource::DeleteInfraction(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "infraction", "uid", uid);
  query.commit();
}

void PqDataSource::CreateCharacter(
  data::Character& character)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);

  // The character is inserted first, the store requires its UID to claim the ownership of its records.
  const auto uid = InsertRecord(
    query, "character", std::to_array<Column>({{"name"}}), "uid", pqxx::params(WriteField(character.name())));
  WriteCharacter(query, uid, character);
  query.commit();
  character.uid = uid;
}

void PqDataSource::RetrieveCharacter(
  const data::Uid uid,
  data::Character& character)
{
  const auto connection = AcquireConnection();
  SnapshotTransaction query(*connection);

  const auto row = SelectRecord(query, "character", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  WriteCharacter(query, uid, character);
  query.commit();
}

void PqDataSource::DeleteCharacter(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "character", "uid", uid);
  query.commit();
}

data::Uid PqDataSource::RetrieveCharacterUidByName(
  const std::string_view& name)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  // The empty name condition allows the use of the partial unique index.
  const auto result = query.exec(
    "select \"uid\" "
    "from \"data\".\"character\" "
    "where \"name\" <> '' and lower(\"name\") = lower($1)",
    pqxx::params(name));
  return result.empty() ? data::InvalidUid : FieldOf(result.front(), "uid").as<data::Uid>();
}

bool PqDataSource::IsCharacterNameUnique(
  const std::string_view& name)
{
  return RetrieveCharacterUidByName(name) == data::InvalidUid;
}

bool PqDataSource::ClaimCharacterName(
  const data::Uid uid,
  const std::string_view& name)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  try
  {
    // The unique index on the character names makes the claim atomic.
    const auto result = query.exec(
      "update \"data\".\"character\" "
      "set \"name\" = $2 "
      "where \"uid\" = $1",
      pqxx::params(uid, name));
    if (result.affected_rows() == 0)
    {
      throw std::runtime_error("Character does not exist");
    }
    query.commit();
  }
  catch (const pqxx::unique_violation&)
  {
    return false;
  }

  return true;
}

void PqDataSource::CreateHorse(
  data::Horse& horse)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "horse", HorseColumns, "uid", HorseParams(horse));
  query.commit();
  horse.uid = uid;
}

void PqDataSource::RetrieveHorse(
  const data::Uid uid,
  data::Horse& horse)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);

  const auto row = SelectRecord(query, "horse", "uid", uid);
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
  horse.ancestors.father = FieldOf(row, "ancestorsFather").as<data::Uid>(data::InvalidUid);
  horse.ancestors.mother = FieldOf(row, "ancestorsMother").as<data::Uid>(data::InvalidUid);
  ReadField(row, "lineage", horse.lineage);
}

void PqDataSource::StoreHorse(
  const data::Uid uid,
  const data::Horse& horse)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "horse", HorseColumns, "uid", uid, HorseParams(horse));
  query.commit();
}

void PqDataSource::DeleteHorse(
  const data::Uid uid)
{
  // Horses are never deleted, the data source rejects the deletion.
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "horse", "uid", uid);
  query.commit();
}

void PqDataSource::CreateItem(
  data::Item& item)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "item", ItemColumns, "uid", ItemParams(item));
  query.commit();
  item.uid = uid;
}

void PqDataSource::RetrieveItem(
  const data::Uid uid,
  data::Item& item)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "item", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "item", ItemColumns, "uid", uid, ItemParams(item));
  query.commit();
}

void PqDataSource::DeleteItem(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "item", "uid", uid);
  query.commit();
}

void PqDataSource::CreateStorageItem(
  data::StorageItem& storageItem)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(
    query, "storageItem", StorageItemColumns, "uid", StorageItemParams(storageItem));
  WriteStorageItemContents(query, uid, storageItem.items());
  query.commit();
  storageItem.uid = uid;
}

void PqDataSource::RetrieveStorageItem(
  const data::Uid uid,
  data::StorageItem& storageItem)
{
  const auto connection = AcquireConnection();
  SnapshotTransaction query(*connection);
  const auto row = SelectRecord(query, "storageItem", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "storageItem", StorageItemColumns, "uid", uid, StorageItemParams(storageItem));
  WriteStorageItemContents(query, uid, storageItem.items());
  query.commit();
}

void PqDataSource::DeleteStorageItem(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "storageItem", "uid", uid);
  query.commit();
}

void PqDataSource::CreateEgg(
  data::Egg& egg)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "egg", EggColumns, "uid", EggParams(egg));
  query.commit();
  egg.uid = uid;
}

void PqDataSource::RetrieveEgg(
  const data::Uid uid,
  data::Egg& egg)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "egg", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "egg", EggColumns, "uid", uid, EggParams(egg));
  query.commit();
}

void PqDataSource::DeleteEgg(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "egg", "uid", uid);
  query.commit();
}

void PqDataSource::CreatePet(
  data::Pet& pet)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "pet", PetColumns, "uid", PetParams(pet));
  query.commit();
  pet.uid = uid;
}

void PqDataSource::RetrievePet(
  const data::Uid uid,
  data::Pet& pet)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "pet", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "pet", PetColumns, "uid", uid, PetParams(pet));
  query.commit();
}

void PqDataSource::DeletePet(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "pet", "uid", uid);
  query.commit();
}

void PqDataSource::CreateHousing(
  data::Housing& housing)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "housing", HousingColumns, "uid", HousingParams(housing));
  query.commit();
  housing.uid = uid;
}

void PqDataSource::RetrieveHousing(
  const data::Uid uid,
  data::Housing& housing)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "housing", "uid", uid);
  ReadField(row, "uid", housing.uid);
  ReadField(row, "housingId", housing.housingId);
  ReadField(row, "expiresAt", housing.expiresAt);
  ReadField(row, "durability", housing.durability);
}

void PqDataSource::StoreHousing(
  const data::Uid uid,
  const data::Housing& housing)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "housing", HousingColumns, "uid", uid, HousingParams(housing));
  query.commit();
}

void PqDataSource::DeleteHousing(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "housing", "uid", uid);
  query.commit();
}

void PqDataSource::CreateGuild(
  data::Guild& guild)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "guild", GuildColumns, "uid", GuildParams(guild));
  WriteUids(query, "guildOfficer", "guildUid", uid,
    "characterUid", "character", guild.officers());
  WriteUids(query, "guildMember", "guildUid", uid,
    "characterUid", "character", guild.members());
  query.commit();
  guild.uid = uid;
}

void PqDataSource::RetrieveGuild(
  const data::Uid uid,
  data::Guild& guild)
{
  const auto connection = AcquireConnection();
  SnapshotTransaction query(*connection);
  const auto row = SelectRecord(query, "guild", "uid", uid);
  ReadField(row, "uid", guild.uid);
  ReadField(row, "name", guild.name);
  ReadField(row, "description", guild.description);
  ReadField(row, "owner", guild.owner);
  ReadField(row, "rank", guild.rank);
  ReadField(row, "totalWins", guild.totalWins);
  ReadField(row, "totalLosses", guild.totalLosses);
  ReadField(row, "seasonalWins", guild.seasonalWins);
  ReadField(row, "seasonalLosses", guild.seasonalLosses);
  guild.officers = ReadUids<std::vector<data::Uid>>(query, "guildOfficer", "guildUid", uid, "characterUid");
  guild.members = ReadUids<std::vector<data::Uid>>(query, "guildMember", "guildUid", uid, "characterUid");
}

void PqDataSource::StoreGuild(
  const data::Uid uid,
  const data::Guild& guild)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "guild", GuildColumns, "uid", uid, GuildParams(guild));
  WriteUids(query, "guildOfficer", "guildUid", uid,
    "characterUid", "character", guild.officers());
  WriteUids(query, "guildMember", "guildUid", uid,
    "characterUid", "character", guild.members());
  query.commit();
}

void PqDataSource::DeleteGuild(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "guild", "uid", uid);
  query.commit();
}

bool PqDataSource::IsGuildNameUnique(
  const std::string_view& name)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  // The empty name condition allows the use of the partial unique index.
  const auto result = query.exec(
    "select 1 "
    "from \"data\".\"guild\" "
    "where \"name\" <> '' and lower(\"name\") = lower($1)",
    pqxx::params(name));
  return result.empty();
}

void PqDataSource::CreateSettings(
  data::Settings& settings)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "settings", SettingsColumns, "uid", SettingsParams(settings));
  WriteSettingsBindings(query, "settingsKeyboardBinding", uid, settings.keyboardBindings());
  WriteSettingsMacros(query, uid, settings.macros());
  WriteSettingsBindings(query, "settingsGamepadBinding", uid, settings.gamepadBindings());
  query.commit();
  settings.uid = uid;
}

void PqDataSource::RetrieveSettings(
  const data::Uid uid,
  data::Settings& settings)
{
  const auto connection = AcquireConnection();
  SnapshotTransaction query(*connection);
  const auto row = SelectRecord(query, "settings", "uid", uid);
  ReadField(row, "uid", settings.uid);
  ReadField(row, "age", settings.age);
  ReadField(row, "hideAge", settings.hideAge);
  settings.keyboardBindings = ReadSettingsBindings(
    query, "settingsKeyboardBinding", uid, FieldOf(row, "hasKeyboardBindings").as<bool>());
  settings.macros = ReadSettingsMacros(
    query, uid, FieldOf(row, "hasMacros").as<bool>());
  settings.gamepadBindings = ReadSettingsBindings(
    query, "settingsGamepadBinding", uid, FieldOf(row, "hasGamepadBindings").as<bool>());
}

void PqDataSource::StoreSettings(
  const data::Uid uid,
  const data::Settings& settings)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "settings", SettingsColumns, "uid", uid, SettingsParams(settings));
  WriteSettingsBindings(query, "settingsKeyboardBinding", uid, settings.keyboardBindings());
  WriteSettingsMacros(query, uid, settings.macros());
  WriteSettingsBindings(query, "settingsGamepadBinding", uid, settings.gamepadBindings());
  query.commit();
}

void PqDataSource::DeleteSettings(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "settings", "uid", uid);
  query.commit();
}

void PqDataSource::CreateDailyQuestGroup(
  data::DailyQuestGroup& group)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(
    query, "dailyQuestGroup", DailyQuestGroupColumns, "uid", DailyQuestGroupParams(group));
  WriteDailyQuestEntries(query, uid, group.quests());
  query.commit();
  group.uid = uid;
}

void PqDataSource::RetrieveDailyQuestGroup(
  const data::Uid uid,
  data::DailyQuestGroup& group)
{
  const auto connection = AcquireConnection();
  SnapshotTransaction query(*connection);
  const auto row = SelectRecord(query, "dailyQuestGroup", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(
    query, "dailyQuestGroup", DailyQuestGroupColumns, "uid", uid, DailyQuestGroupParams(group));
  WriteDailyQuestEntries(query, uid, group.quests());
  query.commit();
}

void PqDataSource::DeleteDailyQuestGroup(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "dailyQuestGroup", "uid", uid);
  query.commit();
}

void PqDataSource::CreateMail(
  data::Mail& mail)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "mail", MailColumns, "uid", MailParams(mail));
  query.commit();
  mail.uid = uid;
}

void PqDataSource::RetrieveMail(
  const data::Uid uid,
  data::Mail& mail)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "mail", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "mail", MailColumns, "uid", uid, MailParams(mail));
  query.commit();
}

void PqDataSource::DeleteMail(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "mail", "uid", uid);
  query.commit();
}

void PqDataSource::CreateQuest(
  data::Quest& quest)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "quest", QuestColumns, "uid", QuestParams(quest));
  query.commit();
  quest.uid = uid;
}

void PqDataSource::RetrieveQuest(
  const data::Uid uid,
  data::Quest& quest)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "quest", "uid", uid);
  ReadField(row, "uid", quest.uid);
  ReadField(row, "questId", quest.questId);
  ReadField(row, "isCompleted", quest.isCompleted);
  ReadField(row, "progress", quest.progress);
}

void PqDataSource::StoreQuest(
  const data::Uid uid,
  const data::Quest& quest)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "quest", QuestColumns, "uid", uid, QuestParams(quest));
  query.commit();
}

void PqDataSource::DeleteQuest(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "quest", "uid", uid);
  query.commit();
}

void PqDataSource::CreateStallion(
  data::Stallion& stallion)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto uid = InsertRecord(query, "stallion", StallionColumns, "uid", StallionParams(stallion));
  query.commit();
  stallion.uid = uid;
}

void PqDataSource::RetrieveStallion(
  const data::Uid uid,
  data::Stallion& stallion)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "stallion", "uid", uid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "stallion", StallionColumns, "uid", uid, StallionParams(stallion));
  query.commit();
}

void PqDataSource::DeleteStallion(
  const data::Uid uid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "stallion", "uid", uid);
  query.commit();
}

std::vector<data::Uid> PqDataSource::ListRegisteredStallions()
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto result = query.exec(
    "select \"uid\" "
    "from \"data\".\"stallion\" "
    "order by \"uid\"");

  std::vector<data::Uid> uids;
  uids.reserve(result.size());
  for (const auto& row : result)
  {
    uids.emplace_back(FieldOf(row, "uid").as<data::Uid>());
  }
  return uids;
}

void PqDataSource::CreateReward(
  data::Reward& reward)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  const auto claimUid = InsertRecord(query, "reward", RewardColumns, "claimUid", RewardParams(reward));
  query.commit();
  reward.claimUid = claimUid;
}

void PqDataSource::RetrieveReward(
  const data::Uid claimUid,
  data::Reward& reward)
{
  const auto connection = AcquireConnection();
  pqxx::nontransaction query(*connection);
  const auto row = SelectRecord(query, "reward", "claimUid", claimUid);
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
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  UpdateRecord(query, "reward", RewardColumns, "claimUid", claimUid, RewardParams(reward));
  query.commit();
}

void PqDataSource::DeleteReward(
  const data::Uid claimUid)
{
  const auto connection = AcquireConnection();
  pqxx::work query(*connection);
  DeleteRecord(query, "reward", "claimUid", claimUid);
  query.commit();
}

} // namespace server
