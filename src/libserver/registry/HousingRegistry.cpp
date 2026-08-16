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

#include "libserver/registry/HousingRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <ranges>
#include <stdexcept>

namespace server::registry
{

namespace
{

HousingResource ReadResource(const YAML::Node& node)
{
  return HousingResource{
    .itemTid = node["itemTid"].as<data::Tid>(),
    .quantity = node["quantity"].as<uint32_t>()};
}

void ReadHousingInfo(const YAML::Node& node, HousingInfo& info)
{
  info.id = node["id"].as<uint32_t>();
  info.category = node["category"].as<uint32_t>();
  info.property = static_cast<HousingProperty>(node["property"].as<uint32_t>(0));
  info.isDefault = node["isDefault"].as<bool>(false);
  info.isBasic = node["isBasic"].as<bool>(false);
  info.minLevel = node["minLevel"].as<uint32_t>(1);
  info.lifetime = std::chrono::seconds(node["lifetime"].as<uint32_t>(0));

  if (const auto buildResources = node["buildResources"])
  {
    for (const auto& resource : buildResources)
      info.buildResources.emplace_back(ReadResource(resource));
  }

  if (const auto repairResource = node["repairResource"])
    info.repairResource = ReadResource(repairResource);

  info.buildExp = node["buildExp"].as<uint32_t>(0);
  info.regularExp = node["regularExp"].as<uint32_t>(0);
  info.regularMoney = node["regularMoney"].as<uint32_t>(0);

  info.value = node["value"].as<uint32_t>(0);
  info.value2 = node["value2"].as<uint32_t>(0);
  info.value3 = node["value3"].as<uint32_t>(0);
}

void ReadHousingSetInfo(const YAML::Node& node, HousingSetInfo& info)
{
  info.id = node["id"].as<uint32_t>();
  info.regularBonusPercent = node["regularBonusPercent"].as<uint32_t>(0);
  info.housingIds = node["housingIds"].as<std::vector<uint32_t>>();
}

} // anon namespace

void HousingRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  _housing.clear();
  _housingById.clear();
  _sets.clear();
  _ranchLevels.clear();

  for (const auto& node : root["housing"])
  {
    HousingInfo info;
    ReadHousingInfo(node, info);

    if (info.category >= HousingCategoryCount)
    {
      throw std::runtime_error(
        "Housing " + std::to_string(info.id) + " has category "
        + std::to_string(info.category) + ", but a ranch only has "
        + std::to_string(HousingCategoryCount) + " slots");
    }

    _housing.emplace_back(std::move(info));
  }

  std::ranges::sort(_housing, {}, &HousingInfo::id);

  for (size_t index = 0; index < _housing.size(); ++index)
  {
    const auto& [iter, inserted] = _housingById.try_emplace(_housing[index].id, index);
    if (not inserted)
      throw std::runtime_error("Duplicate housing ID " + std::to_string(_housing[index].id));
  }

  for (const auto& node : root["housingSets"])
  {
    HousingSetInfo info;
    ReadHousingSetInfo(node, info);

    // A set naming a housing that does not exist would silently never complete.
    for (const uint32_t housingId : info.housingIds)
    {
      if (not _housingById.contains(housingId))
      {
        throw std::runtime_error(
          "Housing set " + std::to_string(info.id) + " references unknown housing "
          + std::to_string(housingId));
      }
    }

    _sets.emplace_back(std::move(info));
  }

  for (const auto& node : root["ranchLevels"])
  {
    _ranchLevels.emplace_back(RanchLevelInfo{
      .level = node["level"].as<uint32_t>(),
      .expRequired = node["expRequired"].as<uint32_t>()});
  }

  std::ranges::sort(_ranchLevels, {}, &RanchLevelInfo::level);

  spdlog::info(
    "Housing registry loaded {} housing(s), {} set(s) and {} ranch level(s)",
    _housing.size(),
    _sets.size(),
    _ranchLevels.size());
}

const HousingInfo* HousingRegistry::GetHousing(const uint32_t housingId) const
{
  const auto it = _housingById.find(housingId);
  return it != _housingById.cend() ? &_housing[it->second] : nullptr;
}

const std::vector<HousingInfo>& HousingRegistry::GetAllHousing() const
{
  return _housing;
}

std::vector<const HousingInfo*> HousingRegistry::GetHousingByCategory(
  const uint32_t category) const
{
  std::vector<const HousingInfo*> result;
  for (const auto& info : _housing)
  {
    if (info.category == category)
      result.emplace_back(&info);
  }
  return result;
}

std::vector<const HousingInfo*> HousingRegistry::GetDefaultHousing() const
{
  std::vector<const HousingInfo*> result;
  for (const auto& info : _housing)
  {
    if (info.isDefault)
      result.emplace_back(&info);
  }
  return result;
}

const HousingSetInfo* HousingRegistry::GetSet(const uint32_t setId) const
{
  const auto it = std::ranges::find(_sets, setId, &HousingSetInfo::id);
  return it != _sets.cend() ? &*it : nullptr;
}

const std::vector<HousingSetInfo>& HousingRegistry::GetSets() const
{
  return _sets;
}

uint32_t HousingRegistry::GetSetBonusPercent(
  const std::unordered_set<uint32_t>& ownedHousingIds) const
{
  uint32_t bonusPercent = 0;

  for (const auto& set : _sets)
  {
    const bool isComplete = std::ranges::all_of(
      set.housingIds,
      [&ownedHousingIds](const uint32_t housingId)
      {
        return ownedHousingIds.contains(housingId);
      });

    if (isComplete)
      bonusPercent += set.regularBonusPercent;
  }

  return bonusPercent;
}

std::optional<RanchLevelInfo> HousingRegistry::GetRanchLevelInfo(const uint32_t level) const
{
  const auto it = std::ranges::find(_ranchLevels, level, &RanchLevelInfo::level);

  if (it == _ranchLevels.cend())
    return std::nullopt;

  return *it;
}

uint32_t HousingRegistry::GetRanchLevelForExp(const uint32_t totalExp) const
{
  uint32_t currentLevel = 1;
  for (const auto& info : _ranchLevels)
  {
    if (totalExp < info.expRequired)
      break;
    currentLevel = info.level;
  }
  return currentLevel;
}

std::optional<uint32_t> HousingRegistry::GetExpRequiredForRanchLevel(const uint32_t level) const
{
  const auto result = GetRanchLevelInfo(level);
  if (not result)
    return std::nullopt;
  return result->expRequired;
}

} // namespace server::registry
