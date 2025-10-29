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
#include "libserver/util/Deferred.hpp"

#include <spdlog/spdlog.h>

namespace server
{

DataDirector::DataDirector(const std::filesystem::path& basePath)
  : _userStorage(
      [&](const auto& key, auto& user)
      {
        try
        {
          _primaryDataSource->RetrieveUser(key, user);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving user '{}' from the primary data source: {}",
            key,
            x.what());
        }

        return false;
      },
      [&](const auto& key, auto& user)
      {
        try
        {
          _primaryDataSource->StoreUser(key, user);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing user '{}' from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        spdlog::error("Invalid delete operation on user '{}' from the primary data source", key);
        return false;
      })
  , _infractionStorage(
    [&](const auto& key, auto& infraction)
    {
      try
      {
        _primaryDataSource->RetrieveInfraction(key, infraction);
        return true;
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Exception retrieving infraction {} from the primary data source: {}", key, x.what());
      }

      return false;
    },
    [&](const auto& key, auto& infraction)
    {
      try
      {
        _primaryDataSource->StoreInfraction(key, infraction);
        return true;
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Exception storing infraction {} on the primary data source: {}", key, x.what());
      }

      return false;
    },
    [&](const auto& key)
    {
      try
      {
        _primaryDataSource->DeleteInfraction(key);
        return true;
      }
      catch (const std::exception& x)
      {
        spdlog::error(
          "Exception deleting infraction {} from the primary data source: {}", key, x.what());
      }
      return false;
    })
  , _characterStorage(
      [&](const auto& key, auto& character)
      {
        try
        {
          _primaryDataSource->RetrieveCharacter(key, character);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving character {} from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key, auto& character)
      {
        try
        {
          _primaryDataSource->StoreCharacter(key, character);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing character {} on the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteCharacter(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting character {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _horseStorage(
      [&](const auto& key, auto& horse)
      {
        try
        {
          _primaryDataSource->RetrieveHorse(key, horse);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving horse {} from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key, auto& horse)
      {
        try
        {
          _primaryDataSource->StoreHorse(key, horse);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing horse {} on the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteHorse(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting horse {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _itemStorage(
      [&](const auto& key, auto& item)
      {
        try
        {
          _primaryDataSource->RetrieveItem(key, item);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving item {} from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key, auto& item)
      {
        try
        {
          _primaryDataSource->StoreItem(key, item);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing item {} on the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteItem(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting item {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _storageItemStorage(
      [&](const auto& key, auto& storedItem)
      {
        try
        {
          _primaryDataSource->RetrieveStorageItem(key, storedItem);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving storage item {} from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key, auto& storedItem)
      {
        try
        {
          _primaryDataSource->StoreStorageItem(key, storedItem);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing storage item {} on the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteStorageItem(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting storage item {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _eggStorage(
      [&](const auto& key, auto& egg)
      {
        try
        {
          _primaryDataSource->RetrieveEgg(key, egg);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving egg {} from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key, auto& egg)
      {
        try
        {
          _primaryDataSource->StoreEgg(key, egg);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing egg {} on the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteEgg(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting egg {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _petStorage(
      [&](const auto& key, auto& pet)
      {
        try
        {
          _primaryDataSource->RetrievePet(key, pet);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving pet {} from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key, auto& pet)
      {
        try
        {
          _primaryDataSource->StorePet(key, pet);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing pet {} on the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeletePet(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting pet {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _housingStorage(
      [&](const auto& key, auto& housing)
      {
        try
        {
          _primaryDataSource->RetrieveHousing(key, housing);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception retrieving housing {} from the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key, auto& housing)
      {
        try
        {
          _primaryDataSource->StoreHousing(key, housing);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception storing housing {} on the primary data source: {}", key, x.what());
        }

        return false;
      },
      [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteHousing(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting housing {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _guildStorage(
     [&](const auto& key, auto& guild)
     {
       try
       {
         _primaryDataSource->RetrieveGuild(key, guild);
         return true;
       }
       catch (const std::exception& x)
       {
         spdlog::error(
           "Exception retrieving guild {} from the primary data source: {}", key, x.what());
       }

       return false;
     },
     [&](const auto& key, auto& guild)
     {
       try
       {
         _primaryDataSource->StoreGuild(key, guild);
         return true;
       }
       catch (const std::exception& x)
       {
         spdlog::error(
           "Exception storing guild {} on the primary data source: {}", key, x.what());
       }

       return false;
     },
     [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteGuild(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting guild {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
  , _stallionStorage(
     [&](const auto& key, auto& stallion)
     {
       try
       {
         _primaryDataSource->RetrieveStallion(key, stallion);
         return true;
       }
       catch (const std::exception& x)
       {
         spdlog::error(
           "Exception retrieving stallion {} from the primary data source: {}", key, x.what());
       }

       return false;
     },
     [&](const auto& key, auto& stallion)
     {
       try
       {
         _primaryDataSource->StoreStallion(key, stallion);
         return true;
       }
       catch (const std::exception& x)
       {
         spdlog::error(
           "Exception storing stallion {} on the primary data source: {}", key, x.what());
       }

       return false;
     },
     [&](const auto& key)
      {
        try
        {
          _primaryDataSource->DeleteStallion(key);
          return true;
        }
        catch (const std::exception& x)
        {
          spdlog::error(
            "Exception deleting stallion {} from the primary data source: {}", key, x.what());
        }
        return false;
      })
{
  _primaryDataSource = std::make_unique<FileDataSource>();
  _primaryDataSource->Initialize(basePath);
}

DataDirector::~DataDirector()
{
}

void DataDirector::Initialize()
{
}

void DataDirector::Terminate()
{
  try
  {
    _userStorage.Terminate();
    _infractionStorage.Terminate();
    _characterStorage.Terminate();
    _horseStorage.Terminate();
    _itemStorage.Terminate();
    _storageItemStorage.Terminate();
    _eggStorage.Terminate();
    _petStorage.Terminate();
    _guildStorage.Terminate();
    _stallionStorage.Terminate();
    _housingStorage.Terminate();
  }
  catch (const std::exception& x)
  {
    spdlog::error("Unhandled in exception while terminating data director: {}", x.what());
  }

  _primaryDataSource->Terminate();
}

void DataDirector::Tick()
{
  try
  {
    _userStorage.Tick();
    _infractionStorage.Tick();
    _characterStorage.Tick();
    _horseStorage.Tick();
    _itemStorage.Tick();
    _storageItemStorage.Tick();
    _eggStorage.Tick();
    _petStorage.Tick();
    _guildStorage.Tick();
    _housingStorage.Tick();
  }
  catch (const std::exception& x)
  {
    spdlog::error("Unhandled in exception ticking the storages in data director: {}", x.what());
  }

  try
  {
    _scheduler.Tick();
  }
  catch (std::exception& x)
  {
    spdlog::error("Unhandled in exception ticking the scheduler in the data director: {}", x.what());
  }
}

void DataDirector::RequestLoadUserData(
  const std::string& userName)
{
  auto& userDataContext = _userDataContext[userName];

  // If the user data are being loaded or are already loaded, prevent the user load.
  if (userDataContext.isBeingLoaded.load(std::memory_order::relaxed) ||
    userDataContext.isUserDataLoaded.load(std::memory_order::relaxed))
  {
    return;
  }

  // Indicate that the user data are being loaded and set the timeout.
  userDataContext.isBeingLoaded.store(true, std::memory_order::relaxed);
  userDataContext.timeout = Scheduler::Clock::now() + std::chrono::seconds(10);

  spdlog::info("Load for data of user '{}' requested", userName);

  // Todo schedule load directly from the data source instead of this partial loading hell.
  ScheduleUserLoad(userDataContext, userName);
}

void DataDirector::RequestLoadCharacterData(
  const std::string& userName,
  data::Uid characterUid)
{
  auto& userDataContext = _userDataContext[userName];

  // If the user data are being loaded or are already loaded, prevent the character load.
  if (userDataContext.isBeingLoaded.load(std::memory_order::relaxed) ||
    userDataContext.isCharacterDataLoaded.load(std::memory_order::relaxed))
  {
    return;
  }

  // Indicate that the user data are being loaded and set the timeout.
  userDataContext.isBeingLoaded.store(true, std::memory_order::relaxed);
  userDataContext.timeout = Scheduler::Clock::now() + std::chrono::seconds(10);

  spdlog::info("Load for character data of user '{}' requested", userName);

  // Todo schedule load directly from the data source instead of this partial loading hell.
  ScheduleCharacterLoad(userDataContext, characterUid);
}

bool DataDirector::AreDataBeingLoaded(const std::string& userName)
{
  const auto& userDataContext = _userDataContext[userName];
  return userDataContext.isBeingLoaded.load(std::memory_order::relaxed);
}

bool DataDirector::AreUserDataLoaded(const std::string& userName)
{
  const auto& userDataContext = _userDataContext[userName];
  return userDataContext.isUserDataLoaded.load(std::memory_order::relaxed);
}

bool DataDirector::AreCharacterDataLoaded(const std::string& userName)
{
  const auto& userDataContext = _userDataContext[userName];
  return userDataContext.isCharacterDataLoaded.load(std::memory_order::relaxed);
}

Record<data::User> DataDirector::GetUser(const std::string& userName)
{
  return _userStorage.Get(userName).value_or(Record<data::User>{});
}

DataDirector::UserStorage& DataDirector::GetUserCache()
{
  return _userStorage;
}

Record<data::Character> DataDirector::GetCharacter(data::Uid characterUid) noexcept
{
  if (characterUid == data::InvalidUid)
    return {};
  return _characterStorage.Get(characterUid).value_or(Record<data::Character>{});
}

Record<data::Character> DataDirector::CreateCharacter() noexcept
{
  return _characterStorage.Create(
    [this]()
    {
      data::Character character;
      _primaryDataSource->CreateCharacter(character);

      return std::make_pair(character.uid(), std::move(character));
    });
}

DataDirector::CharacterStorage& DataDirector::GetCharacterCache()
{
  return _characterStorage;
}

Record<data::Infraction> DataDirector::CreateInfraction() noexcept
{
  return _infractionStorage.Create(
    [this]()
    {
      data::Infraction infraction;
      _primaryDataSource->CreateInfraction(infraction);

      return std::make_pair(infraction.uid(), std::move(infraction));
    });
}

DataDirector::InfractionStorage& DataDirector::GetInfractionCache()
{
  return _infractionStorage;
}

Record<data::Horse> DataDirector::GetHorse(data::Uid horseUid) noexcept
{
  if (horseUid == data::InvalidUid)
    return {};
  return _horseStorage.Get(horseUid).value_or(Record<data::Horse>{});
}

Record<data::Horse> DataDirector::CreateHorse() noexcept
{
  return _horseStorage.Create(
    [this]()
    {
      data::Horse horse;
      _primaryDataSource->CreateHorse(horse);

      return std::make_pair(horse.uid(), std::move(horse));
    });
}

DataDirector::HorseStorage& DataDirector::GetHorseCache()
{
  return _horseStorage;
}

Record<data::Item> DataDirector::GetItem(data::Uid itemUid) noexcept
{
  if (itemUid == data::InvalidUid)
    return {};
  return _itemStorage.Get(itemUid).value_or(Record<data::Item>{});
}

Record<data::Item> DataDirector::CreateItem() noexcept
{
  return _itemStorage.Create(
    [this]()
    {
      data::Item item;
      _primaryDataSource->CreateItem(item);

      return std::make_pair(item.uid(), std::move(item));
    });
}

DataDirector::ItemStorage& DataDirector::GetItemCache()
{
  return _itemStorage;
}

Record<data::StorageItem> DataDirector::GetStorageItemCache(data::Uid storedItemUid) noexcept
{
  if (storedItemUid == data::InvalidUid)
    return {};
  return _storageItemStorage.Get(storedItemUid).value_or(Record<data::StorageItem>{});
}

Record<data::StorageItem> DataDirector::CreateStorageItem() noexcept
{
  return _storageItemStorage.Create(
    [this]()
    {
      data::StorageItem item;
      _primaryDataSource->CreateStorageItem(item);

      return std::make_pair(item.uid(), std::move(item));
    });
}

DataDirector::StorageItemStorage& DataDirector::GetStorageItemCache()
{
  return _storageItemStorage;
}

Record<data::Egg> DataDirector::GetEgg(data::Uid eggUid) noexcept
{
  if (eggUid == data::InvalidUid)
    return {};
  return _eggStorage.Get(eggUid).value_or(Record<data::Egg>{});
}

Record<data::Egg> DataDirector::CreateEgg() noexcept
{
  return _eggStorage.Create(
    [this]()
    {
      data::Egg egg;
      _primaryDataSource->CreateEgg(egg);

      return std::make_pair(egg.uid(), std::move(egg));
    });
}

DataDirector::EggStorage& DataDirector::GetEggCache()
{
  return _eggStorage;
}

Record<data::Pet> DataDirector::GetPet(data::Uid petUid) noexcept
{
  if (petUid == data::InvalidUid)
    return {};
  return _petStorage.Get(petUid).value_or(Record<data::Pet>{});
}

Record<data::Pet> DataDirector::CreatePet() noexcept
{
  return _petStorage.Create(
    [this]()
    {
      data::Pet pet;
      _primaryDataSource->CreatePet(pet);

      return std::make_pair(pet.uid(), std::move(pet));
    });
}

DataDirector::PetStorage& DataDirector::GetPetCache()
{
  return _petStorage;
}

Record<data::Housing> DataDirector::GetHousingCache(data::Uid housingUid) noexcept
{
  if (housingUid == data::InvalidUid)
    return {};
  return _housingStorage.Get(housingUid).value_or(Record<data::Housing>{});
}

Record<data::Housing> DataDirector::CreateHousing() noexcept
{
  return _housingStorage.Create(
    [this]()
    {
      data::Housing housing;
      _primaryDataSource->CreateHousing(housing);

      return std::make_pair(housing.uid(), std::move(housing));
    });
}

DataDirector::HousingStorage& DataDirector::GetHousingCache()
{
  return _housingStorage;
}

Record<data::Stallion> DataDirector::GetStallion(data::Uid stallionUid) noexcept
{
  if (stallionUid == data::InvalidUid)
    return {};
  return _stallionStorage.Get(stallionUid).value_or(Record<data::Stallion>{});
}

Record<data::Stallion> DataDirector::CreateStallion() noexcept
{
  return _stallionStorage.Create(
    [this]()
    {
      data::Stallion stallion;
      _primaryDataSource->CreateStallion(stallion);

      return std::make_pair(stallion.uid(), std::move(stallion));
    });
}

DataDirector::StallionStorage& DataDirector::GetStallionCache()
{
  return _stallionStorage;
}

std::vector<data::Uid> DataDirector::ListRegisteredStallions()
{
  return _primaryDataSource->ListRegisteredStallions();
}

void DataDirector::ScheduleUserLoad(
  UserDataContext& userDataContext,
  const std::string& userName)
{
  _scheduler.Queue([this, &userDataContext, userName]()
  {
    const Deferred deferred([this, &userDataContext, userName]()
    {
      // If the user is completely loaded we can return.
      if (userDataContext.isUserDataLoaded.load(std::memory_order::relaxed))
      {
        userDataContext.isBeingLoaded.store(false, std::memory_order::relaxed);
        return;
      }

      // If the timeout is reached we should return and warn about the timeout.
      if (Scheduler::Clock::now() > userDataContext.timeout)
      {
        spdlog::warn("Timeout reached loading data for user '{}': {}", userName, userDataContext.debugMessage);
        userDataContext.isBeingLoaded.store(false, std::memory_order::relaxed);
        return;
      }

      ScheduleUserLoad(userDataContext, userName);
    });

    const auto userRecord = GetUser(userName);
    if (not userRecord)
    {
      userDataContext.debugMessage = std::format(
        "User {} is not available", userName);
      return;
    }

    std::vector<data::Uid> infractions;
    userRecord.Immutable([&infractions](const data::User& user)
    {
      infractions = user.infractions();
    });

    const auto infractionRecords = GetInfractionCache().Get(infractions);
    if (not infractionRecords)
    {
      userDataContext.debugMessage = std::format(
        "Infractions are not available");
      return;
    }

    userDataContext.isUserDataLoaded.store(true, std::memory_order::relaxed);
  });
}

Record<data::Guild> DataDirector::GetGuild(data::Uid guildUid) noexcept
{
  if (guildUid == data::InvalidUid)
    return {};
  return _guildStorage.Get(guildUid).value_or(Record<data::Guild>{});
}

Record<data::Guild> DataDirector::CreateGuild() noexcept
{
  return _guildStorage.Create(
    [this]()
    {
      data::Guild guild;
      _primaryDataSource->CreateGuild(guild);

      return std::make_pair(guild.uid(), std::move(guild));
    });
}

DataDirector::GuildStorage& DataDirector::GetGuildCache()
{
  return _guildStorage;
}

void DataDirector::ScheduleCharacterLoad(
  UserDataContext& userDataContext,
  data::Uid characterUid)
{
  _scheduler.Queue([this, &userDataContext, characterUid]()
  {
    const Deferred deferred([this, &userDataContext, characterUid]()
    {
      // If the character is completely loaded we can return.
      if (userDataContext.isCharacterDataLoaded.load(std::memory_order::relaxed))
      {
        userDataContext.isBeingLoaded.store(false, std::memory_order::relaxed);
        return;
      }

      // If the timeout is reached we should return and warn about the timeout.
      if (Scheduler::Clock::now() > userDataContext.timeout)
      {
        spdlog::warn("Timeout reached loading data for character '{}'", characterUid);
        userDataContext.isBeingLoaded.store(false, std::memory_order::relaxed);
        return;
      }

      ScheduleCharacterLoad(userDataContext, characterUid);
    });

    const auto characterRecord = GetCharacter(characterUid);

    if (not characterRecord)
    {
      userDataContext.debugMessage = std::format(
        "Character '{}' not available",
        characterUid);
      return;
    }

    auto guildUid = data::InvalidUid;
    auto petUid = data::InvalidUid;

    std::vector<data::Uid> gifts;
    std::vector<data::Uid> purchases;
    std::vector<data::Uid> items;

    std::vector<data::Uid> horses;

    std::vector<data::Uid> eggs;

    std::vector<data::Uid> housing;

    std::vector<data::Uid> pets;

    characterRecord.Immutable(
      [&guildUid, &petUid, &gifts, &items, &purchases, &horses, &eggs, &housing, &pets](
        const data::Character& character)
      {
        guildUid = character.guildUid();
        petUid = character.petUid();

        gifts = character.gifts();
        purchases = character.purchases();

        std::ranges::copy(character.inventory(), std::back_inserter(items));
        std::ranges::copy(character.characterEquipment(), std::back_inserter(items));
        std::ranges::copy(character.mountEquipment(), std::back_inserter(items));

        horses = character.horses();

        eggs = character.eggs();

        housing = character.housing();

        pets = character.pets();

        // Add the mount to the horses list,
        // so that it is loaded with all the horses.
        horses.emplace_back(character.mountUid());
      });

    const auto guildRecord = GetGuild(guildUid);
    const auto petRecord = GetPet(petUid);

    const auto giftRecords = GetStorageItemCache().Get(gifts);
    const auto purchaseRecords = GetStorageItemCache().Get(purchases);

    const auto horseRecords = GetHorseCache().Get(horses);

    // Queue all ancestors (including grandparents) for loading to support family tree feature
    if (horseRecords)
    {
      std::vector<data::Uid> allAncestors;
      
      // Collect direct parents (first generation)
      for (const auto& horseRecord : *horseRecords)
      {
        horseRecord.Immutable([&allAncestors](const data::Horse& horse)
        {
          const auto& ancestors = horse.ancestors();
          for (const auto ancestorUid : ancestors)
          {
            allAncestors.push_back(ancestorUid);
          }
        });
      }
    
      if (!allAncestors.empty())
      {
        // Load first generation ancestors (parents)
        std::vector<data::Uid> firstGeneration = allAncestors;
        std::vector<data::Uid> loadedFirstGen;
        
        for (const auto ancestorUid : firstGeneration)
        {
          // Load ancestor and check if available by attempting to access it
          GetHorse(ancestorUid).Immutable([&loadedFirstGen, ancestorUid](const data::Horse& horse)
          {
            loadedFirstGen.push_back(ancestorUid);
          });
        }
        
        // Collect second generation ancestors (grandparents)
        std::vector<data::Uid> secondGeneration;
        for (const auto ancestorUid : loadedFirstGen)
        {
          const auto ancestorRecord = GetHorseCache().Get(ancestorUid);
          if (ancestorRecord)
          {
            ancestorRecord->Immutable([&secondGeneration](const data::Horse& horse)
            {
              const auto& grandparents = horse.ancestors();
              for (const auto grandparentUid : grandparents)
              {
                secondGeneration.push_back(grandparentUid);
              }
            });
          }
        }
        
        // Load second generation ancestors (grandparents)
        for (const auto grandparentUid : secondGeneration)
        {
          GetHorse(grandparentUid).Immutable([](const data::Horse&) {
          });
        }
      }
    }

    const auto eggRecords = GetEggCache().Get(eggs);

    const auto housingRecords = GetHousingCache().Get(housing);

    const auto petRecords = GetPetCache().Get(pets);

    // Only require guild if the UID is not invalid.
    if (not guildRecord && guildUid != data::InvalidUid)
    {
      userDataContext.debugMessage = std::format(
        "Guild '{}' not available", guildUid);
      return;
    }

    // Only require pet if the UID is not invalid.
    if (not petRecord && petUid != data::InvalidUid)
    {
      userDataContext.debugMessage = std::format(
        "Pet '{}' not available", petUid);
      return;
    }

    // Require gifts and purchases for the storage and items for the inventory.
    if (not giftRecords || not purchaseRecords)
    {
      userDataContext.debugMessage = std::format(
        "Gifts or purchases not available");
      return;
    }

    // Add items referenced by the purchase records to the item list.
    for (const auto& purchaseRecord : *purchaseRecords)
    {
      purchaseRecord.Immutable([&items](const data::StorageItem& storageItem)
      {
        for (const auto& itemUid : storageItem.items())
        {
          items.emplace_back(itemUid);
        }
      });
    }

    // Add items referenced by the gift records to the item list.
    for (const auto& giftRecord : *giftRecords)
    {
      giftRecord.Immutable([&items](const data::StorageItem& storageItem)
      {
        for (const auto& itemUid : storageItem.items())
        {
          items.emplace_back(itemUid);
        }
      });
    }

    const auto itemRecords = GetItemCache().Get(items);
    if (not itemRecords)
    {
      userDataContext.debugMessage = std::format(
        "Items not available");
      return;
    }

    // Require the horse records and the current mount record.
    if (not horseRecords)
    {
      userDataContext.debugMessage = std::format(
        "Horses or mount not available");
      return;
    }


    // Require housing records.
    if (not housingRecords)
    {
      userDataContext.debugMessage = std::format(
        "Housing not available");
      return;
    }

    // Require pet records.
    if (not petRecords)
    {
      userDataContext.debugMessage = std::format(
        "Pets not available");
      return;
    }
    
    if (not eggRecords)
    {
      userDataContext.debugMessage = std::format(
        "Eggs not available");
      return;
    }

    userDataContext.isCharacterDataLoaded.store(true, std::memory_order::release);
  });
}

} // namespace server
