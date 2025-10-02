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

#ifndef RANCHTRACKER_HPP
#define RANCHTRACKER_HPP

#include "server/tracker/Tracker.hpp"

#include <libserver/data/DataDefinitions.hpp>

#include <map>

namespace server::tracker
{

//! A ranch tracker.
class RanchTracker
{
public:
  //! An object map.
  using ObjectMap = std::map<data::Uid, uint16_t>;

  //! Adds a character for tracking.
  //! @param character Character UID.
  //! @returns The OID assigned to the character.
  Oid AddCharacter(data::Uid character);
  //! Removes a character from tracking.
  //! @param character Character UID.
  void RemoveCharacter(data::Uid character);
  //! Returns the tracker OID assigned to the specified character.
  //! @returns The OID assigned to the character.
  [[nodiscard]] Oid GetCharacterOid(data::Uid character) const;

  //! Adds a horse for tracking.
  //! @param horse Horse UID.
  //! @returns The OID assigned to the horse.
  Oid AddHorse(data::Uid horse);
  //! Removes a horse from tracking.
  //! @param horse Horse UID.
  void RemoveHorse(data::Uid horse);
  //! Removes a horse from tracking.
  //! @param horse Character UID.
  [[nodiscard]] Oid GetHorseOid(data::Uid horse) const;

  //! Returns tracked characters.
  //! @return Tracked characters.
  [[nodiscard]] const ObjectMap& GetCharacters() const;
  //! Returns tracked horses.
  //! @return Tracked horses.
  [[nodiscard]] const ObjectMap& GetHorses() const;

private:
  //! The next entity ID.
  Oid _nextObjectId = 1;
  //! Character entities in the ranch.
  ObjectMap _characters;
  //! Horse entities in the ranch.
  ObjectMap _horses;
};

} // namespace server::tracker

#endif // RANCHTRACKER_HPP
