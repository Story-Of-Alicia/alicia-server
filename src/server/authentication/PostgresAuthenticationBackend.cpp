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

#include "server/auth/PostgresAuthenticationBackend.hpp"

#include "spdlog/spdlog.h"

#include <pqxx/transaction>

namespace server
{

namespace
{

constexpr std::string_view GetUserSessionTokenName = "GetUserSessionToken";

} // anon namespace


PostgresAuthenticationBackend::PostgresAuthenticationBackend(
  const std::string& connectionUri)
  : _pqcx(connectionUri)
{
  _pqcx.emplace(connectionUri);

  _pqcx->prepare(
    GetUserSessionTokenName.data(),
    "SELECT token, expires_at FROM sessions WHERE username = $1");
}

std::optional<bool> PostgresAuthenticationBackend::Authenticate(
  const std::string& userName,
  const std::string& userToken)
{
  if (not _pqcx)
    return std::nullopt;

  try
  {
    pqxx::work tx(*_pqcx);
    const auto result = tx.exec(
      pqxx::prepped{GetUserSessionTokenName.data()},
      userName);

    if (result.empty())
      return false;

    const auto row = result.one_row();
    const auto sessionToken = row["token"].as<std::string>();

    return sessionToken == userToken;
  }
  catch (const std::exception& x)
  {
    spdlog::warn("Exception while authentication user: {}", x.what());
    return std::nullopt;
  }
}

} // namespace server