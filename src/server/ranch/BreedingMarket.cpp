//
// Created for Alicia Server
//

#include "server/ranch/BreedingMarket.hpp"
#include "server/ServerInstance.hpp"

#include <libserver/data/DataDirector.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>

namespace server
{

namespace
{

// Breeding grade restrictions
// TODO: Make these configurable via server config
constexpr uint8_t MinBreedingGrade = 4;
constexpr uint8_t MaxBreedingGrade = 8;

} // namespace

BreedingMarket::BreedingMarket(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void BreedingMarket::Initialize()
{
  // Get all registered stallion UIDs from the data source
  _stallionUidsToLoad = _serverInstance.GetDataDirector().ListRegisteredStallions();
  
  spdlog::info("BreedingMarket::Initialize - Found {} stallion UID(s) from data source", _stallionUidsToLoad.size());
  
  if (_stallionUidsToLoad.empty())
  {
    spdlog::info("No registered stallions found in database");
    _stallionsLoaded = true;
    return;
  }
  
  // Log the stallion UIDs
  for (data::Uid uid : _stallionUidsToLoad)
  {
    spdlog::debug("  - Queuing stallion UID: {}", uid);
  }
  
  spdlog::info("Queuing {} stallion(s) for async loading", _stallionUidsToLoad.size());
  
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
    
    // Check each stallion individually
    for (data::Uid stallionUid : _stallionUidsToLoad)
    {
      auto stallionRecordOpt = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
      if (!stallionRecordOpt)
      {
        // Not loaded yet, will try again next tick
        stillPending.push_back(stallionUid);
        continue;
      }
      
      StallionData cachedData{};
      bool isValid = true;
      
      uint32_t timesMated = 0;
      
      stallionRecordOpt->Immutable([&](const data::Stallion& stallion)
      {
        cachedData.stallionUid = stallion.uid();
        cachedData.horseUid = stallion.horseUid();
        cachedData.ownerUid = stallion.ownerUid();
        cachedData.breedingCharge = stallion.breedingCharge();
        cachedData.expiresAt = stallion.expiresAt();
        timesMated = stallion.timesMated();
        
        // Check if expired immediately
        if (util::Clock::now() >= cachedData.expiresAt)
        {
          spdlog::warn("Breeding market: Stallion {} (horse {}) is already expired, processing cleanup", 
            stallionUid, cachedData.horseUid);
          isValid = false;
        }
        else
        {
          // Preload the horse (async)
          _serverInstance.GetDataDirector().GetHorseCache().Get(stallion.horseUid());
        }
      });
      
      if (!isValid)
      {
        // Handle expired stallion: pay owner, reset horse type, delete record
        uint32_t earnings = cachedData.breedingCharge * timesMated;
        
        // Pay the owner
        auto ownerRecord = _serverInstance.GetDataDirector().GetCharacter(cachedData.ownerUid);
        if (ownerRecord)
        {
          ownerRecord.Mutable([earnings](data::Character& owner)
          {
            owner.carrots() = owner.carrots() + earnings;
          });
          spdlog::info("Breeding market: Paid owner {} a total of {} carrots ({} × {} matings)", 
            cachedData.ownerUid, earnings, cachedData.breedingCharge, timesMated);
        }
        
        // Reset horse type back to Adult (0)
        // Trigger async load of the horse
        auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(cachedData.horseUid);
        if (horseRecord)
        {
          horseRecord->Mutable([&cachedData](data::Horse& horse)
          {
            horse.horseType() = 0; // Adult
            spdlog::info("Breeding market: Reset horse {} type from Stallion to Adult", cachedData.horseUid);
          });
        }
        else
        {
          // Horse not loaded yet - queue it for later processing
          _horsesNeedingTypeReset.push_back(cachedData.horseUid);
          spdlog::debug("Breeding market: Horse {} not in cache yet, queued for type reset", 
            cachedData.horseUid);
        }
        
        // Delete stallion record from database
        _serverInstance.GetDataDirector().GetStallionCache().Delete(stallionUid);
        spdlog::info("Breeding market: Deleted expired stallion {} from database", stallionUid);
        
        // Don't add to stillPending - it's been handled
        continue;
      }
      
      _stallionDataCache[stallionUid] = std::move(cachedData);
      _registeredStallions.emplace_back(cachedData.horseUid);
      _horseToStallionMap[cachedData.horseUid] = stallionUid;
      loadedCount++;
      spdlog::debug("Breeding market: Successfully loaded stallion {} (horse {})", stallionUid, cachedData.horseUid);
    }
    
    // Update the queue with only pending stallions
    _stallionUidsToLoad = std::move(stillPending);
    
    // If all stallions are loaded, mark as complete
    if (_stallionUidsToLoad.empty())
    {
      spdlog::info("Loaded {} registered stallion(s) from database", loadedCount);
      _stallionsLoaded = true;
    }
    else
    {
      spdlog::debug("Breeding market: {} stallions loaded, {} still pending", loadedCount, _stallionUidsToLoad.size());
    }
  }
  
  // Check for expired stallions every tick
  if (_stallionsLoaded)
  {
    CheckExpiredStallions();
  }
  
  // Try to reset horse types for expired stallions whose horses weren't loaded yet
  ProcessPendingHorseTypeResets();
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
    
    if (now >= stallionData.expiresAt)
    {
      spdlog::info("Stallion {} (horse {}) has expired, removing from breeding market", 
        stallionUid, stallionData.horseUid);
      
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
      auto ownerRecord = _serverInstance.GetDataDirector().GetCharacter(stallionData.ownerUid);
      if (ownerRecord)
      {
        ownerRecord.Mutable([earnings](data::Character& owner)
        {
          owner.carrots() = owner.carrots() + earnings;
        });
        spdlog::info("Breeding market: Paid owner {} a total of {} carrots ({} × {} matings)", 
          stallionData.ownerUid, earnings, stallionData.breedingCharge, timesMated);
      }
      
      // Remove from cache
      expiredHorseUids.push_back(stallionData.horseUid);
      
      // Delete stallion record from database
      _serverInstance.GetDataDirector().GetStallionCache().Delete(stallionUid);
      
      // Reset horse type back to Adult (0)
      auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(stallionData.horseUid);
      if (horseRecord)
      {
        horseRecord->Mutable([&stallionData](data::Horse& horse)
        {
          horse.horseType() = 0; // Adult
          spdlog::info("Breeding market: Reset horse {} type from Stallion to Adult", stallionData.horseUid);
        });
      }
      else
      {
        // Horse not loaded yet - queue it for later processing
        _horsesNeedingTypeReset.push_back(stallionData.horseUid);
        spdlog::debug("Breeding market: Horse {} not in cache yet, queued for type reset", 
          stallionData.horseUid);
      }
      
      // Erase from cache
      it = _stallionDataCache.erase(it);
    }
    else
    {
      ++it;
    }
  }
  
  // Remove expired horses from tracking lists
  for (data::Uid horseUid : expiredHorseUids)
  {
    // Remove from registered stallions list
    _registeredStallions.erase(
      std::remove(_registeredStallions.begin(), _registeredStallions.end(), horseUid),
      _registeredStallions.end());
    
    // Remove from horse->stallion map
    _horseToStallionMap.erase(horseUid);
  }
  
  if (!expiredHorseUids.empty())
  {
    spdlog::info("Removed {} expired stallion(s) from breeding market", expiredHorseUids.size());
  }
}

std::optional<data::Uid> BreedingMarket::RegisterStallion(
  data::Uid characterUid,
  data::Uid horseUid,
  uint32_t breedingCharge)
{
  std::lock_guard<std::mutex> lock(_mutex);
  
  // Check if already registered
  if (_horseToStallionMap.contains(horseUid))
  {
    spdlog::warn("RegisterStallion: Horse {} is already registered", horseUid);
    return std::nullopt;
  }

  // Get horse data to validate
  auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(horseUid);
  if (!horseRecord)
  {
    spdlog::warn("RegisterStallion: Horse {} not found", horseUid);
    return std::nullopt;
  }

  uint8_t horseGrade = 0;
  horseRecord->Immutable([&horseGrade](const data::Horse& horse)
  {
    horseGrade = horse.grade();
  });

  // Only allow specific grades to be registered
  if (horseGrade < MinBreedingGrade || horseGrade > MaxBreedingGrade)
  {
    spdlog::warn("RegisterStallion: Horse {} grade {} is not allowed for breeding (must be {}-{})", 
      horseUid, horseGrade, MinBreedingGrade, MaxBreedingGrade);
    return std::nullopt;
  }

  // Set horse type to Stallion
  horseRecord->Mutable([](data::Horse& horse)
  {
    horse.horseType() = 2; // HorseType::Stallion
  });

  // Create persistent stallion registration
  auto stallionRecord = _serverInstance.GetDataDirector().CreateStallion();

  data::Uid stallionUid = data::InvalidUid;
  util::Clock::time_point expiresAt;
  
  stallionRecord.Mutable([&](data::Stallion& stallion)
  {
    stallion.horseUid() = horseUid;
    stallion.ownerUid() = characterUid;
    stallion.breedingCharge() = breedingCharge;
    stallion.registeredAt() = util::Clock::now();
    stallion.expiresAt() = util::Clock::now() + std::chrono::hours(24); // 24 hours
    stallionUid = stallion.uid();
    expiresAt = stallion.expiresAt();
  });

  spdlog::info("RegisterStallion: Created stallion record with UID {}", stallionUid);

  // Add to in-memory structures
  spdlog::debug("RegisterStallion: Adding to _registeredStallions");
  _registeredStallions.emplace_back(horseUid);
  
  spdlog::debug("RegisterStallion: Adding to _horseToStallionMap");
  _horseToStallionMap[horseUid] = stallionUid;
  
  // Cache the stallion data
  spdlog::debug("RegisterStallion: Creating cached data");
  StallionData cachedData{
    .stallionUid = stallionUid,
    .horseUid = horseUid,
    .ownerUid = characterUid,
    .breedingCharge = breedingCharge,
    .expiresAt = expiresAt
  };
  
  spdlog::debug("RegisterStallion: Adding to _stallionDataCache");
  _stallionDataCache[stallionUid] = cachedData;

  spdlog::info("RegisterStallion: Successfully registered horse {} as stallion {}", horseUid, stallionUid);

  return stallionUid;
}

uint32_t BreedingMarket::UnregisterStallion(data::Uid horseUid)
{
  std::lock_guard<std::mutex> lock(_mutex);
  
  spdlog::info("UnregisterStallion: horseUid={}", horseUid);
  
  // Look up stallion UID from horse UID
  auto it = _horseToStallionMap.find(horseUid);
  if (it == _horseToStallionMap.end())
  {
    spdlog::warn("UnregisterStallion: Horse {} is not registered as stallion", horseUid);
    return 0;
  }

  data::Uid stallionUid = it->second;
  
  // Calculate compensation BEFORE deleting from cache
  uint32_t compensation = 0;
  auto cacheIt = _stallionDataCache.find(stallionUid);
  if (cacheIt != _stallionDataCache.end())
  {
    const StallionData& cachedData = cacheIt->second;
    
    // Get times mated from stallion record (during this registration period)
    auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
    if (stallionRecord)
    {
      uint32_t timesMated = 0;
      stallionRecord->Immutable([&timesMated](const data::Stallion& stallion)
      {
        timesMated = stallion.timesMated();
      });
      
      compensation = timesMated * cachedData.breedingCharge;
      spdlog::info("UnregisterStallion: Calculated compensation {} carrots (timesMated={}, charge={})", 
        compensation, timesMated, cachedData.breedingCharge);
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
      horse.horseType() = 0; // HorseType::Adult
    });
  }

  spdlog::info("UnregisterStallion: Successfully unregistered horse {} (stallion {})", 
    horseUid, stallionUid);

  return compensation;
}

std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> BreedingMarket::GetUnregisterEstimate(
  data::Uid horseUid)
{
  std::lock_guard<std::mutex> lock(_mutex);
  
  // Look up stallion UID from horse UID
  auto it = _horseToStallionMap.find(horseUid);
  if (it == _horseToStallionMap.end())
  {
    spdlog::warn("GetUnregisterEstimate: Horse {} is not registered as stallion", horseUid);
    return std::nullopt;
  }

  data::Uid stallionUid = it->second;
  
  // Get cached stallion data
  auto cacheIt = _stallionDataCache.find(stallionUid);
  if (cacheIt == _stallionDataCache.end())
  {
    spdlog::error("GetUnregisterEstimate: Stallion {} not found in cache (inconsistent state)", stallionUid);
    return std::nullopt;
  }

  const StallionData& cachedData = cacheIt->second;
  
  // Get times mated from stallion record (during this registration period)
  auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
  if (!stallionRecord)
  {
    spdlog::error("GetUnregisterEstimate: Stallion {} record not found", stallionUid);
    return std::nullopt;
  }

  uint32_t timesMated = 0;
  stallionRecord->Immutable([&timesMated](const data::Stallion& stallion)
  {
    timesMated = stallion.timesMated();
  });

  uint32_t compensation = timesMated * cachedData.breedingCharge;

  spdlog::info("GetUnregisterEstimate: Horse {} - timesMated={}, compensation={}, price={}", 
    horseUid, timesMated, compensation, cachedData.breedingCharge);

  return std::make_tuple(timesMated, compensation, cachedData.breedingCharge);
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

void BreedingMarket::ProcessPendingHorseTypeResets()
{
  // This is called from Tick() which already holds the mutex
  if (_horsesNeedingTypeReset.empty())
  {
    return;
  }
  
  std::vector<data::Uid> stillPending;
  
  for (data::Uid horseUid : _horsesNeedingTypeReset)
  {
    auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(horseUid);
    if (horseRecord)
    {
      horseRecord->Mutable([horseUid](data::Horse& horse)
      {
        horse.horseType() = 0; // Adult
        spdlog::info("Breeding market: Successfully reset horse {} type from Stallion to Adult (deferred)", 
          horseUid);
      });
    }
    else
    {
      // Still not loaded, keep it in the queue
      stillPending.push_back(horseUid);
    }
  }
  
  _horsesNeedingTypeReset = std::move(stillPending);
  
  if (!_horsesNeedingTypeReset.empty())
  {
    spdlog::debug("Breeding market: {} horse(s) still pending type reset", _horsesNeedingTypeReset.size());
  }
}

} // namespace server

