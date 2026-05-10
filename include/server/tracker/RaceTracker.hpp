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
#include <libserver/network/command/proto/CommonStructureDefinitions.hpp>

#include <array>
#include <chrono>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace server::tracker
{

//! A race tracker.
class RaceTracker
{
public:
  //! A per-racer event item (e.g. egg) visible only to one racer.
  struct EventItem
  {
    Oid oid{};
    uint32_t itemType{};
    std::array<float, 3> position{};
  };

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

    using Team = protocol::TeamColor;

    struct ItemInstance
    {
      std::chrono::steady_clock::time_point expiryTimePoint;
    };

    Oid oid{InvalidEntityOid};
    State state{State::Disconnected};
    Team team{Team::Solo};
    uint32_t starPointValue{};
    uint32_t jumpComboValue{};
    int32_t courseTime{std::numeric_limits<int32_t>::max()};
    std::optional<uint32_t> magicItem{};

    //! A set of tracked items in racer's proximity.
    std::unordered_set<Oid> trackedItems;
    //! Per-racer event items (e.g. eggs) visible only to this racer.
    std::vector<EventItem> eventItems;

    //! Active skill effects indexed by skillEffectId (0-23).
    static constexpr size_t EffectCount = 24;
    std::array<bool, EffectCount> effects{};
    //! Per-effect generation counter, incremented on each apply, used to invalidate stale removal timers.
    std::array<uint32_t, EffectCount> effectGenerations{};

    //! Rank of the currently active removeMagic attack (0 = none active).
    uint32_t attackRank{};
    std::chrono::steady_clock::time_point dragonReceivedAt{};

    struct MagicTargetInfo
    {
      uint16_t casterOid;
      uint16_t effectInstanceId;
    };
    std::optional<MagicTargetInfo> pendingMagicTarget{};
  };

  //! An item
  struct Item
  {
    Oid oid{};
    std::vector<uint32_t> itemTypes{};
    uint32_t currentType{};
    std::chrono::steady_clock::time_point respawnTimePoint{};
    std::array<float, 3> position{};
  };

  //! An event
  struct Event
  {
    uint32_t id{};
    std::chrono::steady_clock::time_point throttledUntil{};
  };

  struct TeamInfo
  {
    uint32_t points{0};
    uint32_t boostCount{0};
    bool gaugeLocked{false};
  };

  TeamInfo blueTeam{};
  TeamInfo redTeam{};

  //! An object map.
  using RacerObjectMap = std::map<data::Uid, Racer>;
  //! An item object map.
  //! Maps itemId -> Item (in the race)
  using ItemObjectMap = std::map<uint16_t, Item>;
  //! An event map.
  using EventMap = std::unordered_map<uint32_t, Event>;

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
  bool IsRacer(data::Uid characterUid) const;
  //! Returns reference to the racer record.
  //! @returns Racer record.
  [[nodiscard]] Racer& GetRacer(data::Uid characterUid);
  //! Returns a reference to all racer records.
  //! @return Reference to racer records.
  [[nodiscard]] RacerObjectMap& GetRacers();

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
  //! Returns the next object instance ID and increments the internal counter.
  //! @param increment The value to increment the internal counter by.
  //! @returns The next object instance ID before incrementing.
  uint16_t GetNextObstacleInstanceIdAndIncrementBy(uint16_t increment);

  uint16_t GetNextEffectInstanceIdAndIncrementBy(uint16_t increment);

  //! Returns a reference to all of the event records.
  //! @return Reference to event records.
  [[nodiscard]] EventMap& GetEvents();
  //! Checks and throttles an event.
  //! @param eventId Event ID.
  //! @returns True if event exists and is throttled, else event is tracked.
  bool IsEventThrottled(uint32_t eventId);
  static inline const std::chrono::milliseconds ThrottleDurationMs{250};

  //! Adds a per-racer event item for the given character.
  //! @returns Reference to the new event item record.
  EventItem& AddEventItem(data::Uid characterUid);
  //! Finds a per-racer event item by OID.
  //! @returns The OID if found, otherwise InvalidEntityOid.
  Oid FindEventItem(data::Uid characterUid, Oid oid);
  //! Returns reference to a per-racer event item by OID.
  //! @throws std::runtime_error if not found.
  [[nodiscard]] EventItem& GetEventItem(data::Uid characterUid, Oid oid);
  //! Removes a per-racer event item by OID.
  void RemoveEventItem(data::Uid characterUid, Oid oid);

  void Clear();


private:
  //! Mapping between character UIDs and their assigned OIDs.
  //! It's important these persist across races in a room as the client does not clear assignments internally. 
  std::unordered_map<data::Uid, Oid> _characterOids;
  //! Next OID for new character entities (100+).
  Oid _nextCharacterOid = 100;
  //! Next OID for item entities (1–99, reset each race).
  Oid _nextItemOid = 1;
  //! Horse entities in the race.
  RacerObjectMap _racers;
  //! Items in the race
  ItemObjectMap _items;
  //! Tracked race map events.
  EventMap _events;
  //! Next effect instance ID.
  uint16_t _nextEffectInstanceId = 0;
};

} // namespace server::tracker

#endif // RACETRACKER_HPP
