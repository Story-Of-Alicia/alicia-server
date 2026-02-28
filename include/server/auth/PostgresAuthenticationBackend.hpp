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

#ifndef ALICIA_SERVER_POSTGRESAUTHENTICATIONBACKEND_HPP
#define ALICIA_SERVER_POSTGRESAUTHENTICATIONBACKEND_HPP

#include "AuthenticationBackend.hpp"

#include <pqxx/connection>

namespace server
{

class PostgresAuthenticationBackend final
  : public AuthenticationBackend
{
public:
  explicit PostgresAuthenticationBackend(const std::string& connectionUri);
  ~PostgresAuthenticationBackend() override = default;

  std::optional<bool> Authenticate(
    const std::string& userName,
    const std::string& userToken) override;

private:
  std::optional<pqxx::connection> _pqcx;
};

} // namespace server

#endif // ALICIA_SERVER_POSTGRESAUTHENTICATIONBACKEND_HPP
