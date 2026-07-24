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
  _dailyQuestGroupDataPath = prepareDataPath("dailyQuestGroups");
  _mailDataPath = prepareDataPath("mails");
  _questDataPath = prepareDataPath("quests");
  _stallionDataPath = prepareDataPath("stallions");
  _rewardDataPath = prepareDataPath("rewards");

  // Read the meta-data file and parse the sequential UIDs.
  const std::filesystem::path metaFilePath = ProduceDataFilePath(
    _metaFilePath, "meta");
  std::ifstream metaFile(metaFilePath);
  if (not metaFile.is_open())
  {
    return;
  }

  const auto meta = nlohmann::json::parse(metaFile);
  _infractionSequentialUid = meta.value("infractionSequentialUid", uint32_t{0});
  _characterSequentialUid = meta.value("characterSequentialUid", uint32_t{0});
  _equipmentSequentialUid = meta.value("equipmentSequentialUid", uint32_t{0});
  _storageItemSequentialUid = meta.value("storageItemSequentialUid", uint32_t{0});
  _eggSequentialUid = meta.value("eggSequentialUid", uint32_t{0});
  _petSequentialUid = meta.value("petSequentialUid", uint32_t{0});
  _housingSequentialUid = meta.value("housingSequentialUid", uint32_t{0});
  _guildSequentialId = meta.value("guildSequentialId", uint32_t{0});
  _settingsSequentialId = meta.value("settingsSequentialId", uint32_t{0});
  _dailyQuestGroupSequentialId = meta.value("dailyQuestGroupSequentialId", uint32_t{0});
  _mailSequentialId = meta.value("mailSequentialId", uint32_t{0});
  _questSequentialId = meta.value("questSequentialId", uint32_t{0});
  _stallionSequentialUid = meta.value("stallionSequentialUid", uint32_t{0});
  _rewardSequentialUid = meta.value("rewardSequentialUid", uint32_t{0});
}

void server::FileDataSource::Terminate()
{
  SaveMetadata();
}

void server::FileDataSource::SaveMetadata()
{
  // dirty fix to make this thread safe
  static std::mutex dirty;
  std::scoped_lock fix(dirty);

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
  meta["dailyQuestGroupSequentialId"] = _dailyQuestGroupSequentialId.load();
  meta["mailSequentialId"] = _mailSequentialId.load();
  meta["questSequentialId"] = _questSequentialId.load();
  meta["stallionSequentialUid"] = _stallionSequentialUid.load();
  meta["rewardSequentialUid"] = _rewardSequentialUid.load();

  metaFile << meta.dump(2);
}

void server::FileDataSource::CreateUser(data::User& user)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _userDataPath, user.name());

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
  user.name = json.value("name", std::string{});
  user.token = json.value("token", std::string{});
  user.characterUid = json.value("characterUid", data::Uid{});
  user.infractions = json.value("infractions", std::vector<data::Uid>{});
  user.lastSeenOnline = data::Clock::time_point(std::chrono::seconds(
    json.value("lastSeenOnline", int64_t(0))));
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
  json["lastSeenOnline"] = std::chrono::ceil<std::chrono::seconds>(
    user.lastSeenOnline().time_since_epoch()).count();

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
  infraction.uid = json.value("uid", data::Uid{});
  infraction.description = json.value("description", std::string{});
  infraction.punishment = json.value("punishment", data::Infraction::Punishment{});
  infraction.duration = std::chrono::seconds(
    json.value("duration", int64_t{}));
  infraction.createdAt = data::Clock::time_point(std::chrono::seconds(
    json.value("createdAt", int64_t{})));
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

  character.uid = json.value("uid", data::Uid{});
  character.name = json.value("name", std::string{});

  character.introduction = json.value("introduction", std::string{});

  character.level = json.value("level", uint32_t{});
  character.experience = json.value("experience", uint32_t{});
  character.carrots = json.value("carrots", int32_t{});
  character.cash = json.value("cash", uint32_t{});

  character.role = static_cast<data::Character::Role>(
    json.value("role", uint32_t{}));

  const auto& parts = json.value("parts", nlohmann::json::object());
  character.parts = data::Character::Parts{
    .modelId = parts.value("modelId", data::Uid{}),
    .mouthId = parts.value("mouthId", data::Uid{}),
    .faceId = parts.value("faceId", data::Uid{})};

  const auto& appearance = json.value("appearance", nlohmann::json::object());
  character.appearance = data::Character::Appearance{
    .voiceId = appearance.value("voiceId", uint32_t{}),
    .headSize = appearance.value("headSize", uint32_t{}),
    .height = appearance.value("height", uint32_t{}),
    .thighVolume = appearance.value("thighVolume", uint32_t{}),
    .legVolume = appearance.value("legVolume", uint32_t{}),
    .emblemId = appearance.value("emblemId", uint32_t{})};

  character.guildUid = json.value("guildUid", data::Uid{});

  const auto& contacts = json.value("contacts", nlohmann::json::object());
  character.contacts.pending = contacts.value("pending", std::set<data::Uid>{});

  for (const auto& groupJson : contacts.value("groups", nlohmann::json::array()))
  {
    data::Character::Contacts::Group group{
      .uid = groupJson.value("uid", data::Uid{}),
      .name = groupJson.value("name", std::string{}),
      .members = groupJson.value("members", std::set<data::Uid>{}),
      .createdAt = data::Clock::time_point(std::chrono::seconds(
          groupJson.value("createdAt", int64_t{})))
    };

    character.contacts.groups().try_emplace(group.uid, group);
  }

  character.gifts = json.value("gifts", std::vector<data::Uid>{});
  character.purchases = json.value("purchases", std::vector<data::Uid>{});

  character.inventory = json.value("inventory", std::vector<data::Uid>{});
  character.characterEquipment = json.value("characterEquipment", std::vector<data::Uid>{});
  // todo: rename after larger refactor
  character.expiredEquipment = json.value("horseEquipment", std::vector<data::Uid>{});

  character.horses = json.value("horses", std::vector<data::Uid>{});
  character.horseSlotCount = json.value("horseSlotCount", uint8_t{});

  character.pets = json.value("pets", std::vector<data::Uid>{});
  character.mountUid = json.value("mountUid", data::Uid{});
  character.petUid = json.value("petUid", data::Uid{});

  character.eggs = json.value("eggs", std::vector<data::Uid>{});

  character.housing = json.value("housing", std::vector<data::Uid>{});

  character.isRanchLocked = json.value("isRanchLocked", bool{});

  character.settingsUid = json.value("settingsUid", data::Uid{});

  const auto readSkills = [](data::Character::Skills::Sets& sets, const nlohmann::json& json)
  {
    const auto readSkillSet = [](data::Character::Skills::Sets::Set& set, const nlohmann::json& json)
    {
      set.slot1 = json.value("slot1", uint32_t{});
      set.slot2 = json.value("slot2", uint32_t{});
    };

    readSkillSet(sets.set1, json.value("set1", nlohmann::json::object()));
    readSkillSet(sets.set2, json.value("set2", nlohmann::json::object()));
    sets.activeSetId = json.value("activeSetId", uint32_t{});
  };

  const auto& skills = json.value("skills", nlohmann::json::object());
  readSkills(character.skills.speed(), skills.value("speed", nlohmann::json::object()));
  readSkills(character.skills.magic(), skills.value("magic", nlohmann::json::object()));

  character.dailyQuestGroupUid = json.value("dailyQuestGroupUid", data::InvalidUid);
  const auto& mailbox = json.value("mailbox", nlohmann::json::object());
  character.mailbox.hasNewMail = mailbox.value("hasNewMail", bool{});
  character.mailbox.inbox = mailbox.value("inbox", std::vector<data::Uid>{});
  character.mailbox.sent = mailbox.value("sent", std::vector<data::Uid>{});

  character.quests = json.value("quests", std::vector<data::Uid>{});
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
  json["experience"] = character.experience();
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

  json["dailyQuestGroupUid"] = character.dailyQuestGroupUid();
  nlohmann::json mailbox;
  mailbox["hasNewMail"] = character.mailbox.hasNewMail();
  mailbox["inbox"] = character.mailbox.inbox();
  mailbox["sent"] = character.mailbox.sent();
  json["mailbox"] = mailbox;

  json["quests"] = character.quests();

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
  horse.uid = json.value("uid", data::Uid{});
  horse.tid = json.value("tid", data::Tid{});
  horse.name = json.value("name", std::string{});

  const auto& parts = json.value("parts", nlohmann::json::object());
  horse.parts = data::Horse::Parts{
    .skinTid = parts.value("skinId", uint32_t{}),
    .faceTid = parts.value("faceId", uint32_t{}),
    .maneTid = parts.value("maneId", uint32_t{}),
    .tailTid = parts.value("tailId", uint32_t{})};

  const auto& appearance = json.value("appearance", nlohmann::json::object());
  horse.appearance = data::Horse::Appearance{
    .scale = appearance.value("scale", uint32_t{}),
    .legLength = appearance.value("legLength", uint32_t{}),
    .legVolume = appearance.value("legVolume", uint32_t{}),
    .bodyLength = appearance.value("bodyLength", uint32_t{}),
    .bodyVolume = appearance.value("bodyVolume", uint32_t{})};

  const auto& stats = json.value("stats", nlohmann::json::object());
  horse.stats = data::Horse::Stats{
    .agility = stats.value("agility", uint32_t{}),
    .courage = stats.value("courage", uint32_t{}),
    .rush = stats.value("rush", uint32_t{}),
    .endurance = stats.value("endurance", uint32_t{}),
    .ambition = stats.value("ambition", uint32_t{})};

  const auto& mastery = json.value("mastery", nlohmann::json::object());
  horse.mastery = data::Horse::Mastery{
    .spurMagicCount = mastery.value("spurMagicCount", uint32_t{}),
    .jumpCount = mastery.value("jumpCount", uint32_t{}),
    .slidingTime = mastery.value("slidingTime", uint32_t{}),
    .glidingDistance = mastery.value("glidingDistance", uint32_t{})};

  const auto& mountCondition = json.value("mountCondition", nlohmann::json::object());
  horse.mountCondition = data::Horse::MountCondition{
    .stamina = mountCondition.value("stamina", uint32_t{}),
    .charm = mountCondition.value("charm", uint32_t{}),
    .friendliness = mountCondition.value("friendliness", uint32_t{}),
    .injury = mountCondition.value("injury", uint32_t{}),
    .plenitude = mountCondition.value("plenitude", uint32_t{}),
    .bodyDirtiness = mountCondition.value("bodyDirtiness", uint32_t{}),
    .maneDirtiness = mountCondition.value("maneDirtiness", uint32_t{}),
    .tailDirtiness = mountCondition.value("tailDirtiness", uint32_t{}),
    .bodyPolish = mountCondition.value("bodyPolish", uint32_t{}),
    .manePolish = mountCondition.value("manePolish", uint32_t{}),
    .tailPolish = mountCondition.value("tailPolish", uint32_t{}),
    .attachment = mountCondition.value("attachment", uint32_t{}),
    .boredom = mountCondition.value("boredom", uint32_t{}),
    .stopAmendsPoint = mountCondition.value("stopAmendsPoint", uint32_t{})};

  horse.rating = json.value("rating", uint32_t{});
  horse.clazz = json.value("clazz", uint32_t{});
  horse.clazzProgress = json.value("clazzProgress", uint32_t{});
  horse.grade = json.value("grade", uint32_t{});
  horse.growthPoints = json.value("growthPoints", uint32_t{});

  const auto& potential = json.value("potential", nlohmann::json::object());
  horse.potential = data::Horse::Potential{
    .type = potential.value("type", uint32_t{}),
    .level = potential.value("level", uint32_t{}),
    .value = potential.value("value", uint32_t{})
  };

  horse.luckState = json.value("luckState", uint32_t{});
  horse.fatigue = json.value("fatigue", uint32_t{});
  horse.emblemUid = json.value("emblem", uint32_t{});
  horse.tendency = json.value("tendency", uint32_t{});
  horse.spirit = json.value("spirit", uint32_t{});

  horse.type = json.value("type", data::Horse::Type{});
  horse.breedingCount = json.value("breedingCount", uint32_t{});
  horse.breedingCombo = json.value("breedingCombo", uint32_t{});
  horse.lineage = json.value("lineage", uint32_t{1});

  const auto& ancestors = json.value("ancestors", nlohmann::json::object());
  horse.ancestors = data::Horse::Ancestors{
    .father = ancestors.value("father", data::Uid{data::InvalidUid}),
    .mother = ancestors.value("mother", data::Uid{data::InvalidUid})};

  horse.dateOfBirth = data::Clock::time_point(std::chrono::seconds(
    json.value("dateOfBirth", uint64_t{})));

  const auto& mountInfo = json.value("mountInfo", nlohmann::json::object());
  horse.mountInfo = data::Horse::MountInfo{
    .boostsInARow = mountInfo.value("boostsInARow", uint32_t{}),
    .winsSpeedSingle = mountInfo.value("winsSpeedSingle", uint32_t{}),
    .winsSpeedTeam = mountInfo.value("winsSpeedTeam", uint32_t{}),
    .winsMagicSingle = mountInfo.value("winsMagicSingle", uint32_t{}),
    .winsMagicTeam = mountInfo.value("winsMagicTeam", uint32_t{}),
    .totalDistance = mountInfo.value("totalDistance", uint32_t{}),
    .topSpeed = mountInfo.value("topSpeed", uint32_t{}),
    .longestGlideDistance = mountInfo.value("longestGlideDistance", uint32_t{}),
    .participated = mountInfo.value("participated", uint32_t{}),
    .cumulativePrize = mountInfo.value("cumulativePrize", uint32_t{}),
    .biggestPrize = mountInfo.value("biggestPrize", uint32_t{})};
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
  
  json["breedingCount"] = horse.breedingCount();
  json["breedingCombo"] = horse.breedingCombo();

  json["type"] = horse.type();
  json["dateOfBirth"] = std::chrono::ceil<std::chrono::seconds>(
    horse.dateOfBirth().time_since_epoch()).count();

  json["tendency"] = horse.tendency();
  json["spirit"] = horse.spirit();

  nlohmann::json potential;
  potential["type"] = horse.potential.type();
  potential["level"] = horse.potential.level();
  potential["value"] = horse.potential.value();
  json["potential"] = potential;

  json["luckState"] = horse.luckState();
  json["fatigue"] = horse.fatigue();
  json["emblem"] = horse.emblemUid();

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

  nlohmann::json ancestorsJson;
  ancestorsJson["father"] = horse.ancestors.father;
  ancestorsJson["mother"] = horse.ancestors.mother;
  json["ancestors"] = ancestorsJson;

  json["lineage"] = horse.lineage();

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

  item.uid = json.value("uid", data::Uid{});
  item.tid = json.value("tid", data::Tid{});
  item.count = json.value("count", uint32_t{});
  item.duration = std::chrono::seconds(json.value("duration", int64_t{}));
  item.createdAt = data::Clock::time_point(
    std::chrono::seconds(json.value("createdAt", int64_t{})));
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

  storageItem.uid = json.value("uid", data::Uid{});
  storageItem.sender = json.value("sender", std::string{});
  storageItem.message = json.value("message", std::string{});
  storageItem.carrots = json.value("carrots", int32_t{});

  for (const auto& itemJson : json.value("items", nlohmann::json::array()))
  {
    storageItem.items().emplace_back(data::StorageItem::Item{
      .tid = itemJson.value("tid", data::Tid{}),
      .count = itemJson.value("count", uint32_t{}),
      .duration = std::chrono::seconds(
        itemJson.value("duration", int64_t{})),});
  }

  storageItem.checked = json.value("checked", bool{});
  storageItem.duration = std::chrono::seconds(
    json.value("duration", int64_t{}));
  storageItem.createdAt = data::Clock::time_point(std::chrono::seconds(
    json.value("createdAt", int64_t{})));

  // Shop data
  storageItem.goodsSq = json.value("goodsSq", uint32_t{});
  storageItem.priceId = json.value("priceId", uint32_t{});
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

  egg.uid = json.value("uid", data::Uid{});
  egg.itemUid = json.value("itemUid", data::Uid{});
  egg.itemTid = json.value("itemTid", data::Tid{});

  egg.incubatedAt = data::Clock::time_point(
    std::chrono::seconds(
      json.value("incubatedAt", uint64_t{})));
  egg.incubatorSlot = json.value("incubatorSlot", uint32_t{});
  egg.boostsUsed = json.value("boostsUsed", uint32_t{});
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

  pet.uid = json.value("uid", data::Uid{});
  pet.itemUid = json.value("itemUid", data::Uid{});
  pet.petId = json.value("petId", data::Uid{});
  pet.name = json.value("name", std::string{});
  pet.birthDate = data::Clock::time_point(std::chrono::seconds(
    json.value("birthDate", uint64_t{})));
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
  housing.uid = json.value("uid", data::Uid{});
  housing.housingId = json.value("housingId", uint32_t{});
  housing.expiresAt = data::Clock::time_point(
    std::chrono::seconds(json.value("expiresAt", uint64_t{})));
  housing.durability = json.value("durability", uint32_t{});
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

  guild.uid = json.value("uid", data::Uid{});
  guild.name = json.value("name", std::string{});
  guild.description = json.value("description", std::string{});
  guild.owner = json.value("owner", data::Uid{});
  guild.officers = json.value("officers", std::vector<data::Uid>{});
  guild.members = json.value("members", std::vector<data::Uid>{});

  guild.rank = json.value("rank", uint32_t{});
  guild.totalWins = json.value("totalWins", uint32_t{});
  guild.totalLosses = json.value("totalLosses", uint32_t{});
  guild.seasonalWins = json.value("seasonalWins", uint32_t{});
  guild.seasonalLosses = json.value("seasonalLosses", uint32_t{});
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
  settings.uid = json.value("uid", data::Uid{});

  settings.age = json.value("age", uint32_t{});
  settings.hideAge = json.value("hideGenderAndAge", bool{});

  // Keyboard bindings
  {
    const auto& keyboardJson = json.value("keyboard", nlohmann::json::object());
    const auto& keyboardBindingsJson = keyboardJson.value("bindings", nlohmann::json::array());
    if (not keyboardBindingsJson.empty())
    {
      auto& keyboardBindings = settings.keyboardBindings().emplace();

      for (const auto& keyboardBindingJson : keyboardBindingsJson)
      {
        keyboardBindings.emplace_back(data::Settings::Option{
          .primaryKey = keyboardBindingJson.value("primaryKey", uint32_t{}),
          .type = keyboardBindingJson.value("type", uint32_t{}),
          .secondaryKey = keyboardBindingJson.value("secondaryKey", uint32_t{})
        });
      }
    }
  }

  // Gamepad bindings
  {
    const auto& gamepadJson = json.value("gamepad", nlohmann::json::object());
    const auto& gamepadBindingsJson = gamepadJson.value("bindings", nlohmann::json::array());
    if (not gamepadBindingsJson.empty())
    {
      auto& gamepadBindings = settings.gamepadBindings().emplace();

      for (const auto& gamepadBindingJson : gamepadBindingsJson)
      {
        gamepadBindings.emplace_back(data::Settings::Option{
          .primaryKey = gamepadBindingJson.value("primaryButton", uint32_t{}),
          .type = gamepadBindingJson.value("type", uint32_t{}),
          .secondaryKey = gamepadBindingJson.value("secondaryButton", uint32_t{})
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

void server::FileDataSource::CreateDailyQuestGroup(data::DailyQuestGroup& group)
{
  group.uid = ++_dailyQuestGroupSequentialId;
  SaveMetadata();
}

void server::FileDataSource::RetrieveDailyQuestGroup(data::Uid uid, data::DailyQuestGroup& group)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _dailyQuestGroupDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Daily quest group file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  group.uid          = json.value("uid", data::Uid{});
  group.rewardId     = json.value("rewardId", uint8_t{});
  group.rewardType   = json.value("rewardType", uint8_t{});
  group.rewardPoints = json.value("rewardPoints", uint32_t{0});
  // Support both boolean `true`/`false` and legacy integer `1`/`0` representations.
  if (const auto it = json.find("carrotsClaimed"); it != json.end())
    group.carrotsClaimed = it->is_boolean() ? it->get<bool>() : (it->get<int>() != 0);
  else
    group.carrotsClaimed = false;

  std::array<data::DailyQuestEntry, 3> quests{};
  const auto& questsJson = json.value("quests", nlohmann::json::array());
  for (size_t i = 0; i < questsJson.size() && i < 3; ++i)
  {
    quests[i].questId  = questsJson[i].value("questId", uint16_t{});
    quests[i].progress = questsJson[i].value("progress", uint32_t{});
  }
  group.quests = quests;
}

void server::FileDataSource::StoreDailyQuestGroup(data::Uid uid, const data::DailyQuestGroup& group)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _dailyQuestGroupDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Daily quest group file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"]          = group.uid();
  json["rewardId"]     = group.rewardId();
  json["rewardType"]   = group.rewardType();
  json["rewardPoints"] = group.rewardPoints();
  json["carrotsClaimed"] = static_cast<bool>(group.carrotsClaimed());

  nlohmann::json questsJson = nlohmann::json::array();
  for (const auto& entry : group.quests())
  {
    questsJson.push_back({
      {"questId",  entry.questId},
      {"progress", entry.progress}
    });
  }
  json["quests"] = questsJson;
  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteDailyQuestGroup(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _dailyQuestGroupDataPath, std::format("{}", uid));
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
  mail.uid = json.value("uid", data::Uid{});
  mail.from = json.value("from", data::Uid{});
  mail.to = json.value("to", data::Uid{});

  mail.isRead = json.value("isRead", bool{});
  mail.isDeleted = json.value("isDeleted", bool{});

  mail.type = json.value("type", data::Mail::MailType{});
  mail.claimUid = json.value("claimUid", data::Uid{});

  mail.createdAt = data::Clock::time_point(
    std::chrono::seconds(
      json.value("createdAt", uint64_t{})));
  mail.body = json.value("body", std::string{});
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
  json["claimUid"] = mail.claimUid();

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

void server::FileDataSource::CreateQuest(data::Quest& quest)
{
  quest.uid = ++_questSequentialId;
  SaveMetadata();
}

void server::FileDataSource::RetrieveQuest(data::Uid uid, data::Quest& quest)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _questDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Quest file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  quest.uid         = json.value("uid", data::Uid{});
  quest.questId     = json.value("questId", uint32_t{});
  quest.isCompleted = json.value("isCompleted", data::Quest::Status{});
  quest.progress    = json.value("progress", uint32_t{});
}

void server::FileDataSource::StoreQuest(data::Uid uid, const data::Quest& quest)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _questDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Quest file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"]         = quest.uid();
  json["questId"]     = quest.questId();
  json["isCompleted"] = static_cast<uint32_t>(quest.isCompleted());
  json["progress"]    = quest.progress();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteQuest(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _questDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

void server::FileDataSource::CreateStallion(data::Stallion& stallion)
{
  stallion.uid = ++_stallionSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveStallion(data::Uid uid, data::Stallion& stallion)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _stallionDataPath, std::format("{}", uid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Stallion file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  stallion.uid() = json.value("uid", data::InvalidUid);
  stallion.horseUid() = json.value("horseUid", data::InvalidUid);
  stallion.ownerUid() = json.value("ownerUid", data::InvalidUid);
  stallion.breedingCharge() = json.value("breedingCharge", data::InvalidUid);
  stallion.timesMated() = json.value("timesMated", uint32_t{0});
  stallion.registeredAt() = data::Clock::time_point(
    std::chrono::seconds(json.value("registeredAt", int64_t{0})));
  stallion.expiresAt() = data::Clock::time_point(
    std::chrono::seconds(json.value("expiresAt", int64_t{0})));
}

void server::FileDataSource::StoreStallion(data::Uid uid, const data::Stallion& stallion)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _stallionDataPath, std::format("{}", uid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Stallion file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["uid"] = stallion.uid();
  json["horseUid"] = stallion.horseUid();
  json["ownerUid"] = stallion.ownerUid();
  json["breedingCharge"] = stallion.breedingCharge();
  json["timesMated"] = stallion.timesMated();
  json["registeredAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    stallion.registeredAt().time_since_epoch()).count();
  json["expiresAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    stallion.expiresAt().time_since_epoch()).count();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteStallion(data::Uid uid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _stallionDataPath, std::format("{}", uid));
  std::filesystem::remove(dataFilePath);
}

std::vector<server::data::Uid> server::FileDataSource::ListRegisteredStallions()
{
  std::vector<data::Uid> stallionUids;

  if (!std::filesystem::exists(_stallionDataPath))
  {
    return stallionUids;
  }

  for (const auto& entry : std::filesystem::directory_iterator(_stallionDataPath))
  {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
      continue;

    try
    {
      // Extract stallion UID from filename (e.g., "123.json" -> 123)
      data::Uid stallionUid = std::stoul(entry.path().stem().string());
      stallionUids.push_back(stallionUid);
    }
    catch (const std::exception&)
    {
      // Silently skip invalid filenames
    }
  }

  return stallionUids;
}

void server::FileDataSource::CreateReward(data::Reward& reward)
{
  reward.claimUid = ++_rewardSequentialUid;
  SaveMetadata();
}

void server::FileDataSource::RetrieveReward(data::Uid claimUid, data::Reward& reward)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _rewardDataPath, std::format("{}", claimUid));

  std::ifstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Reward file '{}' not accessible", dataFilePath.string()));
  }

  const auto json = nlohmann::json::parse(dataFile);
  reward.claimUid() = json.value("claimUid", data::InvalidUid);
  reward.characterUid() = json.value("characterUid", data::InvalidUid);
  reward.type() = static_cast<data::Reward::Type>(json.value("type", uint32_t{0}));
  reward.carrots() = json.value("carrots", uint32_t{0});
  reward.isClaimed() = json.value("isClaimed", false);
  reward.createdAt() = data::Clock::time_point(
    std::chrono::seconds(json.value("createdAt", int64_t{0})));
  reward.claimedAt() = data::Clock::time_point(
    std::chrono::seconds(json.value("claimedAt", int64_t{0})));
}

void server::FileDataSource::StoreReward(data::Uid claimUid, const data::Reward& reward)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _rewardDataPath, std::format("{}", claimUid));

  std::ofstream dataFile(dataFilePath);
  if (not dataFile.is_open())
  {
    throw std::runtime_error(
      std::format("Reward file '{}' not accessible", dataFilePath.string()));
  }

  nlohmann::json json;
  json["claimUid"] = reward.claimUid();
  json["characterUid"] = reward.characterUid();
  json["type"] = static_cast<uint32_t>(reward.type());
  json["carrots"] = reward.carrots();
  json["isClaimed"] = reward.isClaimed();
  json["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    reward.createdAt().time_since_epoch()).count();
  json["claimedAt"] = std::chrono::duration_cast<std::chrono::seconds>(
    reward.claimedAt().time_since_epoch()).count();

  dataFile << json.dump(2);
}

void server::FileDataSource::DeleteReward(data::Uid claimUid)
{
  const std::filesystem::path dataFilePath = ProduceDataFilePath(
    _rewardDataPath, std::format("{}", claimUid));
  std::filesystem::remove(dataFilePath);
}
