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

#ifndef MAGIC_GAMEMODE_HANDLER_HPP
#define MAGIC_GAMEMODE_HANDLER_HPP

#include "server/race/mode/GameModeHandler.hpp"
#include "server/ServerInstance.hpp"

namespace server::race::mode
{

class MagicGameMode : public GameModeHandler
{
private:
  const registry::MagicRegistry& _magicRegistry = _director.GetServerInstance().GetMagicRegistry();
  // Store reference to item pool based on race team mode
  const std::vector<uint32_t>& _itemPool = _raceInstance.raceTeamMode == protocol::TeamMode::Team
    ? _magicRegistry.GetTeamPool()
    : _magicRegistry.GetSoloPool();

public:
  explicit MagicGameMode(RaceDirector& director, RaceDirector::RaceInstance& raceInstance);
  ~MagicGameMode();

  const server::registry::Magic::SlotInfo RandomMagicItem(tracker::RaceTracker::Racer& racer) const;

  void OnHurdleClear(
    ClientId clientId,
    const protocol::AcCmdCRHurdleClearResult& command) override;

  void OnRaceUserPos(
    ClientId clientId,
    const protocol::AcCmdUserRaceUpdatePos& command) override;

  void OnItemGet(
    ClientId clientId,
    const protocol::AcCmdUserRaceItemGet& command) override;

  void OnRequestSpur(
    ClientId clientId,
    const protocol::AcCmdCRRequestSpur& command) override;

  void OnStartingRate(
    ClientId clientId,
    const protocol::AcCmdCRStartingRate& command) override;

  void OnUseMagicItem(
    ClientId clientId,
    const protocol::AcCmdCRUseMagicItem& command) override;

  void OnRequestMagicItem(
    ClientId clientId,
    const protocol::AcCmdCRRequestMagicItem& command) override;
};

} // namespace server::race::mode

#endif // MAGIC_GAMEMODE_HANDLER_HPP
