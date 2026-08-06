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

#include "server/system/RewardSystem.hpp"

#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

#include <limits>

namespace server
{

RewardSystem::RewardSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

data::Uid RewardSystem::CreateReward(
  const data::Uid characterUid,
  const data::Reward::Type type,
  const uint32_t carrots,
  const data::Uid sourceUid)
{
  const auto now = data::Clock::now();
  data::Reward reward;
  reward.characterUid = characterUid;
  reward.sourceUid = sourceUid;
  reward.type = type;
  reward.carrots = carrots;
  reward.isClaimed = false;
  reward.createdAt = now;
  reward.claimedAt = data::Clock::time_point{};

  auto rewardRecord = _serverInstance.GetDataDirector().CreateReward(std::move(reward));
  if (not rewardRecord)
  {
    spdlog::error("Failed to create reward record in for character {}", characterUid);
    return data::InvalidUid;
  }

  data::Uid claimUid{data::InvalidUid};
  rewardRecord.Immutable(
    [&claimUid](const data::Reward& storedReward)
    {
      claimUid = storedReward.claimUid();
    });

  spdlog::debug(
    "Created reward record [claimUid: {}, characterUid: {}, type: {}, carrots: {}]",
    claimUid, characterUid, static_cast<uint32_t>(type), carrots);

  return claimUid;
}

bool RewardSystem::ClaimReward(
  const data::Uid claimUid,
  const data::Uid characterUid)
{
  std::scoped_lock claimLock(_claimMutex);

  if (claimUid == data::InvalidUid || characterUid == data::InvalidUid)
  {
    spdlog::warn("Invalid claimUid {} or characterUid {}", claimUid, characterUid);
    return false;
  }

  auto rewardRecord = _serverInstance.GetDataDirector().GetReward(claimUid);
  if (not rewardRecord)
  {
    spdlog::warn("Reward record {} not found", claimUid);
    return false;
  }

  bool isAlreadyClaimed = false;
  data::Uid targetCharacterUid{data::InvalidUid};
  uint32_t carrotsToGrant{0};

  rewardRecord.Immutable(
    [&isAlreadyClaimed, &targetCharacterUid, &carrotsToGrant](const data::Reward& reward)
    {
      isAlreadyClaimed = reward.isClaimed();
      targetCharacterUid = reward.characterUid();
      carrotsToGrant = reward.carrots();
    });

  if (targetCharacterUid != characterUid)
  {
    spdlog::warn(
      "Claiming character UID {} does not match target character UID {} for reward {}",
      characterUid, targetCharacterUid, claimUid);
    return false;
  }

  if (isAlreadyClaimed)
  {
    const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
    if (characterRecord)
    {
      bool removedMarker = false;
      characterRecord.Mutable([claimUid, &removedMarker](data::Character& character)
      {
        removedMarker = character.pendingRewardClaimUids().erase(claimUid) != 0;
      });
      if (removedMarker)
      {
        _serverInstance.GetDataDirector().GetCharacterCache().StoreNow(characterUid);
      }
    }

    spdlog::warn("Reward record {} was already claimed", claimUid);
    return false;
  }

  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto characterRecord = dataDirector.GetCharacter(characterUid);
  if (not characterRecord)
  {
    spdlog::error("Failed to fetch character record for UID {} to grant reward carrots", characterUid);
    return false;
  }

  bool appliedNow = false;
  bool amountValid = true;
  int32_t originalCarrots{};
  if (carrotsToGrant > 0)
  {
    characterRecord.Mutable(
      [claimUid,
       carrotsToGrant,
       &appliedNow,
       &amountValid,
       &originalCarrots](data::Character& character)
      {
        if (character.pendingRewardClaimUids().contains(claimUid))
          return;

        originalCarrots = character.carrots();
        const int64_t updatedCarrots = static_cast<int64_t>(character.carrots())
          + carrotsToGrant;
        if (updatedCarrots > std::numeric_limits<int32_t>::max())
        {
          amountValid = false;
          return;
        }

        character.carrots() = static_cast<int32_t>(updatedCarrots);
        character.pendingRewardClaimUids().insert(claimUid);
        appliedNow = true;
      });

    if (not amountValid)
    {
      spdlog::error(
        "Reward {} would overflow the carrot balance of character {}",
        claimUid,
        characterUid);
      return false;
    }

    if (appliedNow && not dataDirector.GetCharacterCache().StoreNow(characterUid))
    {
      characterRecord.Mutable([claimUid, originalCarrots](data::Character& character)
      {
        if (character.pendingRewardClaimUids().erase(claimUid) != 0)
          character.carrots() = originalCarrots;
      });
      spdlog::error(
        "Failed to persist reward {} for character {}",
        claimUid,
        characterUid);
      return false;
    }
  }

  const auto now = data::Clock::now();
  rewardRecord.Mutable(
    [now](data::Reward& reward)
    {
      reward.isClaimed() = true;
      reward.claimedAt() = now;
    });

  if (not dataDirector.GetRewardCache().StoreNow(claimUid))
  {
    rewardRecord.Mutable([](data::Reward& reward)
    {
      reward.isClaimed() = false;
      reward.claimedAt() = data::Clock::time_point{};
    });
    spdlog::error("Failed to persist claimed reward {}", claimUid);
    return false;
  }

  if (carrotsToGrant > 0)
  {
    characterRecord.Mutable([claimUid](data::Character& character)
    {
      character.pendingRewardClaimUids().erase(claimUid);
    });
    if (not dataDirector.GetCharacterCache().StoreNow(characterUid))
    {
      spdlog::warn(
        "Failed to clear applied reward marker {} for character {}",
        claimUid,
        characterUid);
    }
  }

  spdlog::debug(
    "Successfully claimed reward {} for character UID {} (carrots granted: {})",
    claimUid, characterUid, carrotsToGrant);

  return true;
}

} // namespace server
