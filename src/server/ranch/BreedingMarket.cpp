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
  for (data::Uid stallionUid : _serverInstance.GetDataDirector().ListRegisteredStallions())
  {
    _serverInstance.GetDataDirector().GetStallionCache().Get(stallionUid);
  }
}

void BreedingMarket::Terminate()
{
  _horses.clear();
}

void BreedingMarket::Tick()
{
}

bool BreedingMarket::HandleRegisterStallion(
  const data::Uid characterUid,
  const data::Uid horseUid,
  const int32_t breedingFee) noexcept
{
  const std::scoped_lock lock(_mutex);
  
  const auto stallionIterator = _horses.find(horseUid);

  // If the horse is a registered stallion do an early return.
  if (stallionIterator != _horses.end())
    return false;

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

  _horses.emplace(horseUid, Horse{
    .stallionUid = stallionUid});

  return true;
}

bool BreedingMarket::HandleUnregisterStallion(
  const data::Uid characterUid,
  const data::Uid horseUid) noexcept
{
  const std::scoped_lock lock(_mutex);

  const auto stallionIterator = _horses.find(horseUid);

  // If the horse is not a registered stallion do an early return.
  if (stallionIterator == _horses.end())
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

  // Populate the earnings.
  Earnings earnings{};
  stallionRecord->Immutable([&earnings](
    const data::Stallion& stallion)
    {
      earnings.timesMated = stallion.timesMated();
      earnings.breedingFee = stallion.breedingCharge();
    });

  earnings.earnings = earnings.timesMated * earnings.breedingFee;

  // todo payout

  // Delete the stallion record.
  _serverInstance.GetDataDirector().GetStallionCache().Delete(stallionUid);
  _horses.erase(stallionIterator);

  horseRecord->Mutable([timesMated = earnings.timesMated](
    data::Horse& horse)
    {
      horse.type() = data::Horse::Type::Adult;
      horse.breeding.breedingCount() += timesMated;
    });

  return true;
}

std::optional<BreedingMarket::Earnings> BreedingMarket::CalculateUnregisterEarnings(
  const data::Uid horseUid) const noexcept
{
  std::scoped_lock lock(_mutex);
  
  // If the horse is not a registered stallion do an early return.
  const auto horseIterator = _horses.find(horseUid);
  if (horseIterator == _horses.end())
    return std::nullopt;

  const data::Uid stallionUid = horseIterator->second.stallionUid;

  const auto stallionRecord = _serverInstance.GetDataDirector().GetStallionCache().Get(
    stallionUid);
  const auto horseRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(
    horseUid);

  // If the stallion record or the horse record are not available do an early return.
  if (not stallionRecord || not horseRecord)
    return std::nullopt;

  // Populate the earnings.
  Earnings earnings;
  stallionRecord->Immutable([&earnings](
    const data::Stallion& stallion)
    {
      earnings.timesMated = stallion.timesMated();
      earnings.breedingFee = stallion.breedingCharge();
    });

  earnings.earnings = earnings.timesMated * earnings.breedingFee;

  return earnings;
}

bool BreedingMarket::IsRegistered(
  const data::Uid horseUid) const noexcept
{
  const std::scoped_lock lock(_mutex);
  return _horses.contains(horseUid);
}

int32_t BreedingMarket::CalculateRegistrationFee(const int32_t breedingFee)
{
  return static_cast<int32_t>(std::ceilf(breedingFee * RegistrationFee));
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

} // namespace server

