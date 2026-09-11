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
  //! A horse was groomed (any part).
  Grooming = 49,
  //! A horse's body was washed.
  BodyWash = 50,
  //! A horse was fed.
  Feeding = 53,
  GoalInUseSpur = 57,
  GoalInUseSliding = 58,
  GoalInUseGliding = 59,
  GetSpurOnGliding = 60,
  PerfectSpurCombo = 61,
  MidairAttackCount = 63,
  GoalInUseBooster = 72,
  MidairAttackFireballCount = 74,
  NPCDialogLevel = 75,
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
};

} // namespace server::registry

#endif // REGISTRYDEFINITIONS_HPP
