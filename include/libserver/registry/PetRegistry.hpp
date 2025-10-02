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

#ifndef PETREGISTRY_HPP
#define PETREGISTRY_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct EggInfo
{
  //! A deck item ID of the egg.
  data::Tid deckItemId;
  //! A time duration it takes the egg to hatch.
  data::Clock::duration hatchDuration;
  //! A vector of pets that can hatch from the egg.
  std::vector<data::Tid> hatchablePets;
};

struct PetInfo
{
  //! A pet ID.
  uint32_t petId{};
};

class PetRegistry final
{
public:
  void ReadConfig(const std::filesystem::path& configPath);

  EggInfo GetEggInfo(data::Tid eggItemTid);
  PetInfo GetPetInfo(data::Tid petItemTid);

private:
  std::unordered_map<data::Tid, EggInfo> _eggs;
  std::unordered_map<data::Tid, PetInfo> _pets;
};

} // namespace server::registry

#endif // PETREGISTRY_HPP
