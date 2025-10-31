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
    return _serverInstance.GetHorseRegistry().GetManeShape(tid);
  }
  else
  {
    return _serverInstance.GetHorseRegistry().GetTailShape(tid);
  }
}

void Genetics::ValidateManeShape(int32_t& maneShape, uint8_t foalGrade)
{
  // Apply grade restrictions to mane shapes
  // Spiky (shape 5): Requires Grade 4
  // Long (shape 6): Requires Grade 6
  // Long Curly (shape 7): Requires Grade 7
  
  // Determine max allowed shape for this grade
  int32_t maxAllowedShape = 4; // Default: basic shapes (0-4)
  if (foalGrade >= 7)
    maxAllowedShape = 7; // Can have Long Curly
  else if (foalGrade >= 6)
    maxAllowedShape = 6; // Can have Long
  else if (foalGrade >= 4)
    maxAllowedShape = 5; // Can have Spiky
  
  // If current shape exceeds what foal can have, pick random valid shape
  if (maneShape > maxAllowedShape)
  {
    maneShape = rand() % (maxAllowedShape + 1); // 0 to maxAllowedShape
  }
}

void Genetics::ValidateTailShape(int32_t& tailShape, uint8_t foalGrade)
{
  // Apply grade restrictions to tail shapes
  // Long Curly tail (shape 5): Requires Grade 7
  
  // Determine max allowed shape for this grade
  int32_t maxAllowedShape = 4; // Default: basic shapes (0-4)
  if (foalGrade >= 7)
    maxAllowedShape = 5; // Can have Long Curly
  
  // If current shape exceeds what foal can have, pick random valid shape
  if (tailShape > maxAllowedShape)
  {
    tailShape = rand() % (maxAllowedShape + 1); // 0 to maxAllowedShape
  }
}

Genetics::ManeTailResult Genetics::CalculateManeTailGenetics(
  data::Uid mareUid,
  data::Uid stallionUid,
  uint8_t foalGrade,
  data::Tid foalSkinTid)
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

  // Get allowed color groups for the foal's coat
  const auto& coatInfo = _serverInstance.GetHorseRegistry().GetCoatInfo(foalSkinTid);
  std::vector<int32_t> allowedColorGroups = {1}; // Default: color group 1
  if (!coatInfo.allowedColorGroups.empty())
  {
    allowedColorGroups = coatInfo.allowedColorGroups;
  }

  // Helper lambda to check if a color group is valid for this coat
  auto isColorGroupValid = [&](int32_t colorGroupId) -> bool
  {
    return std::find(allowedColorGroups.begin(), allowedColorGroups.end(), colorGroupId) != allowedColorGroups.end();
  };

  // Helper lambda to get random valid color group
  auto getRandomValidColorGroup = [&]() -> int32_t
  {
    if (allowedColorGroups.empty()) return 1; // Fallback
    auto dist = std::uniform_int_distribution<size_t>(0, allowedColorGroups.size() - 1);
    return allowedColorGroups[dist(_randomEngine)];
  };

  // === COLOR GROUP GENETICS (same color group for both mane and tail) ===
  
  // Color Group: Mom 10%, Dad 10%, GP1 5%, GP2 5%, GP3 5%, GP4 5%, Random 60%
  int colorRoll = rand() % 100;
  int32_t sharedColorGroupId = 0;
  
  if (colorRoll < 10)
  {
    sharedColorGroupId = GetColorGroupIdFromTid(mareMane, true);  // 10% Mom's mane color group
  }
  else if (colorRoll < 20)
  {
    sharedColorGroupId = GetColorGroupIdFromTid(stallionMane, true);  // 10% Dad's mane color group
  }
  else if (colorRoll < 25 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { sharedColorGroupId = GetColorGroupIdFromTid(gp.parts.maneTid(), true); });
  }
  else if (colorRoll < 30 && mareAncestors.size() >= 2)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[1]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { sharedColorGroupId = GetColorGroupIdFromTid(gp.parts.maneTid(), true); });
  }
  else if (colorRoll < 35 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { sharedColorGroupId = GetColorGroupIdFromTid(gp.parts.maneTid(), true); });
  }
  else if (colorRoll < 40 && stallionAncestors.size() >= 2)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[1]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { sharedColorGroupId = GetColorGroupIdFromTid(gp.parts.maneTid(), true); });
  }
  
  // Validate inherited color group against coat restrictions
  if (sharedColorGroupId == 0 || !isColorGroupValid(sharedColorGroupId))
  {
    sharedColorGroupId = getRandomValidColorGroup();  // Random fallback (60%) or invalid color group replacement
    spdlog::debug("Genetics: Mane/tail color group - inherited group invalid for coat, using random valid group {}", sharedColorGroupId);
  }
  
  int32_t maneColorGroupId = sharedColorGroupId;
  int32_t tailColorGroupId = sharedColorGroupId;  // SAME COLOR GROUP
  
  // === MANE SHAPE GENETICS ===

  // Mane Shape: Mom 10%, Dad 10%, GP1 5%, GP2 5%, GP3 5%, GP4 5%, Random 60%
  int maneShapeRoll = rand() % 100;
  int32_t maneShape = 0;

  if (maneShapeRoll < 10)
  {
    maneShape = GetShapeFromTid(mareMane, true);  // 10% Mom
  }
  else if (maneShapeRoll < 20)
  {
    maneShape = GetShapeFromTid(stallionMane, true);  // 10% Dad
  }
  else if (maneShapeRoll < 25 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneShape = GetShapeFromTid(gp.parts.maneTid(), true); });
  }
  else if (maneShapeRoll < 30 && mareAncestors.size() >= 2)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[1]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneShape = GetShapeFromTid(gp.parts.maneTid(), true); });
  }
  else if (maneShapeRoll < 35 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneShape = GetShapeFromTid(gp.parts.maneTid(), true); });
  }
  else if (maneShapeRoll < 40 && stallionAncestors.size() >= 2)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[1]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { maneShape = GetShapeFromTid(gp.parts.maneTid(), true); });
  }

  // If no inherited shape, use weighted random selection based on inheritance rates
  if (maneShape == 0)
  {
    // Collect valid mane shapes for the foal's grade with their weights
    std::vector<int32_t> validShapes;
    std::vector<float> weights;
    
    // Check all 8 mane shapes (0-7)
    for (int32_t shape = 0; shape <= 7; ++shape)
    {
      // Get any mane TID with this shape to check minGrade and inheritanceRate
      data::Tid sampleTid = (shape * 5) + 1; // White color variant of each shape
      const auto* maneInfo = _serverInstance.GetHorseRegistry().GetMane(sampleTid);
      if (maneInfo && maneInfo->minGrade <= foalGrade)
      {
        validShapes.push_back(shape);
        weights.push_back(maneInfo->inheritanceRate);
      }
    }
    
    if (!validShapes.empty())
    {
      std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
      maneShape = validShapes[dist(_randomEngine)];
    }
    else
    {
      maneShape = 0; // Fallback to shape 0
    }
  }
  else
  {
    // Validate inherited mane shape against grade
    ValidateManeShape(maneShape, foalGrade);
  }

  // === TAIL SHAPE GENETICS ===
  
  // Tail uses same color group as mane (already calculated above)
  // Tail Shape: Mom 10%, Dad 10%, GP1 5%, GP2 5%, GP3 5%, GP4 5%, Random 60%
  int tailShapeRoll = rand() % 100;
  int32_t tailShape = 0;

  if (tailShapeRoll < 10)
  {
    tailShape = GetShapeFromTid(mareTail, false);  // 10% Mom
  }
  else if (tailShapeRoll < 20)
  {
    tailShape = GetShapeFromTid(stallionTail, false);  // 10% Dad
  }
  else if (tailShapeRoll < 25 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailShape = GetShapeFromTid(gp.parts.tailTid(), false); });
  }
  else if (tailShapeRoll < 30 && mareAncestors.size() >= 2)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[1]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailShape = GetShapeFromTid(gp.parts.tailTid(), false); });
  }
  else if (tailShapeRoll < 35 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailShape = GetShapeFromTid(gp.parts.tailTid(), false); });
  }
  else if (tailShapeRoll < 40 && stallionAncestors.size() >= 2)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[1]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { tailShape = GetShapeFromTid(gp.parts.tailTid(), false); });
  }

  // If no inherited shape, use weighted random selection based on inheritance rates
  if (tailShape == 0)
  {
    // Collect valid tail shapes for the foal's grade with their weights
    std::vector<int32_t> validShapes;
    std::vector<float> weights;
    
    // Check all 6 tail shapes (0-5)
    for (int32_t shape = 0; shape <= 5; ++shape)
    {
      // Get any tail TID with this shape to check minGrade and inheritanceRate
      data::Tid sampleTid = (shape * 5) + 1; // White color variant of each shape
      const auto* tailInfo = _serverInstance.GetHorseRegistry().GetTail(sampleTid);
      if (tailInfo && tailInfo->minGrade <= foalGrade)
      {
        validShapes.push_back(shape);
        weights.push_back(tailInfo->inheritanceRate);
      }
    }
    
    if (!validShapes.empty())
    {
      std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
      tailShape = validShapes[dist(_randomEngine)];
    }
    else
    {
      tailShape = 0; // Fallback to shape 0
    }
  }
  else
  {
    // Validate inherited tail shape against grade
    ValidateTailShape(tailShape, foalGrade);
  }

  // Get random mane TID from the selected color group and shape
  result.maneTid = _serverInstance.GetHorseRegistry().GetRandomManeFromColorAndShape(maneColorGroupId, maneShape, _randomEngine);

  // Fallback if lookup fails
  if (result.maneTid == data::InvalidTid)
  {
    spdlog::warn("Genetics: Failed to find mane for color group {} shape {}, using fallback", maneColorGroupId, maneShape);
    result.maneTid = 1; // White short mane
  }

  // Extract the actual color from the mane we selected
  // Then find the tail with the SAME color
  registry::Color selectedColor = _serverInstance.GetHorseRegistry().GetManeColor(result.maneTid);
  result.tailTid = _serverInstance.GetHorseRegistry().FindTailByColorAndShape(selectedColor, tailShape);

  // Fallback if matching tail isn't found
  if (result.tailTid == data::InvalidTid || result.tailTid == 0)
  {
    spdlog::warn("Genetics: Failed to find tail matching mane color for shape {}, using generic lookup", tailShape);
    result.tailTid = _serverInstance.GetHorseRegistry().GetRandomTailFromColorAndShape(tailColorGroupId, tailShape, _randomEngine);
    
    if (result.tailTid == data::InvalidTid)
    {
      result.tailTid = 1; // White short tail as last resort
    }
  }

  // Store color group and shape for debugging
  result.maneColor = maneColorGroupId;
  result.maneShape = maneShape;
  result.tailColor = tailColorGroupId;
  result.tailShape = tailShape;

  spdlog::debug("TryBreeding: Mane(colorGroup:{}, shape:{}, TID:{}), Tail(colorGroup:{}, shape:{}, TID:{})",
                maneColorGroupId, maneShape, result.maneTid, tailColorGroupId, tailShape, result.tailTid);

  return result;
}

uint8_t Genetics::CalculateFoalGrade(uint8_t mareGrade, uint8_t stallionGrade)
{
  // Official breeding grade probability table from libconfig
  // Rows = GradeDistance (0-4, max difference allowed in breeding)
  // Columns = Grade offset from lower parent (Minus3, Minus2, Minus1, Plus0-7)
  static const float gradeTable[5][11] = {
    // GradeDistance 0 (same grade parents)
    {6.0f, 20.0f, 34.0f, 40.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    // GradeDistance 1
    {6.0f, 20.0f, 34.0f, 26.21f, 13.79f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    // GradeDistance 2
    {6.0f, 20.0f, 34.0f, 21.18f, 12.94f, 5.88f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    // GradeDistance 3
    {6.0f, 20.0f, 34.0f, 17.66f, 12.47f, 7.27f, 2.6f, 0.0f, 0.0f, 0.0f, 0.0f},
    // GradeDistance 4
    {6.0f, 20.0f, 34.0f, 13.91f, 11.3f, 7.83f, 4.78f, 2.18f, 0.0f, 0.0f, 0.0f}
  };
  
  // Clamp both grades to 8
  uint8_t clampedMareGrade = std::min(mareGrade, (uint8_t)8);
  uint8_t clampedStallionGrade = std::min(stallionGrade, (uint8_t)8);
  
  uint8_t minGrade = std::min(clampedMareGrade, clampedStallionGrade);
  uint8_t maxGrade = std::max(clampedMareGrade, clampedStallionGrade);
  uint8_t gradeDistance = maxGrade - minGrade;
  
  // Cap grade distance at 4 (This is the max grade distance you can get by breeding a grade 4 horse with a grade 8 horse)
  if (gradeDistance > 4) gradeDistance = 4;
  
  // Build cumulative probability distribution
  float cumulativeProbs[11];
  float total = 0.0f;
  for (int i = 0; i < 11; ++i)
  {
    total += gradeTable[gradeDistance][i];
    cumulativeProbs[i] = total;
  }
  
  // Roll random value
  std::uniform_real_distribution<float> dist(0.0f, 100.0f);
  float roll = dist(_randomEngine);
  
  // Find grade offset based on roll
  int gradeOffset = -3;  // Start at Minus3
  for (int i = 0; i < 11; ++i)
  {
    if (roll < cumulativeProbs[i])
    {
      gradeOffset = i - 3;
      break;
    }
  }
  
  // Calculate final grade (relative to lower parent)
  int finalGrade = minGrade + gradeOffset;
  
  // Clamp to valid range [1, 8]
  if (finalGrade < 1) finalGrade = 1;
  if (finalGrade > 8) finalGrade = 8;
  
  spdlog::debug("Genetics: Grade breeding - Parents {}/{} (clamped {}/{}), distance {}, offset {}, final grade {}", 
    mareGrade, stallionGrade, clampedMareGrade, clampedStallionGrade, gradeDistance, gradeOffset, finalGrade);
  
  return static_cast<uint8_t>(finalGrade);
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
  uint32_t mareAgility, uint32_t mareCourage, uint32_t mareRush,
  uint32_t mareEndurance, uint32_t mareAmbition,
  uint32_t stallionAgility, uint32_t stallionCourage, uint32_t stallionRush,
  uint32_t stallionEndurance, uint32_t stallionAmbition,
  uint8_t targetGrade)
{
  StatResult result;
  
  // Determine target total stats range for the grade
  uint32_t minTotal = (targetGrade - 1) * 10;      // e.g., Grade 5: 40
  uint32_t maxTotal = targetGrade * 10 - 1;        // e.g., Grade 5: 49
  
  // Special cases for grade 1 and 8
  if (targetGrade == 1) minTotal = 0;
  if (targetGrade == 8) maxTotal = 79;
  
  // Pick a random target total within the grade range
  std::uniform_int_distribution<uint32_t> totalDist(minTotal, maxTotal);
  uint32_t targetTotal = totalDist(_randomEngine);
  
  // Calculate base stats as average of parents with ±20% variance
  auto calcBaseStat = [&](uint32_t mareStat, uint32_t stallionStat) -> uint32_t
  {
    uint32_t avgStat = (mareStat + stallionStat) / 2;
    int32_t variance = (avgStat * 20) / 100;
    std::uniform_int_distribution<int32_t> varianceDist(-variance, variance);
    int32_t finalStat = avgStat + varianceDist(_randomEngine);
    if (finalStat < 0) finalStat = 0;
    if (finalStat > 100) finalStat = 100;
    return static_cast<uint32_t>(finalStat);
  };
  
  result.agility = calcBaseStat(mareAgility, stallionAgility);
  result.courage = calcBaseStat(mareCourage, stallionCourage);
  result.rush = calcBaseStat(mareRush, stallionRush);
  result.endurance = calcBaseStat(mareEndurance, stallionEndurance);
  result.ambition = calcBaseStat(mareAmbition, stallionAmbition);
  
  // Calculate current total
  uint32_t currentTotal = result.agility + result.courage + result.rush + 
                          result.endurance + result.ambition;
  
  // Adjust stats proportionally to hit target total
  if (currentTotal != targetTotal && currentTotal > 0)
  {
    double scaleFactor = static_cast<double>(targetTotal) / static_cast<double>(currentTotal);
    
    result.agility = static_cast<uint32_t>(result.agility * scaleFactor);
    result.courage = static_cast<uint32_t>(result.courage * scaleFactor);
    result.rush = static_cast<uint32_t>(result.rush * scaleFactor);
    result.endurance = static_cast<uint32_t>(result.endurance * scaleFactor);
    result.ambition = static_cast<uint32_t>(result.ambition * scaleFactor);
    
    // Handle rounding error by adjusting the largest stat
    uint32_t adjustedTotal = result.agility + result.courage + result.rush + 
                             result.endurance + result.ambition;
    int32_t difference = targetTotal - adjustedTotal;
    
    // Find largest stat and adjust it
    if (difference != 0)
    {
      uint32_t* largestStat = &result.agility;
      if (result.courage > *largestStat) largestStat = &result.courage;
      if (result.rush > *largestStat) largestStat = &result.rush;
      if (result.endurance > *largestStat) largestStat = &result.endurance;
      if (result.ambition > *largestStat) largestStat = &result.ambition;
      
      *largestStat = static_cast<uint32_t>(static_cast<int32_t>(*largestStat) + difference);
    }
    
    // Clamp all stats to 0-100 range
    result.agility = std::min(result.agility, 100u);
    result.courage = std::min(result.courage, 100u);
    result.rush = std::min(result.rush, 100u);
    result.endurance = std::min(result.endurance, 100u);
    result.ambition = std::min(result.ambition, 100u);
  }
  
  spdlog::debug("Genetics: Target grade {}, total range {}-{}, actual total {}", 
    targetGrade, minTotal, maxTotal, 
    result.agility + result.courage + result.rush + result.endurance + result.ambition);
  
  return result;
}

Genetics::PotentialResult Genetics::CalculateFoalPotential(
  data::Uid mareUid,
  data::Uid stallionUid)
{
  PotentialResult result;
  result.level = 0;  // Always 0 for newborns
  result.value = 0;  // Always 0 for newborns

  // Get mare and stallion records
  auto mareRecord = _serverInstance.GetDataDirector().GetHorse(mareUid);
  auto stallionRecord = _serverInstance.GetDataDirector().GetHorse(stallionUid);

  if (!mareRecord || !stallionRecord)
  {
    spdlog::error("Genetics: Mare or stallion not found for potential calculation");
    return result;
  }

  // Count how many horses have potentials
  bool mareHasPotential = false;
  bool stallionHasPotential = false;
  uint8_t marePotentialType = 0;
  uint8_t stallionPotentialType = 0;
  std::vector<data::Uid> mareAncestors, stallionAncestors;
  
  mareRecord.Immutable([&](const data::Horse& mare)
  {
    mareHasPotential = mare.potential.type() > 0;
    marePotentialType = mare.potential.type();
    mareAncestors = mare.ancestors();
  });
  
  stallionRecord.Immutable([&](const data::Horse& stallion)
  {
    stallionHasPotential = stallion.potential.type() > 0;
    stallionPotentialType = stallion.potential.type();
    stallionAncestors = stallion.ancestors();
  });
  
  // Count grandparents with potentials
  int grandparentsWithPotential = 0;
  std::vector<uint8_t> grandparentPotentialTypes;
  
  for (const auto& gpUid : mareAncestors)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(gpUid);
    if (gpRecord)
    {
      gpRecord.Immutable([&](const data::Horse& gp)
      {
        if (gp.potential.type() > 0)
        {
          grandparentsWithPotential++;
          grandparentPotentialTypes.push_back(gp.potential.type());
        }
      });
    }
  }
  
  for (const auto& gpUid : stallionAncestors)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(gpUid);
    if (gpRecord)
    {
      gpRecord.Immutable([&](const data::Horse& gp)
      {
        if (gp.potential.type() > 0)
        {
          grandparentsWithPotential++;
          grandparentPotentialTypes.push_back(gp.potential.type());
        }
      });
    }
  }
  
  // Determine probability of having a potential based on family
  // Formula: Base 5% + (5% per parent) + (2% per grandparent)
  int parentsWithPotential = (mareHasPotential ? 1 : 0) + (stallionHasPotential ? 1 : 0);
  int potentialProbability = 5 + (parentsWithPotential * 5) + (grandparentsWithPotential * 2);
  
  // Roll for whether foal has a potential
  auto roll01_99 = std::uniform_int_distribution<int>(0, 99);
  int potentialRoll = roll01_99(_randomEngine);
  if (potentialRoll >= potentialProbability)
  {
    // No potential
    result.hasPotential = false;
    spdlog::debug("Genetics: Foal potential - no potential (rolled {}, needed < {})", potentialRoll, potentialProbability);
    return result;
  }
  
  // Determine the type of potential if the foal has one
  result.hasPotential = true;
  
  // Potential type inheritance weights: Mom 8%, Dad 8%, each grandparent 5%, rest random
  // Prepare ordered grandparent potential types: mare[0], mare[1], stallion[0], stallion[1]
  uint8_t gpTypes[4] = {0, 0, 0, 0};
  auto fillGpTypes = [&](const std::vector<data::Uid>& ancestorUids, int startIdx)
  {
    for (size_t i = 0; i < ancestorUids.size() && (startIdx + (int)i) < 4; ++i)
    {
      auto gpRecord = _serverInstance.GetDataDirector().GetHorse(ancestorUids[i]);
      if (gpRecord)
      {
        gpRecord.Immutable([&](const data::Horse& gp)
        {
          gpTypes[startIdx + (int)i] = gp.potential.type();
        });
      }
    }
  };
  fillGpTypes(mareAncestors, 0);
  fillGpTypes(stallionAncestors, 2);

  auto rand01_99 = std::uniform_int_distribution<int>(0, 99);
  auto typeRoll = rand01_99(_randomEngine);

  auto randomPotentialType = [&]() -> uint8_t
  {
    // Random type 1-15 excluding 12
    auto dist = std::uniform_int_distribution<int>(1, 15);
    uint8_t t;
    do { t = static_cast<uint8_t>(dist(_randomEngine)); } while (t == 12);
    return t;
  };

  // Helper to finalize selection
  auto chooseTypeOrRandom = [&](bool has, uint8_t type) -> uint8_t
  {
    if (has && type > 0) return type;
    return randomPotentialType();
  };

  if (typeRoll < 8)
  {
    result.type = chooseTypeOrRandom(mareHasPotential, marePotentialType); // Mom 8%
  }
  else if (typeRoll < 16)
  {
    result.type = chooseTypeOrRandom(stallionHasPotential, stallionPotentialType); // Dad 8%
  }
  else if (typeRoll < 21)
  {
    result.type = chooseTypeOrRandom(gpTypes[0] > 0, gpTypes[0]); // Maternal GP #1 5%
  }
  else if (typeRoll < 26)
  {
    result.type = chooseTypeOrRandom(gpTypes[1] > 0, gpTypes[1]); // Maternal GP #2 5%
  }
  else if (typeRoll < 31)
  {
    result.type = chooseTypeOrRandom(gpTypes[2] > 0, gpTypes[2]); // Paternal GP #1 5%
  }
  else if (typeRoll < 36)
  {
    result.type = chooseTypeOrRandom(gpTypes[3] > 0, gpTypes[3]); // Paternal GP #2 5%
  }
  else
  {
    result.type = randomPotentialType(); // Remaining 64%
  }
  
  spdlog::debug("Genetics: Foal potential - has:{}, type:{}, probability:{}%",
    result.hasPotential, result.type, potentialProbability);
  
  return result;
}

data::Tid Genetics::CalculateFoalSkin(
  data::Uid mareUid,
  data::Uid stallionUid,
  uint8_t foalGrade)
{
  // Get parent and grandparent skin data
  auto mareRecord = _serverInstance.GetDataDirector().GetHorse(mareUid);
  auto stallionRecord = _serverInstance.GetDataDirector().GetHorse(stallionUid);
  
  if (!mareRecord || !stallionRecord)
  {
    spdlog::error("Genetics: Mare or stallion not found for skin calculation");
    return 1; // Fallback to Chestnut
  }
  
  data::Tid mareSkin = 0, stallionSkin = 0;
  std::vector<data::Uid> mareAncestors, stallionAncestors;
  
  mareRecord.Immutable([&](const data::Horse& mare) {
    mareSkin = mare.parts.skinTid();
    mareAncestors = mare.ancestors();
  });
  
  stallionRecord.Immutable([&](const data::Horse& stallion) {
    stallionSkin = stallion.parts.skinTid();
    stallionAncestors = stallion.ancestors();
  });
  
  // Get grandparent skins in order: mare's parents, stallion's parents
  std::vector<data::Tid> gpSkins;
  for (const auto& gpUid : mareAncestors)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(gpUid);
    if (gpRecord)
    {
      gpRecord.Immutable([&](const data::Horse& gp) {
        gpSkins.push_back(gp.parts.skinTid());
      });
    }
  }
  for (const auto& gpUid : stallionAncestors)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(gpUid);
    if (gpRecord)
    {
      gpRecord.Immutable([&](const data::Horse& gp) {
        gpSkins.push_back(gp.parts.skinTid());
      });
    }
  }
  
  // Helper lambda to get random valid skin using weighted inheritance rates
  auto getRandomValidSkin = [&]() -> data::Tid
  {
    // Collect all valid coats for the foal's grade with their inheritance rates
    std::vector<data::Tid> validTids;
    std::vector<float> weights;
    
    for (int tid = 1; tid <= 20; ++tid)
    {
      const auto& coatInfo = _serverInstance.GetHorseRegistry().GetCoatInfo(tid);
      if (coatInfo.minGrade <= foalGrade)
      {
        validTids.push_back(tid);
        weights.push_back(coatInfo.inheritanceRate);
      }
    }
    
    // If no valid skins at all, fallback to Chestnut
    if (validTids.empty())
    {
      return 1;
    }
    
    // Use weighted random selection based on inheritance rates
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    size_t selectedIndex = dist(_randomEngine);
    
    return validTids[selectedIndex];
  };
  
  // Helper lambda to check if skin is valid for foal's grade, returns skin or random
  auto getValidSkinOrRandom = [&](data::Tid skinTid) -> data::Tid
  {
    const auto& coatInfo = _serverInstance.GetHorseRegistry().GetCoatInfo(skinTid);
    if (coatInfo.minGrade <= foalGrade)
    {
      return skinTid; // Valid, use it
    }
    // Too high grade requirement, select random instead
    return getRandomValidSkin();
  };
  
  // Weighted inheritance: Mom 10%, Dad 10%, GPs 5% each (up to 4), Random 60%
  auto rand0_99 = std::uniform_int_distribution<int>(0, 99);
  int roll = rand0_99(_randomEngine);
  
  data::Tid selectedSkin = 0;
  
  if (roll < 10)
  {
    selectedSkin = getValidSkinOrRandom(mareSkin); // Mom 10%
  }
  else if (roll < 20)
  {
    selectedSkin = getValidSkinOrRandom(stallionSkin); // Dad 10%
  }
  else if (roll < 25 && gpSkins.size() > 0)
  {
    selectedSkin = getValidSkinOrRandom(gpSkins[0]); // Maternal GP1 5%
  }
  else if (roll < 30 && gpSkins.size() > 1)
  {
    selectedSkin = getValidSkinOrRandom(gpSkins[1]); // Maternal GP2 5%
  }
  else if (roll < 35 && gpSkins.size() > 2)
  {
    selectedSkin = getValidSkinOrRandom(gpSkins[2]); // Paternal GP1 5%
  }
  else if (roll < 40 && gpSkins.size() > 3)
  {
    selectedSkin = getValidSkinOrRandom(gpSkins[3]); // Paternal GP2 5%
  }
  else
  {
    // Random 60% - constrained by foal grade
    selectedSkin = getRandomValidSkin();
  }
  
  // Log final selection
  const auto& coatInfo = _serverInstance.GetHorseRegistry().GetCoatInfo(selectedSkin);
  spdlog::debug("Genetics: Foal skin - selected TID {} (grade requirement: {}, foal grade: {})",
    selectedSkin, coatInfo.minGrade, foalGrade);
  
  return selectedSkin;
}

} // namespace server

