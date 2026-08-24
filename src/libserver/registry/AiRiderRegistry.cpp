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

#include "libserver/registry/AiRiderRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace server::registry
{

void AiRiderRegistry::Clear()
{
  _presets.clear();
}

void AiRiderRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());
  const auto presets = root["presets"];
  if (not presets)
    return;

  Clear();

  for (const auto& entry : presets)
  {
    const uint8_t diff = entry["difficultyLevel"].as<uint8_t>();
    if (diff == 0)
      continue;
    _presets[diff].push_back(AiRiderPreset{
      .id = entry["id"].as<uint32_t>(),
      .name = entry["name"].as<std::string>(),
      .aiType = entry["aiType"].as<uint8_t>(),
    });
  }

  uint32_t total = 0;
  for (const auto& [d, v] : _presets)
    total += static_cast<uint32_t>(v.size());
  spdlog::info("Loaded {} AI presets across {} difficulty levels", total, _presets.size());
}

const std::vector<AiRiderPreset>& AiRiderRegistry::GetPresetsForDifficulty(
  uint8_t difficulty) const
{
  const auto it = _presets.find(difficulty);
  if (it == _presets.cend())
    throw std::runtime_error(
      std::format("AI presets not found for difficulty: {}", difficulty));
  return it->second;
}

} // namespace server::registry
