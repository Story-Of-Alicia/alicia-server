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

#include <mutex>
#include <optional>

#include <pqxx/pqxx>

namespace server
{

class PqDataSource
  : public DataSource
{
public:
  ~PqDataSource() override = default;

  //! Establishes the connection to the data source, creating the schema if the
  //! database does not carry it yet.
  //! @param uri Connection URI.
  void Initialize(const std::string& uri);
  //! Closes the connection to the data source.
  void Terminate();

  void CreateUser(data::User& user) override;
  void RetrieveUser(const std::string_view& name, data::User& user) override;
  void StoreUser(const std::string_view& name, const data::User& user) override;
  bool IsUserNameUnique(const std::string_view& name) override;

  void CreateInfraction(data::Infraction& infraction) override;
  void RetrieveInfraction(data::Uid uid, data::Infraction& infraction) override;
  void StoreInfraction(data::Uid uid, const data::Infraction& infraction) override;
  void DeleteInfraction(data::Uid uid) override;

  void CreateCharacter(data::Character& character) override;
  void RetrieveCharacter(data::Uid uid, data::Character& character) override;
  void StoreCharacter(data::Uid uid, const data::Character& character) override;
  void DeleteCharacter(data::Uid uid) override;
  data::Uid RetrieveCharacterUidByName(const std::string_view& name) override;
  bool IsCharacterNameUnique(const std::string_view& name) override;

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
  bool IsGuildNameUnique(const std::string_view& name) override;

  void CreateSettings(data::Settings& settings) override;
  void RetrieveSettings(data::Uid uid, data::Settings& settings) override;
  void StoreSettings(data::Uid uid, const data::Settings& settings) override;
  void DeleteSettings(data::Uid uid) override;

  void CreateDailyQuestGroup(data::DailyQuestGroup& group) override;
  void RetrieveDailyQuestGroup(data::Uid uid, data::DailyQuestGroup& group) override;
  void StoreDailyQuestGroup(data::Uid uid, const data::DailyQuestGroup& group) override;
  void DeleteDailyQuestGroup(data::Uid uid) override;

  void CreateMail(data::Mail& mail) override;
  void RetrieveMail(data::Uid uid, data::Mail& mail) override;
  void StoreMail(data::Uid uid, const data::Mail& mail) override;
  void DeleteMail(data::Uid uid) override;

  void CreateQuest(data::Quest& quest) override;
  void RetrieveQuest(data::Uid uid, data::Quest& quest) override;
  void StoreQuest(data::Uid uid, const data::Quest& quest) override;
  void DeleteQuest(data::Uid uid) override;

  void CreateStallion(data::Stallion& stallion) override;
  void RetrieveStallion(data::Uid uid, data::Stallion& stallion) override;
  void StoreStallion(data::Uid uid, const data::Stallion& stallion) override;
  void DeleteStallion(data::Uid uid) override;
  std::vector<data::Uid> ListRegisteredStallions() override;

  void CreateReward(data::Reward& reward) override;
  void RetrieveReward(data::Uid claimUid, data::Reward& reward) override;
  void StoreReward(data::Uid claimUid, const data::Reward& reward) override;
  void DeleteReward(data::Uid claimUid) override;

private:
  //! Returns the connection, reconnecting it when it was found broken.
  //! Has to be called while holding the connection mutex.
  //! @returns The connection.
  [[nodiscard]] pqxx::connection& Connection();
  //! Applies the schema when the database does not carry it yet.
  void EnsureSchema();

  //! A connection URI, kept for reconnecting.
  std::string _uri;
  //! A mutex to protect the connection.
  std::mutex _connectionMutex;
  //! A connection to the database.
  std::optional<pqxx::connection> _connection;
};

} // namespace server

#endif // PQDATASOURCE_HPP
