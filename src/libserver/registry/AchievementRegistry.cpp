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

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace server::registry
{

namespace
{

//! Reads a fixed size sequence, tolerating a shorter or missing one so that
//! achievements which only define the first tiers stay valid.
//! @param node The sequence node.
//! @param values The destination, entries beyond the sequence stay zero.
void ReadTierValues(
  const YAML::Node& node,
  std::array<uint32_t, AchievementTierCount>& values)
{
  if (not node)
    return;

  const auto count = std::min<size_t>(node.size(), values.size());
  for (size_t tier = 0; tier < count; ++tier)
  {
    values[tier] = node[tier].as<uint32_t>(0);
  }
}

void ReadAchievement(
  const YAML::Node& section,
  AchievementInfo& info)
{
  info.tid = section["tid"].as<uint16_t>();
  // Until a condition reading names another event, an achievement is measured
  // by the one that triggers it.
  info.measuredEventId = section["eventId"].as<uint16_t>();
  info.function = section["function"].as<std::string>("");
  info.functionValue = section["functionValue"].as<uint32_t>(0);
  info.resetFunction = section["resetFunction"].as<std::string>("");
  info.resetEventId = static_cast<uint16_t>(
    section["resetEventId"].as<uint32_t>(0));
  info.compareType = static_cast<CompareType>(
    section["compareType"].as<uint32_t>(0));
  info.gameModeFlag = static_cast<uint8_t>(
    section["gameModeFlag"].as<uint32_t>(0));
  info.numPlayer = static_cast<uint8_t>(section["numPlayer"].as<uint32_t>(0));
  // yaml-cpp resolves the narrow character types as characters, so read the
  // signed field as an int and narrow it afterwards.
  info.bookType = static_cast<int8_t>(section["bookType"].as<int32_t>(0));

  ReadTierValues(section["successValues"], info.successValues);
  ReadTierValues(section["rewards"], info.rewards);
}

//! Turns the name a condition reading carries into its kind.
//! @param name The name from the configuration.
//! @returns The kind, Direct for an unknown or missing name.
ConditionKind ParseConditionKind(const std::string& name)
{
  if (name == "never")
    return ConditionKind::Never;
  if (name == "atLeast")
    return ConditionKind::AtLeast;
  if (name == "atMost")
    return ConditionKind::AtMost;

  return ConditionKind::Direct;
}

void ReadBookReward(
  const YAML::Node& section,
  AchievementBookRewardInfo& info)
{
  info.bookType = static_cast<int8_t>(section["bookType"].as<int32_t>(0));
  info.characterModelId = static_cast<uint8_t>(
    section["characterModelId"].as<uint32_t>(0));

  ReadTierValues(section["itemTids"], info.itemTids);
}

} // namespace

GameModeFlag ToGameModeFlag(
  const bool isMagic,
  const bool isTeam,
  const bool isMission)
{
  if (isMission)
    return GameModeFlag::Mission;

  if (isMagic)
    return isTeam ? GameModeFlag::MagicTeam : GameModeFlag::MagicSingle;

  return isTeam ? GameModeFlag::SpeedTeam : GameModeFlag::SpeedSingle;
}

bool AchievementInfo::IsLowerBetter() const
{
  return compareType == CompareType::Minimum;
}

bool AchievementInfo::IsGated() const
{
  return conditionKind == ConditionKind::AtLeast
    or conditionKind == ConditionKind::AtMost;
}

bool AchievementInfo::PassesBar(const uint32_t value) const
{
  if (conditionKind == ConditionKind::AtLeast)
    return value >= conditionBar;

  if (conditionKind == ConditionKind::AtMost)
    return value != 0 and value <= conditionBar;

  return true;
}

bool AchievementInfo::CountsInMode(const GameModeFlag mode) const
{
  // 94 of the 246 achievements name no mode, and those count everywhere.
  if (gameModeFlag == 0)
    return true;

  return (gameModeFlag & static_cast<uint8_t>(mode)) != 0;
}

uint8_t AchievementInfo::GetReachedTierCount(const uint32_t progress) const
{
  const bool lowerIsBetter = IsLowerBetter();

  uint8_t reached = 0;
  for (size_t tier = 0; tier < successValues.size(); ++tier)
  {
    // 114 achievements carry no threshold and reward only the first tier, so a
    // missing first threshold counts as one. On a later tier it means the
    // achievement does not use that tier.
    uint32_t threshold = successValues[tier];
    if (threshold == 0)
    {
      if (tier > 0)
        break;

      threshold = 1;
    }

    // Where less is better a progress of zero means nothing was reported yet
    // rather than a perfect result.
    const bool met = lowerIsBetter
      ? progress != 0 and progress <= threshold
      : progress >= threshold;

    if (not met)
      break;

    ++reached;
  }

  return reached;
}

uint32_t AchievementInfo::CombineProgress(
  const uint32_t stored,
  const uint32_t reported,
  const uint32_t lastReported) const
{
  switch (compareType)
  {
    case CompareType::Minimum:
    {
      // A lower value wins, but zero stands for "nothing reported yet" on
      // either side, so it must not overwrite a stored result.
      if (reported == 0)
        return stored;

      if (stored == 0)
        return reported;

      return std::min(stored, reported);
    }

    case CompareType::Maximum:
      return std::max(stored, reported);

    case CompareType::Count:
    case CompareType::Sum:
    default:
    {
      // The client counts cumulatively within one race and starts a fresh
      // count in the next, and the last report is forgotten when a race
      // starts, so only the growth since the last one is new progress.
      const uint32_t gained = reported > lastReported
        ? reported - lastReported
        : 0;

      // Saturate rather than wrap, a total is never meant to fall.
      if (stored > std::numeric_limits<uint32_t>::max() - gained)
        return std::numeric_limits<uint32_t>::max();

      return stored + gained;
    }
  }
}

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
    _achievements.emplace(tid, std::move(info));
  }

  RebuildEventIndex();

  for (const auto& entry : achievementsSection["bookRewards"])
  {
    AchievementBookRewardInfo info;
    ReadBookReward(entry, info);
    _bookRewards.emplace_back(info);
  }

  spdlog::info(
    "Achievement registry loaded {} achievements and {} book rewards",
    _achievements.size(),
    _bookRewards.size());
}

void AchievementRegistry::RebuildEventIndex()
{
  // Achievements are looked up by the event that measures them, which is what
  // a report carries. The lists hold pointers into the map, which stays valid
  // because nothing is inserted once the configuration has been read.
  _byEvent.clear();
  _byBook.clear();
  for (const auto& [tid, info] : _achievements)
  {
    _byEvent[info.measuredEventId].push_back(&info);
    _byBook[info.bookType].push_back(&info);
  }
}

void AchievementRegistry::ReadConditions(
  const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto conditions = root["conditions"];
  if (not conditions)
    throw std::runtime_error("Missing conditions section");

  for (const auto& entry : root["thresholds"])
  {
    const auto tid = entry["tid"].as<uint16_t>();
    const auto achievement = _achievements.find(tid);
    if (achievement == _achievements.end())
      throw std::runtime_error("Threshold for unknown achievement");

    ReadTierValues(entry["successValues"], achievement->second.successValues);
  }

  size_t readingCount = 0;

  for (const auto& entry : conditions)
  {
    const auto function = entry["function"].as<std::string>();
    const auto kind = ParseConditionKind(entry["kind"].as<std::string>(""));
    const auto measuredEventId = static_cast<uint16_t>(
      entry["measuredEvent"].as<uint32_t>(0));

    // A bar given per achievement overrides the column, and its absence means
    // the column carries it.
    std::unordered_map<uint16_t, uint32_t> bars;
    bool barsGiven = false;
    for (const auto& bar : entry["bars"])
    {
      barsGiven = true;
      bars.emplace(bar["tid"].as<uint16_t>(), bar["bar"].as<uint32_t>(0));
    }

    for (auto& [tid, info] : _achievements)
    {
      if (info.function != function)
        continue;

      // A function may be listed with bars for only some of the achievements
      // that carry it, and the others keep the plain reading.
      if (barsGiven and not bars.contains(tid))
        continue;

      info.conditionKind = kind;
      if (measuredEventId != 0)
        info.measuredEventId = measuredEventId;

      const auto bar = bars.find(tid);
      info.conditionBar = bar != bars.end() and bar->second != 0
        ? bar->second
        : info.functionValue;

      ++readingCount;
    }
  }

  RebuildEventIndex();

  spdlog::info(
    "Achievement registry applied {} condition readings",
    readingCount);
}

std::span<const AchievementInfo* const>
AchievementRegistry::GetAchievementsByEvent(
  const uint16_t eventId) const
{
  const auto it = _byEvent.find(eventId);
  if (it == _byEvent.end())
    return {};

  return it->second;
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

std::span<const AchievementInfo* const>
AchievementRegistry::GetAchievementsByBook(
  const int8_t bookType) const
{
  const auto it = _byBook.find(bookType);
  if (it == _byBook.end())
    return {};

  return it->second;
}
double AchievementRegistry::GetReportedValueScale(const uint16_t eventId) const
{
  const auto scale = _reportedValueScales.find(eventId);
  if (scale == _reportedValueScales.end())
    return 1.0;

  return scale->second;
}

const AchievementBookRewardInfo* AchievementRegistry::GetBookReward(
  const int8_t bookType,
  const uint8_t characterModelId) const
{
  const auto reward = std::ranges::find_if(
    _bookRewards,
    [bookType, characterModelId](const AchievementBookRewardInfo& info)
    {
      return info.bookType == bookType
        and info.characterModelId == characterModelId;
    });

  if (reward == _bookRewards.end())
    return nullptr;

  return &*reward;
}

} // namespace server::registry
