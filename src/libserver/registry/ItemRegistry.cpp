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

#include <libserver/registry/ItemRegistry.hpp>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <cassert>

namespace server::registry
{

namespace
{
void ReadCharacterPartInfo(
  Item::CharacterPartInfo& characterPartInfo,
  const YAML::Node& yaml)
{
  characterPartInfo.characterId = yaml["characterId"].as<
    decltype(Item::CharacterPartInfo::characterId)>(),
  characterPartInfo.slot = static_cast<Item::CharacterPartInfo::Slot>(
    yaml["slot"].as<uint32_t>());
}

void ReadMountPartInfo(
  Item::MountPartInfo& mountPartInfo,
  const YAML::Node& yaml)
{
  mountPartInfo.slot = static_cast<Item::MountPartInfo::Slot>(
    yaml["slot"].as<uint32_t>());
}

void ReadMountPartSetInfo(
  Item::MountPartSetInfo& mountPartSetInfo,
  const YAML::Node& yaml)
{
  mountPartSetInfo.skinId = yaml["skinId"].as<
    decltype(Item::MountPartSetInfo::skinId)>();
  mountPartSetInfo.maneId = yaml["maneId"].as<
    decltype(Item::MountPartSetInfo::maneId)>();
  mountPartSetInfo.tailId = yaml["tailId"].as<
    decltype(Item::MountPartSetInfo::tailId)>();
  mountPartSetInfo.faceId = yaml["faceId"].as<
    decltype(Item::MountPartSetInfo::faceId)>();
  mountPartSetInfo.scale = yaml["scale"].as<
    decltype(Item::MountPartSetInfo::scale)>();
  mountPartSetInfo.legLength = yaml["legLength"].as<
    decltype(Item::MountPartSetInfo::legLength)>();
  mountPartSetInfo.legVolume = yaml["legVolume"].as<
    decltype(Item::MountPartSetInfo::legVolume)>();
  mountPartSetInfo.bodyLength = yaml["bodyLength"].as<
    decltype(Item::MountPartSetInfo::bodyLength)>();
  mountPartSetInfo.bodyVolume = yaml["bodyVolume"].as<
    decltype(Item::MountPartSetInfo::bodyVolume)>();
  mountPartSetInfo.emblemId = yaml["emblemId"].as<
    decltype(Item::MountPartSetInfo::emblemId)>();
}

void ReadCareParameters(
  Item::CareParameters& careParameters,
  const YAML::Node& yaml)
{
  careParameters.cleanPoints = yaml["cleanPoints"].as<
    decltype(careParameters.cleanPoints)>();
  careParameters.polishPoints = yaml["polishPoints"].as<
    decltype(careParameters.cleanPoints)>();
  careParameters.parts = static_cast<Item::CareParameters::Part>(
    yaml["polishPoints"].as<uint32_t>());
}

void ReadCureParameters(
  Item::CureParameters& cureParameters,
  const YAML::Node& yaml)
{
  cureParameters.injury = yaml["injury"].as<decltype(
    Item::CureParameters::injury)>();
}

void ReadFoodParameters(
  Item::FoodParameters& foodParameters,
  const YAML::Node& yaml)
{
  foodParameters.feedType = static_cast<Item::FoodParameters::FeedType>(
    yaml["feedType"].as<uint32_t>());
  foodParameters.friendlinessPoints = yaml["friendlinessPoints"].as<decltype(
    Item::FoodParameters::friendlinessPoints)>();
  foodParameters.plenitudePoints = yaml["plenitudePoints"].as<decltype(
    Item::FoodParameters::plenitudePoints)>();
  foodParameters.preferenceType = yaml["preferenceType"].as<decltype(
    Item::FoodParameters::preferenceType)>();
}

void ReadPlayParameters(
  Item::PlayParameters& playParameters,
  const YAML::Node& yaml)
{
  playParameters.friendlinessPoints = yaml["friendlinessPoints"].as<decltype(
    Item::PlayParameters::friendlinessPoints)>();
  playParameters.friendlinessPointsOnFailure = yaml["friendlinessPointsOnFailure"].as<decltype(
    Item::PlayParameters::friendlinessPointsOnFailure)>();
  playParameters.charmPoints = yaml["charmPoints"].as<decltype(
    Item::PlayParameters::charmPoints)>();
  playParameters.charmPointsOnFailure = yaml["charmPointsOnFailure"].as<decltype(
    Item::PlayParameters::charmPointsOnFailure)>();
  playParameters.minAttachment = yaml["minAttachment"].as<decltype(
    Item::PlayParameters::minAttachment)>();
  playParameters.maxAttachment = yaml["maxAttachment"].as<decltype(
    Item::PlayParameters::maxAttachment)>();
}

} // anon namespace

void ItemRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto itemsSection = root["items"];
  if (not itemsSection)
    throw std::runtime_error("Missing items section");

  const auto collectionSection = itemsSection["collection"];
  if (not collectionSection)
    throw std::runtime_error("Missing collection section");

  _items.clear();

  for (const auto& itemSection : collectionSection)
  {
    Item item{
      .tid = itemSection["tid"].as<decltype(Item::tid)>(0),
      .name = itemSection["name"].as<decltype(Item::name)>(""),
      .level = itemSection["level"].as<decltype(Item::level)>(0),};

    // Read ItemPartInfo

    // Read CharacterPartInfo
    if (const auto characterPartSection = itemSection["characterPartInfo"])
      ReadCharacterPartInfo(item.characterPartInfo.emplace(), characterPartSection);
    // Read MountPartInfo
    else if (const auto mountPartSection = itemSection["mountPartInfo"])
      ReadMountPartInfo(item.mountPartInfo.emplace(), mountPartSection);
    // Read MountPartSetInfo
    else if (const auto mountPartSetSection = itemSection["mountPartSetInfo"])
      ReadMountPartSetInfo(item.mountPartSetInfo.emplace(), mountPartSetSection);

    // Read ItemParameters

    // Read CareParameters
    if (const auto careParametersSection = itemSection["careParameters"])
      ReadCareParameters(item.careParameters.emplace(), careParametersSection);
    // Read CureParameters
    else if (const auto cureParametersSection = itemSection["cureParameters"])
      ReadCureParameters(item.cureParameters.emplace(), cureParametersSection);
    // Read FoodParameters
    else if (const auto foodParametersSection = itemSection["foodParameters"])
      ReadFoodParameters(item.foodParameters.emplace(), foodParametersSection);
    // Read PlayParameters
    else if (const auto playParametersSection = itemSection["playParameters"])
      ReadPlayParameters(item.playParameters.emplace(), playParametersSection);

    const auto [iter, inserted] = _items.try_emplace(item.tid, item);
    assert(inserted);
  }

  spdlog::info("Item registry loaded {} items", _items.size());
}

std::optional<Item> ItemRegistry::GetItem(uint32_t tid)
{
  const auto itemIter = _items.find(tid);
  if (itemIter == _items.cend())
    return std::nullopt;
  return itemIter->second;
}

} // namespace server::registry
