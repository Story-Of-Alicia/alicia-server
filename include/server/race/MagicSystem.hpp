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

#ifndef MAGICSYSTEM_HPP
#define MAGICSYSTEM_HPP

#include "server/tracker/RaceTracker.hpp"

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/registry/MagicRegistry.hpp>

#include <cstdint>

namespace server::race
{

namespace SkillEffect
{

constexpr uint32_t FireBall = 0;
constexpr uint32_t FireBallCritical = 1;
constexpr uint32_t WaterShield = 2;
constexpr uint32_t WaterShieldCritical = 3;
constexpr uint32_t Booster = 5;
constexpr uint32_t HotRodding = 6;
constexpr uint32_t HotRoddingCritical = 7;
constexpr uint32_t IceWall = 8;
constexpr uint32_t IceWallCritical = 9;
constexpr uint32_t JumpStun = 10;
constexpr uint32_t JumpStunCritical = 11;
constexpr uint32_t DarkFire = 12;
constexpr uint32_t DarkFireCritical = 13;
constexpr uint32_t Summon = 14;
constexpr uint32_t SummonCritical = 15;
constexpr uint32_t Lightning = 16;
constexpr uint32_t LightningCritical = 17;
constexpr uint32_t BufPower = 18;
constexpr uint32_t BufPowerCritical = 19;
constexpr uint32_t BufGauge = 20;
constexpr uint32_t BufGaugeCritical = 21;
constexpr uint32_t BufSpeed = 22;
constexpr uint32_t BufSpeedCritical = 23;

} // namespace SkillEffect

namespace MagicType
{

constexpr uint32_t FireBall = 2;
constexpr uint32_t FireBallCritical = 3;
constexpr uint32_t WaterShield = 4;
constexpr uint32_t WaterShieldCritical = 5;
constexpr uint32_t Booster = 6;
constexpr uint32_t BoosterCritical = 7;
constexpr uint32_t HotRodding = 8;
constexpr uint32_t HotRoddingCritical = 9;
constexpr uint32_t IceWall = 10;
constexpr uint32_t IceWallCritical = 11;
constexpr uint32_t JumpStun = 12;
constexpr uint32_t JumpStunCritical = 13;
constexpr uint32_t DarkFire = 14;
constexpr uint32_t DarkFireCritical = 15;
constexpr uint32_t Summon = 16;
constexpr uint32_t SummonCritical = 17;
constexpr uint32_t Lightning = 18;
constexpr uint32_t LightningCritical = 19;
constexpr uint32_t BufPower = 20;
constexpr uint32_t BufPowerCritical = 21;
constexpr uint32_t BufGauge = 22;
constexpr uint32_t BufGaugeCritical = 23;
constexpr uint32_t BufSpeed = 24;
constexpr uint32_t BufSpeedCritical = 25;
constexpr uint32_t Positional = 27;

} // namespace MagicType

class MagicSystem
{
public:
  struct EffectResolution
  {
    uint32_t effectId{};
    bool shieldBlocks{};
    bool isDuplicated{};
  };

  static uint32_t GetMountStatValue(
    const tracker::RaceTracker::Racer::MountStatsSnapshot& stats,
    registry::Magic::MountStat which);

  static tracker::RaceTracker::RacerObjectMap::iterator FindRacerByOid(
    tracker::RaceTracker::RacerObjectMap& racers,
    tracker::Oid oid);

  static bool IsSelfCast(uint32_t magicType);
  static bool IsTeamBuff(uint32_t magicType);
  static bool IsIceWall(uint32_t magicType);
  static bool IsValidSkillEffectId(uint32_t skillEffectId);

  static constexpr uint32_t HeavyAttackRank = 2;

  //! @param attackSlotInfo The attack that just landed on the target.
  //! @param activeSlotInfo The spell whose effect the target currently has.
  static bool IsStrippedByAttack(
    const registry::Magic::SlotInfo& attackSlotInfo,
    const registry::Magic::SlotInfo& activeSlotInfo);

  static bool DrainsGaugeOnHit(const registry::Magic::SlotInfo& attackSlotInfo);

  static EffectResolution ResolveEffect(
    const registry::MagicRegistry& magicRegistry,
    const registry::Magic::SlotInfo& magicSlotInfo,
    const tracker::RaceTracker::Racer& targetRacer);

  static uint32_t ComputeEffectDurationMs(
    const registry::MagicRegistry& magicRegistry,
    const registry::Magic::SlotInfo& magicSlotInfo,
    const tracker::RaceTracker::Racer* attackerRacer,
    const tracker::RaceTracker::Racer& targetRacer);

  //! Function to select a random item based on position weights
  static const registry::Magic::SlotInfo& SelectMagicTypeByPosition(
    const registry::MagicRegistry& magicRegistry,
    uint32_t position,
    bool isTeam);

  static const registry::Magic::SlotInfo& RandomMagicItem(
    const registry::MagicRegistry& magicRegistry,
    tracker::RaceTracker& tracker,
    data::Uid racerUid);
};

} // namespace server::race

#endif // MAGICSYSTEM_HPP
