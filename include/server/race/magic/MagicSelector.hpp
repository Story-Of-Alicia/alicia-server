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

#ifndef ALICIA_SERVER_RACE_MAGIC_SELECTOR_HPP
#define ALICIA_SERVER_RACE_MAGIC_SELECTOR_HPP

#include "server/race/magic/MagicConfig.hpp"
#include "server/tracker/RaceTracker.hpp"

#include <libserver/registry/MagicRegistry.hpp>

#include <random>
#include <vector>

namespace server::race::magic
{

/**
 * @brief Selects magic items for racers based on the official 4-layer hierarchical probability filter.
 *
 * This class implements the official magic distribution system from the game's XML configuration files:
 * 1. MagicAllocInfo (Layer 1): Global allocation rules - filters items by rank/player count
 * 2. MagicGroupRatio (Layer 2): Group probabilities by rank - determines which magic group to use
 * 3. MagicSlotRatio (Layer 3): Slot probabilities by rank - determines which specific type within group
 * 4. MagicGroupTeamModifier (Layer 4): Dynamic modifiers based on race context
 *
 * The system ensures:
 * - Leaders (rank 1-2) get more defensive items (shields, ice walls)
 * - Trailers (rank 7-8) get more offensive items (attacks, speed boosts)
 * - Rubber-banding mechanics help trailing racers catch up
 */
class MagicSelector
{
public:
  explicit MagicSelector(const registry::MagicRegistry& magicRegistry);

  /**
   * @brief Select a magic item for a racer based on their position.
   *
   * @param racer The racer requesting the item
   * @param positionInfo The racer's current position information
   * @param totalActiveRacers Total number of active racers in the race
   * @return The selected magic slot info
   */
  registry::Magic::SlotInfo SelectItem(
    const tracker::RaceTracker::Racer& racer,
    const tracker::RaceTracker::RacerPositionInfo& positionInfo,
    uint32_t totalActiveRacers) const;

  /**
   * @brief Select a magic item by character UID and race context.
   *
   * @param racer The racer requesting the item
   * @param characterUid The character UID
   * @param racePositions All racer positions sorted by track progress
   * @return The selected magic slot info
   */
  registry::Magic::SlotInfo SelectItem(
    const tracker::RaceTracker::Racer& racer,
    data::Uid characterUid,
    const std::vector<tracker::RaceTracker::RacerPositionInfo>& racePositions) const;

  /**
   * @brief Handle critical chance for the selected item.
   *
   * @param slotInfo The selected slot info
   * @param racer The racer
   * @return Final slot info (possibly critical version)
   */
  registry::Magic::SlotInfo HandleCriticalChance(
    registry::Magic::SlotInfo slotInfo,
    const tracker::RaceTracker::Racer& racer) const;

private:
  const registry::MagicRegistry& _magicRegistry;
  mutable std::random_device _randomDevice;

  /**
   * @brief Layer 2: Select a magic group based on rank probabilities.
   *
   * Uses MagicGroupRatio (MagicGroupTeamRatio) to determine the most appropriate group
   * for the current rank. This is the primary group selection layer.
   *
   * @param rank The racer's rank (1-8)
   * @param isTeamMode Whether it's team mode
   * @return The selected magic group
   */
  MagicGroup SelectGroupByRank(uint32_t rank, bool isTeamMode) const;

  /**
   * @brief Select a group from weighted options.
   *
   * @param groupWeights Vector of (group, weight) pairs
   * @return Selected magic group
   */
  MagicGroup SelectGroupFromWeighted(
    const std::vector<std::pair<MagicGroup, uint32_t>>& groupWeights) const;

  /**
   * @brief Layer 3: Filter and weight types within the selected group by slot ratios.
   *
   * Uses MagicSlotRatio to apply rank-based weights to individual magic types.
   * Also applies MagicGroupAttackRatio for offensive types.
   *
   * @param types Valid magic types from layer 1
   * @param rank The racer's rank (1-8)
   * @param totalRacers Total number of racers
   * @return Vector of types with their computed weights
   */
  std::vector<std::pair<MagicType, uint32_t>> ApplySlotRatios(
    const std::vector<MagicType>& types,
    uint32_t rank,
    uint32_t totalRacers) const;

  /**
   * @brief Layer 4: Apply dynamic modifiers based on race context.
   *
   * Uses MagicGroupTeamModifier to adjust weights based on:
   * - Whether the racer is leading
   * - How many players are ahead
   *
   * @param weightedTypes Types with their base weights
   * @param context The race context
   * @return Modified weights with dynamic adjustments
   */
  void ApplyDynamicModifiers(
    std::vector<std::pair<MagicType, uint32_t>>& weightedTypes,
    const RaceContext& context) const;

  /**
   * @brief Select a magic type from weighted options.
   *
   * @param weightedTypes Vector of (type, weight) pairs
   * @return Selected magic type
   */
  MagicType SelectFromWeighted(const std::vector<std::pair<MagicType, uint32_t>>& weightedTypes) const;

  /**
   * @brief Build race context from position info.
   *
   * @param positionInfo The racer's position information
   * @param totalActiveRacers Total number of active racers
   * @return Race context for magic selection
   */
  RaceContext BuildContext(
    const tracker::RaceTracker::RacerPositionInfo& positionInfo,
    uint32_t totalActiveRacers) const;

  /**
   * @brief Normalize rank from [1, totalPlayers] to [1, 8] for consistent distribution.
   *
   * This ensures that last place in any race size gets the same treatment,
   * and rubber-banding scales properly with player count.
   *
   * @param rank The racer's rank (1 = first, N = last)
   * @param totalPlayers Total number of players in the race
   * @return Normalized rank in range [1, 8]
   */
  uint32_t NormalizeRank(uint32_t rank, uint32_t totalPlayers) const;

  /**
   * @brief Get the pool of magic items for the racer's team mode.
   *
   * @param racer The racer
   * @return The appropriate item pool (solo or team)
   */
  const std::vector<uint32_t>& GetItemPool(const tracker::RaceTracker::Racer& racer) const;
};

} // namespace server::race::magic

#endif // ALICIA_SERVER_RACE_MAGIC_SELECTOR_HPP
