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

#ifndef WORLDTRACKER_HPP
#define WORLDTRACKER_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <unordered_map>

namespace alicia
{

//!
using EntityId = uint16_t;
//!
constexpr EntityId InvalidEntityId = 0;

//!
class WorldTracker
{
public:
  //! An entity map.
  using EntityMap = std::unordered_map<soa::data::Uid, uint16_t>;

  //!
  EntityId AddCharacter(soa::data::Uid character);
  //!
  [[nodiscard]] EntityId GetCharacterEntityId(soa::data::Uid character);
  //!
  EntityId AddMount(soa::data::Uid mount);
  //!
  [[nodiscard]] EntityId GetMountEntityId(soa::data::Uid mount);

  //!
  [[nodiscard]] const EntityMap& GetMountEntities();
  //!
  [[nodiscard]] const EntityMap& GetCharacterEntities();

private:
  //! The next entity ID.
  EntityId _nextEntityId = 1;
  //! Mount entities in the world.
  EntityMap _mounts;
  //! Character entities in the world.
  EntityMap _characters;
};

} // namespace alicia

#endif // WORLDTRACKER_HPP
