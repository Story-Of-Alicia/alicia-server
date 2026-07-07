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

#ifndef ALICIA_SERVER_RACE_MAGIC_TYPES_HPP
#define ALICIA_SERVER_RACE_MAGIC_TYPES_HPP

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace server::race::magic
{

//! Magic Types
enum class MagicType : uint8_t
{
  Invalid = 0,
  FireBall = 2,
  FireBallCritical = 3,
  WaterShield = 4,
  WaterShieldCritical = 5,
  Booster = 6,
  BoosterCritical = 7,
  HotRodding = 8,
  HotRoddingCritical = 9,
  IceWall = 10,
  IceWallCritical = 11,
  JumpStun = 12,
  JumpStunCritical = 13,
  DarkFire = 14,
  DarkFireCritical = 15,
  Summon = 16,
  SummonCritical = 17,
  Lightning = 18,
  LightningCritical = 19,
  BufPower = 20,
  BufPowerCritical = 21,
  BufGauge = 22,
  BufGaugeCritical = 23,
  BufSpeed = 24,
  BufSpeedCritical = 25,
  MaxTypes = 26
};

// Convert MagicType to underlying type for storage
inline uint32_t ToUnderlying(MagicType type)
{
  return static_cast<uint32_t>(type);
}

//! Magic Groups
enum class MagicGroup : uint8_t
{
  Invalid = 0,
  Offensive = 1,    // FireBall, DarkFire, Summon, Lightning, JumpStun
  Defensive = 2,    // WaterShield
  SpeedUtility = 3, // Booster, HotRodding
  Special = 4,
  RareOffensive = 5, // JumpStun (from MagicGroupTeamRatio)
  RareSpeed = 6,
  RareUtility = 7,
  MaxGroups = 8
};

//! Basic Types
enum class BasicType : uint8_t
{
  Invalid = 0,
  FireBall = 2,
  WaterShield = 4,
  Booster = 6,
  HotRodding = 8,
  IceWall = 10,
  JumpStun = 12,
  DarkFire = 14,
  Summon = 16,
  Lightning = 18,
  BufPower = 20,
  BufGauge = 22,
  BufSpeed = 24,
  MaxBasicTypes = 25
};

//! Race Context
struct RaceContext
{
  uint32_t rank = 0;           // 1-N (1 = first, N = last, where N = totalPlayers)
  uint32_t normalizedRank = 0; // 1-8 (normalized to 8-player scale for consistent distribution)
  uint32_t totalPlayers = 0;   // 1-8
  uint32_t aheadCount = 0;     // Number of players ahead (0-7)
  bool isLeading = false;      // true if this racer is in 1st place
  bool isTeamMode = false;     // true for team races
  MagicGroup group = MagicGroup::Invalid;
};

//! Allocations Rules (from MagicAllocInfo, Table 216)
struct AllocationRule
{
  uint32_t minRank = 1;
  uint32_t maxRank = 8;
  uint32_t minPlayerCount = 1;
  float minRatio = 0.0f;
  float maxRatio = 1.0f;
  uint32_t weight = 1;
  uint32_t maxCount = 1;
  uint32_t conditionType = 0;
  float conditionValue = 0.0f;
};

//! Slot Ratios (from MagicSlotRatio, Table 199)

//! Slot ratio for each magic type by rank (1-8)
//! Values represent percentage probability * 100 (e.g., 70 = 70%)
struct SlotRatio
{
  std::array<uint32_t, 8> rankWeights; // [0] = rank 1, [7] = rank 8
};

//! Group Ratios (from MagicGroupRatio, Table 214)

//! Group probability by rank
struct GroupRatio
{
  std::array<uint32_t, 8> rankWeights; // Weight for each rank 1-8
};

//! Team Modifiers (from MagicGroupTeamModifier, Table 270)
struct TeamModifier
{
  int32_t ahead1 = 0;              // Modifier when 1 player ahead
  int32_t ahead2 = 0;              // Modifier when 2 players ahead
  int32_t ahead3 = 0;              // Modifier when 3 players ahead
  int32_t ahead4 = 0;              // Modifier when 4 players ahead
  bool appliesWhenLeading = false; // true if this applies when leading
};

//! Slot Team Modifiers (from MagicSlotTeamModifier, Table 271)
struct SlotTeamModifier
{
  int32_t ahead1 = 0;              // Modifier when 1 player ahead
  int32_t ahead2 = 0;              // Modifier when 2 players ahead
  int32_t ahead3 = 0;              // Modifier when 3 players ahead
  int32_t ahead4 = 0;              // Modifier when 4 players ahead
  bool appliesWhenLeading = false; // true if this applies when leading
};

} // namespace server::race::magic

#endif // ALICIA_SERVER_RACE_MAGIC_TYPES_HPP
