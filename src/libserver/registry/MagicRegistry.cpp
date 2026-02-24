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

  return slot.type;
}

} // anonymous namespace

void MagicRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto magicSection = root["magic"];
  if (not magicSection)
    throw std::runtime_error("Missing magic section");

  // Slot info
  {
    const auto slotSection = magicSection["slotInfo"];
    if (not slotSection)
      throw std::runtime_error("Missing magic slotInfo section");

    const auto collection = slotSection["collection"];
    if (not collection)
      throw std::runtime_error("Missing magic slotInfo collection");

    for (const auto& entry : collection)
    {
      Magic::SlotInfo slot;
      const auto type = ReadSlotInfo(entry, slot);
      _slotInfo.emplace(type, std::move(slot));
    }
  }

  // Pre-build the pick pools so RandomMagicItem never has to filter at runtime.
  for (const auto& [type, slot] : _slotInfo)
  {
    if (slot.basicType != type)
      continue; // skip critical variants
    _teamPool.push_back(type);
    if (slot.teamMode == 0)
      _soloPool.push_back(type);
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

} // namespace server::registry
