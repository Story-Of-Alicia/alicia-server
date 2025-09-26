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

#include "libserver/data/DataDefinitions.hpp"

namespace server::registry
{

struct Coat
{
  data::Tid tid{data::InvalidTid};
  //! -1 and 0
  int32_t faceType{0};
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

private:
  std::random_device _randomDevice;
  std::unordered_map<data::Tid, Coat> _coats;
  std::unordered_map<data::Tid, Face> _faces;

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
