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

#include "libserver/registry/MissionRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace server::registry
{

namespace
{

void ReadMission(Mission& mission, const YAML::Node& yaml)
{
  mission.id = yaml["id"].as<uint16_t>();
  mission.gameMode = yaml["gameMode"].as<uint8_t>();
  mission.preMissionId = yaml["preMissionId"].as<uint16_t>();
  mission.nextMissionId = yaml["nextMissionId"].as<uint16_t>();
  mission.mapId = yaml["mapId"].as<uint16_t>();
  mission.minLevel = yaml["minLevel"].as<uint32_t>();
  mission.maxLevel = yaml["maxLevel"].as<uint32_t>();
  mission.waitingTime = yaml["waitingTime"].as<uint32_t>();
  mission.reward = yaml["reward"].as<uint32_t>();
  mission.closed = yaml["closed"].as<bool>();
}

} // namespace

void MissionRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  spdlog::info("Reading mission configuration from {}.", configPath.string());

  try
  {
    YAML::Node config = YAML::LoadFile(configPath.string());

    const auto missionsNode = config["missions"];
    if (missionsNode && missionsNode.IsSequence())
    {
      for (const auto& node : missionsNode)
      {
        Mission mission{};
        ReadMission(mission, node);
        _missions[mission.id] = mission;
      }
    }
  }
  catch (const std::exception& ex)
  {
    spdlog::error("Failed to load mission config: {}", ex.what());
    throw;
  }

  spdlog::info("Successfully loaded {} mission entries.", _missions.size());
}

std::optional<Mission> MissionRegistry::GetMission(uint16_t id) const
{
  const auto iter = _missions.find(id);
  if (iter == _missions.end())
  {
    return std::nullopt;
  }
  return iter->second;
}

const std::unordered_map<uint16_t, Mission>& MissionRegistry::GetMissions() const
{
  return _missions;
}

} // namespace server::registry
