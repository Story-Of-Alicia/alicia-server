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

#include "libserver/data/file/FileDataSource.hpp"

#include <algorithm>
#include <format>
#include <fstream>

#include <format>

#include <nlohmann/json.hpp>

namespace
{

std::filesystem::path ProduceDataPath(
  const std::filesystem::path& root,
  const std::string& filename)
{
  if (not std::filesystem::exists(root))
    std::filesystem::create_directories(root);
  return root / (filename + ".json");
}

} // namespace

void server::FileDataSource::Initialize(const std::filesystem::path& path)
{
  _dataPath = path;
  _metaFilePath = _dataPath;

  const auto prepareDataPath = [this](
                                 std::filesystem::path folder)
  {
    const auto path = _dataPath / folder;
    create_directories(path);

    return path;
  };

  _userDataPath = prepareDataPath("users");
  _characterDataPath = prepareDataPath("characters");
  _itemDataPath = prepareDataPath("items");
  _petDataPath = prepareDataPath("pets");
  _guildDataPath = prepareDataPath("guilds");
  _storedItemPath = prepareDataPath("storedItems");
  _horseDataPath = prepareDataPath("horses");
  _ranchDataPath = prepareDataPath("ranches");

  const std::filesystem::path metaFilePath = ProduceDataPath(
    _metaFilePath, "meta");
  std::ifstream metaFile(metaFilePath);
  if (not metaFile.is_open())
  {
    return;
  }

  const auto meta = nlohmann::json::parse(metaFile);
  _sequentialUid = meta["sequentialUid"].get<uint32_t>();
}

void server::FileDataSource::Terminate()
{
  const std::filesystem::path metaFilePath = ProduceDataPath(
    _metaFilePath, "meta");

  std::ofstream metaFile(metaFilePath);
  if (not metaFile.is_open())
  {
    return;
  }

  nlohmann::json meta;
  meta["sequentialUid"] = _sequentialUid.fetch_add(1, std::memory_order::relaxed);

  metaFile << meta.dump(2);
}

void server::FileDataSource::RetrieveUser(std::string name, data::User& user)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _userDataPath, name);

  user.name = name;

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("User file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  user.name = json["name"].get<std::string>();
  user.token = json["token"].get<std::string>();
  user.characterUid = json["characterUid"].get<data::Uid>();
}

void server::FileDataSource::StoreUser(std::string name, const data::User& user)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _userDataPath, name);

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("User file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["name"] = user.name();
  json["token"] = user.token();
  json["characterUid"] = user.characterUid();

  dataFile << json.dump(2);
}

void server::FileDataSource::CreateCharacter(data::Character& character)
{
  character.uid = _sequentialUid.fetch_add(1, std::memory_order::relaxed);
}

void server::FileDataSource::RetrieveCharacter(data::Uid uid, data::Character& character)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _characterDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Character file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  character.uid = json["uid"].get<data::Uid>();
  character.name = json["name"].get<std::string>();

  character.introduction = json["introduction"].get<std::string>();

  character.level = json["level"].get<uint32_t>();
  character.carrots = json["carrots"].get<int32_t>();
  character.cash = json["cash"].get<uint32_t>();

  character.role = static_cast<data::Character::Role>(
    json["role"].get<uint32_t>());

  auto parts = json["parts"];
  character.parts = data::Character::Parts{
    .modelId = parts["modelId"].get<data::Uid>(),
    .mouthId = parts["mouthId"].get<data::Uid>(),
    .faceId = parts["faceId"].get<data::Uid>()};

  auto appearance = json["appearance"];
  character.appearance = data::Character::Appearance{
    .headSize = appearance["headSize"].get<uint32_t>(),
    .height = appearance["height"].get<uint32_t>(),
    .thighVolume = appearance["thighVolume"].get<uint32_t>(),
    .legVolume = appearance["legVolume"].get<uint32_t>()};

  character.petUid = json["petUid"].get<data::Uid>();
  character.guildUid = json["guildUid"].get<data::Uid>();

  character.gifts = json["gifts"].get<std::vector<data::Uid>>();
  character.purchases = json["purchases"].get<std::vector<data::Uid>>();

  character.inventory = json["inventory"].get<std::vector<data::Uid>>();
  character.characterEquipment = json["characterEquipment"].get<std::vector<data::Uid>>();
  character.mountEquipment = json["horseEquipment"].get<std::vector<data::Uid>>();

  character.horses = json["horseUids"].get<std::vector<data::Uid>>();
  character.mountUid = json["mountUid"].get<data::Uid>();
  character.ranchUid = json["ranchUid"].get<data::Uid>();
}

void server::FileDataSource::StoreCharacter(data::Uid uid, const data::Character& character)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _characterDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Character file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = character.uid();
  json["name"] = character.name();

  json["introduction"] = character.introduction();

  json["level"] = character.level();
  json["carrots"] = character.carrots();
  json["cash"] = character.cash();

  json["role"] = character.role();

  // Character parts
  nlohmann::json parts;
  parts["modelId"] = character.parts.modelId();
  parts["mouthId"] = character.parts.mouthId();
  parts["faceId"] = character.parts.faceId();
  json["parts"] = parts;

  // Character appearance
  nlohmann::json appearance;
  appearance["headSize"] = character.appearance.headSize();
  appearance["height"] = character.appearance.height();
  appearance["thighVolume"] = character.appearance.thighVolume();
  appearance["legVolume"] = character.appearance.legVolume();
  json["appearance"] = appearance;

  json["petUid"] = character.petUid();
  json["guildUid"] = character.guildUid();

  json["gifts"] = character.gifts();
  json["purchases"] = character.purchases();

  json["inventory"] = character.inventory();
  json["characterEquipment"] = character.characterEquipment();
  json["horseEquipment"] = character.mountEquipment();

  json["horseUids"] = character.horses();
  json["mountUid"] = character.mountUid();
  json["ranchUid"] = character.ranchUid();

  dataFile << json.dump(2);
}

void server::FileDataSource::CreateItem(data::Item& item)
{
  item.uid = _sequentialUid.fetch_add(1, std::memory_order::relaxed);
}

void server::FileDataSource::RetrieveItem(data::Uid uid, data::Item& item)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _itemDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Item file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  item.uid = json["uid"].get<data::Uid>();
  item.tid = json["tid"].get<data::Tid>();
  item.count = json["count"].get<uint32_t>();
}

void server::FileDataSource::StoreItem(data::Uid uid, const data::Item& item)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _itemDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Item file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = item.uid();
  json["tid"] = item.tid();
  json["count"] = item.count();
  dataFile << json.dump(2);
}

void server::FileDataSource::CreatePet(data::Pet& pet)
{
  pet.uid = _sequentialUid.fetch_add(1, std::memory_order::relaxed);
}

void server::FileDataSource::RetrievePet(data::Uid uid, data::Pet& pet)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _petDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Pet file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  pet.uid = json["uid"].get<data::Uid>();
  pet.tid = json["tid"].get<data::Tid>();
  pet.name = json["name"].get<std::string>();
}

void server::FileDataSource::StorePet(data::Uid uid, const data::Pet& pet)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _petDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Pet file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = pet.uid();
  json["tid"] = pet.tid();
  json["name"] = pet.name();
  dataFile << json.dump(2);
}

void server::FileDataSource::CreateGuild(data::Guild& guild)
{
  guild.uid = _sequentialUid.fetch_add(1, std::memory_order::relaxed);
}

void server::FileDataSource::RetrieveGuild(data::Uid uid, data::Guild& guild)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _guildDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Guild file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  guild.uid = json["uid"].get<data::Uid>();
  guild.name = json["name"].get<std::string>();
}

void server::FileDataSource::StoreGuild(data::Uid uid, const data::Guild& guild)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _guildDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Guild file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = guild.uid();
  json["name"] = guild.name();
  dataFile << json.dump(2);
}

void server::FileDataSource::CreateStoredItem(data::StoredItem& item)
{
  item.uid = _sequentialUid.fetch_add(1, std::memory_order::relaxed);
}

void server::FileDataSource::RetrieveStoredItem(data::Uid uid, data::StoredItem& item)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _storedItemPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Stored item file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  item.uid = json["uid"].get<data::Uid>();
  item.items = json["items"].get<std::vector<data::Uid>>();
  item.sender = json["sender"].get<std::string>();
  item.message = json["message"].get<std::string>();
  item.checked = json["checked"].get<bool>();
  item.expired = json["expired"].get<bool>();
}

void server::FileDataSource::StoreStoredItem(data::Uid uid, const data::StoredItem& item)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _storedItemPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Stored item file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = item.uid();
  json["items"] = item.items();
  json["sender"] = item.sender();
  json["message"] = item.message();
  json["checked"] = item.checked();
  json["expired"] = item.expired();
}

void server::FileDataSource::CreateHorse(data::Horse& horse)
{
  horse.uid = _sequentialUid.fetch_add(1, std::memory_order::relaxed);
}

void server::FileDataSource::RetrieveHorse(data::Uid uid, data::Horse& horse)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _horseDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Horse file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  horse.uid = json["uid"].get<data::Uid>();
  horse.tid = json["tid"].get<data::Tid>();
  horse.name = json["name"].get<std::string>();

  auto parts = json["parts"];
  horse.parts = data::Horse::Parts{
    .skinId = parts["skinId"].get<uint32_t>(),
    .faceId = parts["faceId"].get<uint32_t>(),
    .maneId = parts["maneId"].get<uint32_t>(),
    .tailId = parts["tailId"].get<uint32_t>()};

  auto appearance = json["appearance"];
  horse.appearance = data::Horse::Appearance{
    .scale = appearance["scale"].get<uint32_t>(),
    .legLength = appearance["legLength"].get<uint32_t>(),
    .legVolume = appearance["legVolume"].get<uint32_t>(),
    .bodyLength = appearance["bodyLength"].get<uint32_t>(),
    .bodyVolume = appearance["bodyVolume"].get<uint32_t>()};

  auto stats = json["stats"];
  horse.stats = data::Horse::Stats{
    .agility = stats["agility"].get<uint32_t>(),
    .control = stats["control"].get<uint32_t>(),
    .speed = stats["speed"].get<uint32_t>(),
    .strength = stats["strength"].get<uint32_t>(),
    .spirit = stats["spirit"].get<uint32_t>()};

  auto mastery = json["mastery"];
  horse.mastery = data::Horse::Mastery{
    .spurMagicCount = mastery["spurMagicCount"].get<uint32_t>(),
    .jumpCount = mastery["jumpCount"].get<uint32_t>(),
    .slidingTime = mastery["slidingTime"].get<uint32_t>(),
    .glidingDistance = mastery["glidingDistance"].get<uint32_t>()};

  horse.rating = json["rating"].get<uint32_t>();
  horse.clazz = json["clazz"].get<uint32_t>();
  horse.clazzProgress = json["clazzProgress"].get<uint32_t>();
  horse.grade = json["grade"].get<uint32_t>();
  horse.growthPoints = json["growthPoints"].get<uint32_t>();

  horse.potentialType = json["potentialType"].get<uint32_t>();
  horse.potentialLevel = json["potentialLevel"].get<uint32_t>();

  horse.luckState = json["luckState"].get<uint32_t>();
  horse.emblem = json["emblem"].get<uint32_t>();
}

void server::FileDataSource::StoreHorse(data::Uid uid, const data::Horse& horse)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _horseDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Horse file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = horse.uid();
  json["tid"] = horse.tid();
  json["name"] = horse.name();

  nlohmann::json parts;
  parts["skinId"] = horse.parts.skinId();
  parts["faceId"] = horse.parts.faceId();
  parts["maneId"] = horse.parts.maneId();
  parts["tailId"] = horse.parts.tailId();
  json["parts"] = parts;

  nlohmann::json appearance;
  appearance["scale"] = horse.appearance.scale();
  appearance["legLength"] = horse.appearance.legLength();
  appearance["legVolume"] = horse.appearance.legVolume();
  appearance["bodyLength"] = horse.appearance.bodyLength();
  appearance["bodyVolume"] = horse.appearance.bodyVolume();
  json["appearance"] = appearance;

  nlohmann::json stats;
  stats["agility"] = horse.stats.agility();
  stats["control"] = horse.stats.control();
  stats["speed"] = horse.stats.speed();
  stats["strength"] = horse.stats.strength();
  stats["spirit"] = horse.stats.spirit();
  json["stats"] = stats;

  nlohmann::json mastery;
  mastery["spurMagicCount"] = horse.mastery.spurMagicCount();
  mastery["jumpCount"] = horse.mastery.jumpCount();
  mastery["slidingTime"] = horse.mastery.slidingTime();
  mastery["glidingDistance"] = horse.mastery.glidingDistance();
  json["mastery"] = mastery;

  json["rating"] = horse.rating();
  json["clazz"] = horse.clazz();
  json["clazzProgress"] = horse.clazzProgress();
  json["grade"] = horse.grade();
  json["growthPoints"] = horse.growthPoints();

  json["potentialType"] = horse.potentialType();
  json["potentialLevel"] = horse.potentialLevel();

  json["luckState"] = horse.luckState();
  json["emblem"] = horse.emblem();

  dataFile << json.dump(2);
}

void server::FileDataSource::CreateRanch(data::Ranch& ranch)
{
  ranch.uid = _sequentialUid.fetch_add(1, std::memory_order::relaxed);
}

void server::FileDataSource::RetrieveRanch(data::Uid uid, data::Ranch& ranch)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _ranchDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Ranch file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  ranch.uid = json["uid"].get<data::Uid>();
  ranch.name = json["name"].get<std::string>();
}

void server::FileDataSource::StoreRanch(data::Uid uid, const data::Ranch& ranch)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _ranchDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Ranch file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = ranch.uid();
  json["name"] = ranch.name();
  dataFile << json.dump(2);
}
