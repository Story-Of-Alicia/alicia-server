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

std::vector<data::Uid> HorseSystem::CollectCharacterFoals(
  const data::Character& character) const noexcept
{
  std::vector<data::Uid> maturingFoals;

  for (const auto& horseUid : character.horses())
  {
    const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(horseUid);
    if (not horseRecord)
      continue;

    bool isFoal = false;
    horseRecord.Immutable([&isFoal](const data::Horse& horse)
    {
      isFoal = horse.type() == data::Horse::Type::Foal;
    });

    if (not isFoal)
      continue;

    maturingFoals.emplace_back(horseUid);
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

} // namespace server
