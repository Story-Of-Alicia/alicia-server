/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#include "server/ranch/BreedingMarket.hpp"

#include "server/ServerInstance.hpp"

namespace server
{

namespace
{

// todo: configure me
constexpr float RegistrationFee = 0.50f;
constexpr float EarningTaxes = 0.20f;

constexpr auto MarketDuration = std::chrono::hours(24);

} // anon namespace

BreedingMarket::BreedingMarket(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void BreedingMarket::Initialize()
{
  auto stallionUids = _serverInstance.GetDataDirector().ListRegisteredStallions();

  auto iterator = stallionUids.begin();
  while (not stallionUids.empty())
  {
    if (iterator == stallionUids.end())
    {
      iterator = stallionUids.begin();
    }

    const auto stallionUid = *iterator;
    const auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(
      stallionUid);

    if (not stallionRecord)
    {
      ++iterator;
      continue;
    }

    auto horseUid = data::InvalidUid;

    stallionRecord->Immutable([&horseUid](const data::Stallion& stallion)
    {
      horseUid = stallion.horseUid();
    });

    iterator = stallionUids.erase(iterator);

    _horseRegistrations.try_emplace(horseUid, Registration{
      .stallionUid = stallionUid});
  }

  ScheduleExpirationCheck();
}

void BreedingMarket::Terminate()
{
  _horseRegistrations.clear();
}

void BreedingMarket::Tick()
{
  _scheduler.Tick();
}

bool BreedingMarket::HandleRegisterStallion(
  const data::Uid characterUid,
  const data::Uid horseUid,
  const int32_t breedingFee) noexcept
{
  {
    const std::shared_lock lock(_mutex);

    const auto stallionIterator = _horseRegistrations.find(horseUid);

    // If the horse is a registered stallion do an early return.
    if (stallionIterator != _horseRegistrations.end())
      return false;
  }

  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    characterUid);
  const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(
    horseUid);

  if (not horseRecord || not characterRecord)
    return false;

  // Check if the character can register the stallion.
  bool canRegisterStallion = false;
  characterRecord.Immutable([&canRegisterStallion, horseUid](
    const data::Character& character)
    {
      canRegisterStallion = std::ranges::contains(character.horses(), horseUid);
    });

  // Get the horse grade.
  uint32_t horseGrade = 0;
  horseRecord.Immutable([&horseGrade](const data::Horse& horse)
  {
    horseGrade = horse.grade();
  });

  const auto& gradeFeeRange = GetGradeFeeRange(horseGrade);
  if (not gradeFeeRange)
    return false;

  // Validate the breeding fee according to grade fee range.
  if (breedingFee < gradeFeeRange->min || breedingFee > gradeFeeRange->max)
    return false;

  // Calculate the registration fee.
  const auto registrationFee = CalculateRegistrationFee(
    breedingFee);

  // Take out the registration fee out of character's funds.
  bool canAffordRegistrationFee = false;
  characterRecord.Mutable([&registrationFee, &canAffordRegistrationFee](
    data::Character& character)
    {
      if (character.carrots() > registrationFee)
      {
        canAffordRegistrationFee = true;
        character.carrots() -= registrationFee;
      }
    });

  if (not canAffordRegistrationFee)
    return false;

  const auto stallionUid = RegisterStallion(characterUid, horseUid, breedingFee);
  if (stallionUid == data::InvalidUid)
    return false;

  {
    std::scoped_lock lock(_mutex);

    _horseRegistrations.emplace(horseUid, Registration{
      .stallionUid = stallionUid});
  }

  return true;
}

bool BreedingMarket::HandleUnregisterStallion(
  const data::Uid characterUid,
  const data::Uid horseUid) noexcept
{
  const std::scoped_lock lock(_mutex);

  const auto stallionIterator = _horseRegistrations.find(horseUid);

  // If the horse is not a registered stallion do an early return.
  if (stallionIterator == _horseRegistrations.end())
    return false;

  const auto stallionUid = stallionIterator->second.stallionUid;

  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    characterUid);
  const auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(
    stallionUid);
  const auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(
    horseUid);

  // If the character record, stallion record or the horse record
  // are not available do an early return.
  if (not characterRecord || not stallionRecord || not horseRecord)
    return false;

  // Check if the character can unregister the stallion.
  bool canUnregisterStallion = false;
  characterRecord.Immutable([&canUnregisterStallion, horseUid](
    const data::Character& character)
    {
      canUnregisterStallion = std::ranges::contains(character.horses(), horseUid);
    });

  if (not canUnregisterStallion)
    return false;

  UnregisterStallion(horseUid, stallionUid);

  // Delete the stallion record.
  _horseRegistrations.erase(stallionIterator);

  return true;
}

std::optional<BreedingMarket::Earnings> BreedingMarket::CalculateUnregisterEarnings(
  const data::Uid horseUid) const noexcept
{
  data::Uid stallionUid = data::InvalidUid;

  {
    std::shared_lock lock(_mutex);

    // If the horse is not a registered stallion do an early return.
    const auto horseIterator = _horseRegistrations.find(horseUid);
    if (horseIterator == _horseRegistrations.end())
      return std::nullopt;

    stallionUid = horseIterator->second.stallionUid;
  }

  const auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(
    stallionUid);
  const auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(
    horseUid);

  // If the stallion record or the horse record are not available do an early return.
  if (not stallionRecord || not horseRecord)
    return std::nullopt;

  // Populate the earnings.
  Earnings earnings{
    .taxRate = EarningTaxes};
  stallionRecord->Immutable([&earnings](
    const data::Stallion& stallion)
    {
      earnings.timesMated = stallion.timesMated();
      earnings.breedingFee = stallion.breedingCharge();
    });

  earnings.revenue = earnings.timesMated * earnings.breedingFee;

  return earnings;
}

std::optional<BreedingMarket::StallionData> BreedingMarket::GetStallionData(
  const data::Uid horseUid) const noexcept
{
  data::Uid stallionUid = data::InvalidUid;

  {
    std::shared_lock lock(_mutex);

    // If the horse is not a registered stallion do an early return.
    const auto horseIterator = _horseRegistrations.find(horseUid);
    if (horseIterator == _horseRegistrations.end())
      return std::nullopt;

    stallionUid = horseIterator->second.stallionUid;
  }

  const auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(
    stallionUid);
  if (not stallionRecord)
    return std::nullopt;

  StallionData data;
  data.stallionUid = stallionUid;
  stallionRecord->Immutable([&data](const data::Stallion& stallion)
    {
      data.breedingCharge = stallion.breedingCharge();
    });

  return data;
}

bool BreedingMarket::IsRegistered(
  const data::Uid horseUid) const noexcept
{
  const std::shared_lock lock(_mutex);
  return _horseRegistrations.contains(horseUid);
}

int32_t BreedingMarket::CalculateRegistrationFee(const int32_t breedingFee) const noexcept
{
  return static_cast<int32_t>(std::ceilf(static_cast<float>(breedingFee) * RegistrationFee));
}

std::optional<BreedingMarket::GradeFeeRange> BreedingMarket::GetGradeFeeRange(
  const uint32_t grade) const noexcept
{
  switch (grade)
  {
    case 4: return GradeFeeRange{4000u, 12000u};
    case 5: return GradeFeeRange{5000u, 15000u};
    case 6: return GradeFeeRange{6000u, 18000u};
    case 7: return GradeFeeRange{8000u, 24000u};
    case 8: return GradeFeeRange{10000u, 40000u};
    default: return std::nullopt;
  }
}

BreedingMarket::Snapshot BreedingMarket::CollectMarketSnapshot(
  const SnapshotOrder order,
  const SnapshotFilter filter) const noexcept
{
  std::shared_lock lock(_mutex);

  Snapshot snapshot{};

  // Filter the horse registrations.
  for (const auto [horseUid, registration] : _horseRegistrations)
  {
    const auto stallionRecord = _serverInstance.GetDataDirector().GetStallion(registration.stallionUid);
    if (not stallionRecord)
      continue;

    const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(horseUid);
    if (not horseRecord)
      continue;

    bool isMatch = true;
    horseRecord.Immutable([&filter, &isMatch](const data::Horse& horse)
    {
      if (!filter.coats.empty() && !filter.coats.contains(horse.parts.skinTid()))
        isMatch = false;
      if (!filter.manes.empty() && !filter.manes.contains(horse.parts.maneTid()))
        isMatch = false;
      if (!filter.tails.empty() && !filter.tails.contains(horse.parts.tailTid()))
        isMatch = false;

      // todo: filter major stats
    });

    if (isMatch)
    {
      snapshot.registrations.emplace_back(Snapshot::Registration{
        .horseUid = horseUid,
        .stallionUid = registration.stallionUid});
    }
  }

  std::ranges::sort(
    snapshot.registrations,
    [order, this](
    const Snapshot::Registration firstRegistration,
    const Snapshot::Registration secondRegistration) -> bool
    {
      const auto firstHorseRecord = _serverInstance.GetDataDirector().GetHorse(
        firstRegistration.horseUid);
      const auto firstStallionRecord = _serverInstance.GetDataDirector().GetStallion(
        firstRegistration.stallionUid);

      if (!firstHorseRecord || !firstStallionRecord)
        return false;

      const auto secondHorseRecord = _serverInstance.GetDataDirector().GetHorse(
        secondRegistration.horseUid);
      const auto secondStallionRecord = _serverInstance.GetDataDirector().GetStallion(
        secondRegistration.stallionUid);

      if (!secondHorseRecord || !secondStallionRecord)
        return true;

      // Sort to order by lineage.
      if (order == SnapshotOrder::LineageAscending || order == SnapshotOrder::LineageDescending)
      {
        size_t firstLineage{};
        size_t secondLineage{};

        firstHorseRecord.Immutable([&firstLineage](
          const data::Horse& horse)
        {
          firstLineage = horse.lineage();
        });

        secondHorseRecord.Immutable([&secondLineage](
          const data::Horse& horse)
        {
          secondLineage = horse.lineage();
        });

        // If the sort order is descending, the greater lineage should appear first.
        // Otherwise, the lesser lineage should appear first.
        return order == SnapshotOrder::LineageDescending
          ? firstLineage > secondLineage
          : firstLineage < secondLineage;
      }

      if (order == SnapshotOrder::TimeLeftAscending || order == SnapshotOrder::TimeLeftDescending)
      {
        data::Clock::time_point firstExpiresAt{};
        data::Clock::time_point secondExpiresAt{};

        firstStallionRecord.Immutable([&firstExpiresAt](
          const data::Stallion& horse)
        {
          firstExpiresAt = horse.expiresAt();
        });

        secondStallionRecord.Immutable([&secondExpiresAt](
          const data::Stallion& horse)
        {
          secondExpiresAt = horse.expiresAt();
        });

        // If the sort order is descending, the expiration time further in the future should appear first.
        // Otherwise, the expiration time sooner in future should appear first.
        return order == SnapshotOrder::TimeLeftDescending
          ? firstExpiresAt > secondExpiresAt
          : firstExpiresAt < secondExpiresAt;
      }

      if (order == SnapshotOrder::FeeAscending || order == SnapshotOrder::FeeDescending)
      {
        size_t firstFee{};
        size_t secondFee{};

        firstStallionRecord.Immutable([&firstFee](
          const data::Stallion& horse)
        {
          firstFee = horse.breedingCharge();
        });

        secondStallionRecord.Immutable([&secondFee](
          const data::Stallion& horse)
        {
          secondFee = horse.breedingCharge();
        });

        // If the sort order is descending, the greater breeding fee should appear first.
        // Otherwise, the lesser breeding fee should appear first.
        return order == SnapshotOrder::FeeDescending
          ? firstFee > secondFee
          : firstFee < secondFee;
      }

      if (order == SnapshotOrder::PregnancyChanceAscending
        || order == SnapshotOrder::PregnancyChanceDescending)
      {
        // todo: pregnancy chance
        return std::less<data::Uid>()(firstRegistration.stallionUid, secondRegistration.stallionUid);
      }

      return true;
    });

  return snapshot;
}

data::Uid BreedingMarket::RegisterStallion(
  const data::Uid characterUid,
  const data::Uid horseUid,
  const int32_t breedingFee) const noexcept
{
  const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(
    horseUid);

  if (not horseRecord)
    return data::InvalidUid;

  // Set horse type to Stallion
  horseRecord.Mutable([](data::Horse& horse)
  {
    horse.type() = data::Horse::Type::Stallion;
  });

  // Create the stallion record.
  const auto stallionRecord = _serverInstance.GetDataDirector().CreateStallion();

  data::Uid stallionUid = data::InvalidUid;

  stallionRecord.Mutable([&](data::Stallion& stallion)
  {
    stallion.horseUid() = horseUid;
    stallion.ownerUid() = characterUid;
    stallion.breedingCharge() = breedingFee;
    stallion.registeredAt() = util::Clock::now();

    // todo: this should be configurable if the client supports it.
    stallion.expiresAt() = util::Clock::now() + MarketDuration;

    stallionUid = stallion.uid();;
  });

  return stallionUid;
}

void BreedingMarket::UnregisterStallion(
  data::Uid horseUid,
  data::Uid stallionUid) const noexcept
{
  const auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(
    stallionUid);
  const auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(
    horseUid);

  if (not stallionRecord || not horseRecord)
    return;

  // Populate the earnings.
  Earnings earnings{
    .taxRate = EarningTaxes};

  data::Uid ownerUid{data::InvalidUid};
  stallionRecord->Immutable([&earnings, &ownerUid](
    const data::Stallion& stallion)
    {
      ownerUid = stallion.ownerUid();

      earnings.timesMated = stallion.timesMated();
      earnings.breedingFee = stallion.breedingCharge();
    });

  earnings.revenue = earnings.timesMated * earnings.breedingFee;
  earnings.earnings = earnings.revenue - static_cast<uint32_t>(
    static_cast<float>(earnings.revenue) * earnings.taxRate);

  // Register payout in the RewardSystem
  if (earnings.timesMated > 0)
  {
    earnings.claimUid = _serverInstance.GetRewardSystem().CreateReward(
      ownerUid,
      data::Reward::Type::Breeding,
      earnings.earnings);
  }

  // Send mail with payout information
  _serverInstance.GetMessengerDirector().SendStallionReward(
    ownerUid,
    horseUid,
    earnings);

  // Update the horse status and statistics.
  horseRecord->Mutable([timesMated = earnings.timesMated](
    data::Horse& horse)
    {
      horse.type() = data::Horse::Type::Adult;
      horse.breedingCount() += timesMated;
    });

  // Delete the stallion record.
  _serverInstance.GetDataDirector().GetStallionCache().Delete(stallionUid);
}

void BreedingMarket::ScheduleExpirationCheck() noexcept
{
  _scheduler.Queue(
    [this]()
    {
      RunExpirationCheck();
      ScheduleExpirationCheck();
    },
    Scheduler::Clock::now() + std::chrono::seconds(60));
}

void BreedingMarket::RunExpirationCheck() noexcept
{
  struct Entry
  {
    data::Uid horseUid{data::InvalidUid};
    data::Uid stallionUid{data::InvalidUid};
  };

  std::vector<Entry> expiredHorseUids;

  // Collect expired horse stallion registrations.
  {
    std::shared_lock lock(_mutex);

    for (const auto& [horseUid, registration] : _horseRegistrations)
    {
      const auto stallionRecord = _serverInstance.GetDataDirector().GetStallion(
        registration.stallionUid);

      if (not stallionRecord)
        continue;

      data::Clock::time_point expiresAt;
      stallionRecord.Immutable([&expiresAt](const data::Stallion& stallion)
      {
        expiresAt = stallion.expiresAt();
      });

      const auto now = data::Clock::now();
      if (now > expiresAt)
      {
        expiredHorseUids.emplace_back(Entry{
          .horseUid = horseUid,
          .stallionUid = registration.stallionUid});
      }
    }
  }

  // Erase collected horse stallion registrations.
  {
    std::scoped_lock lock(_mutex);

    for (const auto [horseUid, stallionUid] : expiredHorseUids)
    {
      UnregisterStallion(horseUid, stallionUid);

      // Remove from horse stallion registration.
      _horseRegistrations.erase(horseUid);
    }
  }
}

} // namespace server

