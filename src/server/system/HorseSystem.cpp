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

} // namespace server
