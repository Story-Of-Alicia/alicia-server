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

#include <libserver/registry/CharacterRegistry.hpp>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <stdexcept>

namespace server::registry
{

void CharacterRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto characterSection = root["character"];
  if (not characterSection)
    throw std::runtime_error("Missing character section");

  const auto levelInfoSection = characterSection["levelInfo"];
  if (not levelInfoSection)
    throw std::runtime_error("Missing levelInfo section");

  const auto collectionSection = levelInfoSection["collection"];
  if (not collectionSection)
    throw std::runtime_error("Missing collection section");

  _levelInfo.clear();

  for (const auto& entry : collectionSection)
  {
    CharacterLevelInfo info{
      .level = entry["level"].as<decltype(CharacterLevelInfo::level)>(),
      .expRequired = entry["expRequired"].as<decltype(CharacterLevelInfo::expRequired)>()};
    _levelInfo.push_back(info);
  }

  std::ranges::sort(_levelInfo, {}, &CharacterLevelInfo::level);

  spdlog::info("Character registry loaded {} level entries", _levelInfo.size());
}

std::optional<CharacterLevelInfo> CharacterRegistry::GetLevelInfo(uint32_t level) const
{
  const auto it = std::ranges::find_if(
    _levelInfo,
    [level](const CharacterLevelInfo& info) { return info.level == level; });

  if (it == _levelInfo.cend())
    return std::nullopt;

  return *it;
}

uint32_t CharacterRegistry::GetLevelForExp(uint32_t totalExp) const
{
  uint32_t currentLevel = 1;
  for (const auto& info : _levelInfo)
  {
    if (totalExp < info.expRequired)
      break;
    currentLevel = info.level;
  }
  return currentLevel;
}

std::optional<uint32_t> CharacterRegistry::GetExpRequiredForLevel(uint32_t level) const
{
  const auto result = GetLevelInfo(level);
  if (not result)
    return std::nullopt;
  return result->expRequired;
}

} // namespace server::registry
