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

#include "server/ServerInstance.hpp"
#include "server/system/MatchmakingSystem.hpp"

namespace server
{

MatchmakingSystem::MatchmakingSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance) {}

std::optional<Room::GameMode> ResolveRoomGameMode(
  const protocol::GameMode gameMode)
{
  // 1:1 mapping of protocol to room gamemode
  switch (gameMode)
  {
    case protocol::GameMode::Speed:
      return Room::GameMode::Speed;
    case protocol::GameMode::Magic:
      return Room::GameMode::Magic;
    case protocol::GameMode::Tutorial:
      return Room::GameMode::Tutorial;
    default:
      return std::nullopt;
  }
}

std::optional<Room::TeamMode> ResolveRoomTeamMode(
  const protocol::TeamMode teamMode)
{
  // 1:1 mapping of protocol to room teammode
  switch (teamMode)
  {
    case protocol::TeamMode::FFA:
      return Room::TeamMode::FFA;
    case protocol::TeamMode::Team:
      return Room::TeamMode::Team;
    case protocol::TeamMode::Single:
      return Room::TeamMode::Single;
    default:
      return std::nullopt;
  }
}

std::optional<data::Uid> MatchmakingSystem::Matchmake(const Entry& entry)
{
  data::Uid selectedRoomUid{data::InvalidUid};
  size_t selectedPlayerCount = 0;

  // Iterate through room snapshots
  const auto& roomSnapshots = _serverInstance.GetRoomSystem().GetRoomsSnapshot();
  for (const auto& roomSnapshot : roomSnapshots)
  {
    const auto& roomDetails = roomSnapshot.details;

    // Ignore locked or playing rooms
    if (not roomDetails.password.empty() or roomSnapshot.isPlaying)
      continue;

    // Check if gamemode + teammode matches
    const bool isGameModeMatch = roomDetails.gameMode == ResolveRoomGameMode(entry.gameMode);
    const bool isTeamModeMatch = roomDetails.teamMode == ResolveRoomTeamMode(entry.teamMode);
    const bool doesModeMatch = isGameModeMatch and isTeamModeMatch;
    if (not doesModeMatch)
      continue;

    // Check if the room's current player count exceeds max player count.
    // In other words, check that there is space for at least one player.
    if (roomSnapshot.playerCount >= roomDetails.maxPlayerCount)
      continue;

    // Best candidate is as follows:
    // - No room selected yet
    // - Player count of this room more than the current selected room
    const bool roomNotFound = selectedRoomUid == data::InvalidUid;
    const bool moreFullThanSeenBefore = roomSnapshot.playerCount > selectedPlayerCount;

    const bool isBestCandidate = roomNotFound or moreFullThanSeenBefore;
    if (isBestCandidate)
    {
      // New best candidate found, set room details
      selectedRoomUid = roomSnapshot.uid;
      selectedPlayerCount = roomSnapshot.playerCount;
    }
  }

  if (selectedRoomUid == data::InvalidUid)
    return std::nullopt;

  return selectedRoomUid;
}

const void MatchmakingSystem::Search(
  const data::Uid characterUid,
  const protocol::GameMode gameMode,
  const protocol::TeamMode teamMode)
{
  // Queue matchmaking
  _serverInstance.GetLobbyDirector().GetScheduler().Queue(
    [this, characterUid, gameMode, teamMode]()
    {
      // Safely get matchmaking entry for the character
      MatchmakingSystem::Entry entry{};
      {
        std::scoped_lock lock(_matchmakingQueueMutex);
        const auto queueIter = _matchmakingQueue.find(characterUid);
        if (queueIter == _matchmakingQueue.cend())
          // Entry for this character does not exist, safely return
          return;
        entry = queueIter->second;
      }

      // Check if matchmaking has expired
      const auto now = std::chrono::steady_clock::now();
      const bool hasMatchmakingExpired = now - entry.queuedAt >= MatchmakingQueueTimeoutMs;
      if (hasMatchmakingExpired)
      {
        // Matchmaking expired, erase this character from the queue
        std::scoped_lock lock(_matchmakingQueueMutex);
        const auto erased = _matchmakingQueue.erase(characterUid);
        if (erased == 0)
          return;

        // Room not found
        const MatchmakingSystem::Result result{
          .verdict = MatchmakingSystem::Result::NoRoom};
        _serverInstance.GetLobbyDirector().NotifyMatchmakeResult(characterUid, result);
        return;
      }

      // Attempt to matchmake
      const auto roomUid = this->Matchmake(entry);
      if (roomUid.has_value())
      {
        // Room found!
        std::scoped_lock lock(_matchmakingQueueMutex);
        const auto erased = _matchmakingQueue.erase(characterUid);
        if (erased == 0)
          return;

        const MatchmakingSystem::Result result{
          .verdict = MatchmakingSystem::Result::FoundRoom,
          .roomUid = roomUid.value()};
        _serverInstance.GetLobbyDirector().NotifyMatchmakeResult(characterUid, result);
        return;
      }

      this->Search(characterUid, gameMode, teamMode);
    },
    Scheduler::Clock::now() + MatchmakingIntervalMs);
}

const bool MatchmakingSystem::Queue(
  const data::Uid characterUid,
  const protocol::GameMode gameMode,
  const protocol::TeamMode teamMode)
{
  const auto [iter, inserted] = _matchmakingQueue.try_emplace(characterUid);

  // Check if matchmaking map already has this character queued
  if (not inserted)
    return false;

  // Add matchmaking details to entry
  iter->second = MatchmakingSystem::Entry{
    .queuedAt = Scheduler::Clock::now(),
    .gameMode = gameMode,
    .teamMode = teamMode};

  // Trigger search timer
  this->Search(characterUid, gameMode, teamMode);
  return true;
}

const bool MatchmakingSystem::Dequeue(const data::Uid characterUid)
{
  // Remove character by character uid and return if character was erased
  // from the matchmaking queue
  std::scoped_lock lock(_matchmakingQueueMutex);
  return _matchmakingQueue.erase(characterUid) != 0;
}

} // namespace server

