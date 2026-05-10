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

#ifndef MATCHMAKINGSYSTEM_HPP
#define MATCHMAKINGSYSTEM_HPP

#include "server/system/RoomSystem.hpp"

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/command/proto/CommonStructureDefinitions.hpp>

#include <optional>

namespace server
{

class ServerInstance;

class MatchmakingSystem final
{
public:
  struct Entry
  {
    std::chrono::steady_clock::time_point queuedAt{};
    protocol::GameMode gameMode{};
    protocol::TeamMode teamMode{};
  };

  struct Result
  {
    enum Verdict
    {
      NoRoom,
      MakeRoom,
      FoundRoom
    } verdict{};

    data::Uid roomUid{};
  };

  explicit MatchmakingSystem(ServerInstance& serverInstance);
  ~MatchmakingSystem() = default;

  const bool Queue(
    const data::Uid characterUid,
    const protocol::GameMode gameMode,
    const protocol::TeamMode teamMode);
  const bool Dequeue(const data::Uid characterUid);

private:
  std::optional<data::Uid> Matchmake(const Entry& entry);

  const void Search(
    const data::Uid characterUid,
    const protocol::GameMode gameMode,
    const protocol::TeamMode teamMode);

  std::mutex _matchmakingQueueMutex;
  std::unordered_map<data::Uid, Entry> _matchmakingQueue;
  ServerInstance& _serverInstance;

  static inline const std::chrono::milliseconds MatchmakingIntervalMs{1000};
  static inline const std::chrono::milliseconds MatchmakingQueueTimeoutMs{30000};
};

} // namespace server

#endif // MATCHMAKINGSYSTEM_HPP
