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

#ifndef ROOM_HPP
#define ROOM_HPP

#include <libserver/network/NetworkDefinitions.hpp>
#include <libserver/data/DataDefinitions.hpp>

#include <unordered_map>
#include <unordered_set>

namespace server
{

class Room
{
public:
  enum class GameMode
  {
    Speed = 1,
    Magic = 2,
    Guild,
    Tutorial = 6
  };

  enum class TeamMode
  {
    FFA = 1,
    Team = 2,
    Single = 3
  };

  class Player
  {
  public:
    enum class Team
    {
      Solo, Red, Blue
    };

    Player(network::ClientId clientId) : _clientId(clientId) {}

    bool ToggleReady();
    void SetReady(bool ready);
    [[nodiscard]] bool IsReady() const;
    void SetTeam(Team team);
    [[nodiscard]] Team GetTeam() const;
    network::ClientId GetClientId() const;
  
  private:
    bool _isReady = false;
    Team _team = Team::Solo;

    const network::ClientId _clientId;
  };

  struct Details
  {
    std::string name;
    std::string password;
    uint16_t missionId{};
    uint16_t courseId{};
    uint32_t maxPlayerCount{};
    GameMode gameMode{};
    TeamMode teamMode{};
    uint8_t npcDifficulty{};
    uint8_t skillBracket{};
    //! The UID of the room's current master.
    data::Uid masterUid{data::InvalidUid};
  };

  struct Snapshot
  {
    uint32_t uid;
    Details details;
    size_t playerCount;
    bool isPlaying;
  };

  enum PreventStartReason
  {
    None,
    NotAllPlayersReady,
    TeamImbalance
  };

  explicit Room(uint32_t uid);

  [[nodiscard]] bool IsRoomFull() const;
  bool QueuePlayer(data::Uid characterUid);
  bool DequeuePlayer(data::Uid characterUid);
  bool AddPlayer(network::ClientId clientId, data::Uid characterUid);
  void RemovePlayer(data::Uid characterUid);
  [[nodiscard]] Player& GetPlayer(data::Uid characterUid);
  bool HasPlayer(data::Uid characterUid) const;

  PreventStartReason CanRoomStart();
  void SetRoomPlaying(bool isPlaying);

  [[nodiscard]] uint32_t GetUid() const;
  [[nodiscard]] bool IsRoomPlaying() const;
  [[nodiscard]] size_t GetPlayerCount() const;

  [[nodiscard]] Details& GetRoomDetails();
  [[nodiscard]] Snapshot GetRoomSnapshot() const;
  [[nodiscard]] const std::unordered_map<data::Uid, Player>& GetPlayers() const;

private:
  Details _details;
  uint32_t _uid{};
  std::unordered_set<data::Uid> _queuedPlayers;
  std::unordered_map<data::Uid, Player> _players;
  bool _roomIsPlaying{};
};

} // namespace server

#endif // ROOM_HPP
