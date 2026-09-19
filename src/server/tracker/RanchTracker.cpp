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

#include <spdlog/spdlog.h>

namespace server::tracker
{

Oid RanchTracker::AddCharacter(data::Uid character)
{
  // Don't overwrite an already-tracked character.
  if (_characters.contains(character))
    return _characters.at(character).oid;

  const Oid oid = _nextObjectId++;
  _characters[character] = Character{.oid = oid, .busyState = 0};
  return oid;
}

void RanchTracker::RemoveCharacter(data::Uid character)
{
  // Only erase if the character is actually tracked.
  if (!_characters.contains(character))
    return;

  _characters.erase(character);
}

RanchTracker::Character& RanchTracker::GetCharacter(data::Uid character)
{
  const auto itr = _characters.find(character);
  if (itr == _characters.cend())
    throw std::runtime_error("Character is not tracked in the ranch");
  return itr->second;
}

const RanchTracker::Character& RanchTracker::GetCharacter(data::Uid character) const
{
  const auto itr = _characters.find(character);
  if (itr == _characters.cend())
    throw std::runtime_error("Character is not tracked in the ranch");
  return itr->second;
}

Oid RanchTracker::AddHorse(data::Uid horse)
{
  if (_horses.contains(horse))
    return _horses.at(horse);

  _horses[horse] = _nextObjectId;
  return _nextObjectId++;
}

void RanchTracker::RemoveHorse(data::Uid horse)
{
  // Only erase if the horse is actually tracked.
  if (!_horses.contains(horse))
  {
    spdlog::debug("Could not remove horse UID {} from ranch tracker - not found", horse);
    return;
  }

  _horses.erase(horse);
}

Oid RanchTracker::GetHorseOid(data::Uid horse) const
{
  const auto itr = _horses.find(horse);
  if (itr == _horses.cend())
    return InvalidEntityOid;
  return itr->second;
}

const RanchTracker::CharacterMap& RanchTracker::GetCharacters() const
{
  return _characters;
}

const RanchTracker::ObjectMap& RanchTracker::GetHorses() const
{
  return _horses;
}

} // namespace server::tracker
