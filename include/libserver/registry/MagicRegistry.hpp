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
    uint32_t attackRank{}; //!< Priority rank for removeMagic attacks (0 = not an attack, 1-3 = IceWall/FireBall+Summon/Lightning)

    uint32_t affectByCriticalAura{};
    uint32_t criticalByDarkFire{};
  };

  //! Magic gauge (SP) regen settings for Magic mode.
  struct RegenInfo
  {
    uint32_t pointPerTick{1000};
    uint32_t intervalMs{200};
    uint32_t courageScaleBp{9};
  };

  //! Identifies which mount stat field drives a scaling.
  enum class MountStat : uint8_t
  {
    Agility = 0,
    Ambition = 1,
    Rush = 2,
    Endurance = 3,
    Courage = 4,
  };

  //! Per-spell stat scaling. The named stat scales the spell's duration and crit chance.
  struct StatScaling
  {
    MountStat stat{MountStat::Agility};
    //! Fractional duration bonus per caster stat point in bp. e.g. 10 → +1%/pt.
    uint32_t durationScaleBp{};
    //! Crit bonus per full 10 caster stat points in bp (stepped). e.g. 250 → +2.5%/10pts.
    uint32_t critStepBp{};
    //! Fractional duration reduction per target stat point in bp. e.g. 5 → -0.5%/pt.
    uint32_t targetDurationReductionBp{};
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

  [[nodiscard]] const Magic::RegenInfo& GetRegenInfo() const;
  //! Base magic crit chance in basis points (1bp = 0.01%).
  [[nodiscard]] uint32_t GetBaseCritChanceBp() const;
  //! Returns the stat scaling for a basicType, or nullptr if none applies.
  [[nodiscard]] const Magic::StatScaling* GetStatScaling(uint32_t basicType) const;

private:
  std::unordered_map<uint32_t, Magic::SlotInfo> _slotInfo{};
  std::vector<uint32_t> _soloPool{};
  std::vector<uint32_t> _teamPool{};
  Magic::RegenInfo _regenInfo{};
  uint32_t _baseCritChanceBp{500};
  //! Keyed by basicType.
  std::unordered_map<uint32_t, Magic::StatScaling> _statScalings{};
};

} // namespace server::registry

#endif // MAGICREGISTRY_HPP
