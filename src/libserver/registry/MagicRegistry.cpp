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

#include "libserver/registry/MagicRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <stdexcept>

namespace server::registry
{

namespace
{

uint32_t ReadSlotInfo(const YAML::Node& section, Magic::SlotInfo& slot)
{
  slot.type = section["type"].as<uint32_t>();

  slot.basicType = section["basicType"].as<uint32_t>();
  slot.criticalType = section["criticalType"].as<uint32_t>();
  slot.skillEffectId = section["skillEffectId"].as<uint32_t>();
  slot.attackValue = section["attackValue"].as<uint32_t>();
  slot.defenseValue = section["defenseValue"].as<uint32_t>();

  slot.castingTime = section["castingTime"].as<float>();
  slot.effectDelay = section["effectDelay"].as<float>();
  slot.effectDisappearDelay = section["effectDisappearDelay"].as<float>();
  slot.targetingDelay = section["targetingDelay"].as<float>();
  slot.getStartDelay = section["getStartDelay"].as<float>();

  slot.targetingType = section["targetingType"].as<uint32_t>();
  slot.needTargeting = section["needTargeting"].as<uint32_t>();
  slot.noneTargetable = section["noneTargetable"].as<uint32_t>();
  slot.noneSummonStick = section["noneSummonStick"].as<uint32_t>();
  slot.causeAttackRelease = section["causeAttackRelease"].as<uint32_t>();
  slot.adjustMotionSpeed = section["adjustMotionSpeed"].as<uint32_t>();

  slot.teamKill = section["teamKill"].as<uint32_t>();
  slot.teamMode = section["teamMode"].as<uint32_t>();
  slot.slidingReduce = section["slidingReduce"].as<uint32_t>();
  slot.reflectable = section["reflectable"].as<uint32_t>();
  slot.removeMagic = section["removeMagic"].as<uint32_t>();
  slot.removeHotRodding = section["removeHotRodding"].as<uint32_t>();
  slot.removeSummonTarget = section["removeSummonTarget"].as<uint32_t>();
  slot.replaceEffect = section["replaceEffect"].as<uint32_t>();
  slot.massEffect = section["massEffect"].as<uint32_t>();

  slot.affectByCriticalAura = section["affectByCriticalAura"].as<uint32_t>();
  slot.criticalByDarkFire = section["criticalByDarkFire"].as<uint32_t>();
  slot.givePositionalMagic = section["givePositionalMagic"].as<uint32_t>();
  slot.attackRank = section["attackRank"].as<uint32_t>(0);

  // Crit items do not need the positional weights defined, base items carry that info
  if (slot.type == slot.basicType)
    slot.positionalWeights = section["positionalWeights"].as<std::array<uint32_t, 8>>();

  return slot.type;
}

} // anonymous namespace

void MagicRegistry::Clear()
{
  _slotInfo.clear();
  _soloPool.clear();
  _teamPool.clear();
  _statScalings.clear();
  for (auto& weights : _soloPositionWeights)
    weights.clear();
  for (auto& weights : _teamPositionWeights)
    weights.clear();
  for (auto& weights : _soloGroupWeights)
    weights.clear();
  for (auto& weights : _teamGroupWeights)
    weights.clear();
  for (auto& weights : _attackGroupWeights)
    weights.clear();
  for (auto& weights : _teamAssistanceWeights)
    weights.clear();
  _groupTeamModifiers.clear();
  _slotTeamModifiers.clear();
  for (auto& conv : _rankingConversion)
    conv.clear();
  _regenInfo = {};
  _setBonusInfo = {};
  _config = {};
  _baseCritChanceBp = 500;
}

void MagicRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto magicSection = root["magic"];
  if (not magicSection)
    throw std::runtime_error("Missing magic section");

  const auto slotSection = magicSection["slotInfo"];
  if (not slotSection)
    throw std::runtime_error("Missing magic slotInfo section");

  const auto collection = slotSection["collection"];
  if (not collection)
    throw std::runtime_error("Missing magic slotInfo collection");

  Clear();

  // Slot info
  for (const auto& entry : collection)
  {
    Magic::SlotInfo slot;
    const auto type = ReadSlotInfo(entry, slot);
    _slotInfo.emplace(type, std::move(slot));
  }

  // Pre-build the pick pools and positional weights so RandomMagicItem never has to filter at runtime.
  for (const auto& [type, slot] : _slotInfo)
  {
    if (slot.basicType != type)
      continue; // skip critical variants
    _teamPool.push_back(type);
    if (slot.teamMode == 0)
      _soloPool.push_back(type);

    // Only compile weights from base type
    if (slot.type != slot.basicType)
      continue;

    for (size_t i = 0; i < slot.positionalWeights.size(); ++i)
    {
      const auto& pair = std::make_pair(
        slot.positionalWeights[i],
        slot);

      // Add to team magic item weights since team is a superset of solo
      _teamPositionWeights[i].emplace_back(pair);

      // Do not add to solo position weights if team item
      if (slot.teamMode != 0)
        continue;

      _soloPositionWeights[i].emplace_back(pair);
    }
  }

  if (const auto regenSection = magicSection["regen"])
  {
    _regenInfo.pointPerTick = regenSection["pointPerTick"].as<uint32_t>(_regenInfo.pointPerTick);
    _regenInfo.intervalMs = regenSection["intervalMs"].as<uint32_t>(_regenInfo.intervalMs);
    _regenInfo.courageScaleBp = regenSection["courageScaleBp"].as<uint32_t>(_regenInfo.courageScaleBp);
  }

  if (const auto setBonusSection = magicSection["setBonus"])
  {
    _setBonusInfo.critChanceBonusBp = setBonusSection["critChanceBonusBp"].as<uint32_t>(
      _setBonusInfo.critChanceBonusBp);
    _setBonusInfo.passiveGaugeScaleBp = setBonusSection["passiveGaugeScaleBp"].as<uint32_t>(
      _setBonusInfo.passiveGaugeScaleBp);
    _setBonusInfo.holdingGaugeScaleBp = setBonusSection["holdingGaugeScaleBp"].as<uint32_t>(
      _setBonusInfo.holdingGaugeScaleBp);
  }

  _baseCritChanceBp = magicSection["critChanceBp"].as<uint32_t>(_baseCritChanceBp);

  if (const auto scalingsSection = magicSection["statScalings"])
  {
    for (const auto& entry : scalingsSection)
    {
      const auto basicType = entry["basicType"].as<uint32_t>();
      const auto statName = entry["stat"].as<std::string>();

      Magic::StatScaling scaling{};
      if (statName == "agility")
        scaling.stat = Magic::MountStat::Agility;
      else if (statName == "ambition")
        scaling.stat = Magic::MountStat::Ambition;
      else if (statName == "rush")
        scaling.stat = Magic::MountStat::Rush;
      else if (statName == "endurance")
        scaling.stat = Magic::MountStat::Endurance;
      else if (statName == "courage")
        scaling.stat = Magic::MountStat::Courage;
      else
        throw std::runtime_error("Unknown stat in statScalings: " + statName);

      scaling.durationScaleBp = entry["durationScaleBp"].as<uint32_t>(0);
      scaling.critStepBp = entry["critStepBp"].as<uint32_t>(0);
      scaling.targetDurationReductionBp = entry["targetDurationReductionBp"].as<uint32_t>(0);

      _statScalings.emplace(basicType, scaling);
    }
  }

  // Parse groupRatio (Solo)
  if (const auto groupRatioSection = magicSection["groupRatio"])
  {
    for (const auto& entry : groupRatioSection)
    {
      const auto groupId = entry["group"].as<uint32_t>();
      const auto ranks = entry["ranks"].as<std::array<uint32_t, 8>>();
      for (size_t r = 0; r < 8; ++r)
        _soloGroupWeights[r].emplace_back(groupId, ranks[r]);
    }
  }

  // Parse groupTeamRatio (Team)
  if (const auto groupTeamRatioSection = magicSection["groupTeamRatio"])
  {
    for (const auto& entry : groupTeamRatioSection)
    {
      const auto groupId = entry["group"].as<uint32_t>();
      const auto ranks = entry["ranks"].as<std::array<uint32_t, 8>>();
      for (size_t r = 0; r < 8; ++r)
        _teamGroupWeights[r].emplace_back(groupId, ranks[r]);
    }
  }

  // Parse groupAttackRatio (Attacks)
  if (const auto groupAttackRatioSection = magicSection["groupAttackRatio"])
  {
    for (const auto& entry : groupAttackRatioSection)
    {
      const auto basicType = entry["basicType"].as<uint32_t>();
      const auto ranks = entry["ranks"].as<std::array<uint32_t, 8>>();
      for (size_t r = 0; r < 8; ++r)
        _attackGroupWeights[r].emplace_back(basicType, ranks[r]);
    }
  }

  // Parse groupTeamAssistanceRatio (Team Assistance)
  if (const auto groupTeamAssistanceRatioSection = magicSection["groupTeamAssistanceRatio"])
  {
    for (const auto& entry : groupTeamAssistanceRatioSection)
    {
      const auto basicType = entry["basicType"].as<uint32_t>();
      const auto ranks = entry["ranks"].as<std::array<uint32_t, 8>>();
      for (size_t r = 0; r < 8; ++r)
        _teamAssistanceWeights[r].emplace_back(basicType, ranks[r]);
    }
  }

  // Parse groupTeamModifier
  if (const auto groupTeamModifierSection = magicSection["groupTeamModifier"])
  {
    for (const auto& entry : groupTeamModifierSection)
    {
      Magic::TeamModifier mod{};
      mod.targetId = entry["group"].as<uint32_t>();
      mod.lead = entry["lead"].as<uint32_t>();
      mod.aheadOffsets = entry["aheadOffsets"].as<std::array<int32_t, 4>>();
      _groupTeamModifiers.push_back(mod);
    }
  }

  // Parse slotTeamModifier
  if (const auto slotTeamModifierSection = magicSection["slotTeamModifier"])
  {
    for (const auto& entry : slotTeamModifierSection)
    {
      Magic::TeamModifier mod{};
      mod.targetId = entry["basicType"].as<uint32_t>();
      mod.lead = entry["lead"].as<uint32_t>();
      mod.aheadOffsets = entry["aheadOffsets"].as<std::array<int32_t, 4>>();
      _slotTeamModifiers.push_back(mod);
    }
  }

  // Parse rankingConversion
  if (const auto rankingConversionSection = magicSection["rankingConversion"])
  {
    for (const auto& entry : rankingConversionSection)
    {
      const auto playerCount = entry["playerCount"].as<size_t>();
      if (playerCount >= 1 && playerCount <= 8)
      {
        _rankingConversion[playerCount - 1] = entry["canonicalRanks"].as<std::vector<uint32_t>>();
      }
    }
  }

  // Parse general config
  if (const auto configSection = magicSection["config"])
  {
    _config.lastSpurtCritBonusBp = configSection["lastSpurtCritBonusBp"].as<uint32_t>(_config.lastSpurtCritBonusBp);
    _config.lastSpurtProgressThreshold = configSection["lastSpurtProgressThreshold"].as<float>(_config.lastSpurtProgressThreshold);
  }

  spdlog::info(
    "Magic registry loaded {} slot(s) ({} solo, {} team)",
    _slotInfo.size(),
    _soloPool.size(),
    _teamPool.size());
}

const Magic::SlotInfo& MagicRegistry::GetSlotInfo(uint32_t type) const
{
  const auto it = _slotInfo.find(type);
  if (it == _slotInfo.end())
    throw std::runtime_error("Magic slot not found: " + std::to_string(type));
  return it->second;
}

const Magic::SlotInfo& MagicRegistry::GetSlotInfoByEffectId(uint32_t effectId) const
{
  for (const auto& [type, slot] : _slotInfo)
  {
    if (slot.skillEffectId == effectId)
      return slot;
  }
  throw std::runtime_error("Magic slot not found for effect ID: " + std::to_string(effectId));
}

const std::unordered_map<uint32_t, Magic::SlotInfo>& MagicRegistry::GetSlotInfoMap() const
{
  return _slotInfo;
}

const std::vector<uint32_t>& MagicRegistry::GetSoloPool() const
{
  return _soloPool;
}

const std::vector<uint32_t>& MagicRegistry::GetTeamPool() const
{
  return _teamPool;
}

const Magic::RegenInfo& MagicRegistry::GetRegenInfo() const
{
  return _regenInfo;
}

const Magic::SetBonusInfo& MagicRegistry::GetSetBonusInfo() const
{
  return _setBonusInfo;
}

uint32_t MagicRegistry::GetBaseCritChanceBp() const
{
  return _baseCritChanceBp;
}

const Magic::StatScaling* MagicRegistry::GetStatScaling(uint32_t basicType) const
{
  const auto it = _statScalings.find(basicType);
  return it == _statScalings.cend() ? nullptr : &it->second;
}

const std::vector<std::pair<Magic::SlotWeight, Magic::SlotInfo>>& MagicRegistry::GetSoloPositionWeights(uint32_t position) const
{
  return _soloPositionWeights.at(position);
}

const std::vector<std::pair<Magic::SlotWeight, Magic::SlotInfo>>& MagicRegistry::GetTeamPositionWeights(uint32_t position) const
{
  return _teamPositionWeights.at(position);
}

uint32_t MagicRegistry::GetCanonicalRank(size_t totalRacers, size_t racerRank) const
{
  if (totalRacers == 0 || racerRank == 0)
    return 1;

  const size_t pIdx = std::clamp(totalRacers, size_t{1}, size_t{8}) - 1;
  const auto& table = _rankingConversion[pIdx];
  if (table.empty())
    return std::clamp<uint32_t>(static_cast<uint32_t>(racerRank), 1, 8);

  const size_t rIdx = std::clamp(racerRank, size_t{1}, table.size()) - 1;
  return table[rIdx];
}

const std::vector<std::pair<uint32_t, uint32_t>>& MagicRegistry::GetSoloGroupWeights(uint32_t canonicalRank) const
{
  const size_t idx = std::clamp<uint32_t>(canonicalRank, 1, 8) - 1;
  return _soloGroupWeights[idx];
}

const std::vector<std::pair<uint32_t, uint32_t>>& MagicRegistry::GetTeamGroupWeights(uint32_t canonicalRank) const
{
  const size_t idx = std::clamp<uint32_t>(canonicalRank, 1, 8) - 1;
  return _teamGroupWeights[idx];
}

const std::vector<std::pair<uint32_t, uint32_t>>& MagicRegistry::GetAttackGroupWeights(uint32_t canonicalRank) const
{
  const size_t idx = std::clamp<uint32_t>(canonicalRank, 1, 8) - 1;
  return _attackGroupWeights[idx];
}

const std::vector<std::pair<uint32_t, uint32_t>>& MagicRegistry::GetTeamAssistanceWeights(uint32_t canonicalRank) const
{
  const size_t idx = std::clamp<uint32_t>(canonicalRank, 1, 8) - 1;
  return _teamAssistanceWeights[idx];
}

int32_t MagicRegistry::GetGroupTeamModifier(uint32_t groupId, bool isLead, uint32_t aheadCount) const
{
  const uint32_t leadVal = isLead ? 1 : 0;
  for (const auto& mod : _groupTeamModifiers)
  {
    if (mod.targetId == groupId && mod.lead == leadVal)
    {
      const size_t aheadIdx = std::clamp<uint32_t>(aheadCount, 1, 4) - 1;
      return mod.aheadOffsets[aheadIdx];
    }
  }
  return 0;
}

int32_t MagicRegistry::GetSlotTeamModifier(uint32_t basicType, bool isLead, uint32_t aheadCount) const
{
  const uint32_t leadVal = isLead ? 1 : 0;
  for (const auto& mod : _slotTeamModifiers)
  {
    if (mod.targetId == basicType && mod.lead == leadVal)
    {
      const size_t aheadIdx = std::clamp<uint32_t>(aheadCount, 1, 4) - 1;
      return mod.aheadOffsets[aheadIdx];
    }
  }
  return 0;
}

const Magic::Config& MagicRegistry::GetConfig() const
{
  return _config;
}

} // namespace server::registry
