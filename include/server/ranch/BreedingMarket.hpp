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
#include <set>

namespace server
{

// Forward declarations
class ServerInstance;

//! Manages the breeding market system where players can register stallions for breeding
class BreedingMarket
{
public:
  //! Breeding earnings information
  struct Earnings
  {
    //! A count of times the horse was mated.
    uint32_t timesMated{};
    //! A total earnings of the stallion.
    uint32_t earnings{};
    //! A cost of a breed.
    uint32_t breedingFee{};
  };

  //! Breeding fee range of a grade.
  struct GradeFeeRange
  {
    int32_t min{};
    int32_t max{};
  };

  enum class SnapshotOrder
  {
    LineageDescending,
    TimeLeftDescending,
    FeeDescending,
    PregnancyChanceAscending,
    PregnancyChanceDescending,
    FeeAscending,
    TimeLeftAscending,
    LineageAscending
  };

  struct SnapshotFilter
  {
    enum class Stat
    {
      None,
      Agility,
      //! Also known as spirit.
      Ambition,
      //! Also known as speed.
      Rush,
      //! Also known as strength.
      Endurance,
      //! Also known as control.
      Courage,
    };

    std::set<data::Tid> coats;
    std::set<data::Tid> manes;
    std::set<data::Tid> tails;
    Stat firstPreferredStat{Stat::None};
    Stat secondPreferred{Stat::None};
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

  [[nodiscard]] bool HandleRegisterStallion(
    data::Uid characterUid,
    data::Uid horseUid,
    int32_t breedingFee) noexcept;

  [[nodiscard]] bool HandleUnregisterStallion(
    data::Uid characterUid,
    data::Uid horseUid) noexcept;

  //! Calculates earnings for unregistering a stallion
  //! @param horseUid UID of the horse
  //! @returns Breeding earnings if registered, `std::nullopt` if not registered.
  [[nodiscard]] std::optional<Earnings> CalculateUnregisterEarnings(
    data::Uid horseUid) const noexcept;

  //! Checks if a horse is registered as a stallion
  //! @param horseUid UID of the horse to check
  //! @returns true if registered, false otherwise
  [[nodiscard]] bool IsRegistered(data::Uid horseUid) const noexcept;

  //! Calculates registration fee for registering a stallion.
  //! @param breedingFee Breeding fee of a stallion.
  //! @return Registration fee value.
  [[nodiscard]] int32_t CalculateRegistrationFee(int32_t breedingFee);

  //! Gets the breeding fee range for a grade.
  //! @param grade Grade.
  //! @return Breeding fee range of a grade.
  //!         If grade is not allowed to breed `std::nullopt` is returned.
  [[nodiscard]] std::optional<GradeFeeRange> GetGradeFeeRange(
    uint32_t grade) const noexcept;

  //! Gets all registered stallion horse UIDs
  //! @returns Vector of horse UIDs
  [[nodiscard]] std::vector<data::Uid> CollectMarketSnapshot(
    SnapshotOrder order,
    SnapshotFilter) const noexcept;

private:
  struct Horse
  {
    data::Uid stallionUid{data::InvalidUid};
  };

  //! Reference to the server instance
  ServerInstance& _serverInstance;

  //! Mutex for thread-safe access to breeding market data
  mutable std::mutex _mutex;

  //! Set of horses which are stallions.
  std::unordered_map<data::Uid, Horse> _horses;
};

} // namespace server

#endif // BREEDING_MARKET_HPP

