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

#include "libserver/registry/DailyQuestRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <stdexcept>

namespace server::registry
{

namespace
{

uint8_t ReadDailyQuest(
  const YAML::Node& section,
  DailyQuestInfo& DailyQuest)
{
  DailyQuest.questId = section["questId"].as<decltype(DailyQuest.questId)>();
  DailyQuest.successType = section["successType"].as<decltype(DailyQuest.successType)>();
  DailyQuest.successValue = section["successValue"].as<decltype(DailyQuest.successValue)>();
  DailyQuest.exp = section["exp"].as<decltype(DailyQuest.exp)>();
  DailyQuest.carrots = section["carrots"].as<decltype(DailyQuest.carrots)>();

  return static_cast<uint8_t>(
    section["type"].as<uint32_t>());
}

} // namespace

DailyQuestRegistry::DailyQuestRegistry()
{
}

void DailyQuestRegistry::ReadConfig(
  const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto dailyQuestSection = root["dailyQuests"];
  if (not dailyQuestSection)
    throw std::runtime_error("Missing daily quests section");

  // Game modes
  {
    const auto dailyQuestInfosSection = dailyQuestSection["dailyQuestInfo"];
    if (not dailyQuestInfosSection)
      throw std::runtime_error("Missing daily quest infos section");

    for (const auto& dailyQuestInfosSection : collection)
    {
      DailyQuestInfo dailyQuest;
      const auto type = ReadDailyQuestInfo(dailyQuestInfosSection, dailyQuest);
      _DailyQuestInfo.emplace(type, dailyQuest);
    }
  }

  spdlog::info(
    "Daily quest registry loaded {} daily quests",
    _dailyQuestInfo.size());
}

const DailyQuestInfo& DailyQuestRegistry::GetDailyQuestInfo(
  uint8_t type)
{
  const auto DailyQuestInfo = _dailyQuestInfo.find(type);
  if (dailyQuestInfo == _dailyQuestInfo.cend())
    throw std::runtime_error("Invalid daily quest");
  return DailyQuestInfo->second;
}

} // namespace server::registry
