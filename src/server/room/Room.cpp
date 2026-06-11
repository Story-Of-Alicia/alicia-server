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

#include "server/room/Room.hpp"

#include <cassert>
#include <ranges>
#include <stdexcept>
#include <functional>

namespace server
{

bool Room::Player::ToggleReady()
{
  _isReady = not _isReady;
  return _isReady;
}

void Room::Player::SetReady(bool ready)
{
  _isReady = ready;
}

bool Room::Player::IsReady() const
{
  return _isReady;
}

void Room::Player::SetTeam(Team team)
{
  _team = team;
}

Room::Player::Team Room::Player::GetTeam() const
{
  return _team;
}

network::ClientId Room::Player::GetClientId() const
{
  return _clientId;
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

bool Room::AddPlayer(network::ClientId clientId, data::Uid characterUid)
{
  if (_players.size() >= _details.maxPlayerCount)
    return false;

  Player player{clientId};
  if (_details.teamMode == TeamMode::Team)
  {
      size_t redTeamCount = 0;
      size_t blueTeamCount = 0;
      for (const auto& roomPlayer : _players | std::views::values)
      {
        switch (roomPlayer.GetTeam())
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
  const auto& [iter, inserted] = _players.try_emplace(characterUid, player);
  if (not inserted)
    throw std::runtime_error("Player already exists");

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

bool Room::HasPlayer(data::Uid characterUid) const
{
  return _players.contains(characterUid);
}

Room::PreventStartReason Room::CanRoomStart()
{
  uint32_t redTeamCount = 0;
  uint32_t blueTeamCount = 0;
  const data::Uid masterUid = this->GetRoomDetails().masterUid;

  for (const auto& [characterUid, player] : this->GetPlayers())
  {
    switch (player.GetTeam())
    {
      case Room::Player::Team::Red:
        redTeamCount++;
        break;
      case Room::Player::Team::Blue:
        blueTeamCount++;
        break;
      default:
        break;
    }

    // todo: observer
    const bool isRoomMaster = characterUid == masterUid;

    // Only count the ready state of player which are not masters.
    if (not isRoomMaster && not player.IsReady())
    {
      return PreventStartReason::NotAllPlayersReady;
    }
  }

  const bool isTeamModeTeams =
    GetRoomDetails().teamMode == Room::TeamMode::Team;
  if (isTeamModeTeams and redTeamCount != blueTeamCount)
    return PreventStartReason::TeamImbalance;

  return Room::PreventStartReason::None;
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

const std::unordered_map<data::Uid, Room::Player>& Room::GetPlayers() const
{
  return _players;
}

} // namespace server
