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

#include "libserver/registry/FestivalRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <stdexcept>
#include <utility>

namespace server::registry
{

namespace
{

void ReadFestivalSettings(FestivalSettings& settings, const YAML::Node& yaml)
{
  settings.triggerChance = yaml["triggerChance"].as<decltype(FestivalSettings::triggerChance)>(0);
  settings.forcedMissionType.reset();
  if (const auto forcedMissionType = yaml["forcedMissionType"])
    settings.forcedMissionType = forcedMissionType.as<uint32_t>();
}

void ReadFestivalMission(FestivalMission& mission, const YAML::Node& yaml)
{
  mission.type = yaml["type"].as<decltype(FestivalMission::type)>(0);
  mission.name = yaml["name"].as<decltype(FestivalMission::name)>("");
  mission.option = yaml["option"].as<decltype(FestivalMission::option)>("");
  mission.clientCheck = yaml["clientCheck"].as<decltype(FestivalMission::clientCheck)>(false);
  mission.serverCheck = yaml["serverCheck"].as<decltype(FestivalMission::serverCheck)>(false);
  mission.clientFailCheckTime = yaml["clientFailCheckTime"].as<
    decltype(FestivalMission::clientFailCheckTime)>(0.0f);
  mission.playEvent = yaml["playEvent"].as<decltype(FestivalMission::playEvent)>(0);
  mission.eventValue = yaml["eventValue"].as<decltype(FestivalMission::eventValue)>("");
  mission.eventCompare = yaml["eventCompare"].as<decltype(FestivalMission::eventCompare)>(0);
  mission.gameMode = yaml["gameMode"].as<decltype(FestivalMission::gameMode)>(0);
  mission.teamMode = yaml["teamMode"].as<decltype(FestivalMission::teamMode)>(0);

  if (const auto excludedMaps = yaml["excludedMaps"])
  {
    for (const auto& map : excludedMaps)
      mission.excludedMaps.push_back(map.as<uint32_t>());
  }
}

} // anonymous namespace

void FestivalRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());
  const auto festivalSection = root["festival"];
  if (not festivalSection)
    throw std::runtime_error("Missing festival section");

  const auto settingsSection = festivalSection["settings"];
  if (not settingsSection)
    throw std::runtime_error("Missing festival.settings section");

  const auto missionsSection = festivalSection["missions"];
  if (not missionsSection)
    throw std::runtime_error("Missing festival.missions section");

  _missions.clear();

  ReadFestivalSettings(_settings, settingsSection);

  for (const auto& missionNode : missionsSection)
  {
    FestivalMission mission{};
    ReadFestivalMission(mission, missionNode);
    _missions.try_emplace(mission.type, std::move(mission));
  }

  spdlog::info("Festival registry loaded {} festival types", _missions.size());
}

const FestivalSettings& FestivalRegistry::GetSettings() const
{
  return _settings;
}

const FestivalMission* FestivalRegistry::GetMission(uint32_t type) const
{
  const auto iter = _missions.find(type);
  if (iter == _missions.cend())
    return nullptr;
  return &iter->second;
}

const std::unordered_map<uint32_t, FestivalMission>& FestivalRegistry::GetMissions() const
{
  return _missions;
}

} // namespace server::registry
