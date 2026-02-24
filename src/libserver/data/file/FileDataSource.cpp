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

#include <format>
#include <fstream>
#include <regex>

#include <nlohmann/json.hpp>

namespace
{

std::filesystem::path ProduceDataFilePath(
  const std::filesystem::path& root,
  const std::string& filename)
{
  if (not std::filesystem::exists(root))
    std::filesystem::create_directories(root);
  return root / (filename + ".json");
}

} // anon namespace

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
  _settingsDataPath = prepareDataPath("settings");
  _mailDataPath = prepareDataPath("mails");

  // Read the meta-data file and parse the sequential UIDs.
  const std::filesystem::path metaFilePath = ProduceDataFilePath(
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
  _settingsSequentialId = meta["settingsSequentialId"].get<uint32_t>();
  _mailSequentialId = meta["mailSequentialId"].get<uint32_t>();
}

void server::FileDataSource::Terminate()
{
  SaveMetadata();
}

void server::FileDataSource::SaveMetadata()
{
  const std::filesystem::path metaFilePath = ProduceDataFilePath(
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
  meta["settingsSequentialId"] = _settingsSequentialId.load();
  meta["mailSequentialId"] = _mailSequentialId.load();

  metaFile << meta.dump(2);
}

void server::FileDataSource::CreateUser(data::User&)
{
}

void server::FileDataSource::RetrieveUser(const std::string_view& name, data::User& user)
{
  user.name = std::string(name);

  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _userDataPath, user.name());

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

void server::FileDataSource::StoreUser(const std::string_view&, const data::User& user)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _userDataPath, user.name());

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

bool server::FileDataSource::IsUserNameUnique(const std::string_view& name)
{
  const std::regex rg(
    std::format("{}.*", name),
    std::regex_constants::ECMAScript | std::regex_constants::icase);

  for (const auto& file : std::filesystem::directory_iterator(_userDataPath))
  {
    const auto existingUserName = file.path().filename().string();
    if (std::regex_match(existingUserName, rg))
      return false;
  }

  return true;
}

void server::FileDataSource::CreateInfraction(data::Infraction& infraction)
{
  infraction.uid = ++_infractionSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveInfraction(data::Uid uid, data::Infraction& infraction)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  infraction.duration = std::chrono::seconds(
    json["duration"].get<int64_t>());
  infraction.createdAt = data::Clock::time_point(std::chrono::seconds(
    json["createdAt"].get<int64_t>()));
}

void server::FileDataSource::StoreInfraction(data::Uid uid, const data::Infraction& infraction)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  json["duration"] = infraction.duration().count();
  json["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    infraction.createdAt().time_since_epoch()).count();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteInfraction(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _infractionDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateCharacter(data::Character& character)
{
  character.uid = ++_characterSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveCharacter(data::Uid uid, data::Character& character)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
    .voiceId = appearance["voiceId"].get<uint32_t>(),
    .headSize = appearance["headSize"].get<uint32_t>(),
    .height = appearance["height"].get<uint32_t>(),
    .thighVolume = appearance["thighVolume"].get<uint32_t>(),
    .legVolume = appearance["legVolume"].get<uint32_t>(),
    .emblemId = appearance["emblemId"].get<uint32_t>()};

  character.guildUid = json["guildUid"].get<data::Uid>();

  const auto& contacts = json["contacts"];
  character.contacts.pending = contacts["pending"].get<std::set<data::Uid>>();

  for (const auto& groupJson : contacts["groups"])
  {
    data::Character::Contacts::Group group{
      .uid = groupJson["uid"].get<data::Uid>(),
      .name = groupJson["name"].get<std::string>(),
      .members = groupJson["members"].get<std::set<data::Uid>>(),
      .createdAt = data::Clock::time_point(std::chrono::seconds(
          groupJson["createdAt"].get<int64_t>()))
    };

    character.contacts.groups().try_emplace(group.uid, group);
  }

  character.gifts = json["gifts"].get<std::vector<data::Uid>>();
  character.purchases = json["purchases"].get<std::vector<data::Uid>>();

  character.inventory = json["inventory"].get<std::vector<data::Uid>>();
  character.characterEquipment = json["characterEquipment"].get<std::vector<data::Uid>>();
  // todo: rename after larger refactor
  character.expiredEquipment = json["horseEquipment"].get<std::vector<data::Uid>>();

  character.horses = json["horses"].get<std::vector<data::Uid>>();
  character.horseSlotCount = json["horseSlotCount"].get<uint32_t>();

  character.pets = json["pets"].get<std::vector<data::Uid>>();
  character.mountUid = json["mountUid"].get<data::Uid>();
  character.petUid = json["petUid"].get<data::Uid>();

  character.eggs = json["eggs"].get<std::vector<data::Uid>>();

  character.housing = json["housing"].get<std::vector<data::Uid>>();

  character.isRanchLocked = json["isRanchLocked"].get<bool>();

  character.settingsUid = json["settingsUid"].get<data::Uid>();

  const auto readSkills = [](data::Character::Skills::Sets& sets, const nlohmann::json& json)
  {
    const auto readSkillSet = [](data::Character::Skills::Sets::Set& set, const nlohmann::json& json)
    {
      set.slot1 = json["slot1"].get<uint32_t>();
      set.slot2 = json["slot2"].get<uint32_t>();
    };

    readSkillSet(sets.set1, json["set1"]);
    readSkillSet(sets.set2, json["set2"]);
    sets.activeSetId = json["activeSetId"].get<uint32_t>();
  };

  const auto& skills = json["skills"];
  readSkills(character.skills.speed(), skills["speed"]);
  readSkills(character.skills.magic(), skills["magic"]);

  const auto& mailbox = json["mailbox"];
  character.mailbox.hasNewMail = mailbox["hasNewMail"].get<bool>();
  character.mailbox.inbox = mailbox["inbox"].get<std::vector<data::Uid>>();
  character.mailbox.sent = mailbox["sent"].get<std::vector<data::Uid>>();
}

void server::FileDataSource::StoreCharacter(data::Uid uid, const data::Character& character)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  appearance["voiceId"] = character.appearance.voiceId();
  appearance["headSize"] = character.appearance.headSize();
  appearance["height"] = character.appearance.height();
  appearance["thighVolume"] = character.appearance.thighVolume();
  appearance["legVolume"] = character.appearance.legVolume();
  appearance["emblemId"] = character.appearance.emblemId();
  json["appearance"] = appearance;

  json["guildUid"] = character.guildUid();

  nlohmann::json contacts;
  contacts["pending"] = character.contacts.pending();

  nlohmann::json groups;
  for (const auto& group : character.contacts.groups() | std::views::values)
  {
    nlohmann::json groupJson;
    groupJson["uid"] = group.uid;
    groupJson["name"] = group.name;
    groupJson["members"] = group.members;
    groupJson["createdAt"] = std::chrono::ceil<std::chrono::seconds>(
      group.createdAt.time_since_epoch()).count();

    groups.emplace_back(groupJson);
  }
  contacts["groups"] = groups;

  json["contacts"] = contacts;

  json["gifts"] = character.gifts();
  json["purchases"] = character.purchases();

  json["inventory"] = character.inventory();
  json["characterEquipment"] = character.characterEquipment();
  json["horseEquipment"] = character.expiredEquipment();

  json["horses"] = character.horses();
  json["horseSlotCount"] = character.horseSlotCount();

  json["pets"] = character.pets();
  json["mountUid"] = character.mountUid();
  json["petUid"] = character.petUid();

  json["eggs"] = character.eggs();

  json["housing"] = character.housing();

  json["isRanchLocked"] = character.isRanchLocked();

  json["settingsUid"] = character.settingsUid();

  // Construct game mode skills from skill sets
  const auto& writeSkills = [](const data::Character::Skills::Sets& sets)
  {
    const auto& writeSkillSet = [](const data::Character::Skills::Sets::Set& set)
    {
      nlohmann::json json;
      json["slot1"] = set.slot1;
      json["slot2"] = set.slot2;
      return json;
    };

    nlohmann::json json;
    json["set1"] = writeSkillSet(sets.set1);
    json["set2"] = writeSkillSet(sets.set2);
    json["activeSetId"] = sets.activeSetId;
    return json;
  };

  nlohmann::json skills;
  skills["speed"] = writeSkills(character.skills.speed());
  skills["magic"] = writeSkills(character.skills.magic());
  json["skills"] = skills;

  nlohmann::json mailbox;
  mailbox["hasNewMail"] = character.mailbox.hasNewMail();
  mailbox["inbox"] = character.mailbox.inbox();
  mailbox["sent"] = character.mailbox.sent();
  json["mailbox"] = mailbox;

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteCharacter(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _characterDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

server::data::Uid server::FileDataSource::RetrieveCharacterUidByName(const std::string_view& name)
{
  const std::regex rg(
    std::format("{}", name),
    std::regex_constants::icase);

  for (const auto& file : std::filesystem::directory_iterator(_characterDataPath))
  {
    if (file.is_directory())
      continue;

    std::ifstream dataFile(file.path());
    if (not dataFile.is_open())
      continue;

    const auto json = nlohmann::json::parse(dataFile);
    const auto existingCharacterName = json["name"].get<std::string>();

    if (std::regex_match(existingCharacterName, rg))
      return json["uid"].get<data::Uid>();
  }

  return data::InvalidUid;
}

bool server::FileDataSource::IsCharacterNameUnique(const std::string_view& name)
{
  return RetrieveCharacterUidByName(name) == data::InvalidUid;
}

void server::FileDataSource::CreateHorse(data::Horse& horse)
{
  horse.uid = ++_equipmentSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveHorse(data::Uid uid, data::Horse& horse)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
    .stamina = mountCondition["stamina"].get<uint32_t>(),
    .charm = mountCondition["charm"].get<uint32_t>(),
    .friendliness = mountCondition["friendliness"].get<uint32_t>(),
    .injury = mountCondition["injury"].get<uint32_t>(),
    .plenitude = mountCondition["plenitude"].get<uint32_t>(),
    .bodyDirtiness = mountCondition["bodyDirtiness"].get<uint32_t>(),
    .maneDirtiness = mountCondition["maneDirtiness"].get<uint32_t>(),
    .tailDirtiness = mountCondition["tailDirtiness"].get<uint32_t>(),
    .bodyPolish = mountCondition["bodyPolish"].get<uint32_t>(),
    .manePolish = mountCondition["manePolish"].get<uint32_t>(),
    .tailPolish = mountCondition["tailPolish"].get<uint32_t>(),
    .attachment = mountCondition["attachment"].get<uint32_t>(),
    .boredom = mountCondition["boredom"].get<uint32_t>(),
    .stopAmendsPoint = mountCondition["stopAmendsPoint"].get<uint32_t>()};

  horse.rating = json["rating"].get<uint32_t>();
  horse.clazz = json["clazz"].get<uint32_t>();
  horse.clazzProgress = json["clazzProgress"].get<uint32_t>();
  horse.grade = json["grade"].get<uint32_t>();
  horse.growthPoints = json["growthPoints"].get<uint32_t>();

  auto potential = json["potential"];
  horse.potential = data::Horse::Potential{
    .type = potential["type"].get<uint32_t>(),
    .level = potential["level"].get<uint32_t>(),
    .value = potential["value"].get<uint32_t>()
  };

  horse.luckState = json["luckState"].get<uint32_t>();
  horse.fatigue = json["fatigue"].get<uint32_t>();
  horse.emblemUid = json["emblem"].get<uint32_t>();

  horse.dateOfBirth = data::Clock::time_point(std::chrono::seconds(
    json["dateOfBirth"].get<uint64_t>()));

  auto mountInfo = json["mountInfo"];
  horse.mountInfo = data::Horse::MountInfo{
    .boostsInARow = mountInfo["boostsInARow"].get<uint32_t>(),
    .winsSpeedSingle = mountInfo["winsSpeedSingle"].get<uint32_t>(),
    .winsSpeedTeam = mountInfo["winsSpeedTeam"].get<uint32_t>(),
    .winsMagicSingle = mountInfo["winsMagicSingle"].get<uint32_t>(),
    .winsMagicTeam = mountInfo["winsMagicTeam"].get<uint32_t>(),
    .totalDistance = mountInfo["totalDistance"].get<uint32_t>(),
    .topSpeed = mountInfo["topSpeed"].get<uint32_t>(),
    .longestGlideDistance = mountInfo["longestGlideDistance"].get<uint32_t>(),
    .participated = mountInfo["participated"].get<uint32_t>(),
    .cumulativePrize = mountInfo["cumulativePrize"].get<uint32_t>(),
    .biggestPrize = mountInfo["biggestPrize"].get<uint32_t>()};
}

void server::FileDataSource::StoreHorse(data::Uid uid, const data::Horse& horse)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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

  nlohmann::json potential;
  potential["type"] = horse.potential.type();
  potential["level"] = horse.potential.level();
  potential["value"] = horse.potential.value();
  json["potential"] = potential;

  json["luckState"] = horse.luckState();
  json["fatigue"] = horse.fatigue();
  json["emblem"] = horse.emblemUid();

  json["dateOfBirth"] = std::chrono::ceil<std::chrono::seconds>(
    horse.dateOfBirth().time_since_epoch()).count();

  nlohmann::json mountInfo;
  mountInfo["boostsInARow"] = horse.mountInfo.boostsInARow();
  mountInfo["winsSpeedSingle"] = horse.mountInfo.winsSpeedSingle();
  mountInfo["winsSpeedTeam"] = horse.mountInfo.winsSpeedTeam();
  mountInfo["winsMagicSingle"] = horse.mountInfo.winsMagicSingle();
  mountInfo["winsMagicTeam"] = horse.mountInfo.winsMagicTeam();
  mountInfo["totalDistance"] = horse.mountInfo.totalDistance();
  mountInfo["topSpeed"] = horse.mountInfo.topSpeed();
  mountInfo["longestGlideDistance"] = horse.mountInfo.longestGlideDistance();
  mountInfo["participated"] = horse.mountInfo.participated();
  mountInfo["cumulativePrize"] = horse.mountInfo.cumulativePrize();
  mountInfo["biggestPrize"] = horse.mountInfo.biggestPrize();
  json["mountInfo"] = mountInfo;
  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteHorse(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _horseDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateItem(data::Item& item)
{
  item.uid = ++_equipmentSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveItem(data::Uid uid, data::Item& item)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  item.duration = std::chrono::seconds(json["duration"].get<int64_t>());
  item.createdAt = data::Clock::time_point(
    std::chrono::seconds(json["createdAt"].get<int64_t>()));
}

void server::FileDataSource::StoreItem(data::Uid uid, const data::Item& item)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  json["duration"] = item.duration().count();
  json["createdAt"] = std::chrono::ceil<std::chrono::seconds>(
    item.createdAt().time_since_epoch()).count();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteItem(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _itemDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateStorageItem(data::StorageItem& item)
{
  item.uid = ++_storageItemSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveStorageItem(data::Uid uid, data::StorageItem& storageItem)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _storageItemPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Storage item file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);

  storageItem.uid = json["uid"].get<data::Uid>();
  storageItem.sender = json["sender"].get<std::string>();
  storageItem.message = json["message"].get<std::string>();
  storageItem.carrots = json["carrots"].get<int32_t>();

  for (const auto& itemJson : json["items"])
  {
    storageItem.items().emplace_back(data::StorageItem::Item{
      .tid = itemJson["tid"].get<data::Tid>(),
      .count = itemJson["count"].get<uint32_t>(),
      .duration = std::chrono::seconds(
        itemJson["duration"].get<int64_t>()),});
  }

  storageItem.checked = json["checked"].get<bool>();
  storageItem.duration = std::chrono::seconds(
    json["duration"].get<int64_t>());
  storageItem.createdAt = data::Clock::time_point(std::chrono::seconds(
    json["createdAt"].get<int64_t>()));

  // Shop data
  storageItem.goodsSq = json["goodsSq"].get<uint32_t>();
  storageItem.priceId = json["priceId"].get<uint32_t>();
}

void server::FileDataSource::StoreStorageItem(data::Uid uid, const data::StorageItem& storageItem)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _storageItemPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Storage item file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = storageItem.uid();
  json["sender"] = storageItem.sender();
  json["message"] = storageItem.message();
  json["carrots"] = storageItem.carrots();

  auto& itemsJson = json["items"];
  for (const auto& item : storageItem.items())
  {
    nlohmann::json itemJson;
    itemJson["tid"] = item.tid;
    itemJson["count"] = item.count;
    itemJson["duration"] = item.duration.count();

    itemsJson.emplace_back(itemJson);
  }

  json["checked"] = storageItem.checked();
  json["createdAt"] = std::chrono::ceil<std::chrono::seconds>(
    storageItem.createdAt().time_since_epoch()).count();
  json["duration"] = storageItem.duration().count();

  // Shop data
  json["goodsSq"] = storageItem.goodsSq();
  json["priceId"] = storageItem.priceId();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteStorageItem(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _storageItemPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateEgg(data::Egg& egg)
{
  egg.uid = ++_eggSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveEgg(data::Uid uid, data::Egg& egg)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _eggDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreatePet(data::Pet& pet)
{
  pet.uid = ++_petSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrievePet(data::Uid uid, data::Pet& pet)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _petDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateHousing(data::Housing& housing)
{
  housing.uid = ++_housingSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveHousing(data::Uid uid, data::Housing& housing)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _housingDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Housing file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  housing.uid = json["uid"].get<data::Uid>();
  housing.housingId = json["housingId"].get<uint32_t>();
  housing.expiresAt = data::Clock::time_point(
    std::chrono::seconds(json["expiresAt"].get<uint64_t>()));
  housing.durability = json["durability"].get<uint32_t>();
}

void server::FileDataSource::StoreHousing(data::Uid uid, const data::Housing& housing)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _housingDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateGuild(data::Guild& guild)
{
  guild.uid = ++_guildSequentialId;
  SaveMetadata();
}

void server::FileDataSource::RetrieveGuild(data::Uid uid, data::Guild& guild)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  guild.description = json["description"].get<std::string>();
  guild.owner = json["owner"].get<data::Uid>();
  guild.officers = json["officers"].get<std::vector<data::Uid>>();
  guild.members = json["members"].get<std::vector<data::Uid>>();

  guild.rank = json["rank"].get<uint32_t>();
  guild.totalWins = json["totalWins"].get<uint32_t>();
  guild.totalLosses = json["totalLosses"].get<uint32_t>();
  guild.seasonalWins = json["seasonalWins"].get<uint32_t>();
  guild.seasonalLosses = json["seasonalLosses"].get<uint32_t>();
}

void server::FileDataSource::StoreGuild(data::Uid uid, const data::Guild& guild)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
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
  json["description"] = guild.description();
  json["owner"] = guild.owner();
  json["officers"] = guild.officers();
  json["members"] = guild.members();

  json["rank"] = guild.rank();
  json["totalWins"] = guild.totalWins();
  json["totalLosses"] = guild.totalLosses();
  json["seasonalWins"] = guild.seasonalWins();
  json["seasonalLosses"] = guild.seasonalLosses();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteGuild(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _guildDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

bool server::FileDataSource::IsGuildNameUnique(const std::string_view& name)
{
  const std::regex rg(
    std::format("{}", name),
    std::regex_constants::icase);

  for (const auto& file : std::filesystem::directory_iterator(_guildDataPath))
  {
    if (file.is_directory())
      continue;

    std::ifstream dataFile(file.path());
    if (not dataFile.is_open())
      continue;

    const auto json = nlohmann::json::parse(dataFile);
    const auto existingGuildName = json["name"].get<std::string>();

    if (std::regex_match(existingGuildName, rg))
      return false;
  }

  return true;
}

void server::FileDataSource::CreateSettings(data::Settings& settings)
{
  settings.uid = ++_settingsSequentialId;
  SaveMetadata();
}

void server::FileDataSource::RetrieveSettings(data::Uid uid, data::Settings& settings)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _settingsDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (!dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Settings file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  settings.uid = json["uid"].get<data::Uid>();

  settings.age = json["age"].get<uint32_t>();
  settings.hideAge = json["hideGenderAndAge"].get<bool>();

  // Keyboard bindings
  {
    const auto& keyboardJson = json["keyboard"];
    const auto& keyboardBindingsJson = keyboardJson["bindings"];
    if (not keyboardBindingsJson.empty())
    {
      auto& keyboardBindings = settings.keyboardBindings().emplace();

      for (const auto& keyboardBindingJson : keyboardBindingsJson)
      {
        keyboardBindings.emplace_back(data::Settings::Option{
          .primaryKey = keyboardBindingJson["primaryKey"].get<uint32_t>(),
          .type = keyboardBindingJson["type"].get<uint32_t>(),
          .secondaryKey = keyboardBindingJson["secondaryKey"].get<uint32_t>()
        });
      }
    }
  }

  // Gamepad bindings
  {
    const auto& gamepadJson = json["gamepad"];
    const auto& gamepadBindingsJson = gamepadJson["bindings"];
    if (not gamepadBindingsJson.empty())
    {
      auto& gamepadBindings = settings.gamepadBindings().emplace();

      for (const auto& gamepadBindingJson : gamepadBindingsJson)
      {
        gamepadBindings.emplace_back(data::Settings::Option{
          .primaryKey = gamepadBindingJson["primaryButton"].get<uint32_t>(),
          .type = gamepadBindingJson["type"].get<uint32_t>(),
          .secondaryKey = gamepadBindingJson["secondaryButton"].get<uint32_t>()
        });
      }
    }
  }

  if (json.contains("macros"))
  {
    const auto& macrosJson = json["macros"];
    settings.macros().emplace() = macrosJson.get<std::array<std::string, 8>>();
  }
}

void server::FileDataSource::StoreSettings(data::Uid uid, const data::Settings& settings)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _settingsDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (!dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Settings file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = settings.uid();

  json["age"] = settings.age();
  json["hideGenderAndAge"] = settings.hideAge();

  // Keyboard bindings
  {
    auto& keyboardJson = json["keyboard"];
    auto& bindings = keyboardJson["bindings"];

    if (settings.keyboardBindings())
    {
      for (auto& bindingRecord : settings.keyboardBindings().value())
      {
        auto& bindingJson = bindings.emplace_back();
        bindingJson["type"] = bindingRecord.type;
        bindingJson["primaryKey"] = bindingRecord.primaryKey;
        bindingJson["secondaryKey"] = bindingRecord.secondaryKey;
      }
    }
  }

  // Gamepad bindings
  {
    auto& gamepadJson = json["gamepad"];
    auto& bindings = gamepadJson["bindings"];

    if (settings.gamepadBindings())
    {
      for (auto& bindingRecord : settings.gamepadBindings().value())
      {
        auto& bindingJson = bindings.emplace_back();
        bindingJson["type"] = bindingRecord.type;
        bindingJson["primaryButton"] = bindingRecord.primaryKey;
        bindingJson["secondaryButton"] = bindingRecord.secondaryKey;
      }
    }
  }

  // Macros
  if (settings.macros())
  {
    json["macros"] = settings.macros().value();
  }

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteSettings(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _settingsDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateMail(data::Mail& mail)
{
  mail.uid = ++_mailSequentialId;
  SaveMetadata();
}

void server::FileDataSource::RetrieveMail(data::Uid uid, data::Mail& mail)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _mailDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (!dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Mail file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  mail.uid = json["uid"].get<data::Uid>();
  mail.from = json["from"].get<data::Uid>();
  mail.to = json["to"].get<data::Uid>();

  mail.isRead = json["isRead"].get<bool>();
  mail.isDeleted = json["isDeleted"].get<bool>();

  mail.type = json["type"].get<data::Mail::MailType>();
  mail.origin = json["origin"].get<data::Mail::MailOrigin>();

  mail.createdAt = data::Clock::time_point(
    std::chrono::seconds(
      json["createdAt"].get<uint64_t>()));
  mail.body = json["body"].get<std::string>();
}

void server::FileDataSource::StoreMail(data::Uid uid, const data::Mail& mail)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _mailDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (!dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Mail file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = mail.uid();
  json["from"] = mail.from();
  json["to"] = mail.to();

  json["isRead"] = mail.isRead();
  json["isDeleted"] = mail.isDeleted();

  json["type"] = mail.type();
  json["origin"] = mail.origin();

  json["createdAt"] = std::chrono::duration_cast<
    std::chrono::seconds>(
      mail.createdAt().time_since_epoch()).count();
  json["body"] = mail.body();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteMail(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _mailDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}
