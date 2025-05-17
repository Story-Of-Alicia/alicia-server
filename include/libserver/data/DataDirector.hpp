//
// Created by maros on 4/7/25.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "DataDefinitions.hpp"
#include "DataStorage.hpp"

//#include "pq/PqDataSource.hpp"
#include "file/FileDataSource.hpp"

namespace soa
{

class DataDirector
{
public:
  using UserStorage = DataStorage<std::string, data::User>;
  using CharacterStorage = DataStorage<data::Uid, data::Character>;
  using ItemStorage = DataStorage<data::Uid, data::Item>;
  using HorseStorage = DataStorage<data::Uid, data::Horse>;
  using RanchStorage = DataStorage<data::Uid, data::Ranch>;

  //! Default constructor.
  explicit DataDirector();
  ~DataDirector();

  //! Ticks the data director.
  void Tick();

  UserStorage& GetUsers();
  CharacterStorage& GetCharacters();
  ItemStorage& GetItems();
  HorseStorage& GetHorses();
  RanchStorage& GetRanches();

private:
  std::unique_ptr<FileDataSource> _dataSource;

  UserStorage _userStorage;
  CharacterStorage _characterStorage;
  HorseStorage _horseStorage;
  RanchStorage _ranchStorage;
  ItemStorage _itemStorage;
};

}

#endif //DATADIRECTOR_HPP
