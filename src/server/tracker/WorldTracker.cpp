//
// Created by rgnter on 17/12/2024.
//

#include "server/tracker/WorldTracker.hpp"

namespace alicia
{

EntityId WorldTracker::AddCharacter(DatumUid character)
{
  _characters[character] = _nextEntityId;
  return _nextEntityId++;
}

EntityId WorldTracker::GetCharacterEntityId(DatumUid character)
{
  const auto itr = _characters.find(character);
  if (itr == _characters.cend())
    return InvalidEntityId;
  return itr->second;
}

EntityId WorldTracker::AddMount(DatumUid mount)
{
  _mounts[mount] = _nextEntityId;
  return _nextEntityId++;
}

EntityId WorldTracker::GetMountEntityId(DatumUid mount)
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
