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
  const auto breeding = root["breeding"];
  const auto failureCards = breeding["failureCards"];

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

  if (const auto params = breeding["params"])
  {
    _params.childGradeLimit = params["childGradeLimit"].as<int32_t>(_params.childGradeLimit);
    _params.successDecayPerBreeding = params["successDecayPerBreeding"].as<int32_t>(
      _params.successDecayPerBreeding);
    _params.minSuccessRate = params["minSuccessRate"].as<int32_t>(_params.minSuccessRate);
    _params.chanceCardChance = params["chanceCardChance"].as<int32_t>(_params.chanceCardChance);
    _params.appearanceVariation = params["appearanceVariation"].as<int32_t>(
      _params.appearanceVariation);
    _params.inheritanceRateBonusUnit = params["inheritanceRateBonusUnit"].as<int32_t>(
      _params.inheritanceRateBonusUnit);
  }

  if (const auto genetics = breeding["genetics"])
  {
    for (const auto& rowNode : genetics["gradeProbabilities"])
    {
      GradeProbabilityRow row;
      row.gradeDistance = rowNode["gradeDistance"].as<uint32_t>();
      row.minus3 = rowNode["minus3"].as<float>();
      row.minus2 = rowNode["minus2"].as<float>();
      row.minus1 = rowNode["minus1"].as<float>();
      for (const auto& plusNode : rowNode["plus"])
        row.plus.push_back(plusNode.as<float>());
      _gradeProbabilities.push_back(row);
    }
  }

  if (const auto bonus = breeding["bonus"])
  {
    const auto readBand = [](const YAML::Node& node, BreedingBonusBand& band)
    {
      band.minGrade = node["min"].as<uint32_t>();
      band.maxGrade = node["max"].as<uint32_t>();
      band.activationChance = node["activationChance"].as<int32_t>();
    };
    readBand(bonus["smallGrade"], _smallGradeBand);
    readBand(bonus["bigGrade"], _bigGradeBand);

    for (const auto& entryNode : bonus["entries"])
    {
      BreedingBonusEntry entry;
      entry.id = entryNode["id"].as<uint32_t>();
      entry.type = entryNode["type"].as<uint32_t>();
      entry.value = entryNode["value"].as<uint32_t>();
      entry.ratioSmall = entryNode["ratioSmall"].as<int32_t>();
      entry.ratioBig = entryNode["ratioBig"].as<int32_t>();
      _bonusEntries.push_back(entry);
    }
  }

  spdlog::info(
    "Breeding registry loaded {} prob entries, {} normal rewards, {} chance rewards, "
    "{} bonus entries, {} grade-probability rows",
    _failureCardProbs.size(),
    _normalCard.rewards.size(),
    _chanceCard.rewards.size(),
    _bonusEntries.size(),
    _gradeProbabilities.size());
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

const BreedingParams& BreedingRegistry::GetBreedingParams() const
{
  return _params;
}

const GradeProbabilityRow& BreedingRegistry::GetGradeProbability(uint32_t gradeDistance) const
{
  static const GradeProbabilityRow fallback{};
  if (_gradeProbabilities.empty())
    return fallback;

  for (const auto& row : _gradeProbabilities)
  {
    if (row.gradeDistance == gradeDistance)
      return row;
  }
  // Distances beyond the table clamp to the largest configured row.
  return _gradeProbabilities.back();
}

const BreedingBonusBand& BreedingRegistry::GetSmallGradeBonusBand() const
{
  return _smallGradeBand;
}

const BreedingBonusBand& BreedingRegistry::GetBigGradeBonusBand() const
{
  return _bigGradeBand;
}

const std::vector<BreedingBonusEntry>& BreedingRegistry::GetBonusEntries() const
{
  return _bonusEntries;
}

} // namespace server::registry
