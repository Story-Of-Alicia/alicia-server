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
#include "server/Config.hpp"

namespace server
{

//! A class managing a data source.
class DataSource
{
public:
  //! Default destructor.
  virtual ~DataSource() = default;

  //! Retrieves the user from the data source.
  //! @param name Name of the user.
  //! @param user User to retrieve.
  virtual void RetrieveUser(std::string name, data::User& user) = 0;
  //! Stores the user on the data source.
  //! @param name Name of the user.
  //! @param user User to store.
  virtual void StoreUser(std::string name, const data::User& user) = 0;

  //! Creates the infraction in the data source.
  //! @param infraction Infraction to create.
  virtual void CreateInfraction(data::Infraction& infraction) = 0;
  //! Retrieves the infraction from the data source.
  //! @param uid UID of the infraction.
  //! @param infraction Infraction to retrieve.
  virtual void RetrieveInfraction(data::Uid uid, data::Infraction& infraction) = 0;
  //! Stores the infraction on the data source.
  //! @param uid UID of the infraction.
  //! @param infraction Infraction to store.
  virtual void StoreInfraction(data::Uid uid, const data::Infraction& infraction) = 0;
  //! Deletes the infraction from the data source.
  //! @param uid UID of the infraction.
  virtual void DeleteInfraction(data::Uid uid) = 0;

  //! Creates the character in the data source.
  //! @param character Character to create.
  virtual void CreateCharacter(data::Character& character) = 0;
  //! Retrieves the user from the data source.
  //! @param uid UID of the character.
  //! @param character Character to retrieve.
  virtual void RetrieveCharacter(data::Uid uid, data::Character& character) = 0;
  //! Stores the character on the data source.
  //! @param uid UID of the character.
  //! @param character Character to store.
  virtual void StoreCharacter(data::Uid uid, const data::Character& character) = 0;
  //! Deletes the character from the data source.
  //! @param uid UID of the character.
  virtual void DeleteCharacter(data::Uid uid) = 0;

  //! Creates the horse in the data source.
  //! @param horse Horse to create.
  virtual void CreateHorse(data::Horse& horse) = 0;
  //! Retrieves the horse from the data source.
  //! @param uid UID of the horse.
  //! @param horse Horse to retrieve.
  virtual void RetrieveHorse(data::Uid uid, data::Horse& horse) = 0;
  //! Stores the horse on the data source.
  //! @param uid UID of the horse.
  //! @param horse Horse to store.
  virtual void StoreHorse(data::Uid uid, const data::Horse& horse) = 0;
  //! Deletes the horse from the data source.
  //! @param uid UID of the horse.
  virtual void DeleteHorse(data::Uid uid) = 0;

  //! Creates the item in the data source.
  //! @param item Item to create.
  virtual void CreateItem(data::Item& item) = 0;
  //! Retrieves the item from the data source.
  //! @param uid UID of the item.
  //! @param item Item to retrieve.
  virtual void RetrieveItem(data::Uid uid, data::Item& item) = 0;
  //! Stores the item on the data source.
  //! @param uid UID of the item.
  //! @param item Item to store.
  virtual void StoreItem(data::Uid uid, const data::Item& item) = 0;
  //! Deletes the item from the data source.
  //! @param uid UID of the item.
  virtual void DeleteItem(data::Uid uid) = 0;

  //! Creates the storage item in the data source.
  //! @param storageItem Storage item to create.
  virtual void CreateStorageItem(data::StorageItem& storageItem) = 0;
  //! Retrieves the storage item from the data source.
  //! @param uid UID of the storage item.
  //! @param storageItem Storage item to retrieve.
  virtual void RetrieveStorageItem(data::Uid uid, data::StorageItem& storageItem) = 0;
  //! Stores the storage item on the data source.
  //! @param uid UID of the storage item.
  //! @param storageItem Stored item to store.
  virtual void StoreStorageItem(data::Uid uid, const data::StorageItem& storageItem) = 0;
  //! Deletes the storage item from the data source.
  //! @param uid UID of the storage item.
  virtual void DeleteStorageItem(data::Uid uid) = 0;

  //! Creates the egg in the data source.
  //! @param egg Egg to create.
  virtual void CreateEgg(data::Egg& egg) = 0;
  //! Retrieves the egg from the data source.
  //! @param uid UID of the egg.
  //! @param egg Egg to retrieve.
  virtual void RetrieveEgg(data::Uid uid, data::Egg& egg) = 0;
  //! Stores the egg on the data source.
  //! @param uid UID of the egg.
  //! @param egg Egg to store.
  virtual void StoreEgg(data::Uid uid, const data::Egg& egg) = 0;
  //! Deletes the egg from the data source.
  //! @param uid UID of the egg.
  virtual void DeleteEgg(data::Uid uid) = 0;

  //! Creates the pet in the data source.
  //! @param pet Pet to create.
  virtual void CreatePet(data::Pet& pet) = 0;
  //! Retrieves the pet from the data source.
  //! @param uid UID of the pet.
  //! @param pet Pet to retrieve.
  virtual void RetrievePet(data::Uid uid, data::Pet& pet) = 0;
  //! Stores the pet on the data source.
  //! @param uid UID of the pet.
  //! @param pet Pet to store.
  virtual void StorePet(data::Uid uid, const data::Pet& pet) = 0;
  //! Deletes the pet from the data source.
  //! @param uid UID of the pet.
  virtual void DeletePet(data::Uid uid) = 0;

  //! Creates the housing in the data source.
  //! @param housing Housing to create.
  virtual void CreateHousing(data::Housing& housing) = 0;
  //! Retrieves the housing from the data source.
  //! @param uid UID of the housing.
  //! @param housing Housing to retrieve.
  virtual void RetrieveHousing(data::Uid uid, data::Housing& housing) = 0;
  //! Stores the housing on the data source.
  //! @param uid UID of the housing.
  //! @param housing Housing to store.
  virtual void StoreHousing(data::Uid uid, const data::Housing& housing) = 0;
  //! Deletes the housing from the data source.
  //! @param uid UID of the housing.
  virtual void DeleteHousing(data::Uid uid) = 0;

  //! Creates the guild in the data source.
  //! @param guild Guild to create.
  virtual void CreateGuild(data::Guild& guild) = 0;
  //! Retrieves the guild from the data source.
  //! @param uid UID of the guild.
  //! @param guild Guild to retrieve.
  virtual void RetrieveGuild(data::Uid uid, data::Guild& guild) = 0;
  //! Stores the guild on the data source.
  //! @param uid UID of the guild.
  //! @param guild Guild to store.
  virtual void StoreGuild(data::Uid uid, const data::Guild& guild) = 0;
  //! Deletes the guild from the data source.
  //! @param uid UID of the guild.
  virtual void DeleteGuild(data::Uid uid) = 0;
};

} // namespace server

#endif // DATASOURCE_HPP
