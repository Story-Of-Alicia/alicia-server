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

#include "server/race/MagicSystem.hpp"
#include "server/system/PotentialSystem.hpp"
#include "server/tracker/RaceTracker.hpp"

#include <libserver/util/Util.hpp>

#include <algorithm>
#include <random>
#include <ranges>

namespace server::race
{

uint32_t MagicSystem::GetMountStatValue(
  const tracker::RaceTracker::Racer::MountStatsSnapshot& stats,
  registry::Magic::MountStat which)
{
  switch (which)
  {
    case registry::Magic::MountStat::Agility:   return stats.agility;
    case registry::Magic::MountStat::Ambition:  return stats.ambition;
    case registry::Magic::MountStat::Rush:      return stats.rush;
    case registry::Magic::MountStat::Endurance: return stats.endurance;
    case registry::Magic::MountStat::Courage:   return stats.courage;
  }
  return 0;
}

tracker::RaceTracker::RacerObjectMap::iterator MagicSystem::FindRacerByOid(
  tracker::RaceTracker::RacerObjectMap& racers,
  const tracker::Oid oid)
{
  return std::ranges::find_if(
    racers,
    [oid](const auto& pair)
    {
      return pair.second.oid == oid;
    });
}

bool MagicSystem::IsSelfCast(const uint32_t magicType)
{
  switch (magicType)
  {
    case MagicType::WaterShield:
    case MagicType::WaterShieldCritical:
    case MagicType::Booster:
    case MagicType::BoosterCritical:
    case MagicType::HotRodding:
    case MagicType::HotRoddingCritical:
      return true;
    default:
      return false;
  }
}

bool MagicSystem::IsTeamBuff(const uint32_t magicType)
{
  switch (magicType)
  {
    case MagicType::BufPower:
    case MagicType::BufPowerCritical:
    case MagicType::BufGauge:
    case MagicType::BufGaugeCritical:
    case MagicType::BufSpeed:
    case MagicType::BufSpeedCritical:
      return true;
    default:
      return false;
  }
}

bool MagicSystem::IsIceWall(const uint32_t magicType)
{
  return magicType == MagicType::IceWall || magicType == MagicType::IceWallCritical;
}

bool MagicSystem::IsValidSkillEffectId(const uint32_t skillEffectId)
{
  constexpr uint32_t ClientCrashingEffectId = 4;

  return skillEffectId < tracker::RaceTracker::Racer::EffectCount
    && skillEffectId != ClientCrashingEffectId;
}

bool MagicSystem::IsDowned(const tracker::RaceTracker::Racer& racer)
{
  return racer.attackRank >= HeavyAttackRank;
}

bool MagicSystem::IsStrippedByAttack(
  const registry::Magic::SlotInfo& attackSlotInfo,
  const registry::Magic::SlotInfo& activeSlotInfo)
{
  switch (activeSlotInfo.skillEffectId)
  {
    case SkillEffect::HotRodding:
    case SkillEffect::HotRoddingCritical:
    case SkillEffect::BufPower:
    case SkillEffect::BufPowerCritical:
    case SkillEffect::BufGauge:
    case SkillEffect::BufGaugeCritical:
      return false;

    case SkillEffect::JumpStun:
    case SkillEffect::JumpStunCritical:
    case SkillEffect::DarkFire:
    case SkillEffect::DarkFireCritical:
      return attackSlotInfo.removeMagic && attackSlotInfo.attackRank >= HeavyAttackRank;

    case SkillEffect::Booster:
    case SkillEffect::BufSpeed:
    case SkillEffect::BufSpeedCritical:
      return attackSlotInfo.removeMagic
        || attackSlotInfo.basicType == MagicType::JumpStun;

    case SkillEffect::WaterShield:
      return attackSlotInfo.removeMagic
        || attackSlotInfo.type == MagicType::JumpStunCritical
        || attackSlotInfo.type == MagicType::DarkFireCritical;

    default:
      return attackSlotInfo.removeMagic
        && activeSlotInfo.adjustMotionSpeed && activeSlotInfo.attackValue == 0;
  }
}

bool MagicSystem::DrainsGaugeOnHit(const registry::Magic::SlotInfo& attackSlotInfo)
{
  switch (attackSlotInfo.type)
  {
    case MagicType::FireBallCritical:
    case MagicType::SummonCritical:
    case MagicType::Lightning:
    case MagicType::LightningCritical:
      return true;
    default:
      return false;
  }
}

MagicSystem::EffectResolution MagicSystem::ResolveEffect(
  const registry::MagicRegistry& magicRegistry,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const tracker::RaceTracker::Racer& targetRacer)
{
  const bool isAttack = magicSlotInfo.attackValue > 0;

  const bool hasCriticalShield = targetRacer.effects[SkillEffect::WaterShieldCritical];
  const bool hasShield = hasCriticalShield || targetRacer.effects[SkillEffect::WaterShield];

  const uint32_t shieldThreshold = hasCriticalShield ? 200u : hasShield ? 100u : 0u;

  EffectResolution resolution{};
  resolution.shieldBlocks = isAttack && magicSlotInfo.attackValue < shieldThreshold;

  resolution.effectId = resolution.shieldBlocks
    ? (hasCriticalShield ? SkillEffect::WaterShieldCritical : SkillEffect::WaterShield)
    : magicSlotInfo.skillEffectId;

  const bool isLightning = isAttack && magicSlotInfo.removeHotRodding;
  const bool isCriticalLightning = isLightning && magicSlotInfo.criticalType == 0;

  const bool hotRoddingBlocks = isAttack
    && ((targetRacer.effects[SkillEffect::HotRodding] && not isLightning)
      || (targetRacer.effects[SkillEffect::HotRoddingCritical] && not isCriticalLightning));

  const uint32_t occupiedEffectId = magicSlotInfo.replaceEffect
    ? magicSlotInfo.skillEffectId
    : magicRegistry.GetSlotInfo(magicSlotInfo.basicType).skillEffectId;

  bool criticalEffectActive = false;
  if (isAttack)
  {
    const uint32_t criticalType = magicRegistry.GetSlotInfo(magicSlotInfo.basicType).criticalType;
    if (criticalType != 0)
    {
      const uint32_t criticalEffectId = magicRegistry.GetSlotInfo(criticalType).skillEffectId;
      criticalEffectActive = IsValidSkillEffectId(criticalEffectId)
        && targetRacer.effects[criticalEffectId];
    }
  }

  resolution.isDuplicated = hotRoddingBlocks
    || criticalEffectActive
    || (isAttack && magicSlotInfo.attackRank < 2 && targetRacer.attackRank >= 2)
    || (magicSlotInfo.attackRank > 0
      ? targetRacer.attackRank >= magicSlotInfo.attackRank
      : targetRacer.effects[occupiedEffectId] && (isAttack || not magicSlotInfo.replaceEffect));

  return resolution;
}

uint32_t MagicSystem::ComputeEffectDurationMs(
  const registry::MagicRegistry& magicRegistry,
  const registry::HorseRegistry& horseRegistry,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const tracker::RaceTracker::Racer* attackerRacer,
  const tracker::RaceTracker::Racer& targetRacer)
{
  auto effectDurationMs = static_cast<uint32_t>(magicSlotInfo.effectDelay * 1000.0f);

  if (const auto* scaling = magicRegistry.GetStatScaling(magicSlotInfo.basicType))
  {
    if (scaling->durationScaleBp > 0 && attackerRacer != nullptr)
    {
      constexpr uint32_t MaxDurationBonusBp = 1150;
      const uint32_t statValue = GetMountStatValue(attackerRacer->mountStats, scaling->stat);
      const uint32_t bonusBp = std::min(scaling->durationScaleBp * statValue, MaxDurationBonusBp);

      effectDurationMs = effectDurationMs * (1000u + bonusBp) / 1000u;
    }

    if (scaling->targetDurationReductionBp > 0)
    {
      const uint32_t statValue = GetMountStatValue(targetRacer.mountStats, scaling->stat);
      const uint32_t reductionBp = std::min<uint32_t>(
        scaling->targetDurationReductionBp * statValue, 1000u);

      effectDurationMs = effectDurationMs * (1000u - reductionBp) / 1000u;
    }
  }

  if (attackerRacer != nullptr)
  {
    if (magicSlotInfo.basicType == MagicType::WaterShield)
    {
      effectDurationMs += PotentialSystem::GetShieldDurationBonusMs(
        horseRegistry, attackerRacer->potential);
    }
    else if (magicSlotInfo.basicType == MagicType::JumpStun)
    {
      effectDurationMs += PotentialSystem::GetShackleDurationBonusMs(
        horseRegistry, attackerRacer->potential);
    }
  }

  return effectDurationMs;
}

// Function to select a random item based on position weights
const registry::Magic::SlotInfo& MagicSystem::SelectMagicTypeByPosition(
  const registry::MagicRegistry& magicRegistry,
  uint32_t position,
  bool isTeam)
{
  // Validate position
  if (position > 7)
    throw std::out_of_range("Position must be between 0 and 7");

  // Get the weights for the specified position
  const auto& positionSlotInfoWeights = isTeam ? magicRegistry.GetTeamPositionWeights(position) : magicRegistry.GetSoloPositionWeights(position);

  // Keep reference to weights only for the discrete distribution
  const auto& positionWeights = positionSlotInfoWeights | std::views::keys;

  // Create a discrete distribution based on the weights
  std::discrete_distribution<uint32_t> dist(
    positionWeights.cbegin(),
    positionWeights.cend());

  // Select a random index based on the distribution
  // and map the discrete index to the corresponding magic item
  const uint32_t selectedIndex = dist(server::util::GetRandomEngine());
  return positionSlotInfoWeights[selectedIndex].second;
}

const registry::Magic::SlotInfo& MagicSystem::RandomMagicItem(
  const registry::MagicRegistry& magicRegistry,
  tracker::RaceTracker& tracker,
  data::Uid racerUid)
{
  const auto& racer = tracker.GetRacer(racerUid);

  // Determine the racer's position (0 = 1st place)
  uint32_t racerPosition = 0;
  const auto& allRacers = tracker.GetRacers();
  size_t totalRacers = allRacers.size();

  for (const auto& [uid, instanceRacer] : allRacers)
  {
    if (uid == racerUid)
      continue;

    // TODO: do we ignore disconnected racers too?

    // Check if instance racer is ahead of the racer requesting item
    if (instanceRacer.raceProgress > racer.raceProgress)
      racerPosition++;
  }

  // Map the actual position to one of the 8 weight slots [0, 7] via linear interpolation.
  // This ensures last place in a small race gets "last-place" item weights.
  uint32_t effectivePosition = racerPosition;
  if (totalRacers > 1)
  {
    effectivePosition = static_cast<uint32_t>(
      static_cast<float>(racerPosition) * 7.0f /
      static_cast<float>(totalRacers - 1));
  }

  // Clamp effective position to [0, 7] for safety
  effectivePosition = std::clamp(effectivePosition, 0u, 7u);

  const registry::Magic::SlotInfo& magicSlotInfo = SelectMagicTypeByPosition(
    magicRegistry,
    effectivePosition,
    racer.team == protocol::TeamColor::Red or racer.team == protocol::TeamColor::Blue);

  uint32_t critChanceBp = magicRegistry.GetBaseCritChanceBp();
  if (magicSlotInfo.criticalType != 0)
  {
    if (const auto* scaling = magicRegistry.GetStatScaling(magicSlotInfo.basicType))
    {
      const uint32_t statValue = GetMountStatValue(racer.mountStats, scaling->stat);
      critChanceBp += scaling->critStepBp * (statValue / 10u);
    }

    // Mount-equipment set bonus: increased critical spell chance.
    if (racer.activeSetEffect == registry::SetEquipEffect::CriticalSpellChance)
      critChanceBp += magicRegistry.GetSetBonusInfo().critChanceBonusBp;
  }

  if (std::uniform_int_distribution<int>(0, 9999)(server::util::GetRandomEngine()) < static_cast<int>(critChanceBp))
    return magicRegistry.GetSlotInfo(magicSlotInfo.criticalType);

  return magicSlotInfo;
}

} // namespace server::race
