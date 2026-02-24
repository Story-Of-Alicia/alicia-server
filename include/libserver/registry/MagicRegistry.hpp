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

#ifndef MAGICREGISTRY_HPP
#define MAGICREGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct Magic
{
  //! Per-magic-slot definition (MagicSlotInfo).
  struct SlotInfo
  {
    uint32_t type{};

    uint32_t basicType{};
    uint32_t criticalType{};
    uint32_t skillEffectId{};
    uint32_t attackValue{};
    uint32_t defenseValue{};

    float castingTime{};
    float effectDelay{};
    float effectDisappearDelay{};
    float targetingDelay{};
    float getStartDelay{};

    uint32_t targetingType{};
    uint32_t needTargeting{};
    uint32_t noneTargetable{};
    uint32_t noneSummonStick{};
    uint32_t causeAttackRelease{};
    uint32_t adjustMotionSpeed{};

    uint32_t teamKill{};
    uint32_t teamMode{};
    uint32_t slidingReduce{};
    uint32_t reflectable{};
    uint32_t removeMagic{};
    uint32_t removeHotRodding{};
    uint32_t removeSummonTarget{};
    uint32_t replaceEffect{};
    uint32_t massEffect{};

    uint32_t affectByCriticalAura{};
    uint32_t criticalByDarkFire{};
  };

};

class MagicRegistry
{
public:
  MagicRegistry() = default;

  void ReadConfig(const std::filesystem::path& configPath);

  [[nodiscard]] const Magic::SlotInfo& GetSlotInfo(uint32_t type) const;
  [[nodiscard]] const Magic::SlotInfo& GetSlotInfoByEffectId(uint32_t effectId) const;
  [[nodiscard]] const std::unordered_map<uint32_t, Magic::SlotInfo>& GetSlotInfoMap() const;

  //! Basic-type slot IDs available in solo mode (teamMode == 0).
  [[nodiscard]] const std::vector<uint32_t>& GetSoloPool() const;
  //! Basic-type slot IDs available in team mode (all teamMode values).
  [[nodiscard]] const std::vector<uint32_t>& GetTeamPool() const;

private:
  std::unordered_map<uint32_t, Magic::SlotInfo> _slotInfo{};
  std::vector<uint32_t> _soloPool{};
  std::vector<uint32_t> _teamPool{};
};

} // namespace server::registry

#endif // MAGICREGISTRY_HPP
