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

#include "server/auth/LocalAuthenticationBackend.hpp"
#include "server/auth/PostgresAuthenticationBackend.hpp"
#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

namespace server
{

AuthenticationService::AuthenticationService(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void AuthenticationService::Initialize()
{
  const auto& authenticationSettings = _serverInstance.GetSettings().authentication;

  if (authenticationSettings.backend == "postgres")
  {
    try
    {
      _backend = std::make_unique<PostgresAuthenticationBackend>(
        authenticationSettings.postgres.connectionUri);
        spdlog::info("Authentication service is using Postgres backend");
    }
    catch (const std::exception& x)
    {
      spdlog::error("Exception initializing Postgres backend for authentication service: {}", x.what());
    }
  }
  else if (authenticationSettings.backend == "local")
  {
    try
    {
      _backend = std::make_unique<LocalAuthenticationBackend>();
      spdlog::info("Authentication service is using local backend");
    }
    catch (const std::exception& x)
    {
      spdlog::error("Exception initializing local backend for authentication service: {}", x.what());
    }
  }
  else
  {
    spdlog::error("Unknown backend for authentication service: '{}'", authenticationSettings.backend);
  }

  if (not _backend)
    throw std::runtime_error("Authentication service backend is not available");
}

void AuthenticationService::Terminate() noexcept
{
  if (_backend)
    _backend.reset();
}

void AuthenticationService::Tick() noexcept
{
  if (_queue.empty())
    return;

  if (not _backend)
    return;

  std::scoped_lock lock(_queueMutex);

  const Authentication& request = _queue.front();
  const auto result = _backend->Authenticate(request.userName, request.userToken);

  if (not result)
    return;

  {
    std::scoped_lock verdictsLock(_verdictsMutex);
    _verdicts.emplace_back(Verdict{
      .userName = request.userName,
      .isAuthenticated = result.value_or(false)});
  }

  _hasVerdicts.store(true, std::memory_order::release);

  _queue.pop();
}

void AuthenticationService::QueueAuthentication(
  const std::string& userName,
  const std::string& userToken) noexcept
{
  std::scoped_lock lock(_queueMutex);
  _queue.emplace(Authentication{
    .userName = userName,
    .userToken = userToken});
}

bool AuthenticationService::HasAuthenticationVerdicts() noexcept
{
  return _hasVerdicts.load(std::memory_order::acquire);
}

std::vector<AuthenticationService::Verdict>
AuthenticationService::PollAuthenticationVerdicts() noexcept
{
  std::scoped_lock lock(_verdictsMutex);

  if (_verdicts.empty())
    return {};

  const auto results = _verdicts;

  _verdicts.clear();
  _hasVerdicts.store(false, std::memory_order::release);

  return results;
}

} // namespace server