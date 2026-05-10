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

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/NetworkDefinitions.hpp>

#include <chrono>
#include <functional>
#include <unordered_set>

namespace server
{

class RaceDirector;
class Room;

class RaceInstance
{
public:
  struct Parameters
  {
    //! A game mode of the race.
    protocol::GameMode raceGameMode{};
    //! A team mode of the race.
    protocol::TeamMode raceTeamMode{};
    //! A map block ID of the race.
    uint16_t raceMapBlockId{};
    //! A mission ID of the race.
    uint16_t raceMissionId{};

    //! The current stage of the race.
    enum class Stage
    {
      Waiting,
      Loading,
      Racing,
      Finishing,
    } stage{Stage::Waiting};

    //! Represents when a room started loading.
    std::chrono::steady_clock::time_point loadingStartTimePoint{};
    //! A time point of when the race is actually started (a countdown is finished).
    std::chrono::steady_clock::time_point raceStartTimePoint{};
    //! A time point of when the stage timeout occurs.
    std::chrono::steady_clock::time_point stageTimeoutTimePoint{};
  };

  explicit RaceInstance(
    RaceDirector& raceDirector,
    uint32_t roomUid);
  ~RaceInstance() = default;

  uint32_t GetRoomUid();
  Parameters& GetParameters();
  const Parameters& GetParameters() const;
  tracker::RaceTracker& GetTracker();
  const tracker::RaceTracker& GetTracker() const;

  void GetRoom(const std::function<void(Room&)>& consumer);
  void GetRoom(const std::function<void(const Room&)>& consumer) const;

  void Tick();

private:
  void TickLoading();
  void TickRacing();
  void TickFinishing();

  //! The race parameters.
  Parameters _parameters;
  //! A race object tracker.
  tracker::RaceTracker _tracker;

  RaceDirector& _raceDirector;
  const uint32_t _roomUid{};
};

} // namespace server

#endif // RACEINSTANCE_HPP
