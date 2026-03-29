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

#include <filesystem>

#include <rfl/json.hpp>
#include <rfl.hpp>

namespace server
{

template <typename K, typename V>
class JsonFileDataInterface
  : public DataInterface<K, V>
{
public:
  ~JsonFileDataInterface() noexcept override = default;

  [[nodiscard]] std::expected<Datum<K, V>, std::runtime_error> Create() noexcept override
  {
    const auto uid = ++_sequentialCounter;

    return {uid, {}};
  }

  [[nodiscard]] std::expected<V, std::runtime_error> Retrieve(
    const K key) noexcept override
  {
    return {{}};
  }

  [[nodiscard]] std::vector<std::expected<V, std::runtime_error>> Retrieve(
    const std::span<K> keys) noexcept override
  {
    return {};
  }

  [[nodiscard]] std::vector<std::expected<V, std::runtime_error>> Retrieve(
    const Datum<K, V>::Predicate predicate,
    const bool justOne) noexcept override
  {
    return {};
  }

  void Store(const Datum<K, V> datum) noexcept override
  {
  }

  void Store(const std::span<Datum<K, V>> data) noexcept override
  {

  }

private:
  std::filesystem::path _dataPath{};
  K _sequentialCounter{};
};

class FileDataSource
  : public DataSource
{
public:
  ~FileDataSource() override = default;

  using UserInterface = JsonFileDataInterface<data::Uid, data::User>;
  using CharacterInterface = JsonFileDataInterface<data::Uid, data::Character>;

  void Initialize(const std::filesystem::path& path);
  void Terminate();

  void SaveMetadata();

  [[nodiscard]] DataInterface<data::Uid, data::User>& GetUserInterface() noexcept override;
  [[nodiscard]] DataInterface<data::Uid, data::Character>& GetCharacterInterface() noexcept override;

private:
  //! A root data path.
  std::filesystem::path _dataPath;
  //! A path to meta-data file.
  std::filesystem::path _metaFilePath;

  UserInterface _userInterface;
  CharacterInterface _characterInterface;
};

} // namespace server

#endif // FILEDATASOURCE_HPP
