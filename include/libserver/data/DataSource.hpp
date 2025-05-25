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

#ifndef DATASOURCE_HPP
#define DATASOURCE_HPP

#include "libserver/data/DataDefinitions.hpp"
#include "server/Settings.hpp"

namespace soa
{

//! A class managing a data source.
class DataSource
{
public:
  //! Default destructor.
  virtual ~DataSource() = default;

  //! Retrieves the user from the data source.
  //! @param user User to retrieve.
  virtual void RetrieveUser(std::string name, data::User& user) = 0;

  //! Stores the user on the data source.
  //! @param user User to store.
  virtual void StoreUser(std::string name, const data::User& user) = 0;

  //! Retrieves the user from the data source.
  //! @param character Character to retrieve.
  virtual void RetrieveCharacter(data::Uid uid, data::Character& character) = 0;

  //! Stores the character on the data source.
  //! @param character Character to store.
  virtual void StoreCharacter(data::Uid uid, const data::Character& character) = 0;

  //! Retrieves the item from the data source.
  //! @param item Item to retrieve.
  virtual void RetrieveItem(data::Uid uid, data::Item& item) = 0;

  //! Stores the item on the data source.
  //! @param item Item to store.
  virtual void StoreItem(data::Uid uid, const data::Item& item) = 0;

  //! Retrieves the horse from the data source.
  //! @param horse Horse to retrieve.
  virtual void RetrieveHorse(data::Uid uid, data::Horse& horse) = 0;

  //! Stores the horse on the data source.
  //! @param horse Horse to store.
  virtual void StoreHorse(data::Uid uid, const data::Horse& horse) = 0;

  //! Retrieves the ranch from the data source.
  //! @param ranch Ranch to retrieve.
  virtual void RetrieveRanch(data::Uid uid, data::Ranch& ranch) = 0;

  //! Stores the ranch on the data source.
  //! @param ranch Ranch to store.
  virtual void StoreRanch(data::Uid uid, const data::Ranch& ranch) = 0;
};

} // namespace soa

#endif // DATASOURCE_HPP
