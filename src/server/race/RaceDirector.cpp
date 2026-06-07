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

#include "server/race/RaceDirector.hpp"

#include "server/race/RaceNetworkHandler.hpp"
#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

namespace server
{

RaceDirector::RaceDirector(
  ServerInstance& serverInstance)
    : _serverInstance(serverInstance)
    , _networkHandler(new RaceNetworkHandler(serverInstance))
{
}

RaceDirector::~RaceDirector()
{
  delete _networkHandler;
}

void RaceDirector::Initialize()
{
  GetNetworkHandler().Initialize();
}

void RaceDirector::Terminate()
{
  GetNetworkHandler().Terminate();
}

void RaceDirector::Tick()
{
  try
  {
    _scheduler.Tick();
  }
  catch (const std::exception& x)
  {
    spdlog::error("Exception ticking a race scheduler: {}", x.what());
  }
}

void RaceDirector::DisconnectCharacter(const data::Uid characterUid)
{
  GetNetworkHandler().DisconnectCharacter(
    characterUid);
}

void RaceDirector::SummonCharacter(
  const data::Uid characterUid,
  const bool force,
  const std::string& characterName,
  const uint32_t roomUid,
  const uint32_t ranchUid) noexcept
{
  GetNetworkHandler().NotifySummonCharacter(
    characterUid,
    force,
    characterName,
    roomUid,
    ranchUid);
}

void RaceDirector::NotifyRoomNameChanged(const uint32_t roomUid) noexcept
{
  GetNetworkHandler().NotifyRoomNameChanged(roomUid);
}

RaceNetworkHandler& RaceDirector::GetNetworkHandler()
{
  if (_networkHandler == nullptr)
    throw std::runtime_error("Race director does not have a network handler");

  return *_networkHandler;
}

} // namespace server