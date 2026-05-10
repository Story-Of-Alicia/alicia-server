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

  // Reuse the OID from a previous race if the player was here before, otherwise assign a new one.
  const auto [oidIter, isNew] = _characterOids.try_emplace(characterUid, _nextCharacterOid);
  if (isNew)
    ++_nextCharacterOid;

  racerIter->second.oid = oidIter->second;

  return racerIter->second;
}

void RaceTracker::RemoveRacer(data::Uid characterUid)
{
  _racers.erase(characterUid);
}

bool RaceTracker::IsRacer(data::Uid characterUid) const
{
  return _racers.contains(characterUid);
}

RaceTracker::Racer& RaceTracker::GetRacer(data::Uid characterUid)
{
  auto racerIter = _racers.find(characterUid);
  if (racerIter == _racers.cend())
    throw std::runtime_error("Character is not a racer");

  return racerIter->second;
}

RaceTracker::RacerObjectMap& RaceTracker::GetRacers()
{
  return _racers;
}

RaceTracker::Item& RaceTracker::AddItem()
{
  const auto [itemIter, created] = _items.try_emplace(_nextItemOid);
  if (not created)
    throw std::runtime_error("Item is already added to the race map");

  itemIter->second.oid = _nextItemOid++;
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
RaceTracker::EventItem& RaceTracker::AddEventItem(data::Uid characterUid)
{
  auto& racer = GetRacer(characterUid);
  auto& eventItem = racer.eventItems.emplace_back();
  eventItem.oid = _nextItemOid++;
  return eventItem;
}

Oid RaceTracker::FindEventItem(data::Uid characterUid, Oid oid)
{
  auto& racer = GetRacer(characterUid);
  for (auto& eventItem : racer.eventItems)
  {
    if (eventItem.oid == oid)
      return eventItem.oid;
  }
  return InvalidEntityOid;
}

RaceTracker::EventItem& RaceTracker::GetEventItem(data::Uid characterUid, Oid oid)
{
  auto& racer = GetRacer(characterUid);
  for (auto& eventItem : racer.eventItems)
  {
    if (eventItem.oid == oid)
      return eventItem;
  }

  throw std::runtime_error("Event item is not tracked for racer");
}

void RaceTracker::RemoveEventItem(data::Uid characterUid, Oid oid)
{
  auto& racer = GetRacer(characterUid);
  std::erase_if(racer.eventItems, [oid](const EventItem& e) { return e.oid == oid; });
}

void RaceTracker::Clear()
{
  _racers.clear();
  _items.clear();
  _events.clear();
  _nextItemOid = 1;
}

uint16_t RaceTracker::GetNextEffectInstanceIdAndIncrementBy(uint16_t increment)
{
  const uint16_t nextId = _nextEffectInstanceId;
  _nextEffectInstanceId += increment;
  if (_nextEffectInstanceId == 0)
    _nextEffectInstanceId = 1;
  return nextId;
}

bool RaceTracker::IsEventThrottled(uint32_t eventId)
{
  const auto& now = std::chrono::steady_clock::now();

  const auto& [eventIter, inserted] = _events.try_emplace(eventId);
  if (not inserted and eventIter->second.throttledUntil > now)
  {
    // Existing event was throttled
    return true;
  }
  else if (inserted)
  {
    eventIter->second.id = eventId;
  }

  // New event or event expired, update throttle time
  eventIter->second.throttledUntil = now + ThrottleDurationMs;
  return false;
}

RaceTracker::EventMap& RaceTracker::GetEvents()
{
  return _events;
}

} // namespace server::tracker
