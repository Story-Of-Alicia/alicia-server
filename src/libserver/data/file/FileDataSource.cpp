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

} // namespace anon

void soa::FileDataSource::Initialize(const std::filesystem::path& path)
{
  _path = path;
  _usersPath = _path / "users";
  create_directories(_usersPath);
  _charactersPath = _path / "characters";
  create_directories(_charactersPath);
  _metaFilePath = _path/ "meta.json";

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
  json["token"] = user.token();
  json["characterUid"] = user.characterUid();

  file << json.dump(2);
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

  file << json.dump(2);
}

void soa::FileDataSource::RetrieveItem(data::Uid uid, data::Item& item)
{

}

void soa::FileDataSource::StoreItem(data::Uid uid, const data::Item& item)
{

}

void soa::FileDataSource::RetrieveHorse(data::Uid uid, data::Horse& horse)
{

}

void soa::FileDataSource::StoreHorse(data::Uid uid, const data::Horse& horse)
{

}

void soa::FileDataSource::RetrieveRanch(data::Uid uid, data::Ranch& ranch)
{

}

void soa::FileDataSource::StoreRanch(data::Uid uid, const data::Ranch& ranch)
{

}
