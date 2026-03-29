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

#include <functional>
#include <span>
#include <vector>
#include <expected>

namespace server
{

template <typename K, typename V>
struct Datum
{
  //!
  using Predicate = std::function<bool(const V&)>;

  K key{};
  V value{};
};

//! Data interface.
template <typename K, typename V>
class DataInterface
{
public:
  //! Default destructor.
  virtual ~DataInterface() noexcept = default;

  //!
  [[nodiscard]] virtual std::expected<Datum<K, V>, std::runtime_error> Create() noexcept;
  //!
  [[nodiscard]] virtual std::expected<V, std::runtime_error> Retrieve(
    const K key) noexcept = 0;
  //!
  [[nodiscard]] virtual std::vector<std::expected<V, std::runtime_error>> Retrieve(
    std::span<K> keys) noexcept = 0;
  //!
  [[nodiscard]] virtual std::vector<std::expected<V, std::runtime_error>> Retrieve(
    Datum<K, V>::Predicate predicate, bool justOne = false) noexcept;
  //!
  virtual void Store(Datum<K, V> datum) noexcept = 0;
  //!
  virtual void Store(std::span<Datum<K, V>> data) noexcept = 0;
};

//! A class managing a data source.
class DataSource
{
public:
  //! Default destructor.
  virtual ~DataSource() = default;

  [[nodiscard]] virtual DataInterface<data::Uid, data::User>& GetUserInterface() noexcept = 0;
  [[nodiscard]] virtual DataInterface<data::Uid, data::Character>& GetCharacterInterface() noexcept = 0;
};

} // namespace server

#endif // DATASOURCE_HPP
