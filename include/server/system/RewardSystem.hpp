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

#ifndef REWARDSYSTEM_HPP
#define REWARDSYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/Record.hpp>

namespace server
{

class ServerInstance;

//! System responsible for creating, storing, and claiming redeemable character rewards.
class RewardSystem
{
public:
  explicit RewardSystem(ServerInstance& serverInstance);

  //! Creates a redeemable reward for a character and persists it.
  //! @param characterUid UID of the recipient character.
  //! @param type Type of the reward (e.g., Breeding, Carnival).
  //! @param carrots Carrot amount of the reward.
  //! @returns The generated unique claimUid.
  data::Uid CreateReward(
    data::Uid characterUid,
    data::Reward::Type type,
    uint32_t carrots);

  //! Claims a redeemable reward for a character.
  //! Validates characterUid against the stored characterUid in the reward record and checks that the reward has not been claimed.
  //! @param claimUid Unique claim identifier of the reward.
  //! @param characterUid UID of the character attempting to claim the reward.
  //! @returns `true` if claimed successfully, `false` otherwise.
  bool ClaimReward(
    data::Uid claimUid,
    data::Uid characterUid);

private:
  ServerInstance& _serverInstance;
};

} // namespace server

#endif // REWARDSYSTEM_HPP
