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

#include <array>
#include <chrono>
#include <map>
#include <unordered_set>

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
      Loading,
      Racing,
      Finishing,
    };

    enum class Team
    {
      Solo, Red, Blue
    };

    struct ItemInstance
    {
      std::chrono::steady_clock::time_point expiryTimePoint;
    };

    enum class Shield
    {
      None,
      Normal,
      Critical
    };

    Oid oid{InvalidEntityOid};
    State state{State::Disconnected};
    Team team{Team::Solo};
    uint32_t starPointValue{};
    uint32_t jumpComboValue{};
    uint32_t courseTime{std::numeric_limits<uint32_t>::max()};
    std::optional<uint32_t> magicItem{};

    //! A set of tracked items in racer's proximity.
    std::unordered_set<Oid> trackedItems;

    Shield shield{Shield::None};
    bool darkness{};
    bool hotRodded{};
  };

  //! An item
  struct Item
  {
    Oid oid{};
    uint32_t deckId{};
    std::chrono::steady_clock::time_point respawnTimePoint{};
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
  //! Returns whether the character is a racer.
  //! @param characterUid Character UID.
  //! @return `true` if the character is a racer,
  //!          otherwise returns `false`;
  bool IsRacer(data::Uid characterUid);
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
  //! @param itemId Item OID.
  void RemoveItem(Oid itemId);
  //! Returns reference to the item record.
  //! @param itemId Item OID.
  //! @returns Item record.
  [[nodiscard]] Item& GetItem(Oid itemId);
  //! Returns a reference to all item records.
  //! @return Reference to item records.
  [[nodiscard]] ItemObjectMap& GetItems();

  void Clear();


private:
  //! The next entity OID.
  Oid _nextRacerOid = 1;
  //! Horse entities in the race.
  ObjectMap _racers;

  //! The next item OID.
  uint16_t _nextItemOid = 1;
  //! Items in the race
  ItemObjectMap _items;
};

} // namespace server::tracker

#endif // RACETRACKER_HPP
