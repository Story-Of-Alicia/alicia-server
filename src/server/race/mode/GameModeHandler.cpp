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

#include "server/race/mode/GameModeHandler.hpp"
#include "server/ServerInstance.hpp"

namespace server::race::mode
{

GameModeHandler::GameModeHandler(RaceDirector& director, RaceDirector::RaceInstance& raceInstance, const protocol::GameMode gameMode)
  : _director(director), _raceInstance(raceInstance), _gameMode(gameMode)
{}

GameModeHandler::~GameModeHandler() = default;

void GameModeHandler::OnRaceUserPos(
  ClientId clientId,
  const protocol::AcCmdUserRaceUpdatePos& command)
{
  const auto& clientContext = _director.GetClientContext(clientId);
  auto& racer = _raceInstance.tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.oid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  for (const auto& [itemOid, item] : _raceInstance.tracker.GetItems())
  {
    const bool canItemRespawn = std::chrono::steady_clock::now() >= item.respawnTimePoint;
    if (not canItemRespawn)
      continue;

    // The distance between the player and the item.
    const auto distanceBetweenPlayerAndItem = std::sqrt(
      std::pow(command.member2[0] - item.position[0], 2) +
      std::pow(command.member2[1] - item.position[1], 2) +
      std::pow(command.member2[2] - item.position[2], 2));

    // A distance of the player from the item before it can be spawned.
    constexpr double ItemSpawnDistanceThreshold = 90.0;

    const bool isItemInPlayerProximity = distanceBetweenPlayerAndItem < ItemSpawnDistanceThreshold;
    const bool isItemAlreadyTracked = racer.trackedItems.contains(itemOid);

    if (isItemAlreadyTracked)
    {
      // If the item is not in the player's proximity anymore
      // then remove it from the tracked items.
      if (not isItemInPlayerProximity)
        racer.trackedItems.erase(itemOid);

      continue;
    }

    // If the item is not in player's proximity do not spawn it.
    if (not isItemInPlayerProximity)
      continue;

    protocol::AcCmdRCCreateItem spawn{
      .itemId = item.oid,
      .itemType = item.currentType,
      .position = item.position,
      .spawnStyle = 0,  // ITEM_SPAWN_STYLE_NONE, fix the item in position
      .spawnerId = 0,
      .sizeLevel = 0};

    racer.trackedItems.insert(item.oid);

    _director.GetCommandServer().QueueCommand<decltype(spawn)>(clientId, [spawn]()
    {
      return spawn;
    });
  }
}

} // namespace server::race::mode