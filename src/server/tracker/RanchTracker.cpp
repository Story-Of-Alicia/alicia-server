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

#include "server/tracker/RanchTracker.hpp"

namespace server::tracker
{

Oid RanchTracker::AddCharacter(data::Uid character)
{
  _characters[character] = _nextObjectId;
  return _nextObjectId++;
}

void RanchTracker::RemoveCharacter(data::Uid character)
{
  _characters.erase(character);
}

Oid RanchTracker::GetCharacterOid(data::Uid character) const
{
  const auto itr = _characters.find(character);
  if (itr == _characters.cend())
    return InvalidEntityOid;
  return itr->second;
}

Oid RanchTracker::AddHorse(data::Uid horse)
{
  _horses[horse] = _nextObjectId;
  return _nextObjectId++;
}

void RanchTracker::RemoveHorse(data::Uid horse)
{
  _horses.erase(horse);
}

Oid RanchTracker::GetHorseOid(data::Uid horse) const
{
  const auto itr = _horses.find(horse);
  if (itr == _horses.cend())
    return InvalidEntityOid;
  return itr->second;
}

const RanchTracker::ObjectMap& RanchTracker::GetCharacters() const
{
  return _characters;
}

const RanchTracker::ObjectMap& RanchTracker::GetHorses() const
{
  return _horses;
}

} // namespace server::tracker
