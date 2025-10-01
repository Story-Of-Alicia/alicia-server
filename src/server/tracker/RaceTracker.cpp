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

#include "server/tracker/RaceTracker.hpp"

namespace server::tracker
{

RaceTracker::Racer& RaceTracker::AddRacer(data::Uid characterUid)
{
  const auto [racerIter, created] = _racers.try_emplace(characterUid);
  if (not created)
    throw std::runtime_error("Character is already a racer");

  racerIter->second.oid = _nextObjectId++;

  return racerIter->second;
}

void RaceTracker::RemoveRacer(data::Uid characterUid)
{
  _racers.erase(characterUid);
}

RaceTracker::Racer& RaceTracker::GetRacer(data::Uid characterUid)
{
  auto racerIter = _racers.find(characterUid);
  if (racerIter == _racers.cend())
    throw std::runtime_error("Character is not a racer");

  return racerIter->second;
}

RaceTracker::ObjectMap& RaceTracker::GetRacers()
{
  return _racers;
}

RaceTracker::Item& RaceTracker::AddItem()
{
  const auto [itemIter, created] = _items.try_emplace(_nextItemId);
  if (not created)
    throw std::runtime_error("Item is already added to the race map");

  itemIter->second.itemId = _nextItemId++;

  return itemIter->second;
}

void RaceTracker::RemoveItem(uint16_t itemId)
{
  _items.erase(itemId);
}

RaceTracker::Item& RaceTracker::GetItem(uint16_t itemId)
{
  auto itemIter = _items.find(itemId);
  if (itemIter == _items.cend())
    throw std::runtime_error("Item is not in the race map");

  return itemIter->second;
}

RaceTracker::ItemObjectMap& RaceTracker::GetItems()
{
  return _items;
}

} // namespace server::tracker
