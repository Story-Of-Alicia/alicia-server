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

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "DataDefinitions.hpp"
#include "DataStorage.hpp"
// #include "pq/PqDataSource.hpp"
#include "file/FileDataSource.hpp"
#include "libserver/util/Scheduler.hpp"

namespace server
{

class DataDirector
{
public:
  using UserStorage = DataStorage<std::string, data::User>;
  using CharacterStorage = DataStorage<data::Uid, data::Character>;
  using HorseStorage = DataStorage<data::Uid, data::Horse>;
  using ItemStorage = DataStorage<data::Uid, data::Item>;
  using EggStorage = DataStorage<data::Uid, data::Egg>;
  using PetStorage = DataStorage<data::Uid, data::Pet>;
  using StorageItemStorage = DataStorage<data::Uid, data::StorageItem>;
  using HousingStorage = DataStorage<data::Uid, data::Housing>;
  using GuildStorage = DataStorage<data::Uid, data::Guild>;

  //! Default constructor.
  explicit DataDirector(const std::filesystem::path& basePath);
  //! Default destructor.
  ~DataDirector();

  //! Initializes the director.
  void Initialize();
  //!  Terminates the director.
  void Terminate();

  //! Ticks the director.
  void Tick();

  //! Requests a load of user data.
  //! @param userName Name of the user.
  void RequestLoadUserData(const std::string& userName);
  //! Requests a load of character data.
  //! @param userName Name of the user.
  //! @param characterUid UID of the character.
  void RequestLoadCharacterData(const std::string& userName, data::Uid characterUid);

  //! Returns whether the data of a user (either user data or character data) are being loaded.
  //! @param userName name of the user.
  bool AreDataBeingLoaded(const std::string& userName);
  //! Returns whether the data of a user are fully loaded.
  //! @param userName name of the user.
  bool AreUserDataLoaded(const std::string& userName);
  //! Returns whether the data of a character are fully loaded.
  //! @param userName name of the user.
  bool AreCharacterDataLoaded(const std::string& userName);

  [[nodiscard]] Record<data::User> GetUser(const std::string& userName);
  [[nodiscard]] UserStorage& GetUsers();

  //! Gets a character.
  //! @param characterUid UID of the character.
  //! @returns The character record.
  [[nodiscard]] Record<data::Character> GetCharacter(data::Uid characterUid) noexcept;
  [[nodiscard]] Record<data::Character> CreateCharacter() noexcept;
  [[nodiscard]] CharacterStorage& GetCharacters();

  [[nodiscard]] Record<data::Horse> GetHorse(data::Uid horseUid) noexcept;
  [[nodiscard]] Record<data::Horse> CreateHorse() noexcept;
  [[nodiscard]] HorseStorage& GetHorses();

  [[nodiscard]] Record<data::Item> GetItem(data::Uid itemUid) noexcept;
  [[nodiscard]] Record<data::Item> CreateItem() noexcept;
  [[nodiscard]] ItemStorage& GetItems();


  [[nodiscard]] Record<data::StorageItem> GetStorageItem(data::Uid storedItemUid) noexcept;
  [[nodiscard]] Record<data::StorageItem> CreateStorageItem() noexcept;
  [[nodiscard]] StorageItemStorage& GetStorageItem();

  [[nodiscard]] Record<data::Egg> GetEgg(data::Uid eggUid) noexcept;
  [[nodiscard]] Record<data::Egg> CreateEgg() noexcept;
  [[nodiscard]] EggStorage& GetEggs();

  [[nodiscard]] Record<data::Pet> GetPet(data::Uid petUid) noexcept;
  [[nodiscard]] Record<data::Pet> CreatePet() noexcept;
  [[nodiscard]] PetStorage& GetPets();

  [[nodiscard]] Record<data::Guild> GetGuild(data::Uid guildUid) noexcept;
  [[nodiscard]] Record<data::Guild> CreateGuild() noexcept;
  [[nodiscard]] GuildStorage& GetGuilds();

  [[nodiscard]] Record<data::Housing> GetHousing(data::Uid housingUid) noexcept;
  [[nodiscard]] Record<data::Housing> CreateHousing() noexcept;
  [[nodiscard]] HousingStorage& GetHousing();

private:
  //! An underlying data source of the data director.
  std::unique_ptr<FileDataSource> _primaryDataSource;

  Scheduler _scheduler;

  struct UserDataContext
  {
    //! A flag indicating whether a load is in progress.
    std::atomic_bool isBeingLoaded = false;
    //! A flag indicating whether an unload is in progress.
    std::atomic_bool isBeingUnloaded = false;

    //! A flag indicating whether the user data are loaded.
    std::atomic_bool isUserDataLoaded = false;
    //! A flag indicating whether the user character data are loaded.
    std::atomic_bool isCharacterDataLoaded = false;

    std::string debugMessage;
    //! The time point when loading or unloading times out.
    Scheduler::Clock::time_point timeout;
  };
  std::unordered_map<std::string, UserDataContext> _userDataContext;

  void ScheduleUserLoad(UserDataContext& userDataContext, const std::string& userName);
  void ScheduleCharacterLoad(UserDataContext& userDataContext, data::Uid characterUid);

  //! An user storage.
  UserStorage _userStorage;
  //! A character storage.
  CharacterStorage _characterStorage;
  //! A horse storage.
  HorseStorage _horseStorage;
  //! An item storage.
  ItemStorage _itemStorage;
  //! A storage item storage.
  StorageItemStorage _storageItemStorage;
  //! An egg storage.
  EggStorage _eggStorage;
  //! A pet storage.
  PetStorage _petStorage;
  //! A housing storage.
  HousingStorage _housingStorage;
  //! A guild storage.
  GuildStorage _guildStorage;
};

} // namespace server

#endif // DATADIRECTOR_HPP
