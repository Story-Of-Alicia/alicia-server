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

#include "server/tracker/WorldTracker.hpp"

namespace server
{

Oid WorldTracker::AddCharacter(data::Uid character)
{
  _characters[character] = _nextObjectId;
  return _nextObjectId++;
}

void WorldTracker::RemoveCharacter(data::Uid character)
{
  _characters.erase(character);
}

Oid WorldTracker::GetCharacterOid(data::Uid character) const
{
  const auto itr = _characters.find(character);
  if (itr == _characters.cend())
    return InvalidEntityOid;
  return itr->second;
}

Oid WorldTracker::AddHorse(data::Uid mount)
{
  _horses[mount] = _nextObjectId;
  return _nextObjectId++;
}

Oid WorldTracker::GetHorseOid(data::Uid mount) const
{
  const auto itr = _horses.find(mount);
  if (itr == _horses.cend())
    return InvalidEntityOid;
  return itr->second;
}

const WorldTracker::ObjectMap& WorldTracker::GetHorses() const
{
  return _horses;
}

const WorldTracker::ObjectMap& WorldTracker::GetCharacters() const
{
  return _characters;
}

} // namespace server
