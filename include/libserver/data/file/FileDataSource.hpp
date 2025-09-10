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

#ifndef FILEDATASOURCE_HPP
#define FILEDATASOURCE_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataSource.hpp>

namespace server
{

class FileDataSource
  : public DataSource
{
public:
  ~FileDataSource() override = default;

  void Initialize(const std::filesystem::path& path);
  void Terminate();

  void RetrieveUser(std::string name, data::User& user) override;
  void StoreUser(std::string name, const data::User& user) override;

  void CreateInfraction(data::Infraction& infraction) override;
  void RetrieveInfraction(data::Uid uid, data::Infraction& infraction) override;
  void StoreInfraction(data::Uid uid, const data::Infraction& infraction) override;
  void DeleteInfraction(data::Uid uid) override;

  void CreateCharacter(data::Character& character) override;
  void RetrieveCharacter(data::Uid uid, data::Character& character) override;
  void StoreCharacter(data::Uid uid, const data::Character& character) override;
  void DeleteCharacter(data::Uid uid) override;

  void CreateHorse(data::Horse& horse) override;
  void RetrieveHorse(data::Uid uid, data::Horse& horse) override;
  void StoreHorse(data::Uid uid, const data::Horse& horse) override;
  void DeleteHorse(data::Uid uid) override;

  void CreateItem(data::Item& item) override;
  void RetrieveItem(data::Uid uid, data::Item& item) override;
  void StoreItem(data::Uid uid, const data::Item& item) override;
  void DeleteItem(data::Uid uid) override;

  void CreateStorageItem(data::StorageItem& storageItem) override;
  void RetrieveStorageItem(data::Uid uid, data::StorageItem& storageItem) override;
  void StoreStorageItem(data::Uid uid, const data::StorageItem& storageItem) override;
  void DeleteStorageItem(data::Uid uid) override;

  void CreateEgg(data::Egg& egg) override;
  void RetrieveEgg(data::Uid uid, data::Egg& egg) override;
  void StoreEgg(data::Uid uid, const data::Egg& egg) override;
  void DeleteEgg(data::Uid uid) override;

  void CreatePet(data::Pet& pet) override;
  void RetrievePet(data::Uid uid, data::Pet& pet) override;
  void StorePet(data::Uid uid, const data::Pet& pet) override;
  void DeletePet(data::Uid uid) override;

  void CreateHousing(data::Housing& housing) override;
  void RetrieveHousing(data::Uid uid, data::Housing& housing) override;
  void StoreHousing(data::Uid uid, const data::Housing& housing) override;
  void DeleteHousing(data::Uid uid) override;

  void CreateGuild(data::Guild& guild) override;
  void RetrieveGuild(data::Uid uid, data::Guild& guild) override;
  void StoreGuild(data::Uid uid, const data::Guild& guild) override;
  void DeleteGuild(data::Uid uid) override;

private:
  //! A root data path.
  std::filesystem::path _dataPath;

  //! A path to the user data files.
  std::filesystem::path _userDataPath;
  //! A path to the infraction data files.
  std::filesystem::path _infractionDataPath;
  //! A path to the character data files.
  std::filesystem::path _characterDataPath;
  //! A path to the horse data files.
  std::filesystem::path _horseDataPath;
  //! A path to the item data files.
  std::filesystem::path _itemDataPath;
  //! A path to the egg data files.
  std::filesystem::path _eggDataPath;
  //! A path to the stored item data files.
  std::filesystem::path _storageItemPath;
  //! A path to the pet data files.
  std::filesystem::path _petDataPath;
  //! A path to the housing data files.
  std::filesystem::path _housingDataPath;
  //! A path to the guild data files.
  std::filesystem::path _guildDataPath;

  //! A path to meta-data file.
  std::filesystem::path _metaFilePath;

  //! Sequential UID for infractions.
  std::atomic_uint32_t _infractionSequentialUid = 0;
  //! Sequential UID for characters.
  std::atomic_uint32_t _characterSequentialUid = 0;
  //! Sequential UID pool for equipment.
  //! Equipment includes items and horses.
  std::atomic_uint32_t _equipmentSequentialUid = 0;
  //! Sequential UID for storage items.
  std::atomic_uint32_t _storageItemSequentialUid = 0;
  //! Sequential UID for eggs.
  std::atomic_uint32_t _eggSequentialUid = 0;
  //! Sequential UID for pets.
  std::atomic_uint32_t _petSequentialUid = 0;
  //! Sequential UID for housing.
  std::atomic_uint32_t _housingSequentialUid = 0;
  //! Sequential UID for guilds.
  std::atomic_uint32_t _guildSequentialId = 0;
};

} // namespace server

#endif // FILEDATASOURCE_HPP
