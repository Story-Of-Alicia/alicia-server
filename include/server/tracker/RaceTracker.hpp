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

#ifndef RACETRACKER_HPP
#define RACETRACKER_HPP

#include "server/tracker/Tracker.hpp"

#include <libserver/data/DataDefinitions.hpp>

#include <map>
#include <array>

namespace server::tracker
{

//! A race tracker.
class RaceTracker
{
public:
  //! A racer.
  struct Racer
  {
    enum class State
    {
      Disconnected,
      NotReady,
      Ready,
      Loading,
      Racing,
      Finished,
    };

    enum class Team
    {
      Solo, Red, Blue
    };

    Oid oid{InvalidEntityOid};
    State state{State::Disconnected};
    Team team{Team::Solo};
    uint32_t starPointValue{};
    uint32_t jumpComboValue{};
    uint32_t courseTime{};
    std::optional<uint32_t> magicItem{};
    
    // Bolt targeting system
    bool isTargeting{false};
    Oid currentTarget{InvalidEntityOid};
  };

  //! An item
  struct Item
  {
    uint32_t itemId{};
    uint32_t itemType{};
    std::array<float, 3> position{};
  };

  //! An object map.
  using ObjectMap = std::map<data::Uid, Racer>;
  //! An item object map.
  //! Maps itemId -> Item (in the race)
  using ItemObjectMap = std::map<uint16_t, Item>;

  //! Adds a racer for tracking.
  //! @param characterUid Character UID.
  //! @returns A reference to the racer record.
  Racer& AddRacer(data::Uid characterUid);
  //! Removes a racer from tracking.
  //! @param characterUid Character UID.
  void RemoveRacer(data::Uid characterUid);
  //! Returns reference to the racer record.
  //! @returns Racer record.
  [[nodiscard]] Racer& GetRacer(data::Uid characterUid);
  //! Returns a reference to all racer records.
  //! @return Reference to racer records.
  [[nodiscard]] ObjectMap& GetRacers();

  //! Adds an item for tracking.
  //! @returns A reference to the new item record.
  Item& AddItem();
  //! Removes an item from tracking.
  //! @param itemId Item ID.
  void RemoveItem(uint16_t itemId);
  //! Returns reference to the item record.
  //! @param itemId Item ID.
  //! @returns Item record.
  [[nodiscard]] Item& GetItem(uint16_t itemId);
  //! Returns a reference to all item records.
  //! @return Reference to item records.
  [[nodiscard]] ItemObjectMap& GetItems();

private:
  //! The next entity ID.
  Oid _nextObjectId = 1;
  //! Horse entities in the race.
  ObjectMap _racers;

  //! The next item ID.
  uint16_t _nextItemId = 1;
  //! Items in the race
  ItemObjectMap _items;
};

} // namespace server::tracker

#endif // RACETRACKER_HPP
