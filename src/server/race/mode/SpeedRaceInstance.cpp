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

#include "server/ServerInstance.hpp"

#include "server/race/mode/SpeedRaceInstance.hpp"

#include <spdlog/spdlog.h>

namespace server
{

SpeedRaceInstance::SpeedRaceInstance(
  ServerInstance& serverInstance,
  CommandServer& commandServer) : RaceInstance(
    serverInstance,
    commandServer,
    protocol::GameMode::Speed)
{}

SpeedRaceInstance::~SpeedRaceInstance() = default;

} // namespace server
