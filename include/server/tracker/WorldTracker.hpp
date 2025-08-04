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

#include <map>

namespace server
{

//!
using Oid = uint16_t;
//!
constexpr Oid InvalidEntityOid = 0;

//!
class WorldTracker
{
public:
  //! An object map.
  using ObjectMap = std::map<data::Uid, uint16_t>;

  //!
  Oid AddCharacter(data::Uid character);
  //!
  void RemoveCharacter(data::Uid character);
  //!
  [[nodiscard]] Oid GetCharacterOid(data::Uid character) const;
  //!
  Oid AddHorse(data::Uid mount);
  //!
  [[nodiscard]] Oid GetHorseOid(data::Uid mount) const;

  //!
  [[nodiscard]] const ObjectMap& GetHorses() const;
  //!
  [[nodiscard]] const ObjectMap& GetCharacters() const;

private:
  //! The next entity ID.
  Oid _nextObjectId = 1;
  //! Mount entities in the world.
  ObjectMap _horses;
  //! Character entities in the world.
  ObjectMap _characters;
};

} // namespace server

#endif // WORLDTRACKER_HPP
