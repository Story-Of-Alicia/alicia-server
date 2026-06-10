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

#include "libserver/registry/BreedingRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace server::registry
{

namespace
{

void ReadFailureCardTable(const YAML::Node& node, FailureCardTable& table)
{
  for (const auto& rangeNode : node["gradeRanges"])
  {
    FailureCardGradeRange range;
    range.grade = rangeNode["grade"].as<uint32_t>();
    range.minId = rangeNode["minId"].as<uint32_t>();
    range.maxId = rangeNode["maxId"].as<uint32_t>();
    table.gradeRanges.push_back(range);
  }

  for (const auto& rewardNode : node["rewards"])
  {
    const auto id = rewardNode["id"].as<uint32_t>();
    FailureCardReward reward;
    reward.itemTid = rewardNode["itemTid"].as<uint32_t>();
    reward.itemCount = rewardNode["itemCount"].as<uint32_t>();
    reward.gameMoney = rewardNode["gameMoney"].as<uint32_t>();
    table.rewards.try_emplace(id, reward);
  }
}

} // anon namespace

void BreedingRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());
  const auto failureCards = root["breeding"]["failureCards"];

  for (const auto& probNode : failureCards["probabilities"])
  {
    FailureCardProbEntry entry;
    entry.moneySpent = probNode["moneySpent"].as<uint32_t>();
    entry.probA = probNode["probA"].as<int32_t>();
    entry.probB = probNode["probB"].as<int32_t>();
    entry.probC = probNode["probC"].as<int32_t>();
    _failureCardProbs.push_back(entry);
  }

  ReadFailureCardTable(failureCards["normalCard"], _normalCard);
  ReadFailureCardTable(failureCards["chanceCard"], _chanceCard);

  spdlog::info(
    "Breeding registry loaded {} prob entries, {} normal rewards, {} chance rewards",
    _failureCardProbs.size(),
    _normalCard.rewards.size(),
    _chanceCard.rewards.size());
}

const FailureCardProbEntry& BreedingRegistry::GetFailureCardProb(uint32_t moneySpent) const
{
  for (const auto& entry : _failureCardProbs)
  {
    if (moneySpent <= entry.moneySpent)
      return entry;
  }
  return _failureCardProbs.back();
}

const FailureCardGradeRange* BreedingRegistry::GetNormalCardGradeRange(uint32_t grade) const
{
  for (const auto& range : _normalCard.gradeRanges)
  {
    if (range.grade == grade)
      return &range;
  }
  return nullptr;
}

const FailureCardGradeRange* BreedingRegistry::GetChanceCardGradeRange(uint32_t grade) const
{
  for (const auto& range : _chanceCard.gradeRanges)
  {
    if (range.grade == grade)
      return &range;
  }
  return nullptr;
}

const FailureCardReward* BreedingRegistry::GetNormalCardReward(uint32_t rewardId) const
{
  auto it = _normalCard.rewards.find(rewardId);
  if (it != _normalCard.rewards.end())
    return &it->second;
  return nullptr;
}

const FailureCardReward* BreedingRegistry::GetChanceCardReward(uint32_t rewardId) const
{
  auto it = _chanceCard.rewards.find(rewardId);
  if (it != _chanceCard.rewards.end())
    return &it->second;
  return nullptr;
}

} // namespace server::registry
