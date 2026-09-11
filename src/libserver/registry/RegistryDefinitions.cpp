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

#include "libserver/registry/RegistryDefinitions.hpp"

#include <string>
#include <unordered_map>

namespace server::registry
{

Function ParseFunction(const std::string_view value)
{
  static const std::unordered_map<std::string, Function> lookup{
    {"TRUE", Function::True},
    {"True", Function::True},
    {"False", Function::False},
    {"RunMap", Function::RunMap},
    {"TeamWin", Function::TeamWin},
    {"PerfectJump", Function::PerfectJump},
    {"FireballAttack", Function::FireballAttack},
    {"CollectDropItem", Function::CollectDropItem},
    {"GlidingDistanceValue", Function::GlidingDistanceValue},
    {"ClearMission", Function::ClearMission},
    {"PrizeWinnerForLowLevel", Function::PrizeWinnerForLowLevel},
    {"PrizeWinnerInMapForLowLevel", Function::PrizeWinnerInMapForLowLevel},
    {"BalloonLapTime", Function::BalloonLapTime},
    {"BreedingSuccessCombo", Function::BreedingSuccessCombo},
    {"BuyItem", Function::BuyItem},
    {"CarnivalSuccess", Function::CarnivalSuccess},
    {"Cliff", Function::Cliff},
    {"ComboAttackSuccessWithFireball", Function::ComboAttackSuccessWithFireball},
    {"CriticalFireballAttack", Function::CriticalFireballAttack},
    {"DialogLevelCheck", Function::DialogLevelCheck},
    {"FavoriteFoodsFeeding", Function::FavoriteFoodsFeeding},
    {"FireBallSuccess", Function::FireBallSuccess},
    {"ForciblyFeeding", Function::ForciblyFeeding},
    {"GetMaxSpeed", Function::GetMaxSpeed},
    {"GlidingDistance", Function::GlidingDistance},
    {"GlidingSpur", Function::GlidingSpur},
    {"GoalIn", Function::GoalIn},
    {"GoalInRanking", Function::GoalInRanking},
    {"GoalIn_16h_18h", Function::GoalIn_16h_18h},
    {"GoalIn_19h_22h", Function::GoalIn_19h_22h},
    {"GoalIn_8h_13h", Function::GoalIn_8h_13h},
    {"GoodChild", Function::GoodChild},
    {"GoodHorse", Function::GoodHorse},
    {"GradeCompare", Function::GradeCompare},
    {"HotRoddingReversalCount", Function::HotRoddingReversalCount},
    {"JumpParalysis", Function::JumpParalysis},
    {"LastSpurtAttack", Function::LastSpurtAttack},
    {"LevelCheck", Function::LevelCheck},
    {"MaleHorseCombo", Function::MaleHorseCombo},
    {"MaxPerfectJumpCombo", Function::MaxPerfectJumpCombo},
    {"MaxSpurCount", Function::MaxSpurCount},
    {"MaxWinStop", Function::MaxWinStop},
    {"PerfectSpurCombo", Function::PerfectSpurCombo},
    {"PerfectSpurMaster", Function::PerfectSpurMaster},
    {"PerfectStart", Function::PerfectStart},
    {"PolishUp", Function::PolishUp},
    {"PoorHorse", Function::PoorHorse},
    {"PotentialCapacities", Function::PotentialCapacities},
    {"PotentialCapacitiesGrow", Function::PotentialCapacitiesGrow},
    {"Retire", Function::Retire},
    {"Revenge", Function::Revenge},
    {"RevengeAttackWithFireball", Function::RevengeAttackWithFireball},
    {"ReversalWinner", Function::ReversalWinner},
    {"ShieldEndAttackWithFireball", Function::ShieldEndAttackWithFireball},
    {"SlidingTime", Function::SlidingTime},
    {"Spur3thGoalIn", Function::Spur3thGoalIn},
    {"Stuffed", Function::Stuffed},
    {"TeamPerfectWinNew", Function::TeamPerfectWinNew},
    {"TeamWinTheLast", Function::TeamWinTheLast},
    {"TrainingCombo", Function::TrainingCombo},
    {"TrainingCritical", Function::TrainingCritical},
    {"TraningSuccess", Function::TraningSuccess},
    {"UseAllMagic", Function::UseAllMagic},
    {"Win", Function::Win},
    {"WinAI", Function::WinAI},
  };

  const auto iter = lookup.find(std::string{value});
  if (iter == lookup.cend())
    return Function::Unknown;
  return iter->second;
}

} // namespace server::registry
