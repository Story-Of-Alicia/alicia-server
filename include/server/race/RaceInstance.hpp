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
#include <libserver/registry/CourseRegistry.hpp>

#include <chrono>
#include <functional>
#include <unordered_set>

namespace server
{

class RaceNetworkHandler;
class Room;

class RaceInstance
{
public:
  using Clock = std::chrono::steady_clock;
  
  enum class Stage
  {
    Waiting,
    Loading,
    Racing,
    Finishing,
  };

  struct Parameters
  {
    //! A game mode of the race.
    protocol::GameMode gameMode{};
    //! A team mode of the race.
    protocol::TeamMode teamMode{};
    //! A map block ID of the race.
    registry::MapBlockId mapBlockId{};
    //! A mission ID of the race.
    uint16_t missionId{};
    //! A UID of the master.
    data::Uid masterUid{};
  };

  explicit RaceInstance(
    RaceNetworkHandler& raceDirector,
    uint32_t roomUid);
  ~RaceInstance() = default;

  void GetRoom(const std::function<void(Room&)>& consumer);
  void GetRoom(const std::function<void(const Room&)>& consumer) const;

  bool Start(const Parameters& parameters);
  void Stop();

  void Tick();

  uint32_t GetRoomUid();

  const Parameters& GetParameters() const;

  [[nodiscard]] registry::GameModeId GetGameModeId() const;
  [[nodiscard]] registry::MapBlockId GetMapBlockId() const;

  [[nodiscard]] Clock::time_point GetLoadingStartTimePoint() const noexcept;
  [[nodiscard]] Clock::time_point GetRaceStartTimePoint() const noexcept;

  [[nodiscard]] Stage GetStage() const noexcept;
  [[nodiscard]] Clock::time_point GetStageTimeoutTimePoint() const noexcept;

  [[nodiscard]] tracker::RaceTracker& GetTracker();
  [[nodiscard]] const tracker::RaceTracker& GetTracker() const;

private:
  void TickLoading();
  void TickRacing();
  void TickFinishing();

  void TickMagicGauge();

  void PrepareGameMode();
  void PickRandomMapFromCourse();
  void PrepareMap();

public:
  // todo: this needs to be fixed
  void PickRandomItemFromDeck(tracker::RaceTracker::ItemDeck& deck);

private:
  void PrepareItemDecks();

  const uint32_t _roomUid{};

  //! The race parameters.
  Parameters _parameters;

  registry::GameModeId _gameModeId{};
  registry::Course::GameModeInfo _gameModeInfo;
  registry::MapBlockId _mapBlockId{};
  registry::Course::MapBlockInfo _mapBlockInfo;
  
  //! A time point of when the race started loading.
  Clock::time_point _loadingStartTimePoint{
    Clock::time_point::max()};
  //! A time point of when the race started.
  Clock::time_point _raceStartTimePoint{
    Clock::time_point::max()};

  //! The current stage of the race.
  Stage _stage{Stage::Waiting};
  //! A time point of when the stage timeout occurs.
  Clock::time_point _stageTimeoutTimePoint{
    Clock::time_point::max()};

  //! A race object tracker.
  tracker::RaceTracker _tracker;

  RaceNetworkHandler& _raceNetworkHandler;
};

} // namespace server

#endif // RACEINSTANCE_HPP
