//
// Created for Alicia Server
//

#include "server/ranch/BreedingMarket.hpp"
#include "server/ServerInstance.hpp"

#include <libserver/data/DataDirector.hpp>
#include <libserver/util/Deferred.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <exception>

namespace server
{

namespace
{

// Breeding grade restrictions
// TODO: Make these configurable via server config
constexpr uint8_t MinBreedingGrade = 4;
constexpr uint8_t MaxBreedingGrade = 8;

struct ChargeRange
{
  uint32_t min;
  uint32_t max;
};

std::optional<ChargeRange> GetChargeRangeForGrade(uint8_t grade)
{
  switch (grade)
  {
    case 4: return ChargeRange{4000u, 12000u};
    case 5: return ChargeRange{5000u, 15000u};
    case 6: return ChargeRange{6000u, 18000u};
    case 7: return ChargeRange{8000u, 24000u};
    case 8: return ChargeRange{10000u, 40000u};
    default: return std::nullopt;
  }
}

} // anon namespace


BreedingMarket::BreedingMarket(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void BreedingMarket::Initialize()
{
  // Get all registered stallion UIDs from the data source
  _stallionUidsToLoad = _serverInstance.GetDataDirector().ListRegisteredStallions();


  if (_stallionUidsToLoad.empty())
  {
    _stallionsLoaded = true;
    return;
  }


  // Request async loading of stallion records (individual calls trigger loading)
  for (data::Uid stallionUid : _stallionUidsToLoad)
  {
    _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
  }

  _stallionsLoaded = false;
}

void BreedingMarket::Terminate()
{
  _registeredStallions.clear();
  _horseToStallionMap.clear();
  _stallionDataCache.clear();
  _stallionsLoaded = false;
  _stallionUidsToLoad.clear();
}

void BreedingMarket::Tick()
{
  std::lock_guard<std::mutex> lock(_mutex);

  // Load stallions from database
  if (!_stallionsLoaded && !_stallionUidsToLoad.empty())
  {
    int loadedCount = 0;
    std::vector<data::Uid> stillPending;

    struct LoadedStallion
    {
      data::Uid stallionUid;
      StallionData cachedData;
      uint32_t timesMated;
      bool isValid;
    };
    std::vector<LoadedStallion> loadedStallions;

    for (data::Uid stallionUid : _stallionUidsToLoad)
    {
      auto stallionRecordOpt = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
      if (!stallionRecordOpt)
      {
        stillPending.push_back(stallionUid);
        continue;
      }

      LoadedStallion loaded{};
      loaded.stallionUid = stallionUid;

      stallionRecordOpt->Immutable([&](const data::Stallion& stallion)
      {
        loaded.cachedData.stallionUid = stallion.uid();
        loaded.cachedData.horseUid = stallion.horseUid();
        loaded.cachedData.ownerUid = stallion.ownerUid();
        loaded.cachedData.breedingCharge = stallion.breedingCharge();
        loaded.cachedData.registeredAt = stallion.registeredAt();
        loaded.timesMated = stallion.timesMated();

        const auto expiresAt = loaded.cachedData.registeredAt + std::chrono::hours(24);
        loaded.isValid = util::Clock::now() < expiresAt;
      });

      loadedStallions.push_back(std::move(loaded));
    }

    for (const auto& loaded : loadedStallions)
    {
      if (!loaded.isValid)
        continue;

      ScheduleHorseTypeSet(loaded.cachedData.horseUid, 2);

      _stallionDataCache[loaded.stallionUid] = loaded.cachedData;
      _registeredStallions.emplace_back(loaded.cachedData.horseUid);
      _horseToStallionMap[loaded.cachedData.horseUid] = loaded.stallionUid;
      loadedCount++;
    }

    for (const auto& loaded : loadedStallions)
    {
      if (loaded.isValid)
        continue;

      uint32_t earnings = loaded.cachedData.breedingCharge * loaded.timesMated;
      PayOwner(loaded.cachedData.ownerUid, earnings);

      _serverInstance.GetDataDirector().GetStallionCache().Delete(loaded.stallionUid);

      if (!_horseToStallionMap.contains(loaded.cachedData.horseUid))
        ScheduleHorseTypeSet(loaded.cachedData.horseUid, 0);
    }

    // Update the queue with only pending stallions
    _stallionUidsToLoad = std::move(stillPending);

    // If all stallions are loaded, mark as complete
    if (_stallionUidsToLoad.empty())
    {
      _stallionsLoaded = true;
      spdlog::info("Loaded {} stallion(s) from the data source", loadedCount);
    }
    else
    {
    }
  }

  // Check for expired stallions every tick
  if (_stallionsLoaded)
  {
    CheckExpiredStallions();
  }

}

void BreedingMarket::CheckExpiredStallions()
{
  // This is called from Tick() which already holds the mutex
  const auto now = util::Clock::now();
  std::vector<data::Uid> expiredHorseUids;

  for (auto it = _stallionDataCache.begin(); it != _stallionDataCache.end(); )
  {
    const data::Uid stallionUid = it->first;
    const StallionData& stallionData = it->second;

    auto expiresAt = stallionData.registeredAt + std::chrono::hours(24);
    if (now >= expiresAt)
    {

      // Get timesMated before deleting the record
      uint32_t timesMated = 0;
      auto stallionRecordOpt = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
      if (stallionRecordOpt)
      {
        stallionRecordOpt->Immutable([&timesMated](const data::Stallion& stallion)
        {
          timesMated = stallion.timesMated();
        });
      }

      // Calculate and pay owner their earnings
      uint32_t earnings = stallionData.breedingCharge * timesMated;
      PayOwner(stallionData.ownerUid, earnings);

      // Track for removal from tracking lists
      expiredHorseUids.push_back(stallionData.horseUid);

      // Delete stallion record from database
      _serverInstance.GetDataDirector().GetStallionCache().Delete(stallionUid);

      // Erase from cache
      it = _stallionDataCache.erase(it);
    }
    else
    {
      ++it;
    }
  }

  for (data::Uid horseUid : expiredHorseUids)
  {
    _registeredStallions.erase(
      std::remove(_registeredStallions.begin(), _registeredStallions.end(), horseUid),
      _registeredStallions.end());
    _horseToStallionMap.erase(horseUid);

    if (!_horseToStallionMap.contains(horseUid))
      ScheduleHorseTypeSet(horseUid, 0);
  }
}

data::Uid BreedingMarket::RegisterStallion(
  data::Uid characterUid,
  data::Uid horseUid,
  uint32_t breedingCharge)
{
  std::lock_guard<std::mutex> lock(_mutex);
  
  // Check if already registered
  if (_horseToStallionMap.contains(horseUid))
  {
    return data::InvalidUid;
  }

  // Get horse data to validate
  auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(horseUid);
  if (!horseRecord)
  {
    spdlog::warn("RegisterStallion: Horse {} not found", horseUid);
    return data::InvalidUid;
  }

  uint8_t horseGrade = 0;
  horseRecord->Immutable([&horseGrade](const data::Horse& horse)
  {
    horseGrade = horse.grade();
  });

  // Only allow specific grades to be registered
  if (horseGrade < MinBreedingGrade || horseGrade > MaxBreedingGrade)
  {
    return data::InvalidUid;
  }

  if (auto chargeRangeOpt = GetChargeRangeForGrade(horseGrade))
  {
    const auto& chargeRange = *chargeRangeOpt;
    if (breedingCharge < chargeRange.min || breedingCharge > chargeRange.max)
    {
      spdlog::warn(
        "RegisterStallion: Breeding charge {} outside allowed range [{} - {}] for grade {}",
        breedingCharge,
        chargeRange.min,
        chargeRange.max,
        horseGrade);
      return data::InvalidUid;
    }
  }

  // Set horse type to Stallion
  horseRecord->Mutable([](data::Horse& horse)
  {
    horse.type() = 2; // HorseType::Stallion
  });

  // Create persistent stallion registration
  auto stallionRecord = _serverInstance.GetDataDirector().CreateStallion();

  data::Uid stallionUid = data::InvalidUid;
  util::Clock::time_point registeredAt;
  
  stallionRecord.Mutable([&](data::Stallion& stallion)
  {
    stallion.horseUid() = horseUid;
    stallion.ownerUid() = characterUid;
    stallion.breedingCharge() = breedingCharge;
    stallion.registeredAt() = util::Clock::now();
    stallion.expiresAt() = util::Clock::now() + std::chrono::hours(24); // 24 hours
    stallionUid = stallion.uid();
    registeredAt = stallion.registeredAt();
  });


  // Add to in-memory structures
  _registeredStallions.emplace_back(horseUid);

  _horseToStallionMap[horseUid] = stallionUid;

  // Cache the stallion data
  StallionData cachedData{
    .stallionUid = stallionUid,
    .horseUid = horseUid,
    .ownerUid = characterUid,
    .breedingCharge = breedingCharge,
    .registeredAt = registeredAt
  };

  _stallionDataCache[stallionUid] = cachedData;

  return stallionUid;
}

BreedingMarket::StallionBreedingEarnings BreedingMarket::UnregisterStallion(data::Uid horseUid)
{
  std::lock_guard<std::mutex> lock(_mutex);


  // Look up stallion UID from horse UID
  auto it = _horseToStallionMap.find(horseUid);
  if (it == _horseToStallionMap.end())
  {
    return BreedingMarket::StallionBreedingEarnings{0, 0, 0};
  }

  data::Uid stallionUid = it->second;
  
  // Calculate earnings BEFORE deleting from cache
  BreedingMarket::StallionBreedingEarnings earnings{0, 0, 0};
  auto cacheIt = _stallionDataCache.find(stallionUid);
  if (cacheIt != _stallionDataCache.end())
  {
    const StallionData& cachedData = cacheIt->second;
    
    // Get times mated from stallion record (during this registration period)
    auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
    if (stallionRecord)
    {
      stallionRecord->Immutable([&earnings](const data::Stallion& stallion)
      {
        earnings.timesMated = stallion.timesMated();
      });

      earnings.breedingCharge = cachedData.breedingCharge;
      earnings.compensation = earnings.timesMated * earnings.breedingCharge;

    }
    
    // Remove from cache
    _stallionDataCache.erase(cacheIt);
  }
  
  // Delete from database cache (this handles file deletion via DataStorage)
  _serverInstance.GetDataDirector().GetStallionCache().Delete(stallionUid);
  
  // Update in-memory structures
  _registeredStallions.erase(std::ranges::find(_registeredStallions, horseUid));
  _horseToStallionMap.erase(it);
  
  // Reset horse type back to Adult
  auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(horseUid);
  if (horseRecord)
  {
    horseRecord->Mutable([](data::Horse& horse)
    {
      horse.type() = 0; // HorseType::Adult
    });
  }


  return earnings;
}

std::optional<BreedingMarket::StallionBreedingEarnings> BreedingMarket::GetUnregisterEstimate(
  data::Uid horseUid)
{
  std::lock_guard<std::mutex> lock(_mutex);
  
  // Look up stallion UID from horse UID
  auto it = _horseToStallionMap.find(horseUid);
  if (it == _horseToStallionMap.end())
  {
    return std::nullopt;
  }

  data::Uid stallionUid = it->second;

  // Get cached stallion data
  auto cacheIt = _stallionDataCache.find(stallionUid);
  if (cacheIt == _stallionDataCache.end())
  {
    return std::nullopt;
  }

  const StallionData& cachedData = cacheIt->second;
  
  // Get times mated from stallion record (during this registration period)
  const auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
  if (!stallionRecord)
  {
    return std::nullopt;
  }

  // Get times bred from horse record
  auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(cachedData.horseUid);
  uint32_t timesMated = 0;
  if (horseRecord)
  {
    horseRecord->Immutable([&timesMated](const data::Horse& horse)
    {
      timesMated = horse.breeding.breedingCount();
    });
  }

  const uint32_t compensation = timesMated * cachedData.breedingCharge;

  return BreedingMarket::StallionBreedingEarnings{timesMated, compensation, cachedData.breedingCharge};
}

bool BreedingMarket::IsRegistered(data::Uid horseUid) const
{
  std::lock_guard<std::mutex> lock(_mutex);
  return _horseToStallionMap.contains(horseUid);
}

std::vector<data::Uid> BreedingMarket::GetRegisteredStallions() const
{
  std::lock_guard<std::mutex> lock(_mutex);
  return _registeredStallions; // Returns a copy
}

std::optional<BreedingMarket::StallionData> BreedingMarket::GetStallionData(data::Uid horseUid) const
{
  std::lock_guard<std::mutex> lock(_mutex);
  
  auto it = _horseToStallionMap.find(horseUid);
  if (it == _horseToStallionMap.end())
  {
    return std::nullopt;
  }

  data::Uid stallionUid = it->second;
  auto cacheIt = _stallionDataCache.find(stallionUid);
  if (cacheIt == _stallionDataCache.end())
  {
    return std::nullopt;
  }

  return cacheIt->second;
}

void BreedingMarket::ScheduleHorseTypeSet(data::Uid horseUid, uint32_t horseType)
{
  _serverInstance.GetDataDirector().ScheduleTask(
    [this, horseUid, horseType]()
    {
      const Deferred deferred([this, horseUid, horseType]()
      {
        auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(horseUid);
        if (horseRecord)
          return;
        ScheduleHorseTypeSet(horseUid, horseType);
      });

      auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(horseUid);
      if (!horseRecord)
        return;

      horseRecord->Mutable([horseType](data::Horse& horse)
      {
        horse.type() = horseType;
      });
    });
}

void BreedingMarket::PayOwner(data::Uid ownerUid, uint32_t earnings)
{
  if (ownerUid == data::InvalidUid || earnings == 0)
  {
    return;
  }

  auto ownerRecord = _serverInstance.GetDataDirector().GetCharacter(ownerUid);
  if (ownerRecord)
  {
    ownerRecord.Mutable([earnings](data::Character& owner)
    {
      owner.carrots() = owner.carrots() + earnings;
    });
    return;
  }

  try
  {
    auto& dataSource = _serverInstance.GetDataDirector().GetDataSource();
    data::Character offlineOwner{};
    dataSource.RetrieveCharacter(ownerUid, offlineOwner);
    offlineOwner.carrots() = offlineOwner.carrots() + earnings;
    dataSource.StoreCharacter(ownerUid, offlineOwner);
  }
  catch (const std::exception& ex)
  {
    spdlog::error("Breeding market: Failed to pay owner {} ({} carrots): {}", ownerUid, earnings, ex.what());
  }
}


} // namespace server

