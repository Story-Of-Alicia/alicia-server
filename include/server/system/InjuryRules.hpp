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

#ifndef INJURYRULES_HPP
#define INJURYRULES_HPP

#include <array>
#include <cstdint>
#include <optional>

namespace server::injury
{

//! Horse injury states, as in the client's InjuryInfo table.
namespace State
{

constexpr uint32_t None = 0;
constexpr uint32_t MinorMuscleStrain = 17;
constexpr uint32_t SevereMuscleStrain = 18;
constexpr uint32_t MinorWounds = 33;
constexpr uint32_t SevereWounds = 34;
constexpr uint32_t MinorFracture = 65;
constexpr uint32_t SevereFracture = 66;

} // namespace State

//! Races and days it takes a minor injury to heal on its own
//! (InjuryPointInfo table, HealInjury). Severe injuries need a cure item.
constexpr uint32_t NaturalHealCount = 5;

//! Returns whether the injury state is a minor injury.
[[nodiscard]] constexpr bool IsMinorInjury(const uint32_t injury)
{
  return injury == State::MinorMuscleStrain
    || injury == State::MinorWounds
    || injury == State::MinorFracture;
}

//! Play events the client reports during a race, as in the client's InjuryEvent table.
namespace PlayEvent
{

constexpr uint32_t Spur = 25;
constexpr uint32_t Booster = 80;
constexpr uint32_t HotRodding = 81;

} // namespace PlayEvent

//! A row of the client's InjuryEvent table: a play event that turns
//! one injury state into another.
struct EventRule
{
  uint32_t playEvent;
  uint32_t currentInjury;
  uint32_t injury;
  //! The server detects this event itself, the client does not report it.
  bool serverCheck;
};

constexpr std::array<EventRule, 17> EventRules{{
  {70, State::MinorMuscleStrain, State::SevereMuscleStrain, false}, // Sliding while spurring
  {71, State::MinorMuscleStrain, State::SevereMuscleStrain, false}, // Sliding while boosting
  {72, State::MinorMuscleStrain, State::SevereMuscleStrain, false}, // Sliding while hot rodding
  {73, State::MinorWounds, State::SevereWounds, false}, // Jumping again right after a jump
  {31, State::MinorFracture, State::SevereFracture, false}, // Spurring while gliding
  {74, State::MinorFracture, State::SevereFracture, false}, // Boosting while gliding
  {75, State::MinorFracture, State::SevereFracture, false}, // Hot rodding while gliding
  {76, State::MinorFracture, State::SevereFracture, false}, // Gliding while spurring
  {77, State::MinorFracture, State::SevereFracture, false}, // Gliding while boosting
  {78, State::MinorFracture, State::SevereFracture, false}, // Gliding while hot rodding
  {79, State::None, State::MinorMuscleStrain, false}, // Sliding at 75 km/h or more
  {40, State::None, State::MinorWounds, false}, // Landing after jumping an obstacle
  {PlayEvent::Spur, State::None, State::MinorFracture, true}, // Using a spur
  {PlayEvent::Booster, State::None, State::MinorFracture, true}, // Using a booster
  {PlayEvent::HotRodding, State::None, State::MinorFracture, true}, // Using hot rodding
  {79, State::MinorMuscleStrain, State::SevereMuscleStrain, false}, // Sliding
  {30, State::MinorFracture, State::SevereFracture, false}, // Gliding
}};

//! A row of the client's InjuryInfo table.
//! The item that cures each injury is set in the item registry (cureParameters).
struct InjuryInfo
{
  uint32_t injury;
  //! The lowest horse grade that can get this injury.
  uint32_t minGrade;
};

constexpr std::array<InjuryInfo, 6> InjuryInfos{{
  {State::MinorMuscleStrain, 1},
  {State::MinorWounds, 4},
  {State::MinorFracture, 7},
  {State::SevereMuscleStrain, 1},
  {State::SevereWounds, 4},
  {State::SevereFracture, 7},
}};

//! Returns the injury a play event causes for a horse with the given
//! injury state, or nothing if the event does not affect that state.
[[nodiscard]] constexpr std::optional<uint32_t> GetInjuryForEvent(
  const uint32_t playEvent,
  const uint32_t currentInjury,
  const bool isServerCheck)
{
  for (const auto& rule : EventRules)
  {
    if (rule.playEvent == playEvent
      && rule.currentInjury == currentInjury
      && rule.serverCheck == isServerCheck)
    {
      return rule.injury;
    }
  }

  return std::nullopt;
}

//! Returns the info of an injury, or nullptr if the state is not an injury.
[[nodiscard]] constexpr const InjuryInfo* FindInjuryInfo(const uint32_t injury)
{
  for (const auto& info : InjuryInfos)
  {
    if (info.injury == injury)
      return &info;
  }

  return nullptr;
}

} // namespace server::injury

#endif // INJURYRULES_HPP
