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

#ifndef GUILDPARTYSYSTEM_HPP
#define GUILDPARTYSYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <deque>
#include <map>
#include <optional>

namespace server
{

class GuildParty
{
public:
  enum class GameMode : uint32_t
  {
    Speed = 1,
    Magic = 2
  };

  struct Details
  {
    std::string name;
    GameMode gameMode{GameMode::Speed};
    data::Uid guildUid{data::InvalidUid};
    data::Uid leaderUid{data::InvalidUid};
    data::Uid ranchUid{data::InvalidUid};
    static constexpr uint8_t MaxMemberCount = 4;
  };

  struct Member
  {
    data::Uid characterUid{data::InvalidUid};
  };

  struct Snapshot
  {
    uint32_t uid;
    Details details;
    size_t memberCount;
  };

  explicit GuildParty(uint32_t uid);

  bool AddMember(data::Uid characterUid);
  void RemoveMember(data::Uid characterUid);

  [[nodiscard]] bool IsFull() const;
  [[nodiscard]] uint32_t GetUid() const;
  [[nodiscard]] size_t GetMemberCount() const;
  [[nodiscard]] const Details& GetDetails() const;
  [[nodiscard]] Details& GetDetails();
  [[nodiscard]] Snapshot GetSnapshot() const;
  [[nodiscard]] const std::vector<Member>& GetMembers() const;

private:
  void MigrateLeader();

  uint32_t _uid{};
  Details _details;
  std::vector<Member> _members;
};

class GuildPartySystem
{
public:
  void CreateParty(const std::function<void(GuildParty&)>& consumer);
  void GetParty(uint32_t uid, const std::function<void(GuildParty&)>& consumer);
  bool PartyExists(uint32_t uid);
  void DeleteParty(uint32_t uid);

  std::vector<GuildParty::Snapshot> GetPartiesSnapshot();

  void StartMatchmaking(data::Uid partyUid, GuildParty::GameMode gameMode);
  void StopMatchmaking(data::Uid partyUid);
  [[nodiscard]] std::optional<std::pair<data::Uid, data::Uid>> TryMatchmake(GuildParty::GameMode gameMode);

private:
  struct Entry
  {
    GuildParty party;
    std::mutex mutex{};
  };

  uint32_t _sequencedId = 0;
  std::mutex _partiesLock;
  std::unordered_map<uint32_t, Entry> _parties;

  std::mutex _matchmakingLock;
  std::map<GuildParty::GameMode, std::deque<data::Uid>> _matchmakingQueue;
};

} // namespace server

#endif // GUILDPARTYSYSTEM_HPP
