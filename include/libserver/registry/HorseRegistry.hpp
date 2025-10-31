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

struct Coat
{
  data::Tid tid{data::InvalidTid};
  //! -1 and 0
  int32_t faceType{0};
};

struct SkinInfo
{
  data::Tid tid{data::InvalidTid};
  int32_t minGrade{1};
  int32_t rarityTier{1}; // 1=Common, 2=Uncommon, 3=Rare/Epic/Legendary
  int32_t faceType{0};
  std::vector<int32_t> allowedManeColors; // Valid mane/tail colors for this coat (1-5)
};

struct Face
{
  data::Tid tid{data::InvalidTid};
  int32_t type{0};
};

struct Mane
{
  data::Tid tid{data::InvalidTid};
  int32_t colorGroup{0};
};

struct Tail
{
  data::Tid tid{data::InvalidTid};
  int32_t colorGroup{0};
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

  //! Gets the color group for a mane TID.
  //! @param maneTid Mane template ID.
  //! @returns Color group (1-5), or 0 if not found.
  int32_t GetManeColorGroup(data::Tid maneTid) const;

  //! Gets the color group for a tail TID.
  //! @param tailTid Tail template ID.
  //! @returns Color group (1-5), or 0 if not found.
  int32_t GetTailColorGroup(data::Tid tailTid) const;

  //! Gets skin information for a given skin TID.
  //! @param skinTid Skin template ID.
  //! @returns Pointer to SkinInfo, or nullptr if not found.
  const SkinInfo* GetSkinInfo(data::Tid skinTid) const;

private:
  std::random_device _randomDevice;
  std::unordered_map<data::Tid, Coat> _coats;
  std::unordered_map<data::Tid, Face> _faces;
  std::unordered_map<data::Tid, SkinInfo> _skins;

  std::unordered_map<data::Tid, Mane> _manes;
  std::unordered_map<data::Tid, Tail> _tails;

  struct ManeTailColorGroup
  {
    data::Tid maneTid{data::InvalidTid};
    data::Tid tailTid{data::InvalidTid};
  };

  //! A vector of manes and tails with a matching color group.
  std::vector<ManeTailColorGroup> maneTailColorGroups;
};

} // namespace server::registry

#endif //HORSEREGISTRY_HPP
