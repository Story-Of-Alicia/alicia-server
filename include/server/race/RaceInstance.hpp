/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#ifndef RACEINSTANCE_HPP
#define RACEINSTANCE_HPP

#include "server/tracker/RaceTracker.hpp"

#include <libserver/network/NetworkDefinitions.hpp>

namespace server
{

class RaceInstance
{
private:
  friend class RaceDirector;

  //! A stage of the room.
  enum class Stage
  {
      Waiting,
      Loading,
      Racing,
      Finishing,
  } stage{Stage::Waiting};
  //! A time point of when the stage timeout occurs.
  std::chrono::steady_clock::time_point stageTimeoutTimePoint;
  //! Represents when a room started loading.
  std::chrono::steady_clock::time_point loadingStartTimePoint;
  //! A time point of when the race is actually started (a countdown is finished).
  std::chrono::steady_clock::time_point raceStartTimePoint;

  //! The UID of the room this instance belongs to.
  data::Uid roomUid{};
  // TODO: use RoomSystem instead
  //! A master's character UID.
  data::Uid masterUid{data::InvalidUid};
  //! A race object tracker.
  tracker::RaceTracker tracker;

  //! A game mode of the race.
  protocol::GameMode raceGameMode;
  //! A team mode of the race.
  protocol::TeamMode raceTeamMode;
  //! A map block ID of the race.
  uint16_t raceMapBlockId{};
  //! A mission ID of the race.
  uint16_t raceMissionId{};

  //! A room clients.
  std::unordered_set<network::ClientId> clients;
};

}

#endif // RACEINSTANCE_HPP
