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
  std::vector<data::Uid> stallionUids = _serverInstance.GetDataDirector().ListRegisteredStallions();
  
  spdlog::debug("BreedingMarket::Initialize() found {} stallion file(s)", stallionUids.size());
  
  int loadedCount = 0;
  int expiredCount = 0;
  std::vector<data::Uid> horseUidsToPreload;
  std::vector<data::Uid> stallionUidsToDelete;
  
  for (data::Uid stallionUid : stallionUids)
  {
    // Load stallion data synchronously by creating it in the cache
    // This bypasses the async retrieval queue and loads immediately
    data::Stallion stallionData;
    data::Uid horseUid = 0;
    data::Uid ownerUid = 0;
    uint32_t breedingCharge = 0;
    util::Clock::time_point expiresAt;
    
    try
    {
      // Get a mutable stallion to populate
      auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Create(
        [this, stallionUid]()
        {
          data::Stallion stallion;
          // Load directly from file system synchronously
          _serverInstance.GetDataDirector().GetDataSource().RetrieveStallion(stallionUid, stallion);
          return std::make_pair(stallionUid, std::move(stallion));
        });
      
      stallionRecord.Immutable([&](const data::Stallion& stallion)
      {
        horseUid = stallion.horseUid();
        ownerUid = stallion.ownerUid();
        breedingCharge = stallion.breedingCharge();
        expiresAt = stallion.expiresAt();
      });
    }
    catch (const std::exception& e)
    {
      spdlog::warn("Failed to load stallion record {}: {}", stallionUid, e.what());
      continue;
    }
    
    spdlog::debug("Loading stallion {} (horse {})", stallionUid, horseUid);
    
    // Check if stallion has expired
    if (util::Clock::now() >= expiresAt)
    {
      spdlog::info("Stallion {} (horse {}) has expired, removing from database", 
        stallionUid, horseUid);
      
      // Queue stallion UID for deletion via DataStorage
      stallionUidsToDelete.push_back(stallionUid);
      
      // Reset horse type back to Adult
      auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(horseUid);
      if (horseRecord)
      {
        horseRecord->Mutable([](data::Horse& horse)
        {
          horse.horseType() = 0; // Adult
        });
      }
      
      expiredCount++;
      continue;
    }
    
    // Cache the stallion data to avoid async loading issues
    StallionData cachedData{
      .horseUid = horseUid,
      .ownerUid = ownerUid,
      .breedingCharge = breedingCharge,
      .expiresAt = expiresAt
    };
    _stallionDataCache[stallionUid] = cachedData;
    
    // Add to in-memory lists for active stallions
    _registeredStallions.emplace_back(horseUid);
    _horseToStallionMap[horseUid] = stallionUid;
    horseUidsToPreload.push_back(horseUid);
    
    loadedCount++;
  }
  
  // Delete expired stallions using DataStorage
  for (data::Uid stallionUid : stallionUidsToDelete)
  {
    _serverInstance.GetDataDirector().GetStallionCache().Delete(stallionUid);
  }
  
  // Pre-load all stallion horses
  if (!horseUidsToPreload.empty())
  {
    _serverInstance.GetDataDirector().GetHorseCache().Get(horseUidsToPreload);
  }
  
  spdlog::info("Loaded {} registered stallion(s) from database ({} expired and removed)", 
    loadedCount, expiredCount);
  
  // Debug: Log the first few stallions
  if (!_registeredStallions.empty())
  {
    spdlog::debug("Registered stallions count: {}, first entry: {}", 
      _registeredStallions.size(), _registeredStallions[0]);
  }
}

void BreedingMarket::Terminate()
{
  _registeredStallions.clear();
  _horseToStallionMap.clear();
  _stallionDataCache.clear();
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
    stallion.expiresAt() = util::Clock::now() + std::chrono::hours(24 * 7); // 7 days
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
    
    // Get times bred from horse record
    auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(cachedData.horseUid);
    if (horseRecord)
    {
      uint32_t timesMated = 0;
      horseRecord->Immutable([&timesMated](const data::Horse& horse)
      {
        timesMated = horse.timesBreeded();
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
  
  // Get times bred from stallion record
  auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
  if (!stallionRecord)
  {
    spdlog::error("GetUnregisterEstimate: Stallion {} record not found", stallionUid);
    return std::nullopt;
  }

  // Get times bred from horse record
  auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(cachedData.horseUid);
  uint32_t timesMated = 0;
  if (horseRecord)
  {
    horseRecord->Immutable([&timesMated](const data::Horse& horse)
    {
      timesMated = horse.timesBreeded();
    });
  }

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

} // namespace server

