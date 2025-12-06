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

#ifndef ROOMREGISTRY_HPP
#define ROOMREGISTRY_HPP

#include <libserver/data/DataDefinitions.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace server
{

class Room
{
public:
  enum class GameMode
  {
    Speed = 1, Magic = 2, Guild, Tutorial = 6
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

    bool ToggleReady();
    void SetReady(bool ready);
    [[nodiscard]] bool IsReady() const;
    void SetTeam(Team team);
    [[nodiscard]] Team GetTeam() const;
  private:
    bool _isReady = false;
    Team _team = Team::Solo;
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
  };

  struct Snapshot
  {
    uint32_t uid;
    Details details;
    size_t playerCount;
    bool isPlaying;
  };

  explicit Room(uint32_t uid);

  [[nodiscard]] bool IsRoomFull() const;
  bool QueuePlayer(data::Uid characterUid);
  bool DequeuePlayer(data::Uid characterUid);
  bool AddPlayer(data::Uid characterUid);
  void RemovePlayer(data::Uid characterUid);
  [[nodiscard]] Player& GetPlayer(data::Uid characterUid);

  void SetRoomPlaying(bool isPlaying);

  [[nodiscard]] uint32_t GetUid() const;
  [[nodiscard]] bool IsRoomPlaying() const;
  [[nodiscard]] size_t GetPlayerCount() const;

  [[nodiscard]] Details& GetRoomDetails();
  [[nodiscard]] Snapshot GetRoomSnapshot() const;
  [[nodiscard]] std::unordered_map<data::Uid, Player>& GetPlayers();

private:
  Details _details;
  uint32_t _uid{};
  std::unordered_set<data::Uid> _queuedPlayers;
  std::unordered_map<data::Uid, Player> _players;
  bool _roomIsPlaying{};
};

class RoomSystem
{
public:
  void CreateRoom(const std::function<void(Room&)>& consumer);
  void GetRoom(uint32_t uid, const std::function<void(Room&)>& consumer);
  bool RoomExists(uint32_t uid);
  void DeleteRoom(uint32_t uid);

  std::vector<Room::Snapshot> GetRoomsSnapshot();

private:
  struct Entry
  {
    Room room;
    std::mutex mutex{};
  };

  uint32_t _sequencedId = 0;
  std::mutex _roomsLock;
  std::unordered_map<uint32_t, Entry> _rooms;
};

} // namespace server

#endif //ROOMREGISTRY_HPP
