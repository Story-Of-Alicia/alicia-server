/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
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

#include "server/ranch/Genetics.hpp"
#include "server/ServerInstance.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

namespace server
{

Genetics::Genetics(ServerInstance& serverInstance)
  : _serverInstance(serverInstance),
    _randomEngine(std::random_device{}())
{
}

int32_t Genetics::GetColorGroupIdFromTid(data::Tid maneTid, bool isMane)
{
  if (isMane)
  {
    return _serverInstance.GetHorseRegistry().GetManeColorGroupId(maneTid);
  }
  else
  {
    return _serverInstance.GetHorseRegistry().GetTailColorGroupId(maneTid);
  }
}

int32_t Genetics::GetShapeFromTid(data::Tid tid, bool isMane)
{
  if (isMane)
  {
    return _serverInstance.GetHorseRegistry().GetMane(tid).shape;
  }
  else
  {
    return _serverInstance.GetHorseRegistry().GetTail(tid).shape;
  }
}

Genetics::ManeTailResult Genetics::CalculateManeTailGenetics(
  data::Uid,
  data::Uid,
  uint8_t,
  data::Tid)
{
  return {};
}

uint8_t Genetics::CalculateFoalGrade(uint8_t, uint8_t)
{
  return 0;
}

uint32_t Genetics::CalculateFoalStat(uint32_t mareStat, uint32_t stallionStat)
{
  // Calculate base average
  uint32_t avgStat = (mareStat + stallionStat) / 2;
  
  // Add variance: ±20% of average
  int32_t variance = (avgStat * 20) / 100;
  int32_t randomVariance = (rand() % (variance * 2 + 1)) - variance;
  
  int32_t finalStat = avgStat + randomVariance;
  
  // Clamp to reasonable range (0-100)
  if (finalStat < 0) finalStat = 0;
  if (finalStat > 100) finalStat = 100;
  
  return static_cast<uint32_t>(finalStat);
}

uint8_t Genetics::CalculateGradeFromStats(uint32_t totalStats)
{
  // Grade based on total stats:
  // Grade 1: 0-9
  // Grade 2: 10-19
  // Grade 3: 20-29
  // Grade 4: 30-39
  // Grade 5: 40-49
  // Grade 6: 50-59
  // Grade 7: 60-69
  // Grade 8: 70-79 (capped at 79)
  
  if (totalStats < 10) return 1;
  if (totalStats >= 79) return 8;
  
  // Calculate grade from total (dividing by 10, adding 1)
  return static_cast<uint8_t>(totalStats / 10 + 1);
}

Genetics::StatResult Genetics::CalculateFoalStats(
  uint32_t,
  uint32_t,
  uint32_t,
  uint32_t,
  uint32_t,
  uint32_t,
  uint32_t,
  uint32_t,
  uint32_t,
  uint32_t,
  uint8_t)
{
  return {};
}

Genetics::PotentialResult Genetics::CalculateFoalPotential(
  data::Uid,
  data::Uid,
  data::Tid)
{
  return {};
}

data::Tid Genetics::CalculateFoalSkin(
  data::Uid,
  data::Uid,
  uint8_t,
  uint32_t,
  uint32_t,
  uint32_t)
{
  return 0;
}

uint32_t Genetics::CalculateLineage(
  data::Tid,
  data::Uid,
  data::Uid)
{
  return 0;
}

} // namespace server

