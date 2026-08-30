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

#ifndef HORSESYSTEM_HPP
#define HORSESYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>

#include <chrono>
#include <unordered_map>

namespace server
{

class ServerInstance;

//! Runtime horse-lifecycle operations, such as maturing foals into adults.
class HorseSystem
{
public:
  explicit HorseSystem(ServerInstance& serverInstance);

  //! The duration a foal must age before it matures into an adult horse.
  static constexpr std::chrono::hours FoalGrowUpDuration{1};

  //! @param characterUid UID of the owning character.
  //! @returns The still-maturing foals mapped to the time each becomes an adult.
  std::unordered_map<data::Uid, data::Clock::time_point> PromoteMaturedFoals(
    data::Uid characterUid);

  //! @param characterUid UID of the owning character.
  //! @returns The number of horses whose lineage was raised.
  uint32_t RepairLineages(data::Uid characterUid);

  //! Computes and checks if the horse can eat based on
  //! dynamic food preference as used by the game client.
  static uint16_t CanHorseEat(
    data::Uid horseUid,
    uint16_t plenitude,
    uint32_t preferenceType);

  //! Applies post-race condition debuffs to the specified horse.
  //! Deducts charm, friendliness, and plenitude, accumulates dirtiness,
  //! and resets polish.
  //! @param horse Horse record to modify.
  //! @param characterLevel Level of the character that raced the horse.
  void ApplyPostRaceHorseConditionDebuffs(
    data::Horse& horse,
    uint32_t characterLevel);

  static constexpr uint16_t MaxPlenitude = 1'000;
  static constexpr uint16_t MaxDirtiness = 1'000;
  static constexpr uint16_t MaxPolish = 1'000;
  static constexpr uint16_t MaxFriendliness = 1'000;
  static constexpr uint16_t MaxCharm = 1'000;
  static constexpr uint16_t MaxAttachment = 1'000;
  static constexpr uint16_t MaxBoredom = 21;
  static constexpr uint16_t MaxStamina = 4'000;
  //! Maximum fatigue limit before horse experience gain is suppressed
  //! See libconfig: FatigueParam->FatigueLimit
  static constexpr uint32_t MaxFatigue = 1'500;

  //! Post-race charm point deduction.
  //! This is hardcoded in the client.
  static constexpr uint32_t PostRaceCharmDeduction = 10;
  //! Post-race friendliness/intimacy deduction.
  //! This is hardcoded in the client.
  static constexpr uint32_t PostRaceFriendlinessDeduction = 20;
  //! Post-race plenitude deduction.
  //! This is hardcoded in the client.
  static constexpr uint32_t PostRacePlenitudeDeduction = 50;

  //! Post-race dirtiness increase per body part
  //! See libconfig: MountGradeInfo->CleanPointSub
  static constexpr uint32_t PostRaceDirtinessIncrease = 5;
  //! Default post-race fatigue penalty
  //! See libconfig: FatigueParam->FatigueDefaultIncrease
  static constexpr uint32_t PostRaceFatigueDeductionDefault = 30;
  //! Low-level post-race fatigue penalty
  //! See libconfig: FatigueParam->FatigueLowLevelIncrease
  static constexpr uint32_t PostRaceFatigueDeductionLowLevel = 10;
  //! Character level threshold for beginner fatigue protection
  //! See libconfig: FatigueParam->FatigueLowLevelLimit
  static constexpr uint32_t LowLevelThreshold = 15;

private:
  ServerInstance& _serverInstance;
};

} // namespace server

#endif // HORSESYSTEM_HPP
