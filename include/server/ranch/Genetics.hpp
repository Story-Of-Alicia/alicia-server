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
    int32_t maneColor{0};
    int32_t maneShape{0};
    int32_t tailColor{0};
    int32_t tailShape{0};
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
    uint8_t mareCombo = 0,
    uint8_t stallionCombo = 0,
    uint32_t pregnancyChance = 30);

  //! Calculates foal grade based on parent grades.
  //! @param mareGrade Mare's grade (1-8)
  //! @param stallionGrade Stallion's grade (1-8)
  //! @param fertilityPeakLevel Fertility peak bonus level (0-3, default 0)
  //! @returns Foal's grade (1-8)
  uint8_t CalculateFoalGrade(uint8_t mareGrade, uint8_t stallionGrade, uint8_t fertilityPeakLevel = 0);

  //! Calculates foal stat based on parent stats.
  //! @param mareStat Mare's stat value
  //! @param stallionStat Stallion's stat value
  //! @returns Foal's stat value
  uint32_t CalculateFoalStat(uint32_t mareStat, uint32_t stallionStat);

  //! Calculates grade from total stats.
  //! @param totalStats Sum of all 5 stats
  //! @returns Grade (1-8) based on total stats
  static uint8_t CalculateGradeFromStats(uint32_t totalStats);

  //! Result of stat genetics calculation
  struct StatResult
  {
    uint32_t agility{0};
    uint32_t courage{0};
    uint32_t rush{0};
    uint32_t endurance{0};
    uint32_t ambition{0};
  };

  //! Calculates all foal stats based on parents and target grade.
  //! @param mareStats Parent mare's stats
  //! @param stallionStats Parent stallion's stats
  //! @param targetGrade Target grade to fit stats within
  //! @returns All five stats that fit the target grade
  StatResult CalculateFoalStats(
    uint32_t mareAgility, uint32_t mareCourage, uint32_t mareRush, 
    uint32_t mareEndurance, uint32_t mareAmbition,
    uint32_t stallionAgility, uint32_t stallionCourage, uint32_t stallionRush,
    uint32_t stallionEndurance, uint32_t stallionAmbition,
    uint8_t targetGrade);

  //! Result of potential genetics calculation
  struct PotentialResult
  {
    bool hasPotential{false};
    uint8_t type{0};
    uint8_t level{0};  // Always 0 for newborns
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

private:
  ServerInstance& _serverInstance;
  std::mt19937 _randomEngine;

  //! Gets color group ID from a mane or tail TID
  //! @param tid Mane or tail TID
  //! @param isMane True if mane, false if tail
  //! @returns Color group ID (1-3), or 0 if not found
  int32_t GetColorGroupIdFromTid(data::Tid tid, bool isMane);

  //! Extracts shape from a mane or tail TID
  //! @param tid Mane or tail TID
  //! @param isMane True if mane, false if tail
  //! @returns Shape (0-7 for manes, 0-5 for tails)
  int32_t GetShapeFromTid(data::Tid tid, bool isMane);

  //! Validates and adjusts mane shape based on grade requirements
  static void ValidateManeShape(int32_t& maneShape, uint8_t foalGrade);

  //! Validates and adjusts tail shape based on grade requirements
  static void ValidateTailShape(int32_t& tailShape, uint8_t foalGrade);
};

} // namespace server

#endif // GENETICS_HPP

