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

#include "libserver/data/pq/PqDataSource.hpp"

namespace soa
{

namespace
{

constexpr std::string_view UserTableName = "data.users";

} // namespace

void PqDataSource::Establish(const std::string& url)
{
  _connection = std::make_unique<pqxx::connection>(url);
}

bool PqDataSource::IsConnectionFine()
{
  return _connection and _connection->is_open();
};

void PqDataSource::RetrieveUser(data::User& user)
{
  try
  {
    pqxx::work query(*_connection);

    const auto result = query.exec_params1(
      "select (uid, token, characterUid) "
      "from $1 where name=$2",
      pqxx::params(UserTableName, user.name()));

    if (user.uid.IsModified())
      user.uid = result["uid"].as<data::Uid>(data::InvalidUid);
    if (user.token.IsModified())
      user.token = result["token"].as<std::string>({});
    if (user.characterUid.IsModified())
      user.characterUid = result["characterUid"].as<data::Uid>(data::InvalidUid);
  }
  catch (const std::exception& x)
  {
    throw x;
  }
}

void PqDataSource::StoreUser(const data::User& user)
{
  try
  {
    pqxx::work query(*_connection);

    const auto result = query.exec_params0(
      "insert into $1 (token, characterUid) VALUES ($2, $3)",
      pqxx::params(
        UserTableName,
        user.token(),
        user.characterUid()));
  }
  catch (const std::exception& x)
  {
    throw x;
  }
}

} // namespace soa
