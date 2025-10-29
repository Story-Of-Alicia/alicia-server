//
// Created for Alicia Server
//

#include "server/ranch/BreedingMarket.hpp"
#include "server/ServerInstance.hpp"

#include <libserver/data/DataDirector.hpp>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <format>

namespace server
{

BreedingMarket::BreedingMarket(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void BreedingMarket::Initialize()
{
  // Scan the stallions directory and load all registered stallions
  const std::filesystem::path stallionsPath = "data/stallions";
  
  if (!std::filesystem::exists(stallionsPath))
  {
    spdlog::info("Stallions directory does not exist, skipping stallion loading");
    return;
  }

  int loadedCount = 0;
  int expiredCount = 0;
  std::vector<data::Uid> horseUidsToPreload;
  std::vector<data::Uid> stallionUidsToDelete;
  
  for (const auto& entry : std::filesystem::directory_iterator(stallionsPath))
  {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
      continue;
      
    try
    {
      // Extract stallion UID from filename (e.g., "123.json" -> 123)
      data::Uid stallionUid = std::stoul(entry.path().stem().string());
      
      // Read the JSON file directly to get stallion info
      std::ifstream file(entry.path());
      if (!file.is_open())
      {
        spdlog::warn("Failed to open stallion file {}", entry.path().string());
        continue;
      }
      
      nlohmann::json json = nlohmann::json::parse(file);
      
      spdlog::debug("Loading stallion {} from {}", stallionUid, entry.path().string());
      
      if (!json.contains("horseUid") || !json.contains("expiresAt") || 
          !json.contains("ownerUid") || !json.contains("breedingCharge"))
      {
        spdlog::warn("Stallion file {} is missing required fields, skipping", entry.path().string());
        continue;
      }
      
      data::Uid horseUid = json["horseUid"];
      uint64_t expiresAtSeconds = json["expiresAt"];
      util::Clock::time_point expiresAt = util::Clock::time_point(std::chrono::seconds(expiresAtSeconds));
      
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
      data::Uid ownerUid = json["ownerUid"];
      uint32_t breedingCharge = json["breedingCharge"];
      
      StallionData cachedData{
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
    catch (const std::exception& e)
    {
      spdlog::warn("Failed to load stallion from {}: {}", 
        entry.path().string(), e.what());
    }
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

  // Only allow grades 4-8 to be registered
  if (horseGrade < 4 || horseGrade > 8)
  {
    spdlog::warn("RegisterStallion: Horse {} grade {} is not allowed for breeding (must be 4-8)", 
      horseUid, horseGrade);
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
    stallion.timesBreeded() = 0;
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
    
    // Get times bred from stallion record
    auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
    if (stallionRecord)
    {
      uint32_t timesMated = 0;
      stallionRecord->Immutable([&timesMated](const data::Stallion& stallion)
      {
        timesMated = stallion.timesBreeded();
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

  uint32_t timesMated = 0;
  stallionRecord->Immutable([&timesMated](const data::Stallion& stallion)
  {
    timesMated = stallion.timesBreeded();
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

} // namespace server

