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

#ifndef HORSEREGISTRY_HPP
#define HORSEREGISTRY_HPP

#include <random>
#include <unordered_map>
#include <vector>

#include "libserver/data/DataDefinitions.hpp"

namespace server::registry
{

enum class Color
{
  White = 1,
  LightBrown = 2,
  Brown = 3,
  DarkBrown = 4,
  Grey = 5,
  Black = 6
};

struct ColorGroup
{
  int32_t id{0};
  std::vector<Color> colors;
};

struct Coat
{
  enum class Tier
  {
    Common = 1,
    Uncommon = 2,
    Rare = 3
  };

  data::Tid tid{data::InvalidTid};
  //! -1 and 0
  int32_t faceType{0};
  int32_t minGrade{1};
  Tier tier{Tier::Common};
  float inheritanceRate{1.0f}; // Base probability weight for this coat (from DNA_SkinInfo)
  std::vector<int32_t> allowedColorGroups; // Valid color groups for this coat
};

struct Face
{
  data::Tid tid{data::InvalidTid};
  int32_t type{0};
};

struct ManeShapeInfo
{
  int32_t shape{0};
  float inheritanceRate{1.0f};
  int32_t minGrade{1};
};

struct TailShapeInfo
{
  int32_t shape{0};
  float inheritanceRate{1.0f};
  int32_t minGrade{1};
};

struct Mane
{
  data::Tid tid{data::InvalidTid};
  Color color{Color::White};
  int32_t shape{0};
  float inheritanceRate{1.0f};
  int32_t minGrade{1};
};

struct Tail
{
  data::Tid tid{data::InvalidTid};
  Color color{Color::White};
  int32_t shape{0};
  float inheritanceRate{1.0f};
  int32_t minGrade{1};
};


class HorseRegistry
{
public:
  HorseRegistry();

  void ReadConfig();

  void BuildRandomHorse(
    data::Horse::Parts& parts,
    data::Horse::Appearance& appearance);

  void SetHorsePotential(
    data::Horse::Potential& potential,
    uint8_t type,
    uint8_t level,
    uint8_t value);

  void GiveHorseRandomPotential(
    data::Horse::Potential& potential);

  //! Gets coat information for a given coat TID.
  //! @param coatTid Coat template ID.
  //! @returns Reference to Coat, with fallback to default if not found.
  const Coat& GetCoatInfo(data::Tid coatTid) const;

  //! Gets a random mane TID from the specified color group and shape.
  //! @param colorGroupId Color group ID.
  //! @param shape Mane shape (0-7).
  //! @param rng Random number generator.
  //! @returns Mane TID, or InvalidTid if not found.
  data::Tid GetRandomManeFromColorAndShape(int32_t colorGroupId, int32_t shape, std::mt19937& rng) const;

  //! Gets a random tail TID from the specified color group and shape.
  //! @param colorGroupId Color group ID.
  //! @param shape Tail shape (0-5).
  //! @param rng Random number generator.
  //! @returns Tail TID, or InvalidTid if not found.
  data::Tid GetRandomTailFromColorAndShape(int32_t colorGroupId, int32_t shape, std::mt19937& rng) const;

  //! Gets the color group ID for a mane TID.
  //! @param maneTid Mane template ID.
  //! @returns Color group ID, or 0 if not found.
  int32_t GetManeColorGroupId(data::Tid maneTid) const;

  //! Gets the color group ID for a tail TID.
  //! @param tailTid Tail template ID.
  //! @returns Color group ID, or 0 if not found.
  int32_t GetTailColorGroupId(data::Tid tailTid) const;

  //! Gets mane shape from TID.
  //! @param maneTid Mane template ID.
  //! @returns Mane shape (0-7), or 0 if not found.
  int32_t GetManeShape(data::Tid maneTid) const;

  //! Gets tail shape from TID.
  //! @param tailTid Tail template ID.
  //! @returns Tail shape (0-5), or 0 if not found.
  int32_t GetTailShape(data::Tid tailTid) const;

  //! Gets the color of a mane TID.
  //! @param maneTid Mane template ID.
  //! @returns Color enum value.
  Color GetManeColor(data::Tid maneTid) const;

  //! Finds a tail TID with a specific color and shape.
  //! @param color Desired color.
  //! @param shape Desired shape (0-5).
  //! @returns Tail TID, or InvalidTid if not found.
  data::Tid FindTailByColorAndShape(Color color, int32_t shape) const;

  //! Gets mane shape information.
  //! @param shape Mane shape (0-7).
  //! @returns ManeShapeInfo with inheritance rate and grade requirements.
  const ManeShapeInfo& GetManeShapeInfo(int32_t shape) const;

  //! Gets tail shape information.
  //! @param shape Tail shape (0-5).
  //! @returns TailShapeInfo with inheritance rate and grade requirements.
  const TailShapeInfo& GetTailShapeInfo(int32_t shape) const;

  //! Gets mane by TID (for accessing inheritance rate/minGrade).
  //! @param tid Mane TID.
  //! @returns Pointer to Mane, or nullptr if not found.
  const Mane* GetMane(data::Tid tid) const;

  //! Gets tail by TID (for accessing inheritance rate/minGrade).
  //! @param tid Tail TID.
  //! @returns Pointer to Tail, or nullptr if not found.
  const Tail* GetTail(data::Tid tid) const;

private:
  std::random_device _randomDevice;
  std::unordered_map<data::Tid, Coat> _coats;
  std::unordered_map<data::Tid, Face> _faces;
  std::unordered_map<int32_t, ColorGroup> _colorGroups;

  std::unordered_map<data::Tid, Mane> _manes;
  std::unordered_map<data::Tid, Tail> _tails;
  
  std::unordered_map<int32_t, ManeShapeInfo> _maneShapes;
  std::unordered_map<int32_t, TailShapeInfo> _tailShapes;

  // Lookup tables for efficient querying: [colorGroupId][shape] -> vector of TIDs
  std::unordered_map<int32_t, std::unordered_map<int32_t, std::vector<data::Tid>>> _manesByColorAndShape;
  std::unordered_map<int32_t, std::unordered_map<int32_t, std::vector<data::Tid>>> _tailsByColorAndShape;
};

} // namespace server::registry

#endif //HORSEREGISTRY_HPP
