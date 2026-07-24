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

#ifndef HORSESYSTEM_HPP
#define HORSESYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>

#include <chrono>
#include <unordered_map>

namespace server
{

class ServerInstance;

//! Runtime horse-lifecycle operations, such as maturing foals into adults.
class HorseSystem
{
public:
  explicit HorseSystem(ServerInstance& serverInstance);

  //! The duration a foal must age before it matures into an adult horse.
  static constexpr std::chrono::hours FoalGrowUpDuration{1};

  //! @param characterUid UID of the owning character.
  //! @returns The still-maturing foals mapped to the time each becomes an adult.
  std::unordered_map<data::Uid, data::Clock::time_point> PromoteMaturedFoals(
    data::Uid characterUid);

private:
  ServerInstance& _serverInstance;
};

} // namespace server

#endif // HORSESYSTEM_HPP
