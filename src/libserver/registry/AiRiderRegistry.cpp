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
  _presetsById.clear();
  _clearRewardCarrots.clear();
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
    AiRiderPreset preset{
      .id = entry["id"].as<uint32_t>(),
      .name = entry["name"].as<std::string>(),
      .aiType = entry["aiType"].as<uint8_t>(),
      .equipmentTids = entry["equipmentTids"].as<std::vector<uint32_t>>(std::vector<uint32_t>{}),
    };

    _presetsById.emplace(preset.id, preset);
  }

  spdlog::info("Loaded {} AI presets", _presetsById.size());

  if (const auto clearRewards = root["clearRewards"])
  {
    for (const auto& entry : clearRewards)
    {
      _clearRewardCarrots.emplace(
        entry["difficultyLevel"].as<uint8_t>(),
        entry["carrots"].as<uint32_t>());
    }
  }
}

const AiRiderPreset& AiRiderRegistry::GetPresetById(uint32_t presetId) const
{
  const auto it = _presetsById.find(presetId);
  if (it == _presetsById.cend())
    throw std::runtime_error(
      std::format("AI preset not found for preset ID: {}", presetId));
  return it->second;
}

uint32_t AiRiderRegistry::GetClearRewardCarrots(uint8_t difficultyLevel) const
{
  const auto it = _clearRewardCarrots.find(difficultyLevel);
  if (it == _clearRewardCarrots.cend())
    throw std::runtime_error(
      std::format("AI clear reward not found for difficulty level: {}", difficultyLevel));
  return it->second;
}

} // namespace server::registry
