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

#include "server/system/RanchManagementSystem.hpp"

#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

#include <unordered_set>

namespace server
{

namespace
{

bool IsHousingActive(
  const data::Housing& housing,
  const registry::HousingInfo& housingInfo)
{
  if (housingInfo.category == registry::IncubatorCategory)
    return true;

  return housing.expiresAt() > data::Clock::now();
}

} // anon namespace

RanchManagementSystem::RanchManagementSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

RanchManagementSystem::Payout RanchManagementSystem::CalculatePayout(
  const data::Character& character) const
{
  const auto& housingRegistry = _serverInstance.GetHousingRegistry();

  const auto housingRecords = _serverInstance.GetDataDirector().GetHousingCache().Get(
    character.housing());
  if (not housingRecords)
    return {};

  Payout payout{};
  std::unordered_set<uint32_t> builtHousingIds;

  for (const auto& housingRecord : *housingRecords)
  {
    housingRecord.Immutable(
      [&housingRegistry, &payout, &builtHousingIds](const data::Housing& housing)
      {
        const auto* housingInfo = housingRegistry.GetHousing(housing.housingId());
        if (not housingInfo || not IsHousingActive(housing, *housingInfo))
          return;

        builtHousingIds.emplace(housingInfo->id);
        payout.ranchExperience += housingInfo->regularExp;
        payout.carrots += housingInfo->regularMoney;
      });
  }

  // A completed housing set raises the whole payout by its bonus.
  const uint32_t bonusPercent = housingRegistry.GetSetBonusPercent(builtHousingIds);
  if (bonusPercent > 0)
  {
    const auto applyBonus = [bonusPercent](const uint32_t value)
    {
      const uint64_t bonus = (static_cast<uint64_t>(value) * bonusPercent + 99) / 100;
      return static_cast<uint32_t>(value + bonus);
    };

    payout.ranchExperience = applyBonus(payout.ranchExperience);
    payout.carrots = applyBonus(payout.carrots);
  }

  return payout;
}

std::optional<RanchManagementSystem::Payout> RanchManagementSystem::RecordRaceCompletion(
  const data::Uid characterUid)
{
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
  if (not characterRecord)
    return std::nullopt;

  std::optional<Payout> payout;

  characterRecord.Mutable(
    [this, &payout](data::Character& character)
    {
      const Payout calculated = CalculatePayout(character);
      if (calculated.ranchExperience == 0 && calculated.carrots == 0)
        return;

      character.ranchManagement.totalRaces() = character.ranchManagement.totalRaces() + 1;

      if (character.ranchManagement.totalRaces() % RacesPerPayout != 0)
        return;

      character.ranchManagement.ranchExperience() =
        character.ranchManagement.ranchExperience() + calculated.ranchExperience;
      character.carrots() = character.carrots() + calculated.carrots;

      payout = calculated;
    });

  return payout;
}

} // namespace server
