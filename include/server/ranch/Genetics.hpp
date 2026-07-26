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

#ifndef GENETICS_HPP
#define GENETICS_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataStorage.hpp>

#include <array>
#include <random>

namespace server
{

// Forward declaration
class ServerInstance;

//! Handles genetics calculations for horse breeding
class Genetics
{
public:
  explicit Genetics(ServerInstance& serverInstance);

  //! Result of mane/tail genetics calculation
  struct ManeTailResult
  {
    data::Tid maneTid{0};
    data::Tid tailTid{0};
  };

  //! Calculates mane and tail genetics based on parents and grandparents.
  //! Color and shape are inherited separately with weighted probabilities.
  //! @param mareUid Mare's UID
  //! @param stallionUid Stallion's UID
  //! @param foalGrade Foal's calculated grade (for shape restrictions)
  //! @param foalSkinTid Foal's skin TID (determines allowed colors)
  //! @returns Mane and tail TIDs with separately inherited color and shape
  ManeTailResult CalculateManeTailGenetics(
    data::Uid mareUid,
    data::Uid stallionUid,
    uint8_t foalGrade,
    data::Tid foalSkinTid);

  //! Calculates skin/coat genetics based on parents and grandparents.
  //! Weighted inheritance: Mom 10%, Dad 10% (+ inheritance rate bonus), GPs 5% each, Random 60%
  //! @param mareUid Mare's UID
  //! @param stallionUid Stallion's UID
  //! @param foalGrade Foal's calculated grade (for minimum grade requirements)
  //! @param mareCombo Mare's consecutive breeding success count
  //! @param stallionCombo Stallion's consecutive breeding success count
  //! @param pregnancyChance Stallion's pregnancy chance (0-30, lower = better)
  //! @returns Skin TID that the foal inherits
  data::Tid CalculateFoalSkin(
    data::Uid mareUid,
    data::Uid stallionUid,
    uint8_t foalGrade,
    uint32_t mareCombo = 0,
    uint32_t stallionCombo = 0,
    uint32_t pregnancyChance = 30);

  //! Calculates foal grade based on parent grades.
  //! @param mareGrade Mare's grade (1-8)
  //! @param stallionGrade Stallion's grade (1-8)
  //! @returns Foal's grade (1-8)
  uint8_t CalculateFoalGrade(uint8_t mareGrade, uint8_t stallionGrade);

  //! Calculates all foal stats based on parents and target grade.
  //! @param mareStats Parent mare's stats.
  //! @param stallionStats Parent stallion's stats.
  //! @param targetGrade Target grade to fit stats within.
  //! @returns All five stats that fit the target grade.
  data::Horse::Stats CalculateFoalStats(
    const data::Horse::Stats& mareStats,
    const data::Horse::Stats& stallionStats,
    uint8_t targetGrade);

  //! Inherits each foal appearance value as the parent average scaled by a random factor
  //! within +/- the configured variation (breeding.yaml -> appearanceVariation),
  //! clamped to the [1, 10] appearance range.
  //! @param mareAppearance Parent mare's appearance.
  //! @param stallionAppearance Parent stallion's appearance.
  //! @returns The foal's appearance.
  data::Horse::Appearance CalculateFoalAppearance(
    const data::Horse::Appearance& mareAppearance,
    const data::Horse::Appearance& stallionAppearance);

  //! Result of potential genetics calculation
  struct PotentialResult
  {
    uint8_t type{0};
    uint8_t level{0};  // 1 when the foal has a potential, else 0
    uint8_t value{0};  // Always 0 for newborns
  };

  //! Calculates whether foal has a potential and what type.
  //! Base 5% chance + coat star bonus + parent potential bonus
  //! @param mareUid Mare's UID
  //! @param stallionUid Stallion's UID
  //! @param foalSkinTid Foal's skin TID (determines star bonus)
  //! @returns Potential result
  PotentialResult CalculateFoalPotential(
    data::Uid mareUid,
    data::Uid stallionUid,
    data::Tid foalSkinTid);

  //! Calculates lineage score based on coat matching with parents and grandparents.
  //! Base lineage is 1. +2 for each parent with matching coat, +1 for each grandparent with matching coat.
  //! @param foalSkinTid Foal's skin TID (coat to match against)
  //! @param mareUid Mare's UID
  //! @param stallionUid Stallion's UID
  //! @returns Lineage score (1-9)
  uint32_t CalculateLineage(
    data::Tid foalSkinTid,
    data::Uid mareUid,
    data::Uid stallionUid);

  //! Fully initialises a newborn foal record bred from the two parents: administrative
  //! defaults plus every genetic attribute (breed, grade, coat, mane/tail, stats,
  //! appearance, potential, ancestry and lineage). Does not touch any protocol response.
  //! @param foal Foal record to populate.
  //! @param mareUid Mare's UID.
  //! @param stallionUid Stallion's UID.
  //! @param gradeBonus Extra grade from a fertility-peak breeding bonus (0 if none).
  void CreateFoal(
    data::Horse& foal,
    data::Uid mareUid,
    data::Uid stallionUid,
    uint32_t gradeBonus);

private:
  ServerInstance& _serverInstance;
  std::mt19937 _randomEngine;

  //! Rolls a foal tendency weighted by horses.yaml -> tendencyRatios.breedingRatio.
  uint32_t RollTendency();

  //! Rolls a foal emblem: a rarity tier weighted by emblems.yaml -> emblemRatios,
  //! then a uniform emblem within that tier. Always returns an emblem.
  uint32_t RollEmblem();

  //! Selects which body part of a horse to read.
  enum class Part
  {
    Skin,
    Mane,
    Tail
  };

  //! The two parents and their four grandparents, in roll order:
  //! maternal grandmother, maternal grandfather, paternal grandmother, paternal grandfather.
  //! Absent ancestors are data::InvalidUid.
  struct Ancestry
  {
    data::Uid mare{data::InvalidUid};
    data::Uid stallion{data::InvalidUid};
    std::array<data::Uid, 4> grandparents{
      data::InvalidUid, data::InvalidUid, data::InvalidUid, data::InvalidUid};
  };

  //! Resolves the parents and grandparents of a prospective foal.
  Ancestry BuildAncestry(data::Uid mareUid, data::Uid stallionUid);

  //! Rolls which ancestor a trait is inherited from.
  //! Mare 10%, stallion 10%, each grandparent 5%, otherwise random (60%).
  //! @returns The donor's UID, or data::InvalidUid when the trait should be rolled randomly
  //!          (also when the rolled grandparent does not exist).
  data::Uid RollInheritedDonor(const Ancestry& ancestry);

  //! Reads a single part TID from a horse by UID.
  //! @returns The part TID, or 0 if the horse is not found.
  data::Tid ReadPart(data::Uid horseUid, Part part);

  //! Rolls a percentage value in [0, 99].
  int RollPercent();

  //! Extracts shape from a mane or tail TID.
  //! @param tid Mane or tail TID.
  //! @param part Part::Mane or Part::Tail.
  //! @returns Shape (0-7 for manes, 0-5 for tails).
  int32_t GetShapeFromTid(data::Tid tid, Part part);

  //! Inherits a mane or tail shape from an ancestor, or rolls a grade-weighted random one.
  int32_t InheritShape(const Ancestry& ancestry, uint8_t foalGrade, Part part);

  //! Clamps a mane or tail shape to the highest shape the foal's grade may wear,
  //! per the registry's per-shape minGrade data. Rerolls uniformly if out of range.
  void ValidateShape(int32_t& shape, uint8_t foalGrade, Part part);

  //! Combines the combo, pregnancy and lineage bonuses into a multiplier (1.0-2.0)
  //! that biases coat inheritance towards the stallion.
  float StallionCoatBonusMultiplier(
    uint32_t mareCombo,
    uint32_t stallionCombo,
    uint32_t pregnancyChance,
    uint32_t stallionLineage);
};

} // namespace server

#endif // GENETICS_HPP

