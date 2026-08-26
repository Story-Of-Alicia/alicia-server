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

namespace server
{

RewardSystem::RewardSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

data::Uid RewardSystem::CreateReward(
  data::Uid characterUid,
  data::Reward::Type type,
  uint32_t carrots)
{
  auto rewardRecord = _serverInstance.GetDataDirector().CreateReward();
  if (not rewardRecord)
  {
    spdlog::error("Failed to create reward record in for character {}", characterUid);
    return data::InvalidUid;
  }

  data::Uid claimUid{data::InvalidUid};
  const auto now = data::Clock::now();

  rewardRecord.Mutable(
    [characterUid, type, carrots, now, &claimUid](data::Reward& reward)
    {
      reward.characterUid() = characterUid;
      reward.type() = type;
      reward.carrots() = carrots;
      reward.isClaimed() = false;
      reward.createdAt() = now;
      reward.claimedAt() = data::Clock::time_point{};

      claimUid = reward.claimUid();
    });

  spdlog::debug(
    "Created reward record [claimUid: {}, characterUid: {}, type: {}, carrots: {}]",
    claimUid, characterUid, static_cast<uint32_t>(type), carrots);

  return claimUid;
}

bool RewardSystem::ClaimReward(
  data::Uid claimUid,
  data::Uid characterUid)
{
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

  if (isAlreadyClaimed)
  {
    spdlog::warn("Reward record {} was already claimed", claimUid);
    return false;
  }

  if (targetCharacterUid != characterUid)
  {
    spdlog::warn(
      "Claiming character UID {} does not match target character UID {} for reward {}",
      characterUid, targetCharacterUid, claimUid);
    return false;
  }

  // Grant carrots to the character record if carrots > 0
  if (carrotsToGrant > 0)
  {
    auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);
    if (characterRecord)
    {
      characterRecord.Mutable(
        [carrotsToGrant](data::Character& character)
        {
          character.carrots() += carrotsToGrant;
        });
    }
    else
    {
      spdlog::error("Failed to fetch character record for UID {} to grant reward carrots", characterUid);
      return false;
    }
  }

  // Update reward record status
  const auto now = data::Clock::now();
  rewardRecord.Mutable(
    [now](data::Reward& reward)
    {
      reward.isClaimed() = true;
      reward.claimedAt() = now;
    });

  spdlog::debug(
    "Successfully claimed reward {} for character UID {} (carrots granted: {})",
    claimUid, characterUid, carrotsToGrant);

  return true;
}

} // namespace server
