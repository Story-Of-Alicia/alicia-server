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
  if (yaml["id"])
    mission.id = yaml["id"].as<uint16_t>(0);
  else if (yaml["MissionID"])
    mission.id = yaml["MissionID"].as<uint16_t>(0);

  if (yaml["preMissionId"])
    mission.preMissionId = yaml["preMissionId"].as<uint16_t>(0);
  else if (yaml["PreMissionID"])
    mission.preMissionId = yaml["PreMissionID"].as<uint16_t>(0);

  if (yaml["nextMissionId"])
    mission.nextMissionId = yaml["nextMissionId"].as<uint16_t>(0);
  else if (yaml["NextMissionID"])
    mission.nextMissionId = yaml["NextMissionID"].as<uint16_t>(0);

  if (yaml["mapId"])
    mission.mapId = yaml["mapId"].as<uint16_t>(0);
  else if (yaml["MapID"])
    mission.mapId = yaml["MapID"].as<uint16_t>(0);

  if (yaml["minLevel"])
    mission.minLevel = yaml["minLevel"].as<uint32_t>(0);
  else if (yaml["MinLevel"])
    mission.minLevel = yaml["MinLevel"].as<uint32_t>(0);

  if (yaml["maxLevel"])
    mission.maxLevel = yaml["maxLevel"].as<uint32_t>(0);
  else if (yaml["MaxLevel"])
    mission.maxLevel = yaml["MaxLevel"].as<uint32_t>(0);

  if (yaml["waitingTime"])
    mission.waitingTime = yaml["waitingTime"].as<uint32_t>(0);
  else if (yaml["WaitingTime"])
    mission.waitingTime = yaml["WaitingTime"].as<uint32_t>(0);

  if (yaml["reward"])
    mission.reward = yaml["reward"].as<uint32_t>(0);
  else if (yaml["Reward"])
    mission.reward = yaml["Reward"].as<uint32_t>(0);

  if (yaml["closed"])
    mission.closed = yaml["closed"].as<bool>(false);
  else if (yaml["Closed"])
    mission.closed = (yaml["Closed"].as<uint32_t>(0) != 0);
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
