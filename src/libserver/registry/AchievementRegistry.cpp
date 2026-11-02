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

#include "libserver/registry/AchievementRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace server::registry
{

namespace
{

void ReadAchievement(Achievement& achievement, const YAML::Node& yaml)
{
  achievement.tid = yaml["tid"].as<decltype(Achievement::tid)>(0);
  achievement.numPlayer = yaml["numPlayer"].as<decltype(Achievement::numPlayer)>(0);
  achievement.gameModeFlag = static_cast<GameModeFlag>(yaml["gameModeFlag"].as<uint32_t>(0));
  achievement.successType = yaml["successType"].as<decltype(Achievement::successType)>(0);
  achievement.compareType = yaml["compareType"].as<decltype(Achievement::compareType)>(0);

  const auto successValues = yaml["successValues"];
  for (std::size_t i = 0; i < achievement.successValues.size(); ++i)
    achievement.successValues[i] = successValues && successValues[i] ? successValues[i].as<uint32_t>(0) : 0;

  achievement.userAchvEvent = yaml["userAchvEvent"].as<decltype(Achievement::userAchvEvent)>(0);
  achievement.function = ParseFunction(yaml["function"].as<std::string>(""));

  const auto functionValues = yaml["functionValues"];
  for (std::size_t i = 0; i < achievement.functionValues.size(); ++i)
    achievement.functionValues[i] = functionValues && functionValues[i] ? functionValues[i].as<uint32_t>(0) : 0;

  achievement.resetUserAchvEvent = yaml["resetUserAchvEvent"].as<decltype(Achievement::resetUserAchvEvent)>(0);
  achievement.resetFunction = ParseFunction(yaml["resetFunction"].as<std::string>(""));
  achievement.resetFunctionValue = yaml["resetFunctionValue"].as<decltype(Achievement::resetFunctionValue)>(0);

  const auto gameMoney = yaml["gameMoney"];
  for (std::size_t i = 0; i < achievement.gameMoney.size(); ++i)
    achievement.gameMoney[i] = gameMoney && gameMoney[i] ? gameMoney[i].as<uint32_t>(0) : 0;

  achievement.achvPoint = yaml["achvPoint"].as<decltype(Achievement::achvPoint)>(0);
}

} // anonymous namespace

void AchievementRegistry::Clear()
{
  _achievements.clear();
}

void AchievementRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto achievementsSection = root["achievements"];
  if (not achievementsSection)
    throw std::runtime_error("Missing achievements section");

  const auto collectionSection = achievementsSection["collection"];
  if (not collectionSection)
    throw std::runtime_error("Missing achievements.collection section");

  Clear();

  for (const auto& achievementNode : collectionSection)
  {
    Achievement achievement{};
    ReadAchievement(achievement, achievementNode);
    _achievements.try_emplace(achievement.tid, achievement);
  }

  spdlog::info("Achievement registry loaded {} achievements", _achievements.size());
}

std::optional<Achievement> AchievementRegistry::GetAchievement(uint32_t tid) const
{
  const auto iter = _achievements.find(tid);
  if (iter == _achievements.cend())
    return std::nullopt;
  return iter->second;
}

const std::unordered_map<uint32_t, Achievement>& AchievementRegistry::GetAchievements() const
{
  return _achievements;
}

} // namespace server::registry
