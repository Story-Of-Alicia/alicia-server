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

#include "server/auth/AuthenticationService.hpp"

#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

namespace server
{

AuthenticationService::AuthenticationService(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void AuthenticationService::Initialize() noexcept
{
  try
  {
    _pqcx = pqxx::connection(
      _serverInstance.GetSettings().authentication.connectionUri);
  }
  catch (const std::exception& x)
  {
    spdlog::error("Exception wile connecting to the Postgres backend: {}", x.what());
  }
}

void AuthenticationService::Terminate() noexcept
{
  _pqcx.close();
}

void AuthenticationService::Tick() noexcept
{
  if (_queue.empty())
    return;

 [[maybe_unused]] const Authentication& request = _queue.front();


  _queue.pop();
}

void AuthenticationService::QueueAuthentication(
  const std::string& userName,
  const std::string& userToken) noexcept
{
  _queue.emplace(Authentication{
    .userName = userName,
    .userToken = userToken});
}

std::vector<AuthenticationService::Verdict>
AuthenticationService::PollAuthentications() noexcept
{
  std::scoped_lock lock(_verdictsMutex);

  const auto results = _verdicts;
  _verdicts.clear();

  return results;
}

} // namespace server