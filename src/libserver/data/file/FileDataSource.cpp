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

  const auto prepareDataPath = [this](const std::filesystem::path& folder)
  {
    const auto path = _dataPath / folder;
    create_directories(path);

    return path;
  };

  // Prepare the data paths.
  _userDataPath = prepareDataPath("users");
  _infractionDataPath = prepareDataPath("infractions");
  _characterDataPath = prepareDataPath("characters");
  _itemDataPath = prepareDataPath("characters/equipment/items");
  _horseDataPath = prepareDataPath("characters/equipment/horses");
  _storageItemPath = prepareDataPath("storage");
  _eggDataPath = prepareDataPath("eggs");
  _petDataPath = prepareDataPath("pets");
  _housingDataPath = prepareDataPath("housing");
  _guildDataPath = prepareDataPath("guilds");

  // Read the meta-data file and parse the sequential UIDs.
  const std::filesystem::path metaFilePath = ProduceDataPath(
    _metaFilePath, "meta");
  std::ifstream metaFile(metaFilePath);
  if (not metaFile.is_open())
  {
    return;
  }

  const auto meta = nlohmann::json::parse(metaFile);
  _infractionSequentialUid = meta["infractionSequentialUid"].get<uint32_t>();
  _characterSequentialUid = meta["characterSequentialUid"].get<uint32_t>();
  _equipmentSequentialUid = meta["equipmentSequentialUid"].get<uint32_t>();
  _storageItemSequentialUid = meta["storageItemSequentialUid"].get<uint32_t>();
  _eggSequentialUid = meta["eggSequentialUid"].get<uint32_t>();
  _petSequentialUid = meta["petSequentialUid"].get<uint32_t>();
  _housingSequentialUid = meta["housingSequentialUid"].get<uint32_t>();
  _guildSequentialId = meta["guildSequentialId"].get<uint32_t>();
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
  meta["infractionSequentialUid"] = _infractionSequentialUid.load();
  meta["characterSequentialUid"] = _characterSequentialUid.load();
  meta["equipmentSequentialUid"] = _equipmentSequentialUid.load();
  meta["storageItemSequentialUid"] = _storageItemSequentialUid.load();
  meta["eggSequentialUid"] = _eggSequentialUid.load();
  meta["petSequentialUid"] = _petSequentialUid.load();
  meta["housingSequentialUid"] = _housingSequentialUid.load();
  meta["guildSequentialId"] = _guildSequentialId.load();

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
  user.infractions = json["infractions"].get<std::vector<data::Uid>>();
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
  json["infractions"] = user.infractions();

  dataFile << json.dump(2);
}

void server::FileDataSource::CreateInfraction(data::Infraction& infraction)
{
  infraction.uid = ++_infractionSequentialUid;
}

void server::FileDataSource::RetrieveInfraction(data::Uid uid, data::Infraction& infraction)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
   _infractionDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Infraction file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  infraction.uid = json["uid"].get<data::Uid>();
  infraction.description = json["description"].get<std::string>();
  infraction.punishment = json["punishment"].get<data::Infraction::Punishment>();
  infraction.duration = data::Clock::duration(std::chrono::seconds(
    json["duration"].get<uint64_t>()));
  infraction.createdAt = data::Clock::time_point(std::chrono::seconds(
    json["createdAt"].get<uint64_t>()));
}

void server::FileDataSource::StoreInfraction(data::Uid uid, const data::Infraction& infraction)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _infractionDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Infraction file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = infraction.uid();
  json["description"] = infraction.description();
  json["punishment"] = infraction.punishment();
  json["duration"] = std::chrono::duration_cast<std::chrono::seconds>(
    infraction.duration()).count();
  json["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    infraction.createdAt().time_since_epoch()).count();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteInfraction(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _infractionDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateCharacter(data::Character& character)
{
  character.uid = ++_characterSequentialUid;
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
  character.age = json["age"].get<uint8_t>();
  character.hideGenderAndAge = json["hideGenderAndAge"].get<bool>();

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
    .voiceId = appearance["voiceId"].get<uint32_t>(),
    .headSize = appearance["headSize"].get<uint32_t>(),
    .height = appearance["height"].get<uint32_t>(),
    .thighVolume = appearance["thighVolume"].get<uint32_t>(),
    .legVolume = appearance["legVolume"].get<uint32_t>(),
    .emblemId = appearance["emblemId"].get<uint32_t>()};

  character.guildUid = json["guildUid"].get<data::Uid>();

  character.gifts = json["gifts"].get<std::vector<data::Uid>>();
  character.purchases = json["purchases"].get<std::vector<data::Uid>>();

  character.items = json["inventory"].get<std::vector<data::Uid>>();
  character.characterEquipment = json["characterEquipment"].get<std::vector<data::Uid>>();
  character.mountEquipment = json["horseEquipment"].get<std::vector<data::Uid>>();

  character.horses = json["horses"].get<std::vector<data::Uid>>();
  character.pets = json["pets"].get<std::vector<data::Uid>>();
  character.mountUid = json["mountUid"].get<data::Uid>();
  character.petUid = json["petUid"].get<data::Uid>();

  character.eggs = json["eggs"].get<std::vector<data::Uid>>();

  character.housing = json["housing"].get<std::vector<data::Uid>>();

  character.isRanchLocked = json["isRanchLocked"].get<bool>();
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
  json["age"] = character.age();
  json["hideGenderAndAge"] = character.hideGenderAndAge();

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
  appearance["voiceId"] = character.appearance.voiceId();
  appearance["headSize"] = character.appearance.headSize();
  appearance["height"] = character.appearance.height();
  appearance["thighVolume"] = character.appearance.thighVolume();
  appearance["legVolume"] = character.appearance.legVolume();
  appearance["emblemId"] = character.appearance.emblemId();
  json["appearance"] = appearance;

  json["guildUid"] = character.guildUid();

  json["gifts"] = character.gifts();
  json["purchases"] = character.purchases();

  json["inventory"] = character.items();
  json["characterEquipment"] = character.characterEquipment();
  json["horseEquipment"] = character.mountEquipment();

  json["horses"] = character.horses();
  json["pets"] = character.pets();
  json["mountUid"] = character.mountUid();
  json["petUid"] = character.petUid();

  json["eggs"] = character.eggs();

  json["housing"] = character.housing();

  json["isRanchLocked"] = character.isRanchLocked();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteCharacter(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _characterDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateHorse(data::Horse& horse)
{
  // can be standalone
  horse.uid = ++_equipmentSequentialUid;
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
    .skinTid = parts["skinId"].get<uint32_t>(),
    .faceTid = parts["faceId"].get<uint32_t>(),
    .maneTid = parts["maneId"].get<uint32_t>(),
    .tailTid = parts["tailId"].get<uint32_t>()};

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
    .courage = stats["courage"].get<uint32_t>(),
    .rush = stats["rush"].get<uint32_t>(),
    .endurance = stats["endurance"].get<uint32_t>(),
    .ambition = stats["ambition"].get<uint32_t>()};

  auto mastery = json["mastery"];
  horse.mastery = data::Horse::Mastery{
    .spurMagicCount = mastery["spurMagicCount"].get<uint32_t>(),
    .jumpCount = mastery["jumpCount"].get<uint32_t>(),
    .slidingTime = mastery["slidingTime"].get<uint32_t>(),
    .glidingDistance = mastery["glidingDistance"].get<uint32_t>()};

  auto mountCondition = json["mountCondition"];
  horse.mountCondition = data::Horse::MountCondition{
    .stamina = mountCondition["stamina"].get<uint16_t>(),
    .charm = mountCondition["charm"].get<uint16_t>(),
    .friendliness = mountCondition["friendliness"].get<uint16_t>(),
    .injury = mountCondition["injury"].get<uint16_t>(),
    .plenitude = mountCondition["plenitude"].get<uint16_t>(),
    .bodyDirtiness = mountCondition["bodyDirtiness"].get<uint16_t>(),
    .maneDirtiness = mountCondition["maneDirtiness"].get<uint16_t>(),
    .tailDirtiness = mountCondition["tailDirtiness"].get<uint16_t>(),
    .bodyPolish = mountCondition["bodyPolish"].get<uint16_t>(),
    .manePolish = mountCondition["manePolish"].get<uint16_t>(),
    .tailPolish = mountCondition["tailPolish"].get<uint16_t>(),
    .attachment = mountCondition["attachment"].get<uint16_t>(),
    .boredom = mountCondition["boredom"].get<uint16_t>(),
    .stopAmendsPoint = mountCondition["stopAmendsPoint"].get<uint16_t>()};

  horse.rating = json["rating"].get<uint32_t>();
  horse.clazz = json["clazz"].get<uint32_t>();
  horse.clazzProgress = json["clazzProgress"].get<uint32_t>();
  horse.grade = json["grade"].get<uint32_t>();
  horse.growthPoints = json["growthPoints"].get<uint32_t>();

  horse.potentialType = json["potentialType"].get<uint32_t>();
  horse.potentialLevel = json["potentialLevel"].get<uint32_t>();

  horse.luckState = json["luckState"].get<uint32_t>();
  horse.emblemUid = json["emblem"].get<uint32_t>();

  horse.dateOfBirth = data::Clock::time_point(std::chrono::seconds(
    json["dateOfBirth"].get<uint64_t>()));
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
  parts["skinId"] = horse.parts.skinTid();
  parts["faceId"] = horse.parts.faceTid();
  parts["maneId"] = horse.parts.maneTid();
  parts["tailId"] = horse.parts.tailTid();
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
  stats["courage"] = horse.stats.courage();
  stats["rush"] = horse.stats.rush();
  stats["endurance"] = horse.stats.endurance();
  stats["ambition"] = horse.stats.ambition();
  json["stats"] = stats;

  nlohmann::json mastery;
  mastery["spurMagicCount"] = horse.mastery.spurMagicCount();
  mastery["jumpCount"] = horse.mastery.jumpCount();
  mastery["slidingTime"] = horse.mastery.slidingTime();
  mastery["glidingDistance"] = horse.mastery.glidingDistance();
  json["mastery"] = mastery;

  nlohmann::json mountCondition;
  mountCondition["stamina"] = horse.mountCondition.stamina();
  mountCondition["charm"] = horse.mountCondition.charm();
  mountCondition["friendliness"] = horse.mountCondition.friendliness();
  mountCondition["injury"] = horse.mountCondition.injury();
  mountCondition["plenitude"] = horse.mountCondition.plenitude();
  mountCondition["bodyDirtiness"] = horse.mountCondition.bodyDirtiness();
  mountCondition["maneDirtiness"] = horse.mountCondition.maneDirtiness();
  mountCondition["tailDirtiness"] = horse.mountCondition.tailDirtiness();
  mountCondition["bodyPolish"] = horse.mountCondition.bodyPolish();
  mountCondition["manePolish"] = horse.mountCondition.manePolish();
  mountCondition["tailPolish"] = horse.mountCondition.tailPolish();
  mountCondition["attachment"] = horse.mountCondition.attachment();
  mountCondition["boredom"] = horse.mountCondition.boredom();
  mountCondition["stopAmendsPoint"] = horse.mountCondition.stopAmendsPoint();
  json["mountCondition"] = mountCondition;

  json["rating"] = horse.rating();
  json["clazz"] = horse.clazz();
  json["clazzProgress"] = horse.clazzProgress();
  json["grade"] = horse.grade();
  json["growthPoints"] = horse.growthPoints();

  json["potentialType"] = horse.potentialType();
  json["potentialLevel"] = horse.potentialLevel();

  json["luckState"] = horse.luckState();
  json["emblem"] = horse.emblemUid();

  json["dateOfBirth"] = std::chrono::ceil<std::chrono::seconds>(
    horse.dateOfBirth().time_since_epoch()).count();
  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteHorse(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _horseDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateItem(data::Item& item)
{
  item.uid = ++_equipmentSequentialUid;
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
  item.expiresAt = data::Clock::time_point(
    std::chrono::seconds(json["expiresAt"].get<int64_t>()));
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
  json["expiresAt"] = std::chrono::ceil<std::chrono::seconds>(
    item.expiresAt().time_since_epoch()).count();
  json["count"] = item.count();
  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteItem(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _itemDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateStorageItem(data::StorageItem& item)
{
  item.uid = ++_storageItemSequentialUid;
}

void server::FileDataSource::RetrieveStorageItem(data::Uid uid, data::StorageItem& item)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _storageItemPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Storage item file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  item.uid = json["uid"].get<data::Uid>();
  item.items = json["items"].get<std::vector<data::Uid>>();
  item.sender = json["sender"].get<std::string>();
  item.message = json["message"].get<std::string>();
  item.created = data::Clock::time_point(std::chrono::seconds(
    json["created"].get<uint64_t>()));
  item.checked = json["checked"].get<bool>();
  item.expired = json["expired"].get<bool>();
}

void server::FileDataSource::StoreStorageItem(data::Uid uid, const data::StorageItem& item)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _storageItemPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Storage item file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = item.uid();
  json["items"] = item.items();
  json["sender"] = item.sender();
  json["message"] = item.message();
  json["created"] = std::chrono::ceil<std::chrono::seconds>(
    item.created().time_since_epoch()).count();
  json["checked"] = item.checked();
  json["expired"] = item.expired();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteStorageItem(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _storageItemPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateEgg(data::Egg& egg)
{
  egg.uid = ++_eggSequentialUid;
}

void server::FileDataSource::RetrieveEgg(data::Uid uid, data::Egg& egg)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _eggDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Egg file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  egg.uid = json["uid"].get<data::Uid>();
  egg.itemUid = json["itemUid"].get<data::Uid>();
  egg.itemTid = json["itemTid"].get<data::Tid>();

  egg.incubatedAt = data::Clock::time_point(
    std::chrono::seconds(
      json["incubatedAt"].get<uint64_t>()));
  egg.incubatorSlot = json["incubatorSlot"].get<uint32_t>();
  egg.boostsUsed = json["boostsUsed"].get<uint32_t>();
}

void server::FileDataSource::StoreEgg(data::Uid uid, const data::Egg& egg)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _eggDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Egg file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = egg.uid();
  json["itemUid"] = egg.itemUid();
  json["itemTid"] = egg.itemTid();
  json["incubatedAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    egg.incubatedAt().time_since_epoch()).count();
  json["incubatorSlot"] = egg.incubatorSlot();
  json["boostsUsed"] = egg.boostsUsed();
  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteEgg(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _eggDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreatePet(data::Pet& pet)
{
  pet.uid = ++_petSequentialUid;
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
  pet.itemUid = json["itemUid"].get<data::Uid>();
  pet.petId = json["petId"].get<data::Uid>();
  pet.name = json["name"].get<std::string>();
  pet.birthDate = data::Clock::time_point(std::chrono::seconds(
    json["birthDate"].get<uint64_t>()));
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
  json["itemUid"] = pet.itemUid();
  json["petId"] = pet.petId();
  json["name"] = pet.name();
  json["birthDate"] = std::chrono::duration_cast<std::chrono::seconds>(
    pet.birthDate().time_since_epoch()).count();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeletePet(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _petDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateHousing(data::Housing& housing)
{
  housing.uid = ++_housingSequentialUid;
}

void server::FileDataSource::RetrieveHousing(data::Uid uid, data::Housing& housing)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _housingDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Housing file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  housing.uid = json["uid"].get<data::Uid>();
  housing.housingId = json["housingId"].get<uint16_t>();
  housing.expiresAt = data::Clock::time_point(
    std::chrono::seconds(json["expiresAt"].get<uint64_t>()));
  housing.durability = json["durability"].get<uint32_t>();
}

void server::FileDataSource::StoreHousing(data::Uid uid, const data::Housing& housing)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _housingDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Housing file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = housing.uid();
  json["housingId"] = housing.housingId();
  json["expiresAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    housing.expiresAt().time_since_epoch()).count();
  json["durability"] = housing.durability();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteHousing(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _housingDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateGuild(data::Guild& guild)
{
  guild.uid = ++_guildSequentialId;
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

void server::FileDataSource::DeleteGuild(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataPath(
    _guildDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}
