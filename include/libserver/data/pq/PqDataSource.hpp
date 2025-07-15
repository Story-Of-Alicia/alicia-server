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

#ifndef PQDATASOURCE_HPP
#define PQDATASOURCE_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataSource.hpp>

#include <pqxx/pqxx>

namespace soa
{

class PqDataSource
  : public DataSource
{
public:
  //! Establishes the connection to the data source.
  void Establish(const std::string& url);
  //! Returns whether the connection is fine.
  //! @returns `true` if the connection is fine, `false` otherwise.
  bool IsConnectionFine();

  void RetrieveUser(data::User&) override;
  void StoreUser(const data::User&) override;

private:
  std::unique_ptr<pqxx::connection> _connection;
};

} // namespace server

#endif // PQDATASOURCE_HPP
