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

namespace soa
{

class FileDataSource
  : public DataSource
{
public:
  void Initialize(const std::filesystem::path& path);
  void Terminate();

  void RetrieveUser(std::string name, data::User& user) override;
  void StoreUser(std::string name, const data::User& user) override;
  void RetrieveCharacter(data::Uid uid, data::Character& character) override;
  void StoreCharacter(data::Uid uid, const data::Character& character) override;
  void RetrieveItem(data::Uid uid, data::Item& item) override;
  void StoreItem(data::Uid uid, const data::Item& item) override;
  void RetrieveHorse(data::Uid uid, data::Horse& horse) override;
  void StoreHorse(data::Uid uid, const data::Horse& horse) override;
  void RetrieveRanch(data::Uid uid, data::Ranch& ranch) override;
  void StoreRanch(data::Uid uid, const data::Ranch& ranch) override;

private:
  std::filesystem::path _path;
  std::filesystem::path _usersPath;
  std::filesystem::path _charactersPath;
  std::filesystem::path _horsesPath;
  std::filesystem::path _ranchesPath;
  std::filesystem::path _itemsPath;
  std::filesystem::path _metaFilePath;

  uint32_t _sequentialUid = 0;
};

} // namespace soa

#endif // FILEDATASOURCE_HPP
