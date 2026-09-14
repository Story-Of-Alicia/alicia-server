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

#include "server/system/HorseSystem.hpp"

#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

#include <vector>

namespace server
{

HorseSystem::HorseSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

std::unordered_map<data::Uid, data::Clock::time_point> HorseSystem::PromoteMaturedFoals(
  const data::Uid characterUid)
{
  std::unordered_map<data::Uid, data::Clock::time_point> maturingFoals;

  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not characterRecord)
    return maturingFoals;

  std::vector<data::Uid> horseUids;
  characterRecord.Immutable([&horseUids](const data::Character& character)
  {
    horseUids = character.horses();
  });

  const auto now = data::Clock::now();
  for (const auto& horseUid : horseUids)
  {
    const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(horseUid);
    if (not horseRecord)
      continue;

    bool isFoal = false;
    data::Clock::time_point dateOfBirth;
    horseRecord.Immutable([&isFoal, &dateOfBirth](const data::Horse& horse)
    {
      isFoal = horse.type() == data::Horse::Type::Foal;
      dateOfBirth = horse.dateOfBirth();
    });

    if (not isFoal)
      continue;

    if (now >= dateOfBirth + FoalGrowUpDuration)
    {
      horseRecord.Mutable([](data::Horse& horse)
      {
        horse.type() = data::Horse::Type::Adult;
      });
    }
    else
    {
      maturingFoals.emplace(horseUid, dateOfBirth + FoalGrowUpDuration);
    }
  }

  return maturingFoals;
}

uint32_t HorseSystem::RepairLineages(const data::Uid characterUid)
{
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not characterRecord)
    return 0;

  std::vector<data::Uid> horseUids;
  characterRecord.Immutable([&horseUids](const data::Character& character)
  {
    horseUids = character.horses();
    horseUids.emplace_back(character.mountUid());
  });

  auto& genetics = _serverInstance.GetGenetics();

  uint32_t repairedCount = 0;
  for (const auto& horseUid : horseUids)
  {
    const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(horseUid);
    if (not horseRecord)
      continue;

    const uint32_t recalculated = genetics.RecalculateLineage(horseUid);

    uint32_t storedLineage = 0;
    horseRecord.Immutable([&storedLineage](const data::Horse& horse)
    {
      storedLineage = horse.lineage();
    });

    if (recalculated <= storedLineage)
      continue;

    horseRecord.Mutable([recalculated](data::Horse& horse)
    {
      horse.lineage() = recalculated;
    });
    ++repairedCount;

    spdlog::info(
      "Repaired the lineage of horse {} of character {}: {} -> {}",
      horseUid,
      characterUid,
      storedLineage,
      recalculated);
  }

  return repairedCount;
}

namespace
{
int64_t CareDayIndex(const data::Clock::time_point time)
{
  using namespace std::chrono;
  return floor<days>(time - hours(6)).time_since_epoch().count();
}

} // namespace

void HorseSystem::ApplyDailyCareTick(const data::Uid characterUid)
{
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not characterRecord)
    return;

  std::vector<data::Uid> horseUids;
  characterRecord.Immutable([&horseUids](const data::Character& character)
  {
    horseUids = character.horses();
    horseUids.emplace_back(character.mountUid());
  });

  const auto now = data::Clock::now();
  const auto currentDay = CareDayIndex(now);

  for (const auto& horseUid : horseUids)
  {
    const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(horseUid);
    if (not horseRecord)
      continue;

    horseRecord.Mutable([currentDay, now](data::Horse& horse)
    {
      const auto daysPassed = currentDay - CareDayIndex(horse.mountCondition.lastDailyCareTick());
      if (daysPassed <= 0)
        return;

      const auto ticks = static_cast<uint32_t>(daysPassed);

      horse.mountCondition.boredom() = std::min<uint32_t>(
        MaxBoredom,
        horse.mountCondition.boredom() + ticks * DailyBoredomRegenAmount);

      horse.mountCondition.bodyDirtiness() = std::min<uint32_t>(
        MaxDirtiness,
        horse.mountCondition.bodyDirtiness() + ticks * DailyDirtinessIncrease);
      horse.mountCondition.maneDirtiness() = std::min<uint32_t>(
        MaxDirtiness,
        horse.mountCondition.maneDirtiness() + ticks * DailyDirtinessIncrease);
      horse.mountCondition.tailDirtiness() = std::min<uint32_t>(
        MaxDirtiness,
        horse.mountCondition.tailDirtiness() + ticks * DailyDirtinessIncrease);

      horse.mountCondition.lastDailyCareTick() = now;
    });
  }
}

uint16_t HorseSystem::CanHorseEat(
  data::Uid horseUid,
  uint16_t plenitude,
  uint32_t preferenceType)
{
  // Hungry (< 710)
  // Slightly full (710..999)
  static constexpr uint32_t MinSlightlyFullPlenitude = 710;

  // If horse is full (>= 1000), it has no food preference (cannot eat)
  if (plenitude >= MaxPlenitude)
    return 0;

  // Hungry:        mode = 15 (0x0F)
  // Slightly full: mode = 16 (0x10)
  const uint32_t mode = plenitude < MinSlightlyFullPlenitude
    ? 0x0F
    : 0x10;

  // Cast horseUid to uint32_t for posterity
  uint32_t seed = static_cast<uint32_t>(horseUid) + mode;
  seed = (seed * 0x343FD) - 0x1613D;
  seed = (seed * 0x343FD) - 0x1613D;
  const uint32_t bitIndex = (seed >> 16) & 7;

  // Calculate the horse's preference bitmask
  const uint16_t mask = static_cast<uint16_t>(1 << bitIndex);
  return (mask & preferenceType) != 0;
}
// found in FUN_00766ac0 in the tag10 binary
uint32_t HorseSystem::CalculateFriendlinessCharmThreshold(
  const data::Uid horseUid,
  const CareAmendsCategory category,
  const uint32_t priority)
{
  const uint32_t rowCount = category == CareAmendsCategory::CharmPoint
    ? CharmPointMilestoneCount
    : FriendlyPointMilestoneCount;

  uint32_t seed = horseUid + static_cast<uint32_t>(category) + priority;
  seed = seed * 214013 + 2531011;
  seed = seed * 214013 + 2531011;
  const uint32_t bucket = (seed >> 16) & 0x7FFF;

  const uint32_t segmentSize = 1000 / rowCount;
  return 1 + (bucket % segmentSize) + (priority - 1) * segmentSize;
}

void HorseSystem::ApplyPostRaceHorseConditionDebuffs(
  data::Horse& horse,
  [[maybe_unused]] const uint32_t characterLevel)
{
  // Charm point reduction
  if (horse.mountCondition.charm() >= PostRaceCharmDeduction)
    horse.mountCondition.charm() -= PostRaceCharmDeduction;
  else
    horse.mountCondition.charm() = 0;

  // Friendliness reduction
  if (horse.mountCondition.friendliness() >= PostRaceFriendlinessDeduction)
    horse.mountCondition.friendliness() -= PostRaceFriendlinessDeduction;
  else
    horse.mountCondition.friendliness() = 0;

  // Plenitude reduction
  if (horse.mountCondition.plenitude() >= PostRacePlenitudeDeduction)
    horse.mountCondition.plenitude() -= PostRacePlenitudeDeduction;
  else
    horse.mountCondition.plenitude() = 0;

  // Dirtiness accumulation
  horse.mountCondition.bodyDirtiness() = std::min(
    static_cast<uint32_t>(MaxDirtiness),
    horse.mountCondition.bodyDirtiness() + PostRaceDirtinessIncrease);

  horse.mountCondition.maneDirtiness() = std::min(
    static_cast<uint32_t>(MaxDirtiness),
    horse.mountCondition.maneDirtiness() + PostRaceDirtinessIncrease);

  horse.mountCondition.tailDirtiness() = std::min(
    static_cast<uint32_t>(MaxDirtiness),
    horse.mountCondition.tailDirtiness() + PostRaceDirtinessIncrease);

  // Polish suppression (reset to 0 when dirtiness is accumulated)
  // TODO: is this behaviour correct? Do we immediately reset the polish after a race?
  horse.mountCondition.bodyPolish() = 0;
  horse.mountCondition.manePolish() = 0;
  horse.mountCondition.tailPolish() = 0;

  // TODO: Implement post-race stamina and fatigue updates:
  // - Deduct stamina and accumulate horse fatigue based on characterLevel
  //   (< LowLevelThreshold ? PostRaceFatigueDeductionLowLevel : PostRaceFatigueDeductionDefault)
  // - Clamp horse.fatigue() to MaxFatigue
  // - Handle daily fatigue reset (ResetFatiguePoint) and EXP gain suppression when at max fatigue

  spdlog::debug(
    "Applied post-race condition debuffs to horse {}: "
      "Charm={}, Friendly={}, Plenitude={}, Dirtiness=({},{},{})",
    horse.uid(),
    horse.mountCondition.charm(),
    horse.mountCondition.friendliness(),
    horse.mountCondition.plenitude(),
    horse.mountCondition.bodyDirtiness(),
    horse.mountCondition.maneDirtiness(),
    horse.mountCondition.tailDirtiness());
}

} // namespace server
