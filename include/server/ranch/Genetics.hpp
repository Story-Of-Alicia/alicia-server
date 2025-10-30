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
  //! @returns Mane and tail TIDs with separately inherited color and shape
  ManeTailResult CalculateManeTailGenetics(
    data::Uid mareUid,
    data::Uid stallionUid,
    uint8_t foalGrade);

  //! Calculates foal grade based on parent grades.
  //! @param mareGrade Mare's grade (1-8)
  //! @param stallionGrade Stallion's grade (1-8)
  //! @returns Foal's grade (1-8)
  uint8_t CalculateFoalGrade(uint8_t mareGrade, uint8_t stallionGrade);

  //! Calculates foal stat based on parent stats.
  //! @param mareStat Mare's stat value
  //! @param stallionStat Stallion's stat value
  //! @returns Foal's stat value
  uint32_t CalculateFoalStat(uint32_t mareStat, uint32_t stallionStat);

private:
  ServerInstance& _serverInstance;

  //! Extracts color from TID (1-5, cycles every 5)
  static int32_t GetColorFromTid(data::Tid tid);

  //! Extracts shape group from TID
  static int32_t GetShapeFromTid(data::Tid tid);

  //! Validates and adjusts mane shape based on grade requirements
  static void ValidateManeShape(int32_t& maneShape, uint8_t foalGrade);

  //! Validates and adjusts tail shape based on grade requirements
  static void ValidateTailShape(int32_t& tailShape, uint8_t foalGrade);
};

} // namespace server

#endif // GENETICS_HPP

