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
    State state{State::NotReady};
    Team team{Team::Solo};
    uint32_t starPointValue{};
    uint32_t jumpComboValue{};
  };

  //! An object map.
  using ObjectMap = std::map<data::Uid, Racer>;

  //! Adds a racer for tracking.
  //! @param characterUid Character UID.
  //! @returns A reference to the racer record.
  Racer AddRacer(data::Uid characterUid);
  //! Removes a racer from tracking.
  //! @param characterUid Character UID.
  void RemoveRacer(data::Uid characterUid);
  //! Returns reference to the racer record.
  //! @returns Racer record.
  [[nodiscard]] Racer& GetRacer(data::Uid characterUid);
  //! Returns a reference to all racer records.
  //! @return Reference to racer records.
  [[nodiscard]] const ObjectMap& GetRacers() const;

private:
  //! The next entity ID.
  Oid _nextObjectId = 1;
  //! Horse entities in the ranch.
  ObjectMap _racers;
};

} // namespace server::tracker

#endif // RACETRACKER_HPP
