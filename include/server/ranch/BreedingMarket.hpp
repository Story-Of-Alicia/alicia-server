//
// Created for Alicia Server
//

#ifndef BREEDING_MARKET_HPP
#define BREEDING_MARKET_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/util/Util.hpp>

#include <vector>
#include <unordered_map>
#include <optional>
#include <mutex>

namespace server
{

// Forward declarations
class ServerInstance;

namespace data
{
  class Horse;
  class Character;
}

//! Manages the breeding market system where players can register stallions for breeding
class BreedingMarket
{
public:
  //! Cached stallion metadata for quick access without async loading
  struct StallionData
  {
    data::Uid stallionUid;
    data::Uid horseUid;
    data::Uid ownerUid;
    uint32_t breedingCharge;
    util::Clock::time_point expiresAt;
  };

  //! Constructor
  //! @param serverInstance Reference to the server instance
  explicit BreedingMarket(ServerInstance& serverInstance);

  //! Destructor
  ~BreedingMarket() = default;

  //! Initializes the breeding market
  //! Loads all registered stallions from persistent storage
  void Initialize();

  //! Terminates the breeding market
  void Terminate();

  //! Ticks the breeding market (checks for expired stallions)
  void Tick();

  //! Registers a horse as a stallion in the breeding market
  //! @param characterUid UID of the character registering the stallion
  //! @param horseUid UID of the horse to register
  //! @param breedingCharge Price per breeding session in carrots
  //! @returns Stallion UID if successful, std::nullopt if failed
  std::optional<data::Uid> RegisterStallion(
    data::Uid characterUid,
    data::Uid horseUid,
    uint32_t breedingCharge);

  //! Unregisters a stallion from the breeding market
  //! @param horseUid UID of the horse to unregister
  //! @returns Compensation amount (timesMated * breedingCharge) or 0 if failed
  uint32_t UnregisterStallion(data::Uid horseUid);

  //! Gets estimate information for unregistering a stallion
  //! @param horseUid UID of the horse
  //! @returns Tuple of {timesMated, compensation, breedingCharge} or nullopt if not registered
  std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> GetUnregisterEstimate(data::Uid horseUid);

  //! Checks if a horse is registered as a stallion
  //! @param horseUid UID of the horse to check
  //! @returns true if registered, false otherwise
  bool IsRegistered(data::Uid horseUid) const;

  //! Gets all registered stallion horse UIDs
  //! @returns Vector of horse UIDs
  std::vector<data::Uid> GetRegisteredStallions() const;

  //! Gets stallion data for a specific horse
  //! @param horseUid UID of the horse
  //! @returns StallionData if found, nullopt otherwise
  std::optional<StallionData> GetStallionData(data::Uid horseUid) const;

private:
  //! Reference to the server instance
  ServerInstance& _serverInstance;

  //! Mutex for thread-safe access to breeding market data
  mutable std::mutex _mutex;

  //! List of all registered horse UIDs (for quick iteration)
  std::vector<data::Uid> _registeredStallions;

  //! Maps horseUid -> stallionUid for quick lookup
  std::unordered_map<data::Uid, data::Uid> _horseToStallionMap;

  //! Cached stallion metadata (stallionUid -> data)
  std::unordered_map<data::Uid, StallionData> _stallionDataCache;

  //! Flag indicating whether stallions are loaded from the database
  bool _stallionsLoaded{false};

  //! List of stallion UIDs being loaded
  std::vector<data::Uid> _stallionUidsToLoad;

  //! Horses that need their type reset from Stallion (2) back to Adult (0)
  //! These are horses whose stallion registration expired but weren't in cache yet
  std::vector<data::Uid> _horsesNeedingTypeReset;

  //! Checks and removes expired stallions
  void CheckExpiredStallions();
  
  //! Attempts to reset horse types for horses in _horsesNeedingTypeReset
  void ProcessPendingHorseTypeResets();
};

} // namespace server

#endif // BREEDING_MARKET_HPP

