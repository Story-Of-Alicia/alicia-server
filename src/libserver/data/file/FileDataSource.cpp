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

#include <fstream>

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

void soa::FileDataSource::Initialize(const std::filesystem::path& path)
{
  _path = path;
  _usersPath = _path / "users";
  create_directories(_usersPath);
  _charactersPath = _path / "characters";
  create_directories(_charactersPath);
  _horsesPath = _path / "horses";
  create_directories(_horsesPath);
  _ranchesPath = _path / "ranches";
  create_directories(_ranchesPath);
  _itemsPath = _path / "items";
  create_directories(_itemsPath);
  _metaFilePath = _path / "meta.json";

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

void soa::FileDataSource::Terminate()
{
  const std::filesystem::path metaFilePath = ProduceDataPath(
    _metaFilePath, "meta");

  std::ofstream metaFile(metaFilePath);
  if (not metaFile.is_open())
  {
    return;
  }

  nlohmann::json meta;
  meta["sequentialUid"] = _sequentialUid;

  metaFile << meta.dump(2);
}

void soa::FileDataSource::RetrieveUser(std::string name, data::User& user)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _usersPath, name);

  std::ifstream file(dataFilePath);
  if (not file.is_open())
    return;

  const auto json = nlohmann::json::parse(file);
  user.name = json["name"].get<std::string>();
  user.token = json["token"].get<std::string>();
  user.characterUid = json["characterUid"].get<data::Uid>();
}

void soa::FileDataSource::StoreUser(std::string name, const data::User& user)
{
  const std::filesystem::path userFilePath = ProduceDataPath(
    _usersPath, name);

  std::ofstream file(userFilePath);
  if (not file.is_open())
    return;

  nlohmann::json json;
  json["name"] = user.name();
  json["token"] = user.token();
  json["characterUid"] = user.characterUid();

  file << json.dump(2);
}

void soa::FileDataSource::CreateCharacter(data::Character& character)
{
  _sequentialUid++;
  character.uid = _sequentialUid;
}

void soa::FileDataSource::RetrieveCharacter(data::Uid uid, data::Character& character)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _charactersPath, std::format("{}", uid));

  std::ifstream file(dataFilePath);
  if (not file.is_open())
    return;

  const auto json = nlohmann::json::parse(file);

  character.name = json["name"].get<std::string>();
  character.level = json["level"].get<uint32_t>();
  character.carrots = json["carrots"].get<uint32_t>();
  character.cash = json["cash"].get<uint32_t>();

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

  character.characterEquipment = json["characterEquipment"].get<std::vector<data::Uid>>();
  character.horseEquipment = json["horseEquipment"].get<std::vector<data::Uid>>();

  character.horseUids = json["horseUids"].get<std::vector<data::Uid>>();
  character.mountUid = json["mountUid"].get<data::Uid>();

}

void soa::FileDataSource::StoreCharacter(data::Uid uid, const data::Character& character)
{
  const std::filesystem::path userFilePath = ProduceDataPath(
    _charactersPath, std::format("{}", uid));

  std::ofstream file(userFilePath);
  if (not file.is_open())
    return;

  nlohmann::json json;
  json["name"] = character.name();
  json["level"] = character.level();
  json["carrots"] = character.carrots();
  json["cash"] = character.cash();

  // character parts
  nlohmann::json parts;
  parts["modelId"] = character.parts.modelId();
  parts["mouthId"] = character.parts.mouthId();
  parts["faceId"] = character.parts.faceId();
  json["parts"] = parts;

  // character appearance
  nlohmann::json appearance;
  appearance["headSize"] = character.appearance.headSize();
  appearance["height"] = character.appearance.height();
  appearance["thighVolume"] = character.appearance.thighVolume();
  appearance["legVolume"] = character.appearance.legVolume();
  json["appearance"] = appearance;


  json["characterEquipment"] = character.characterEquipment();
  json["horseEquipment"] = character.horseEquipment();

  json["horseUids"] = character.horseUids();
  json["mountUid"] = character.mountUid();

  file << json.dump(2);
}

void soa::FileDataSource::RetrieveItem(data::Uid uid, data::Item& item)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _itemsPath, std::format("{}", uid));

  std::ifstream file(dataFilePath);
  if (not file.is_open())
    return;

  const auto json = nlohmann::json::parse(file);

  item.uid = json["uid"].get<data::Uid>();
  item.tid = json["tid"].get<data::Tid>();
  item.count = json["count"].get<uint32_t>();
  item.slot = static_cast<soa::data::Item::Slot>(json["slot"].get<int32_t>());
}

void soa::FileDataSource::StoreItem(data::Uid uid, const data::Item& item)
{
  const std::filesystem::path userFilePath = ProduceDataPath(
    _itemsPath, std::format("{}", uid));

  std::ofstream file(userFilePath);
  if (not file.is_open())
    return;

  nlohmann::json json;
  json["uid"] = item.uid();
  json["tid"] = item.tid();
  json["count"] = item.count();
  json["slot"] = item.slot();
  file << json.dump(2);
}

void soa::FileDataSource::RetrieveHorse(data::Uid uid, data::Horse& horse)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _horsesPath, std::format("{}", uid));

  std::ifstream file(dataFilePath);
  if (not file.is_open())
    return;

  const auto json = nlohmann::json::parse(file);
  horse.name = json["name"].get<std::string>();

  auto parts = json["parts"];
  horse.parts = data::Horse::Parts{
    .skinId = parts["skinId"].get<uint32_t>(),
    .maneId = parts["maneId"].get<uint32_t>(),
    .tailId = parts["tailId"].get<uint32_t>(),
    .faceId = parts["faceId"].get<uint32_t>()};

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

void soa::FileDataSource::StoreHorse(data::Uid uid, const data::Horse& horse)
{
  const std::filesystem::path userFilePath = ProduceDataPath(
    _horsesPath, std::format("{}", uid));

  std::ofstream file(userFilePath);
  if (not file.is_open())
    return;

  nlohmann::json json;
  json["name"] = horse.name();

  nlohmann::json parts;
  parts["skinId"] = horse.parts.skinId();
  parts["maneId"] = horse.parts.maneId();
  parts["tailId"] = horse.parts.tailId();
  parts["faceId"] = horse.parts.faceId();
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

  file << json.dump(2);
}

void soa::FileDataSource::RetrieveRanch(data::Uid uid, data::Ranch& ranch)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _ranchesPath, std::format("{}", uid));

  std::ifstream file(dataFilePath);
  if (not file.is_open())
    return;

  const auto json = nlohmann::json::parse(file);

  ranch.name = json["name"].get<std::string>();
}

void soa::FileDataSource::StoreRanch(data::Uid uid, const data::Ranch& ranch)
{
  const std::filesystem::path userFilePath = ProduceDataPath(
    _ranchesPath, std::format("{}", uid));

  std::ofstream file(userFilePath);
  if (not file.is_open())
    return;

  nlohmann::json json;
  json["name"] = ranch.name();
  file << json.dump(2);
}
