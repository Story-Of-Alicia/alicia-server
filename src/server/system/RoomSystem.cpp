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

#include "server/system/RoomSystem.hpp"

#include <cassert>
#include <ranges>
#include <stdexcept>
namespace server
{

bool Room::Player::ToggleReady()
{
  _isReady = not _isReady;
  return _isReady;
}

bool Room::Player::IsReady() const
{
  return _isReady;
}

void Room::Player::SetTeam(Room::Player::Team team)
{
  _team = team;
}

Room::Player::Team Room::Player::GetTeam() const
{
  return _team;
}

Room::Room(uint32_t uid)
  : _uid(uid)
{
}

bool Room::IsRoomFull() const
{
  return _players.size() + _queuedPlayers.size() >= _details.maxPlayerCount;
}

bool Room::QueuePlayer(data::Uid characterUid)
{
  if (IsRoomFull())
    return false;

  _queuedPlayers.emplace(characterUid);
  return true;
}

bool Room::DequeuePlayer(data::Uid characterUid)
{
  return _queuedPlayers.erase(characterUid) != 0;
}

bool Room::AddPlayer(data::Uid characterUid)
{
  if (_players.size() >= _details.maxPlayerCount)
    return false;

  Player player {};
  if (_details.teamMode == TeamMode::Team)
  {
      size_t redTeamCount = 0;
      size_t blueTeamCount = 0;
      for (const auto& [_, player] : _players)
      {
        switch (player.GetTeam())
        {
          case Player::Team::Red:
            ++redTeamCount;
            break;
          case Player::Team::Blue:
            ++blueTeamCount;
            break;
          default:
            break;
        }
      }

      if (redTeamCount > blueTeamCount)
      {
        player.SetTeam(Player::Team::Blue);
      }
      else if (blueTeamCount > redTeamCount)
      {
        player.SetTeam(Player::Team::Red);
      }
      else
      {
        player.SetTeam(
          (std::rand() % 2 == 0) ? Player::Team::Red : Player::Team::Blue);
      }
  }

  _queuedPlayers.erase(characterUid);
  _players.try_emplace(characterUid, player);

  return true;
}

void Room::RemovePlayer(data::Uid characterUid)
{
  _players.erase(characterUid);
}

Room::Player& Room::GetPlayer(data::Uid characterUid)
{
  const auto playerIter = _players.find(characterUid);
  if (playerIter == _players.cend())
    throw std::runtime_error("Room player does not exist");
  return playerIter->second;
}

void Room::SetRoomPlaying(bool state)
{
  _roomIsPlaying = state;
}

uint32_t Room::GetUid() const
{
  return _uid;
}

bool Room::IsRoomPlaying() const
{
  return _roomIsPlaying;
}

size_t Room::GetPlayerCount() const
{
  return _players.size();
}

Room::Details& Room::GetRoomDetails()
{
  return _details;
}

Room::Snapshot Room::GetRoomSnapshot() const
{
  return Snapshot{
    .uid = _uid,
    .details = _details,
    .playerCount = _players.size(),
    .isPlaying = _roomIsPlaying,
  };
}

std::unordered_map<data::Uid, Room::Player>& Room::GetPlayers()
{
  return _players;
}

void RoomSystem::CreateRoom(const std::function<void(Room&)>& consumer)
{
  std::unique_lock roomsLock(_roomsLock);
  const auto roomUid = ++_sequencedId;
  const auto [it, inserted] = _rooms.try_emplace(
    roomUid,
    std::move(Room(roomUid)));
  assert(inserted);

  auto& [room, roomMutex] = it->second;
  roomsLock.unlock();

  std::scoped_lock lock(roomMutex);
  consumer(room);
}

void RoomSystem::GetRoom(const uint32_t uid, const std::function<void(Room&)>& consumer)
{
  std::unique_lock roomsLock(_roomsLock);
  const auto it = _rooms.find(uid);
  if (it == _rooms.end())
    throw std::runtime_error("Room does not exist");

  auto& [room, roomMutex] = it->second;
  roomsLock.unlock();

  std::scoped_lock lock(roomMutex);
  consumer(it->second.room);
}

bool RoomSystem::RoomExists(uint32_t uid)
{
  std::scoped_lock lock(_roomsLock);
  return _rooms.contains(uid);
}

std::vector<Room::Snapshot> RoomSystem::GetRoomsSnapshot()
{
  std::scoped_lock roomsLock(_roomsLock);

  std::vector<Room::Snapshot> rooms;
  for (auto& entry : _rooms)
  {
    std::scoped_lock roomLock(entry.second.mutex);
    rooms.emplace_back(entry.second.room.GetRoomSnapshot());
  }

  return rooms;
}

void RoomSystem::DeleteRoom(uint32_t uid)
{
  const auto it = _rooms.find(uid);
  if (it == _rooms.end())
    throw std::runtime_error("Room does not exist");
  _rooms.erase(it);
}

} // namespace server
