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

#include "libserver/registry/AchievementRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <stdexcept>

namespace server::registry
{

namespace
{

void ReadAchievement(
  const YAML::Node& section,
  AchievementInfo& info)
{
  info.tid = section["tid"].as<uint16_t>();
  info.eventId = section["eventId"].as<uint16_t>();
  info.function = section["function"].as<std::string>();
  info.successValue = section["successValue"].as<uint32_t>();
  info.bookType = section["bookType"].as<uint8_t>();
  info.reward = section["reward"].as<uint32_t>();
  info.points = section["points"].as<uint32_t>();
}

} // namespace

AchievementRegistry::AchievementRegistry()
{
  // Empty.
}

void AchievementRegistry::ReadConfig(
  const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto achievementsSection = root["achievements"];
  if (not achievementsSection)
    throw std::runtime_error("Missing achievements section");

  const auto collection = achievementsSection["collection"];
  if (not collection)
    throw std::runtime_error("Missing achievements collection");

  for (const auto& entry : collection)
  {
    AchievementInfo info;
    ReadAchievement(entry, info);

    const auto tid = info.tid;
    const auto eventId = info.eventId;

    _achievements.emplace(tid, std::move(info));
    _eventToTids.emplace(eventId, tid);
  }

  spdlog::info(
    "Achievement registry loaded {} achievements",
    _achievements.size());
}

std::vector<const AchievementInfo*>
AchievementRegistry::GetAchievementsByEvent(
  const uint16_t eventId) const
{
  std::vector<const AchievementInfo*> result;
  const auto range = _eventToTids.equal_range(eventId);

  for (auto it = range.first; it != range.second; ++it)
  {
    const auto achievementIt = _achievements.find(it->second);
    if (achievementIt != _achievements.end())
    {
      result.push_back(&achievementIt->second);
    }
  }

  return result;
}

const AchievementInfo* AchievementRegistry::GetAchievement(
  const uint16_t tid) const
{
  const auto it = _achievements.find(tid);
  if (it == _achievements.end())
  {
    return nullptr;
  }
  return &it->second;
}

} // namespace server::registry
