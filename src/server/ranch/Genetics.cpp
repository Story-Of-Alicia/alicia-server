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
#include <cmath>
#include <vector>

#include <spdlog/spdlog.h>

namespace server
{

namespace
{

//! Inheritance roll order shared by every trait:
//! mare 10%, stallion 10%, each of the four grandparents 5%, otherwise random.
constexpr int kMareChance = 10;
constexpr int kStallionChance = 20;
constexpr int kGrandparentStep = 5;

} // namespace

Genetics::Genetics(ServerInstance& serverInstance)
  : _serverInstance(serverInstance),
    _randomEngine(std::random_device{}())
{
}

int Genetics::RollPercent()
{
  return std::uniform_int_distribution<int>(0, 99)(_randomEngine);
}

uint32_t Genetics::RollTendency()
{
  const auto& horseRegistry = _serverInstance.GetHorseRegistry();

  std::vector<int32_t> weights;
  std::vector<uint32_t> tendencies;
  for (uint32_t tendency = 1; tendency <= 6; ++tendency)
  {
    if (const auto* ratio = horseRegistry.GetTendencyRatio(tendency); ratio && ratio->breedingRatio > 0)
    {
      weights.push_back(ratio->breedingRatio);
      tendencies.push_back(tendency);
    }
  }

  if (tendencies.empty())
    return 1;

  std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
  return tendencies[dist(_randomEngine)];
}

void Genetics::CreateFoal(
  data::Horse& foal,
  const data::Uid mareUid,
  const data::Uid stallionUid,
  const uint32_t gradeBonus)
{
  // The parent attributes the calculations need, read once per parent.
  struct ParentInfo
  {
    data::Tid tid{0};
    data::Tid faceTid{0};
    uint8_t grade{0};
    uint32_t combo{0};
    uint32_t breedingCount{0};
    data::Horse::Stats stats{};
    data::Horse::Appearance appearance{};
  };

  const auto readParent = [this](data::Uid horseUid) -> ParentInfo
  {
    ParentInfo info;
    if (const auto record = _serverInstance.GetDataDirector().GetHorse(horseUid))
    {
      record.Immutable([&info](const data::Horse& horse)
      {
        info.tid = horse.tid();
        info.faceTid = horse.parts.faceTid();
        info.grade = static_cast<uint8_t>(horse.grade());
        info.combo = horse.breedingCombo();
        info.breedingCount = horse.breedingCount();
        info.stats = {
          horse.stats.agility(), horse.stats.courage(), horse.stats.rush(),
          horse.stats.endurance(), horse.stats.ambition()};
        info.appearance = {
          horse.appearance.scale(), horse.appearance.legLength(), horse.appearance.legVolume(),
          horse.appearance.bodyLength(), horse.appearance.bodyVolume()};
      });
    }
    return info;
  };

  const ParentInfo mare = readParent(mareUid);
  const ParentInfo stallion = readParent(stallionUid);

  // Newborn administrative defaults.
  foal.name() = std::string{}; // Empty until the player names it.
  foal.type() = data::Horse::Type::Foal;
  foal.dateOfBirth() = data::Clock::now();
  foal.clazz() = 1;
  foal.clazzProgress() = 0;
  foal.growthPoints() = 0;
  foal.mountCondition.stamina() = 4000;

  foal.tid() = mare.tid; // Foal uses the mare's breed/TID.
  foal.tendency() = RollTendency();

  // Grade: base genetics plus any fertility-peak bonus, capped at the child grade limit.
  const auto childGradeLimit = static_cast<uint32_t>(
    _serverInstance.GetBreedingRegistry().GetBreedingParams().childGradeLimit);
  const auto foalGrade = static_cast<uint8_t>(
    std::min(CalculateFoalGrade(mare.grade, stallion.grade) + gradeBonus, childGradeLimit));
  foal.grade() = foalGrade;

  // Skin/coat, influenced by parent combos and the stallion's pregnancy chance.
  const uint32_t pregnancyChance = std::min(stallion.breedingCount, 30u);
  const data::Tid foalSkin = CalculateFoalSkin(
    mareUid, stallionUid, foalGrade, mare.combo, stallion.combo, pregnancyChance);
  foal.parts.skinTid() = foalSkin;

  // Face inherited from a random parent.
  foal.parts.faceTid() = std::uniform_int_distribution<int>(0, 1)(_randomEngine) == 0
    ? mare.faceTid : stallion.faceTid;

  const auto maneTail = CalculateManeTailGenetics(mareUid, stallionUid, foalGrade, foalSkin);
  foal.parts.maneTid() = maneTail.maneTid;
  foal.parts.tailTid() = maneTail.tailTid;

  const auto appearance = CalculateFoalAppearance(mare.appearance, stallion.appearance);
  foal.appearance.scale() = appearance.scale();
  foal.appearance.legLength() = appearance.legLength();
  foal.appearance.legVolume() = appearance.legVolume();
  foal.appearance.bodyLength() = appearance.bodyLength();
  foal.appearance.bodyVolume() = appearance.bodyVolume();

  const auto stats = CalculateFoalStats(mare.stats, stallion.stats, foalGrade);
  foal.stats.agility() = stats.agility();
  foal.stats.courage() = stats.courage();
  foal.stats.rush() = stats.rush();
  foal.stats.endurance() = stats.endurance();
  foal.stats.ambition() = stats.ambition();

  // CalculateFoalPotential already zeroes type/level/value when the foal has no potential.
  const auto potential = CalculateFoalPotential(mareUid, stallionUid, foalSkin);
  foal.potential.type() = potential.type;
  foal.potential.level() = potential.level;
  foal.potential.value() = potential.value;

  // Ancestry and lineage.
  foal.ancestors.father = stallionUid;
  foal.ancestors.mother = mareUid;
  foal.lineage() = CalculateLineage(foalSkin, mareUid, stallionUid);
}

data::Tid Genetics::ReadPart(const data::Uid horseUid, const Part part)
{
  if (horseUid == data::InvalidUid)
    return 0;

  const auto record = _serverInstance.GetDataDirector().GetHorse(horseUid);
  if (not record)
    return 0;

  data::Tid tid = 0;
  record.Immutable([&](const data::Horse& horse)
  {
    switch (part)
    {
      case Part::Skin: tid = horse.parts.skinTid(); break;
      case Part::Mane: tid = horse.parts.maneTid(); break;
      case Part::Tail: tid = horse.parts.tailTid(); break;
    }
  });
  return tid;
}

Genetics::Ancestry Genetics::BuildAncestry(const data::Uid mareUid, const data::Uid stallionUid)
{
  Ancestry ancestry;
  ancestry.mare = mareUid;
  ancestry.stallion = stallionUid;

  // Each parent's own parents become the foal's grandparents.
  const auto readGrandparents = [&](data::Uid parentUid, size_t firstSlot)
  {
    if (parentUid == data::InvalidUid)
      return;

    const auto record = _serverInstance.GetDataDirector().GetHorse(parentUid);
    if (not record)
      return;

    record.Immutable([&](const data::Horse& parent)
    {
      ancestry.grandparents[firstSlot] = parent.ancestors.mother;
      ancestry.grandparents[firstSlot + 1] = parent.ancestors.father;
    });
  };

  readGrandparents(mareUid, 0);     // Maternal grandparents.
  readGrandparents(stallionUid, 2); // Paternal grandparents.
  return ancestry;
}

data::Uid Genetics::RollInheritedDonor(const Ancestry& ancestry)
{
  const int roll = RollPercent();

  if (roll < kMareChance)
    return ancestry.mare;
  if (roll < kStallionChance)
    return ancestry.stallion;

  // Grandparent bands: [20,25), [25,30), [30,35), [35,40). A missing grandparent
  // yields data::InvalidUid, which callers treat as "roll randomly".
  for (size_t i = 0; i < ancestry.grandparents.size(); ++i)
  {
    const int upper = kStallionChance + static_cast<int>(i + 1) * kGrandparentStep;
    if (roll < upper)
      return ancestry.grandparents[i];
  }

  return data::InvalidUid; // Random (60%).
}

int32_t Genetics::GetShapeFromTid(const data::Tid tid, const bool isMane)
{
  const auto& registry = _serverInstance.GetHorseRegistry();
  return isMane ? registry.GetMane(tid).shape : registry.GetTail(tid).shape;
}

void Genetics::ValidateManeShape(int32_t& maneShape, const uint8_t foalGrade)
{
  // Basic shapes (0-4) are always allowed; the longer shapes need a higher grade.
  int32_t maxAllowedShape = 4;
  if (foalGrade >= 7)
    maxAllowedShape = 7; // Long Curly
  else if (foalGrade >= 6)
    maxAllowedShape = 6; // Long
  else if (foalGrade >= 4)
    maxAllowedShape = 5; // Spiky

  if (maneShape > maxAllowedShape)
    maneShape = std::uniform_int_distribution<int32_t>(0, maxAllowedShape)(_randomEngine);
}

void Genetics::ValidateTailShape(int32_t& tailShape, const uint8_t foalGrade)
{
  // Only Long Curly (5) is grade-restricted for tails.
  const int32_t maxAllowedShape = (foalGrade >= 7) ? 5 : 4;

  if (tailShape > maxAllowedShape)
    tailShape = std::uniform_int_distribution<int32_t>(0, maxAllowedShape)(_randomEngine);
}

int32_t Genetics::InheritManeShape(const Ancestry& ancestry, const uint8_t foalGrade)
{
  const auto& registry = _serverInstance.GetHorseRegistry();

  if (const data::Uid donor = RollInheritedDonor(ancestry); donor != data::InvalidUid)
  {
    int32_t shape = GetShapeFromTid(ReadPart(donor, Part::Mane), true);
    ValidateManeShape(shape, foalGrade);
    return shape;
  }

  // No inheritance: weight each grade-eligible shape by its inheritance rate.
  std::vector<int32_t> shapes;
  std::vector<float> weights;
  for (const auto& shapeInfo : registry.GetManeShapeInheritance())
  {
    if (shapeInfo.minGrade <= foalGrade)
    {
      shapes.push_back(shapeInfo.shape);
      weights.push_back(shapeInfo.inheritanceRate);
    }
  }

  if (shapes.empty())
    return 0;

  std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
  return shapes[dist(_randomEngine)];
}

int32_t Genetics::InheritTailShape(const Ancestry& ancestry, const uint8_t foalGrade)
{
  const auto& registry = _serverInstance.GetHorseRegistry();

  if (const data::Uid donor = RollInheritedDonor(ancestry); donor != data::InvalidUid)
  {
    int32_t shape = GetShapeFromTid(ReadPart(donor, Part::Tail), false);
    ValidateTailShape(shape, foalGrade);
    return shape;
  }

  std::vector<int32_t> shapes;
  std::vector<float> weights;
  for (const auto& shapeInfo : registry.GetTailShapeInheritance())
  {
    if (shapeInfo.minGrade <= foalGrade)
    {
      shapes.push_back(shapeInfo.shape);
      weights.push_back(shapeInfo.inheritanceRate);
    }
  }

  if (shapes.empty())
    return 0;

  std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
  return shapes[dist(_randomEngine)];
}

Genetics::ManeTailResult Genetics::CalculateManeTailGenetics(
  const data::Uid mareUid,
  const data::Uid stallionUid,
  const uint8_t foalGrade,
  const data::Tid foalSkinTid)
{
  ManeTailResult result;

  // Non-const: GetRandomMane/Tail advance the registry's RNG.
  auto& registry = _serverInstance.GetHorseRegistry();
  const Ancestry ancestry = BuildAncestry(mareUid, stallionUid);

  // The coat fixes which colour group the mane and tail belong to.
  const int32_t colorGroupId = registry.GetCoatInfo(foalSkinTid).allowedColorGroups;

  // Shapes are inherited independently from the colour group.
  const int32_t maneShape = InheritManeShape(ancestry, foalGrade);
  const int32_t tailShape = InheritTailShape(ancestry, foalGrade);

  // Resolve the mane from the coat's colour group and the chosen shape.
  result.maneTid = registry.GetRandomManeFromColorAndShape(colorGroupId, maneShape);
  if (result.maneTid == data::InvalidTid)
  {
    spdlog::warn("Genetics: no mane for colour group {} shape {}, using fallback", colorGroupId, maneShape);
    result.maneTid = 1; // White short mane.
  }

  // Match the tail to the mane's exact colour so the pair looks consistent.
  const registry::Color maneColor = registry.GetMane(result.maneTid).color;
  result.tailTid = registry.FindTailByColorAndShape(maneColor, tailShape);
  if (result.tailTid == data::InvalidTid || result.tailTid == 0)
  {
    spdlog::warn("Genetics: no tail matching mane colour for shape {}, using colour-group lookup", tailShape);
    result.tailTid = registry.GetRandomTailByColorGroupAndShape(colorGroupId, tailShape);
    if (result.tailTid == data::InvalidTid)
      result.tailTid = 1; // White short tail.
  }

  result.maneColor = colorGroupId;
  result.maneShape = maneShape;
  result.tailColor = colorGroupId;
  result.tailShape = tailShape;
  return result;
}

uint8_t Genetics::CalculateFoalGrade(const uint8_t mareGrade, const uint8_t stallionGrade)
{
  const uint8_t minGrade = std::min(mareGrade, stallionGrade);
  const uint8_t maxGrade = std::max(mareGrade, stallionGrade);

  const auto& breedingRegistry = _serverInstance.GetBreedingRegistry();

  // Outcome distribution keyed by the grade gap between the parents (breeding.yaml).
  const auto& row = breedingRegistry.GetGradeProbability(maxGrade - minGrade);

  // Columns laid out as [Minus3, Minus2, Minus1, Plus0, Plus1, ...]; column k is offset k-3.
  std::vector<float> probabilities{row.minus3, row.minus2, row.minus1};
  probabilities.insert(probabilities.end(), row.plus.begin(), row.plus.end());

  const float roll = std::uniform_real_distribution<float>(0.0f, 100.0f)(_randomEngine);

  int gradeOffset = -3;
  float cumulative = 0.0f;
  for (size_t i = 0; i < probabilities.size(); ++i)
  {
    cumulative += probabilities[i];
    if (roll < cumulative)
    {
      gradeOffset = static_cast<int>(i) - 3;
      break;
    }
  }

  // Clamp to [1, child grade limit]; the caller may still add a fertility bonus and re-cap.
  const int childGradeLimit = breedingRegistry.GetBreedingParams().childGradeLimit;
  const int finalGrade = static_cast<int>(minGrade) + gradeOffset;
  return static_cast<uint8_t>(std::clamp(finalGrade, 1, childGradeLimit));
}

data::Horse::Stats Genetics::CalculateFoalStats(
  const data::Horse::Stats& mareStats,
  const data::Horse::Stats& stallionStats,
  const uint8_t targetGrade)
{
  data::Horse::Stats result;

  // Target total for the grade: grade N occupies [(N-1)*10, N*10-1], matching the
  const uint32_t minTotal = (targetGrade - 1) * 10;
  const uint32_t maxTotal = targetGrade * 10 - 1;

  const uint32_t targetTotal =
    std::uniform_int_distribution<uint32_t>(minTotal, maxTotal)(_randomEngine);

  // Base each stat on the parent average. Very low averages get a small mutation bonus
  // so two weak parents can still produce a slightly better foal.
  const auto calcBaseStat = [&](uint32_t mareStat, uint32_t stallionStat) -> uint32_t
  {
    const uint32_t avgStat = (mareStat + stallionStat) / 2;

    if (avgStat <= 2)
    {
      // Weighted bonus of +0..+3, increasingly generous as the average rises.
      static const std::array<std::array<int, 4>, 3> bonusWeights{{
        {{30, 40, 20, 10}}, // avg 0
        {{20, 50, 25, 5}},  // avg 1
        {{10, 30, 50, 10}}, // avg 2
      }};
      const auto& w = bonusWeights[avgStat];
      std::discrete_distribution<int> bonusDist(w.begin(), w.end());
      return std::min<uint32_t>(avgStat + bonusDist(_randomEngine), 100);
    }

    const int32_t offset = std::uniform_int_distribution<int32_t>(-3, 3)(_randomEngine);
    return static_cast<uint32_t>(std::clamp(static_cast<int32_t>(avgStat) + offset, 0, 100));
  };

  uint32_t agility = calcBaseStat(mareStats.agility(), stallionStats.agility());
  uint32_t courage = calcBaseStat(mareStats.courage(), stallionStats.courage());
  uint32_t rush = calcBaseStat(mareStats.rush(), stallionStats.rush());
  uint32_t endurance = calcBaseStat(mareStats.endurance(), stallionStats.endurance());
  uint32_t ambition = calcBaseStat(mareStats.ambition(), stallionStats.ambition());

  const uint32_t currentTotal = agility + courage + rush + endurance + ambition;

  // Scale the stats proportionally so their sum lands inside the target grade's bucket.
  if (currentTotal != targetTotal && currentTotal > 0)
  {
    const double scale = static_cast<double>(targetTotal) / static_cast<double>(currentTotal);
    agility = static_cast<uint32_t>(agility * scale);
    courage = static_cast<uint32_t>(courage * scale);
    rush = static_cast<uint32_t>(rush * scale);
    endurance = static_cast<uint32_t>(endurance * scale);
    ambition = static_cast<uint32_t>(ambition * scale);

    // Absorb the rounding remainder into the largest stat.
    const uint32_t scaledTotal = agility + courage + rush + endurance + ambition;
    const int32_t difference = static_cast<int32_t>(targetTotal) - static_cast<int32_t>(scaledTotal);
    if (difference != 0)
    {
      uint32_t* largest = &agility;
      if (courage > *largest) largest = &courage;
      if (rush > *largest) largest = &rush;
      if (endurance > *largest) largest = &endurance;
      if (ambition > *largest) largest = &ambition;
      *largest = static_cast<uint32_t>(static_cast<int32_t>(*largest) + difference);
    }

    agility = std::min(agility, 100u);
    courage = std::min(courage, 100u);
    rush = std::min(rush, 100u);
    endurance = std::min(endurance, 100u);
    ambition = std::min(ambition, 100u);
  }

  result.agility = agility;
  result.courage = courage;
  result.rush = rush;
  result.endurance = endurance;
  result.ambition = ambition;

  return result;
}

data::Horse::Appearance Genetics::CalculateFoalAppearance(
  const data::Horse::Appearance& mareAppearance,
  const data::Horse::Appearance& stallionAppearance)
{
  const float spread = static_cast<float>(
    _serverInstance.GetBreedingRegistry().GetBreedingParams().appearanceVariation) / 100.0f;

  // Each value is the parent average scaled by a random factor in [1-spread, 1+spread],
  // clamped to the [1, 10] appearance range.
  const auto inherit = [&](uint32_t mareValue, uint32_t stallionValue) -> uint32_t
  {
    const float average = (static_cast<float>(mareValue) + static_cast<float>(stallionValue)) / 2.0f;
    std::uniform_real_distribution<float> variationDist(1.0f - spread, 1.0f + spread);
    const auto value = static_cast<int32_t>(std::lround(average * variationDist(_randomEngine)));
    return static_cast<uint32_t>(std::clamp(value, 1, 10));
  };

  data::Horse::Appearance result;
  result.scale = inherit(mareAppearance.scale(), stallionAppearance.scale());
  result.legLength = inherit(mareAppearance.legLength(), stallionAppearance.legLength());
  result.legVolume = inherit(mareAppearance.legVolume(), stallionAppearance.legVolume());
  result.bodyLength = inherit(mareAppearance.bodyLength(), stallionAppearance.bodyLength());
  result.bodyVolume = inherit(mareAppearance.bodyVolume(), stallionAppearance.bodyVolume());
  return result;
}

Genetics::PotentialResult Genetics::CalculateFoalPotential(
  const data::Uid mareUid,
  const data::Uid stallionUid,
  const data::Tid foalSkinTid)
{
  PotentialResult result; // level/value default to 0 for newborns.

  const auto& registry = _serverInstance.GetHorseRegistry();

  // Count parents that carry a potential.
  int parentsWithPotential = 0;
  const auto countPotential = [&](data::Uid uid)
  {
    if (uid == data::InvalidUid)
      return;
    const auto record = _serverInstance.GetDataDirector().GetHorse(uid);
    if (not record)
      return;
    record.Immutable([&](const data::Horse& horse)
    {
      if (horse.potential.type() > 0)
        ++parentsWithPotential;
    });
  };
  countPotential(mareUid);
  countPotential(stallionUid);

  // Probability: base 5%, plus a coat-tier bonus, plus 10% per parent with a potential.
  int probability = 5;
  switch (registry.GetCoatInfo(foalSkinTid).tier)
  {
    case registry::Coat::Tier::Rare:     probability += 10; break;
    case registry::Coat::Tier::Uncommon: probability += 5;  break;
    case registry::Coat::Tier::Common:   break;
  }
  probability += parentsWithPotential * 10;

  if (RollPercent() >= probability)
  {
    spdlog::debug("Genetics: foal gets no potential (needed < {})", probability);
    return result;
  }

  // Pick a random potential type in [1, 15] excluding 12.
  result.hasPotential = true;
  std::uniform_int_distribution<int> typeDist(1, 15);
  uint8_t type = 0;
  do
  {
    type = static_cast<uint8_t>(typeDist(_randomEngine));
  } while (type == 12);
  result.type = type;

  return result;
}

data::Tid Genetics::CalculateFoalSkin(
  const data::Uid mareUid,
  const data::Uid stallionUid,
  const uint8_t foalGrade,
  const uint32_t mareCombo,
  const uint32_t stallionCombo,
  const uint32_t pregnancyChance)
{
  const auto& registry = _serverInstance.GetHorseRegistry();
  const Ancestry ancestry = BuildAncestry(mareUid, stallionUid);

  const data::Tid mareSkin = ReadPart(mareUid, Part::Skin);
  const data::Tid stallionSkin = ReadPart(stallionUid, Part::Skin);
  if (mareSkin == 0 || stallionSkin == 0)
  {
    spdlog::error("Genetics: parent missing for skin calculation");
    return 1; // Chestnut.
  }

  uint32_t stallionLineage = 1;
  if (const auto record = _serverInstance.GetDataDirector().GetHorse(stallionUid))
    record.Immutable([&](const data::Horse& stallion) { stallionLineage = stallion.lineage(); });

  // Bonuses all push the foal towards the stallion's coat.
  const int32_t bonusUnit = _serverInstance.GetBreedingRegistry().GetBreedingParams().inheritanceRateBonusUnit;
  const uint16_t comboBonus = static_cast<uint16_t>((mareCombo + stallionCombo) * bonusUnit);
  const uint16_t pregnancyBonus = static_cast<uint16_t>(30 - std::min<uint32_t>(pregnancyChance, 30u));
  const uint16_t lineageBonus = (stallionLineage > 1) ? static_cast<uint16_t>(stallionLineage - 1) : 0;
  const uint16_t totalBonus = std::min<uint16_t>(comboBonus + pregnancyBonus + lineageBonus, 100);
  const float bonusMultiplier = 1.0f + (totalBonus / 100.0f);

  // Grandparent coats, in roll order (maternal then paternal).
  std::vector<data::Tid> gpSkins;
  for (const data::Uid gpUid : ancestry.grandparents)
  {
    if (const data::Tid skin = ReadPart(gpUid, Part::Skin); skin != 0)
      gpSkins.push_back(skin);
  }

  // Picks a grade-eligible coat at random, weighted by inheritance rate.
  const auto getRandomValidSkin = [&]() -> data::Tid
  {
    std::vector<data::Tid> tids;
    std::vector<float> weights;
    for (const data::Tid tid : registry.GetPossibleCoats())
    {
      const auto& coatInfo = registry.GetCoatInfo(tid);
      if (coatInfo.minGrade <= foalGrade)
      {
        tids.push_back(tid);
        weights.push_back(coatInfo.inheritanceRate);
      }
    }
    if (tids.empty())
      return 1;
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    return tids[dist(_randomEngine)];
  };

  // Keeps an inherited coat only if the foal's grade can wear it, else rolls random.
  const auto getValidSkinOrRandom = [&](data::Tid skinTid) -> data::Tid
  {
    return registry.GetCoatInfo(skinTid).minGrade <= foalGrade ? skinTid : getRandomValidSkin();
  };

  // Build weighted bands: mare, stallion (boosted), up to four grandparents, then random.
  const float mareWeight = registry.GetCoatInfo(mareSkin).inheritanceRate;
  const float stallionWeight = registry.GetCoatInfo(stallionSkin).inheritanceRate * bonusMultiplier;
  const float gpUnitWeight = 0.5f;
  const auto gpCount = std::min<size_t>(gpSkins.size(), 4);
  const float totalGpWeight = gpUnitWeight * gpCount;

  const float parentWeight = mareWeight + stallionWeight + totalGpWeight;
  const float randomWeight = std::max(30.0f, 60.0f - parentWeight * 0.2f);

  const float totalWeight = parentWeight + randomWeight;
  const float mareThreshold = (mareWeight / totalWeight) * 100.0f;
  const float stallionThreshold = mareThreshold + (stallionWeight / totalWeight) * 100.0f;
  const float gpBand = (gpUnitWeight / totalWeight) * 100.0f;

  float gpThresholds[4];
  float running = stallionThreshold;
  for (size_t i = 0; i < 4; ++i)
  {
    if (i < gpCount)
      running += gpBand;
    gpThresholds[i] = running;
  }

  const float roll = std::uniform_real_distribution<float>(0.0f, 100.0f)(_randomEngine);

  if (roll < mareThreshold)
    return getValidSkinOrRandom(mareSkin);
  if (roll < stallionThreshold)
    return getValidSkinOrRandom(stallionSkin);
  for (size_t i = 0; i < gpCount; ++i)
  {
    if (roll < gpThresholds[i])
      return getValidSkinOrRandom(gpSkins[i]);
  }
  return getRandomValidSkin();
}

uint32_t Genetics::CalculateLineage(
  const data::Tid foalSkinTid,
  const data::Uid mareUid,
  const data::Uid stallionUid)
{
  // Base 1 for the foal, +2 per parent sharing the coat, +1 per grandparent sharing it.
  uint32_t lineage = 1;
  const Ancestry ancestry = BuildAncestry(mareUid, stallionUid);

  if (ReadPart(ancestry.mare, Part::Skin) == foalSkinTid)
    lineage += 2;
  if (ReadPart(ancestry.stallion, Part::Skin) == foalSkinTid)
    lineage += 2;

  for (const data::Uid gpUid : ancestry.grandparents)
  {
    if (gpUid != data::InvalidUid && ReadPart(gpUid, Part::Skin) == foalSkinTid)
      lineage += 1;
  }

  return std::min(lineage, 9u);
}

} // namespace server
