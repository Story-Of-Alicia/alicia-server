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

#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace server
{

class PqDataSource
  : public DataSource
{
public:
  //! Establishes the connection pool to the data source.
  //! @param url Connection URL of the data source.
  //! @param connectionCount Count of connections in the pool.
  void Establish(const std::string& url, std::size_t connectionCount = 4);
  //! Returns whether the connection is fine.
  //! @returns `true` if the connection is fine, `false` otherwise.
  bool IsConnectionFine();

  ~PqDataSource() override;
  void CreateUser(data::User& user) override;
  std::optional<bool> RetrieveUser(const std::string_view& name, data::User& user) override;
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
  bool ClaimCharacterName(data::Uid uid, const std::string_view& name) override;
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
  //! A connection leased from the pool, returned to the pool on destruction.
  class ConnectionLease
  {
  public:
    ConnectionLease(
      PqDataSource& dataSource,
      std::unique_ptr<pqxx::connection> connection);
    ~ConnectionLease();

    ConnectionLease(const ConnectionLease&) = delete;
    ConnectionLease& operator=(const ConnectionLease&) = delete;
    ConnectionLease(ConnectionLease&&) = delete;
    ConnectionLease& operator=(ConnectionLease&&) = delete;

    pqxx::connection& operator*() const;

  private:
    PqDataSource& _dataSource;
    std::unique_ptr<pqxx::connection> _connection;
  };

  //! Leases a connection from the pool, waiting until one is idle.
  //! A broken connection is re-established before it is leased.
  //! @returns The leased connection.
  ConnectionLease AcquireConnection();
  //! Returns the connection to the pool.
  //! @param connection Connection to return, may be null if it could not be re-established.
  void ReleaseConnection(std::unique_ptr<pqxx::connection> connection);

  //! Connection URL of the data source.
  std::string _url;
  //! A mutex guarding the idle connections.
  std::mutex _poolMutex;
  //! A condition notified when a connection is returned to the pool.
  std::condition_variable _poolCondition;
  //! Idle connections of the pool.
  std::vector<std::unique_ptr<pqxx::connection>> _idleConnections;
};

} // namespace server

#endif // PQDATASOURCE_HPP
