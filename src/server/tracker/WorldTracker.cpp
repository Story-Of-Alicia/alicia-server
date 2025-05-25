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

namespace alicia
{

EntityId WorldTracker::AddCharacter(soa::data::Uid character)
{
  _characters[character] = _nextEntityId;
  return _nextEntityId++;
}

EntityId WorldTracker::GetCharacterEntityId(soa::data::Uid character)
{
  const auto itr = _characters.find(character);
  if (itr == _characters.cend())
    return InvalidEntityId;
  return itr->second;
}

EntityId WorldTracker::AddMount(soa::data::Uid mount)
{
  _mounts[mount] = _nextEntityId;
  return _nextEntityId++;
}

EntityId WorldTracker::GetMountEntityId(soa::data::Uid mount)
{
  const auto itr = _mounts.find(mount);
  if (itr == _mounts.cend())
    return InvalidEntityId;
  return itr->second;
}

const WorldTracker::EntityMap& WorldTracker::GetMountEntities()
{
  return _mounts;
}

const WorldTracker::EntityMap& WorldTracker::GetCharacterEntities()
{
  return _characters;
}

} // namespace alicia
