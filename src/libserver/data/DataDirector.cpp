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

#include "libserver/data/DataDirector.hpp"

namespace soa
{

DataDirector::DataDirector()
  : _userStorage(
      [&](const auto& key, auto& user)
      {
        _dataSource->RetrieveUser(key, user);
      },
      [&](const auto& key, auto& user)
      {
        _dataSource->StoreUser(key, user);
      })
  , _characterStorage(
      [&](const auto& key, auto& character)
      {
        _dataSource->RetrieveCharacter(key, character);
      },
      [&](const auto& key, auto& character)
      {
        _dataSource->StoreCharacter(key, character);
      })
  , _horseStorage(
      [&](const auto& key, auto& horse)
      {
        _dataSource->RetrieveHorse(key, horse);
      },
      [&](const auto& key, auto& horse)
      {
        _dataSource->StoreHorse(key, horse);
      })
  , _ranchStorage(
      [&](const auto& key, auto& ranch)
      {
        _dataSource->RetrieveRanch(key, ranch);
      },
      [&](const auto& key, auto& ranch)
      {
        _dataSource->StoreRanch(key, ranch);
      })
  , _itemStorage(
      [&](const auto& key, auto& item)
      {
        _dataSource->RetrieveItem(key, item);
      },
      [&](const auto& key, auto& item)
      {
        _dataSource->StoreItem(key, item);
      })
{
  _dataSource = std::make_unique<FileDataSource>();
  _dataSource->Initialize(".");
}

DataDirector::~DataDirector() { _dataSource->Terminate(); }

void DataDirector::Initialize()
{
}

void DataDirector::Terminate()
{
  _userStorage.Terminate();
  _characterStorage.Terminate();
  _itemStorage.Terminate();
  _horseStorage.Terminate();
  _ranchStorage.Terminate();
}

void DataDirector::Tick()
{
  _userStorage.Tick();
  _characterStorage.Tick();
  _itemStorage.Tick();
  _horseStorage.Tick();
  _ranchStorage.Tick();
}

DataDirector::UserStorage& DataDirector::GetUsers()
{
  return _userStorage;
}

DataDirector::CharacterStorage& DataDirector::GetCharacters()
{
  return _characterStorage;
}

DataDirector::ItemStorage& DataDirector::GetItems()
{
  return _itemStorage;
}

DataDirector::HorseStorage& DataDirector::GetHorses()
{
  return _horseStorage;
}

DataDirector::RanchStorage& DataDirector::GetRanches()
{
  return _ranchStorage;
}

} // namespace soa
