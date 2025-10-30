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
  : _serverInstance(serverInstance),
    _randomEngine(std::random_device{}())
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

  // === COLOR GENETICS (same color for both mane and tail) ===
  
  // Color: Mom 20%, Dad 20%, Maternal GP 10%, Paternal GP 10%, Random 40%
  int colorRoll = rand() % 100;
  int32_t sharedColor = 0;
  
  if (colorRoll < 20)
  {
    sharedColor = GetColorFromTid(mareMane);  // 20% Mom's mane color
  }
  else if (colorRoll < 40)
  {
    sharedColor = GetColorFromTid(stallionMane);  // 20% Dad's mane color
  }
  else if (colorRoll < 50 && mareAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(mareAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { sharedColor = GetColorFromTid(gp.parts.maneTid()); });
  }
  else if (colorRoll < 60 && stallionAncestors.size() >= 1)
  {
    auto gpRecord = _serverInstance.GetDataDirector().GetHorse(stallionAncestors[0]);
    if (gpRecord)
      gpRecord.Immutable([&](const data::Horse& gp) { sharedColor = GetColorFromTid(gp.parts.maneTid()); });
  }
  
  if (sharedColor == 0) sharedColor = 1 + (rand() % 5);  // Random fallback
  
  int32_t maneColor = sharedColor;
  int32_t tailColor = sharedColor;  // SAME COLOR
  
  // === MANE GENETICS ===

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

  // === TAIL GENETICS ===
  
  // Tail uses same color as mane (already calculated above)
  // Tail Shape: Mom 20%, Dad 20%, Maternal GP 10%, Paternal GP 10%, Random 40%
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
    if (finalStat < 1) finalStat = 1;
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
    
    // Clamp all stats to 1-100 range
    result.agility = std::clamp(result.agility, 1u, 100u);
    result.courage = std::clamp(result.courage, 1u, 100u);
    result.rush = std::clamp(result.rush, 1u, 100u);
    result.endurance = std::clamp(result.endurance, 1u, 100u);
    result.ambition = std::clamp(result.ambition, 1u, 100u);
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

} // namespace server

