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

#ifndef MAGICSYSTEM_HPP
#define MAGICSYSTEM_HPP

#include "server/tracker/RaceTracker.hpp"

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/registry/MagicRegistry.hpp>

#include <cstdint>

namespace server::race
{

class MagicSystem
{
public:
  static uint32_t GetMountStatValue(
    const tracker::RaceTracker::Racer::MountStatsSnapshot& stats,
    registry::Magic::MountStat which);

  //! Function to select a random item based on position weights
  static const registry::Magic::SlotInfo& SelectMagicTypeByPosition(
    const registry::MagicRegistry& magicRegistry,
    uint32_t position,
    bool isTeam);

  static const registry::Magic::SlotInfo& RandomMagicItem(
    const registry::MagicRegistry& magicRegistry,
    tracker::RaceTracker& tracker,
    data::Uid racerUid);
};

} // namespace server::race

#endif // MAGICSYSTEM_HPP
