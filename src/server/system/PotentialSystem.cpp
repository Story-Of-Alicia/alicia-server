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

#include "server/system/PotentialSystem.hpp"

#include <libserver/util/Util.hpp>

#include <algorithm>
#include <random>

namespace server
{

const registry::PotentialInfo* PotentialSystem::GetInfo(
  const registry::HorseRegistry& horseRegistry,
  const tracker::RaceTracker::Racer::PotentialSnapshot& potential,
  const Type type)
{
  if (potential.type != static_cast<uint32_t>(type))
    return nullptr;

  return horseRegistry.GetPotentialInfo(potential.type);
}

uint32_t PotentialSystem::Scale(const uint32_t magnitude, const uint32_t potentialValue)
{
  // Randomly generated mounts can carry a value above the growth cap.
  return magnitude * std::min(potentialValue, MaxPotentialValue) / MaxPotentialValue;
}

bool PotentialSystem::Rolls(
  const registry::HorseRegistry& horseRegistry,
  const tracker::RaceTracker::Racer::PotentialSnapshot& potential,
  const Type type)
{
  const auto* info = GetInfo(horseRegistry, potential, type);
  if (info == nullptr)
    return false;

  const uint32_t chanceBp = Scale(info->chanceBp, potential.value);
  if (chanceBp == 0)
    return false;

  return std::uniform_int_distribution<uint32_t>(0, 9999)(util::GetRandomEngine()) < chanceBp;
}

bool PotentialSystem::PreventsMagicItemLoss(
  const registry::HorseRegistry& horseRegistry,
  const tracker::RaceTracker::Racer::PotentialSnapshot& potential)
{
  return Rolls(horseRegistry, potential, Type::LuckyCharm);
}

bool PotentialSystem::GrantsExtraMagicItem(
  const registry::HorseRegistry& horseRegistry,
  const tracker::RaceTracker::Racer::PotentialSnapshot& potential)
{
  return Rolls(horseRegistry, potential, Type::OnePlusOne);
}

uint32_t PotentialSystem::GetDurationBonusMs(
  const registry::HorseRegistry& horseRegistry,
  const tracker::RaceTracker::Racer::PotentialSnapshot& potential,
  const Type type)
{
  const auto* info = GetInfo(horseRegistry, potential, type);
  if (info == nullptr)
    return 0;

  return Scale(info->durationBonusMs, potential.value);
}

uint32_t PotentialSystem::GetShieldDurationBonusMs(
  const registry::HorseRegistry& horseRegistry,
  const tracker::RaceTracker::Racer::PotentialSnapshot& potential)
{
  return GetDurationBonusMs(horseRegistry, potential, Type::IronWall);
}

uint32_t PotentialSystem::GetShackleDurationBonusMs(
  const registry::HorseRegistry& horseRegistry,
  const tracker::RaceTracker::Racer::PotentialSnapshot& potential)
{
  return GetDurationBonusMs(horseRegistry, potential, Type::Opportunity);
}

} // namespace server
