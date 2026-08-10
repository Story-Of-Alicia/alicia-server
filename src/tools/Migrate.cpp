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

//! Copies a file data source into a postgres data source.
//!
//! Every record is read through FileDataSource and written through
//! PqDataSource, so both sides go through exactly the code paths the server
//! uses rather than a separate reimplementation of the layout.
//!
//! The server must not be running against the target database while this runs.
//! DataStorage keeps every loaded record in memory and writes them all out
//! again during DataDirector::Terminate, so a server which was started before
//! the migration will flush its stale records over the migrated ones as soon as
//! it shuts down. The symptom is a record whose UID is beyond its own sequence.
//!
//! Usage:
//!   alicia-migrate --data <path> --uri <connection uri> [--wipe] [--dry-run]
//!
//!   --wipe     Empties every table of the data schema first. Without it, a
//!              record whose UID or name already exists makes that record fail.
//!   --dry-run  Reads and reports, without writing anything.

#include "libserver/data/file/FileDataSource.hpp"
#include "libserver/data/pq/PqDataSource.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

namespace
{

using namespace server;

struct Options
{
  std::filesystem::path dataPath;
  std::string uri;
  bool wipe{false};
  bool dryRun{false};
};

struct Result
{
  size_t migrated{0};
  size_t failed{0};
};

//! The root tables of the data schema. Every child table cascades from one of
//! these, so truncating these empties the whole schema.
constexpr std::string_view RootTables =
  "data.\"user\", data.character, data.horse, data.item, data.pet, data.egg, "
  "data.housing, data.guild, data.settings, data.mail, data.quest, "
  "data.stallion, data.reward, data.storage_item, data.daily_quest_group, "
  "data.infraction";

//! The sequences of the data schema, paired with the counter of the meta file
//! they continue from.
constexpr std::pair<std::string_view, std::string_view> Sequences[]{
  {"data.infraction_uid_seq", "infractionSequentialUid"},
  {"data.character_uid_seq", "characterSequentialUid"},
  {"data.equipment_uid_seq", "equipmentSequentialUid"},
  {"data.storage_item_uid_seq", "storageItemSequentialUid"},
  {"data.egg_uid_seq", "eggSequentialUid"},
  {"data.pet_uid_seq", "petSequentialUid"},
  {"data.housing_uid_seq", "housingSequentialUid"},
  {"data.guild_uid_seq", "guildSequentialId"},
  {"data.settings_uid_seq", "settingsSequentialId"},
  {"data.daily_quest_group_uid_seq", "dailyQuestGroupSequentialId"},
  {"data.mail_uid_seq", "mailSequentialId"},
  {"data.quest_uid_seq", "questSequentialId"},
  {"data.stallion_uid_seq", "stallionSequentialUid"},
  {"data.reward_uid_seq", "rewardSequentialUid"}};

bool ParseOptions(const int argc, char** argv, Options& options)
{
  for (int index = 1; index < argc; ++index)
  {
    const std::string argument = argv[index];

    if (argument == "--wipe")
      options.wipe = true;
    else if (argument == "--dry-run")
      options.dryRun = true;
    else if (argument == "--data" && index + 1 < argc)
      options.dataPath = argv[++index];
    else if (argument == "--uri" && index + 1 < argc)
      options.uri = argv[++index];
    else
    {
      std::cerr << "Unrecognised argument: " << argument << "\n";
      return false;
    }
  }

  if (options.dataPath.empty() || options.uri.empty())
  {
    std::cerr
      << "Usage: alicia-migrate --data <path> --uri <connection uri> "
         "[--wipe] [--dry-run]\n";
    return false;
  }

  return true;
}

//! Migrates every record of a directory whose file names are UIDs.
template <typename Data>
Result MigrateByUid(
  const std::string_view label,
  const std::filesystem::path& directory,
  const std::function<void(data::Uid, Data&)>& retrieve,
  const std::function<void(data::Uid, const Data&)>& store,
  const bool dryRun)
{
  Result result;

  if (not exists(directory))
  {
    std::cout << std::format("  {:<20} directory absent\n", label);
    return result;
  }

  for (const auto& entry : std::filesystem::directory_iterator(directory))
  {
    // The character directory holds an equipment sub-directory next to the
    // record files, so anything which is not a json file is skipped.
    if (not entry.is_regular_file() || entry.path().extension() != ".json")
      continue;

    const auto stem = entry.path().stem().string();

    data::Uid uid{};
    try
    {
      uid = static_cast<data::Uid>(std::stoul(stem));
    }
    catch (const std::exception&)
    {
      std::cerr << std::format(
        "  {}: file name '{}' is not a UID, skipped\n", label, stem);
      ++result.failed;
      continue;
    }

    try
    {
      Data record;
      retrieve(uid, record);

      if (not dryRun)
        store(uid, record);

      ++result.migrated;
    }
    catch (const std::exception& x)
    {
      std::cerr << std::format("  {} {}: {}\n", label, uid, x.what());
      ++result.failed;
    }
  }

  std::cout << std::format(
    "  {:<20} {} migrated, {} failed\n", label, result.migrated, result.failed);
  return result;
}

//! Migrates the users, whose file names are user names rather than UIDs.
Result MigrateUsers(
  const std::filesystem::path& directory,
  FileDataSource& source,
  PqDataSource& target,
  const bool dryRun)
{
  Result result;

  if (not exists(directory))
  {
    std::cout << "  users                directory absent\n";
    return result;
  }

  for (const auto& entry : std::filesystem::directory_iterator(directory))
  {
    if (not entry.is_regular_file() || entry.path().extension() != ".json")
      continue;

    const auto name = entry.path().stem().string();

    try
    {
      data::User user;
      source.RetrieveUser(name, user);

      if (not dryRun)
        target.StoreUser(name, user);

      ++result.migrated;
    }
    catch (const std::exception& x)
    {
      std::cerr << std::format("  user '{}': {}\n", name, x.what());
      ++result.failed;
    }
  }

  std::cout << std::format(
    "  {:<20} {} migrated, {} failed\n", "users", result.migrated, result.failed);
  return result;
}

//! Empties every table of the data schema and rewinds every sequence.
void Wipe(const std::string& uri)
{
  pqxx::connection connection(uri);
  pqxx::work transaction(connection);

  transaction.exec(std::format("TRUNCATE {} CASCADE", RootTables));

  for (const auto& [sequence, unused] : Sequences)
    transaction.exec(std::format("SELECT setval('{}', 1, false)", sequence));

  transaction.commit();
  std::cout << "Emptied the data schema and rewound the sequences\n";
}

//! Continues the sequences from the counters of the meta file, so that the
//! UIDs handed out from now on carry on where the file data source left off.
void RestoreSequences(
  const std::string& uri,
  const std::filesystem::path& dataPath)
{
  const auto metaPath = dataPath / "meta.json";

  std::ifstream metaFile(metaPath);
  if (not metaFile.is_open())
  {
    std::cerr << std::format(
      "No meta file at '{}', the sequences keep their current values\n",
      metaPath.string());
    return;
  }

  const auto meta = nlohmann::json::parse(metaFile);

  pqxx::connection connection(uri);
  pqxx::work transaction(connection);

  for (const auto& [sequence, counter] : Sequences)
  {
    const auto value = meta.value(std::string(counter), uint32_t{0});

    // A counter of zero means nothing was ever handed out, so the sequence has
    // to stay before its first value rather than skip it.
    if (value == 0)
      transaction.exec(std::format("SELECT setval('{}', 1, false)", sequence));
    else
      transaction.exec(std::format("SELECT setval('{}', {}, true)", sequence, value));
  }

  transaction.commit();
  std::cout << "Continued the sequences from the meta file\n";
}

} // anon namespace

int main(int argc, char** argv)
{
  spdlog::set_level(spdlog::level::warn);

  Options options;
  if (not ParseOptions(argc, argv, options))
    return 1;

  if (not exists(options.dataPath))
  {
    std::cerr << std::format(
      "The data path '{}' does not exist\n", options.dataPath.string());
    return 1;
  }

  try
  {
    if (options.wipe)
    {
      if (options.dryRun)
        std::cout << "Would empty the data schema\n";
      else
        Wipe(options.uri);
    }

    FileDataSource source;
    source.Initialize(options.dataPath);

    PqDataSource target;
    target.Initialize(options.uri);

    std::cout << std::format(
      "Migrating '{}'{}\n",
      options.dataPath.string(),
      options.dryRun ? " (dry run, nothing is written)" : "");

    Result total;
    const auto accumulate = [&total](const Result& result)
    {
      total.migrated += result.migrated;
      total.failed += result.failed;
    };

    accumulate(MigrateUsers(
      options.dataPath / "users", source, target, options.dryRun));

    accumulate(MigrateByUid<data::Infraction>(
      "infractions", options.dataPath / "infractions",
      [&](auto uid, auto& record) { source.RetrieveInfraction(uid, record); },
      [&](auto uid, const auto& record) { target.StoreInfraction(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Character>(
      "characters", options.dataPath / "characters",
      [&](auto uid, auto& record) { source.RetrieveCharacter(uid, record); },
      [&](auto uid, const auto& record) { target.StoreCharacter(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Item>(
      "items", options.dataPath / "characters/equipment/items",
      [&](auto uid, auto& record) { source.RetrieveItem(uid, record); },
      [&](auto uid, const auto& record) { target.StoreItem(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Horse>(
      "horses", options.dataPath / "characters/equipment/horses",
      [&](auto uid, auto& record) { source.RetrieveHorse(uid, record); },
      [&](auto uid, const auto& record) { target.StoreHorse(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::StorageItem>(
      "storage items", options.dataPath / "storage",
      [&](auto uid, auto& record) { source.RetrieveStorageItem(uid, record); },
      [&](auto uid, const auto& record) { target.StoreStorageItem(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Egg>(
      "eggs", options.dataPath / "eggs",
      [&](auto uid, auto& record) { source.RetrieveEgg(uid, record); },
      [&](auto uid, const auto& record) { target.StoreEgg(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Pet>(
      "pets", options.dataPath / "pets",
      [&](auto uid, auto& record) { source.RetrievePet(uid, record); },
      [&](auto uid, const auto& record) { target.StorePet(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Housing>(
      "housing", options.dataPath / "housing",
      [&](auto uid, auto& record) { source.RetrieveHousing(uid, record); },
      [&](auto uid, const auto& record) { target.StoreHousing(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Guild>(
      "guilds", options.dataPath / "guilds",
      [&](auto uid, auto& record) { source.RetrieveGuild(uid, record); },
      [&](auto uid, const auto& record) { target.StoreGuild(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Settings>(
      "settings", options.dataPath / "settings",
      [&](auto uid, auto& record) { source.RetrieveSettings(uid, record); },
      [&](auto uid, const auto& record) { target.StoreSettings(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::DailyQuestGroup>(
      "daily quest groups", options.dataPath / "dailyQuestGroups",
      [&](auto uid, auto& record) { source.RetrieveDailyQuestGroup(uid, record); },
      [&](auto uid, const auto& record) { target.StoreDailyQuestGroup(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Mail>(
      "mails", options.dataPath / "mails",
      [&](auto uid, auto& record) { source.RetrieveMail(uid, record); },
      [&](auto uid, const auto& record) { target.StoreMail(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Quest>(
      "quests", options.dataPath / "quests",
      [&](auto uid, auto& record) { source.RetrieveQuest(uid, record); },
      [&](auto uid, const auto& record) { target.StoreQuest(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Stallion>(
      "stallions", options.dataPath / "stallions",
      [&](auto uid, auto& record) { source.RetrieveStallion(uid, record); },
      [&](auto uid, const auto& record) { target.StoreStallion(uid, record); },
      options.dryRun));

    accumulate(MigrateByUid<data::Reward>(
      "rewards", options.dataPath / "rewards",
      [&](auto uid, auto& record) { source.RetrieveReward(uid, record); },
      [&](auto uid, const auto& record) { target.StoreReward(uid, record); },
      options.dryRun));

    if (not options.dryRun)
      RestoreSequences(options.uri, options.dataPath);

    target.Terminate();

    std::cout << std::format(
      "\n{} record(s) migrated, {} failed\n", total.migrated, total.failed);

    return total.failed == 0 ? 0 : 1;
  }
  catch (const std::exception& x)
  {
    std::cerr << "Migration failed: " << x.what() << "\n";
    return 1;
  }
}
