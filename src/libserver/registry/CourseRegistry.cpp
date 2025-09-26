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

#include "libserver/registry/CourseRegistry.hpp"

#include <yaml-cpp/yaml.h>

#include <stdexcept>

namespace server::registry
{

namespace
{

Course::GameModeInfo::Type ReadGameModeInfo(
  const YAML::Node& section,
  Course::GameModeInfo& gameMode)
{
  gameMode.goodJumpStarPoints = section["goodJumpStarPoints"].as<
    decltype(gameMode.goodJumpStarPoints)>();
  gameMode.perfectJumpStarPoints = section["perfectJumpStarPoints"].as<
    decltype(gameMode.perfectJumpStarPoints)>();
  gameMode.perfectJumpUnitStarPoints = section["perfectJumpUnitStarPoints"].as<
    decltype(gameMode.perfectJumpUnitStarPoints)>();
  gameMode.perfectJumpMaxBonusCombo = section["perfectJumpMaxBonusCombo"].as<
    decltype(gameMode.perfectJumpMaxBonusCombo)>();
  gameMode.perfectSpurCheckTime = section["perfectSpurCheckTime"].as<
    decltype(gameMode.perfectSpurCheckTime)>();
  gameMode.spurConsumeStarPoints = section["spurConsumeStarPoints"].as<
    decltype(gameMode.spurConsumeStarPoints)>();
  gameMode.starPointsMax = section["starPointsMax"].as<
    decltype(gameMode.starPointsMax)>();

  return static_cast<Course::GameModeInfo::Type>(
    section["type"].as<uint32_t>());
}

uint32_t ReadMapBlockInfo(
  const YAML::Node& section,
  Course::MapBlockInfo& mapBlock)
{
  mapBlock.requiredLevel = section["requiredLevel"].as<
    decltype(mapBlock.requiredLevel)>();
  mapBlock.podiumId = section["podiumId"].as<
    decltype(mapBlock.podiumId)>();
  mapBlock.trainingFee = section["trainingFee"].as<
    decltype(mapBlock.trainingFee)>();
  mapBlock.timeLimit = section["timeLimit"].as<
    decltype(mapBlock.timeLimit)>();
  mapBlock.waitTime = section["waitTime"].as<
    decltype(mapBlock.waitTime)>();

  return section["id"].as<uint32_t>();
}

} // anon namespace

CourseRegistry::CourseRegistry()
{
}

void CourseRegistry::ReadConfig(
  const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto coursesSection = root["courses"];
  if (not coursesSection)
    throw std::runtime_error("Missing courses section");

  // Game modes
  {
    const auto gameModeInfosSection = coursesSection["gameModeInfo"];
    if (not gameModeInfosSection)
      throw std::runtime_error("Missing gameModes section");

    const auto collection = gameModeInfosSection["collection"];
    if (not collection)
      throw std::runtime_error("Missing collection section");

    for (const auto& gameModeInfoSection : collection)
    {
      Course::GameModeInfo gameMode;
      const auto type = ReadGameModeInfo(gameModeInfoSection, gameMode);
      _gameModeInfo.emplace(type, gameMode);
    }
  }

  // Map blocks
  {
    const auto mapBlockInfosSection = coursesSection["mapBlockInfo"];
    if (not mapBlockInfosSection)
      throw std::runtime_error("Missing gameModes section");

    const auto collection = mapBlockInfosSection["collection"];
    if (not collection)
      throw std::runtime_error("Missing collection section");

    for (const auto& mapBlockInfoSection : collection)
    {
      Course::MapBlockInfo mapBlock;
      const auto id = ReadMapBlockInfo(mapBlockInfoSection, mapBlock);
      _mapBlockInfo.emplace(id, mapBlock);
    }
  }
}

const Course::GameModeInfo& CourseRegistry::GetCourseGameModeInfo(
  Course::GameModeInfo::Type type)
{
  const auto gameModeInfo = _gameModeInfo.find(type);
  if (gameModeInfo == _gameModeInfo.cend())
    throw std::runtime_error("Invalid course game mode");
  return gameModeInfo->second;
}

const Course::MapBlockInfo& CourseRegistry::GetMapBlockInfo(uint32_t id)
{
  const auto mapBlockInfo = _mapBlockInfo.find(id);
  if (mapBlockInfo == _mapBlockInfo.cend())
    throw std::runtime_error("Invalid course map block");
  return mapBlockInfo->second;
}

} // namespace server::registry
