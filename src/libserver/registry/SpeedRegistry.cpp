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

#include <libserver/registry/SpeedRegistry.hpp>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <format>
#include <stdexcept>

namespace server::registry
{

void SpeedRegistry::Clear()
{
  _teamSpurGaugeInfoMap.clear();
}

void SpeedRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto speedSection = root["speed"];
  if (not speedSection)
    throw std::runtime_error("Missing speed section in speed.yaml");

  const auto teamSpurGaugeInfoSection = speedSection["teamSpurGaugeInfo"];
  if (not teamSpurGaugeInfoSection)
    throw std::runtime_error("Missing teamSpurGaugeInfo section in speed.yaml");

  Clear();

  for (const auto& entry : teamSpurGaugeInfoSection)
  {
    const auto racerCount = entry.first.as<uint32_t>();
    const auto& val = entry.second;

    TeamSpurGaugeInfo info{
      .racerCount = racerCount,
      .teamSpurMax = val["teamSpurMax"].as<float>(),
      .winTeamSpurConsumeRate = val["winTeamSpurConsumeRate"].as<float>(),
      .loseTeamSpurConsumeRate = val["loseTeamSpurConsumeRate"].as<float>(),
      .points = val["points"].as<std::vector<float>>(),
      .fillRates = val["fillRates"].as<std::vector<float>>(),
      .reduceWaitTime = val["reduceWaitTime"].as<uint32_t>(),
      .reduceDelay = val["reduceDelay"].as<uint32_t>(),
      .reduceRate = val["reduceRate"].as<float>(),
      .goodTiming = val["goodTiming"].as<uint32_t>()
    };
    _teamSpurGaugeInfoMap[racerCount] = info;
  }

  spdlog::info("Speed registry loaded {} TeamSpurGaugeInfo entries", _teamSpurGaugeInfoMap.size());
}

const TeamSpurGaugeInfo& SpeedRegistry::GetTeamSpurGaugeInfo(uint32_t racerCount) const
{
  const auto it = _teamSpurGaugeInfoMap.find(racerCount);
  if (it == _teamSpurGaugeInfoMap.cend())
  {
    throw std::runtime_error(
      std::format("TeamSpurGaugeInfo for racer count {} not found in speed.yaml", racerCount));
  }
  return it->second;
}

} // namespace server::registry
