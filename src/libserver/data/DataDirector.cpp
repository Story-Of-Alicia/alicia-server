//
// Created by maros on 4/7/25.
//

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
    , _itemStorage(
        [&](const auto& key, auto& item)
        {
          _dataSource->RetrieveItem(key, item);
        },
        [&](const auto& key, auto& item)
        {
          _dataSource->StoreItem(key, item);
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
{
  _dataSource = std::make_unique<FileDataSource>();
  _dataSource->Initialize(".");
}

DataDirector::~DataDirector()
{
  _dataSource->Terminate();
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
