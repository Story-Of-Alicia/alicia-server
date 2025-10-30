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

#include <spdlog/spdlog.h>

namespace server
{

Genetics::Genetics(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

int32_t Genetics::GetColorFromTid(data::Tid tid)
{
  return ((tid - 1) % 5) + 1;  // Returns 1-5
}

int32_t Genetics::GetShapeFromTid(data::Tid tid)
{
  return ((tid - 1) / 5);  // Groups TIDs by sets of 5
}

void Genetics::ValidateManeShape(int32_t& maneShape, uint8_t foalGrade)
{
  // Apply grade restrictions to mane shapes
  // Spiky (shape 5): Requires Grade 4
  // Long (shape 6): Requires Grade 6
  // Long Curly (shape 7): Requires Grade 7
  if (maneShape == 5 && foalGrade < 4)
    maneShape = rand() % 5;  // Fall back to basic shapes (0-4)
  else if (maneShape == 6 && foalGrade < 6)
    maneShape = rand() % 5;
  else if (maneShape == 7 && foalGrade < 7)
    maneShape = rand() % 5;
}

void Genetics::ValidateTailShape(int32_t& tailShape, uint8_t foalGrade)
{
  // Apply grade restrictions to tail shapes
  // Long Curly tail (shape 5): Requires Grade 7
  if (tailShape == 5 && foalGrade < 7)
    tailShape = rand() % 5;  // Fall back to basic shapes (0-4)
}

Genetics::ManeTailResult Genetics::CalculateManeTailGenetics(
  data::Uid mareUid,
  data::Uid stallionUid,
  uint8_t foalGrade)
{
  ManeTailResult result;

  // Get mare and stallion records
  auto mareRecord = _serverInstance.GetDataDirector().GetHorse(mareUid);
  auto stallionRecord = _serverInstance.GetDataDirector().GetHorse(stallionUid);

  if (!mareRecord || !stallionRecord)
  {
    spdlog::error("Genetics: Mare or stallion not found");
    return result;
  }

  // Extract parent and ancestor data
  data::Tid mareMane = 0, mareTail = 0;
  data::Tid stallionMane = 0, stallionTail = 0;
  std::vector<data::Uid> mareAncestors, stallionAncestors;

  mareRecord.Immutable([&](const data::Horse& mare)
  {
    mareMane = mare.parts.maneTid();
    mareTail = mare.parts.tailTid();
    mareAncestors = mare.ancestors();
  });

  stallionRecord.Immutable([&](const data::Horse& stallion)
  {
    stallionMane = stallion.parts.maneTid();
    stallionTail = stallion.parts.tailTid();
    stallionAncestors = stallion.ancestors();
  });

  // === MANE GENETICS ===

  // Mane Color: Mom 20%, Dad 20%, Maternal GP 10%, Paternal GP 10%, Random 40%
  int maneColorRoll = rand() % 100;
  int32_t maneColor = 0;

  if (maneColorRoll < 20)
  {
    maneColor = GetColorFromTid(mareMane);  // 20% Mom
  }
  else if (maneColorRoll < 40)
  {
    maneColor = GetColorFromTid(stallionMane);  // 20% Dad
  }
  else if (maneColorRoll < 50 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneColor = GetColorFromTid(gp.parts.maneTid()); });
  }
  else if (maneColorRoll < 60 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneColor = GetColorFromTid(gp.parts.maneTid()); });
  }

  if (maneColor == 0) maneColor = 1 + (rand() % 5);  // Random fallback

  // Mane Shape: Mom 20%, Dad 20%, Maternal GP 10%, Paternal GP 10%, Random 40%
  int maneShapeRoll = rand() % 100;
  int32_t maneShape = 0;

  if (maneShapeRoll < 20)
  {
    maneShape = GetShapeFromTid(mareMane);  // 20% Mom
  }
  else if (maneShapeRoll < 40)
  {
    maneShape = GetShapeFromTid(stallionMane);  // 20% Dad
  }
  else if (maneShapeRoll < 50 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneShape = GetShapeFromTid(gp.parts.maneTid()); });
  }
  else if (maneShapeRoll < 60 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneShape = GetShapeFromTid(gp.parts.maneTid()); });
  }

  if (maneShape == 0 && maneShapeRoll >= 60)
    maneShape = rand() % 8;  // Random shape (8 groups)

  // Validate mane shape against grade
  ValidateManeShape(maneShape, foalGrade);

  // === TAIL GENETICS (independent) ===

  // Tail Color
  int tailColorRoll = rand() % 100;
  int32_t tailColor = 0;

  if (tailColorRoll < 20)
    tailColor = GetColorFromTid(mareTail);
  else if (tailColorRoll < 40)
    tailColor = GetColorFromTid(stallionTail);
  else if (tailColorRoll < 50 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailColor = GetColorFromTid(gp.parts.tailTid()); });
  }
  else if (tailColorRoll < 60 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailColor = GetColorFromTid(gp.parts.tailTid()); });
  }

  if (tailColor == 0) tailColor = 1 + (rand() % 5);

  // Tail Shape
  int tailShapeRoll = rand() % 100;
  int32_t tailShape = 0;

  if (tailShapeRoll < 20)
    tailShape = GetShapeFromTid(mareTail);
  else if (tailShapeRoll < 40)
    tailShape = GetShapeFromTid(stallionTail);
  else if (tailShapeRoll < 50 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailShape = GetShapeFromTid(gp.parts.tailTid()); });
  }
  else if (tailShapeRoll < 60 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailShape = GetShapeFromTid(gp.parts.tailTid()); });
  }

  if (tailShape == 0 && tailShapeRoll >= 60)
    tailShape = rand() % 6;  // Random shape (6 groups)

  // Validate tail shape against grade
  ValidateTailShape(tailShape, foalGrade);

  // Combine color + shape into final TIDs
  result.maneColor = maneColor;
  result.maneShape = maneShape;
  result.maneTid = (maneShape * 5) + maneColor;
  if (result.maneTid < 1) result.maneTid = maneColor;
  if (result.maneTid > 40) result.maneTid = 40;

  result.tailColor = tailColor;
  result.tailShape = tailShape;
  result.tailTid = (tailShape * 5) + tailColor;
  if (result.tailTid < 1) result.tailTid = tailColor;
  if (result.tailTid > 30) result.tailTid = 30;

  return result;
}

uint8_t Genetics::CalculateFoalGrade(uint8_t mareGrade, uint8_t stallionGrade)
{
  // Simple average for now
  // TODO: Use BreedingGradeProbInfo table for proper probability distribution
  uint8_t avgGrade = (mareGrade + stallionGrade) / 2;
  return std::min((uint8_t)8, avgGrade);
}

uint32_t Genetics::CalculateFoalStat(uint32_t mareStat, uint32_t stallionStat)
{
  // Simple average for now
  // TODO: Add variance/randomization based on game configuration
  return (mareStat + stallionStat) / 2;
}

} // namespace server

