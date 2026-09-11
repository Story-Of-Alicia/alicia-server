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

#ifndef REGISTRYDEFINITIONS_HPP
#define REGISTRYDEFINITIONS_HPP

#include <cstdint>
#include <string_view>

namespace server::registry
{

enum class Region : uint32_t
{
  Unknown = 0,
  Meadow = 1,
  Forest = 2,
  City = 3,
  Desert = 4,
  Ice = 5
};

enum class UserAchvEvent : uint32_t
{
  None = 0,

  //! A race ended. Covers goal-in, placing and team results.
  RaceCompleted = 2,
  //! An item was bought in the shop.
  ItemPurchase = 3,
  //! The character leveled up.
  CharacterLevel = 5,
  //! A breeding attempt was made (regardless of outcome).
  BreedAttempt = 10,
  //! A breeding attempt succeeded.
  BreedSuccess = 11,
  //! A breeding attempt failed.
  BreedFail = 12,
  //! A horse was registered as a stud/sire for others to breed with.
  BreedRegister = 13,
  //! A registered stud/sire was bred by someone within one registration cycle.
  StudBreedCount = 14,
  //! A spur/boost charge was consumed.
  SpurUsed = 17,
  //! A deckItem was collected.
  CollectDropItem = 20,
  ScreenCapture = 23,
  CourseOut = 24,
  GlidingCount = 25,
  SlidingCount = 26,
  GlidingSpurCount = 27,
  PerfectStart = 28,
  PerfectJumpCount = 29,
  GoodJumpCount = 30,
  JumpFailCount = 31,
  MaxPerfectJumpCombo = 32,
  MaxSlidingTime = 33,
  MaxGlidingTime = 34,
  HotRoddingReversalCount = 35,
  MaxVelocity = 38,
  //! A successful fireball/bolt attack.
  FireballAttack = 42,
  GlidingDistance = 43,
  MaxGlidingDistance = 44,
  IntroEnd = 45,
  BalloonLapTime = 46,
  //! A horse was named for the first time.
  HorseNamed = 47,
  //! A horse was groomed (any part).
  Grooming = 49,
  //! A horse's body was washed.
  BodyWash = 50,
  //! A horse's mane was brushed.
  HorseBrushed = 51,
  //! A horse's tail was cleaned.
  HorseTailCleaned = 52,
  //! A horse was fed.
  Feeding = 53,
  //! Playing with a horse in the horse care minigame.
  HorsePlay = 54,
  //! A horse was released back to nature (retired).
  HorseReleased = 55,
  //! A potential capacity was maxed out.
  PotentialMaxed = 56,
  GoalInUseSpur = 57,
  GoalInUseSliding = 58,
  GoalInUseGliding = 59,
  GetSpurOnGliding = 60,
  PerfectSpurCombo = 61,
  //! The magic target (e.g. a passed dragon/summon) was changed.
  ChangeMagicTarget = 62,
  MidairAttackCount = 63,
  //! The shop menu was entered for the first time.
  ShopVisited = 65,
  //! The equipment menu was entered for the first time.
  EquipmentMenuVisited = 66,
  //! The horse care menu was entered for the first time.
  HorseCareMenuVisited = 67,
  //! All 4 horse proficiencies were achieved.
  AllProficienciesAchieved = 68,
  //! A horse was injured.
  HorseInjured = 69,
  //! A horse's injury was treated.
  HorseInjuryTreated = 70,
  //! A horse's injury healed on its own.
  HorseInjuryHealedNaturally = 71,
  GoalInUseBooster = 72,
  MidairAttackFireballCount = 74,
  NPCDialogLevel = 75,
};

//! Game mode flag bitmask.
enum class GameModeFlag : uint32_t
{
  None           = 0,
  SpeedTeam      = 2,
  MagicTeam      = 8,
  WinSpeedSolo   = 33,
  SpeedSoloAction = 35,  //!< Perfect jumps, boosts
  WinMagicSolo   = 68,
  MagicSoloAction = 76,  //!< Bolt attack
  Any            = 111,
};

//! Completion condition type.
enum class Function
{
  Unknown,
  True,                    //!< Used by "complete N races" quests.
  RunMap,                  //!< Complete a specific map (matched against functionValue).
  TeamWin,                 //!< Win a team race.
  PerfectJump,             //!< Land a perfect jump over a hurdle.
  FireballAttack,          //!< Hit an opponent with a fireball.
  CollectDropItem,         //!< Collect a drop item during a race.
  GlidingDistanceValue,    //!< Accumulate gliding distance.
  ClearMission,            //!< Clear a mission stage.
  PrizeWinnerForLowLevel,          //!< Place in the top 3 (low-level variant).
  PrizeWinnerInMapForLowLevel,     //!< Place in the top 3 on a specific map.
  BalloonLapTime,
  BreedingSuccessCombo,
  BuyItem,
  CarnivalSuccess,
  Cliff,
  ComboAttackSuccessWithFireball,
  CriticalFireballAttack,
  DialogLevelCheck,
  False,
  FavoriteFoodsFeeding,
  FireBallSuccess,
  ForciblyFeeding,
  GetMaxSpeed,
  GlidingDistance,
  GlidingSpur,
  GoalIn,
  GoalInRanking,
  GoalIn_16h_18h,
  GoalIn_19h_22h,
  GoalIn_8h_13h,
  GoodChild,
  GoodHorse,
  GradeCompare,
  HotRoddingReversalCount,
  JumpParalysis,
  LastSpurtAttack,
  LevelCheck,
  MaleHorseCombo,
  MaxPerfectJumpCombo,
  MaxSpurCount,
  MaxWinStop,
  PerfectSpurCombo,
  PerfectSpurMaster,
  PerfectStart,
  PolishUp,
  PoorHorse,
  PotentialCapacities,
  PotentialCapacitiesGrow,
  Retire,
  Revenge,
  RevengeAttackWithFireball,
  ReversalWinner,
  ShieldEndAttackWithFireball,
  SlidingTime,
  Spur3thGoalIn,
  Stuffed,
  TeamPerfectWinNew,
  TeamWinTheLast,
  TrainingCombo,
  TrainingCritical,
  TraningSuccess,
  UseAllMagic,
  Win,
  WinAI,
};

//! Maps a Function's client string.
//! @param value Client Function string (e.g. "TRUE", "RunMap", "BuyItem").
//! @returns The matching Function, or Function::Unknown if unrecognised.
[[nodiscard]] Function ParseFunction(std::string_view value);

} // namespace server::registry

#endif // REGISTRYDEFINITIONS_HPP
