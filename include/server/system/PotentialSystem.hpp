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

#ifndef POTENTIALSYSTEM_HPP
#define POTENTIALSYSTEM_HPP

#include "server/tracker/RaceTracker.hpp"

#include <libserver/registry/HorseRegistry.hpp>

#include <cstdint>

namespace server
{

//! System responsible for the passive bonuses a mount's potential grants its rider.
//! A potential has a type, which picks the bonus, and a value, which scales how
//! strongly it applies: the magnitude configured in the potential registry is the
//! one at the maximum value, and scales linearly down to nothing at zero.
class PotentialSystem
{
public:
  //! A potential type, as listed in the potential registry.
  enum class Type : uint32_t
  {
    None = 0,
    NaturalTalent = 1,
    SpeedAddict = 2,
    LuckyCharm = 3,
    IronHeart = 4,
    OnePlusOne = 5,
    SuperiorReward = 6,
    Perfectionist = 7,
    Sniper = 8,
    IronWall = 9,
    Opportunity = 10,
    Fearless = 11,
    SuperiorStart = 13,
    TwoHearts = 14,
    Bulldozer = 15,
  };

  //! The highest potential value a mount can grow to, matching the cap the
  //! horse registry applies on potential growth.
  static constexpr uint32_t MaxPotentialValue = 100;

  //! Returns whether the racer's Lucky Charm saves the magic item they are
  //! holding from an attack that would strip it.
  static bool PreventsMagicItemLoss(
    const registry::HorseRegistry& horseRegistry,
    const tracker::RaceTracker::Racer::PotentialSnapshot& potential);

  //! Returns whether the racer's One Plus One grants them an extra magic item
  //! for landing an attack.
  static bool GrantsExtraMagicItem(
    const registry::HorseRegistry& horseRegistry,
    const tracker::RaceTracker::Racer::PotentialSnapshot& potential);

  //! Returns the extra duration the racer's Iron Wall adds to a shield they cast.
  static uint32_t GetShieldDurationBonusMs(
    const registry::HorseRegistry& horseRegistry,
    const tracker::RaceTracker::Racer::PotentialSnapshot& potential);

  //! Returns the extra duration the racer's Opportunity adds to the shackles they land.
  static uint32_t GetShackleDurationBonusMs(
    const registry::HorseRegistry& horseRegistry,
    const tracker::RaceTracker::Racer::PotentialSnapshot& potential);

private:
  //! Returns the registry entry for the potential, or `nullptr` if the mount
  //! does not carry the potential the caller is asking about.
  static const registry::PotentialInfo* GetInfo(
    const registry::HorseRegistry& horseRegistry,
    const tracker::RaceTracker::Racer::PotentialSnapshot& potential,
    Type type);

  //! Rolls the potential's configured chance, scaled by its value.
  static bool Rolls(
    const registry::HorseRegistry& horseRegistry,
    const tracker::RaceTracker::Racer::PotentialSnapshot& potential,
    Type type);

  //! Returns the potential's configured duration bonus, scaled by its value.
  static uint32_t GetDurationBonusMs(
    const registry::HorseRegistry& horseRegistry,
    const tracker::RaceTracker::Racer::PotentialSnapshot& potential,
    Type type);

  //! Scales a magnitude configured for the maximum potential value down to the
  //! value the mount has actually grown to.
  static uint32_t Scale(uint32_t magnitude, uint32_t potentialValue);
};

} // namespace server

#endif // POTENTIALSYSTEM_HPP
