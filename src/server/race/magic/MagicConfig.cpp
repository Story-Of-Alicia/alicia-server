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

#include "server/race/magic/MagicConfig.hpp"

namespace server::magic
{

MagicConfig::MagicConfig()
{
  InitializeAllocationRules();
  InitializeGroupRatios();
  InitializeSlotRatios();
  InitializeTeamModifiers();
  InitializeTypeMappings();
}

void MagicConfig::InitializeAllocationRules()
{
  // Initialize all rules with default values (allow all ranks and player counts)
  // Then override specific types with data from MagicAllocInfo.xml
  for (auto& rule : allocationRules_)
  {
    rule = {
      .minRank = 1,
      .maxRank = 8,
      .minPlayerCount = 1,
      .minRatio = 0.0f,
      .maxRatio = 1.0f,
      .weight = 1,
      .maxCount = 1,
      .conditionType = 0,
      .conditionValue = 0.0f};
  }

  // Data from MagicAllocInfo.xml (Table 216)
  // Only types with specific rules are overridden
  allocationRules_[ToUnderlying(MagicType::WaterShieldCritical)] = {
    .minRank = 3,
    .maxRank = 6,
    .minPlayerCount = 3,
    .minRatio = 0.25f,
    .maxRatio = 0.80f,
    .weight = 80,
    .maxCount = 1,
    .conditionType = 1,
    .conditionValue = 200.0f};

  allocationRules_[ToUnderlying(MagicType::Booster)] = {
    .minRank = 7,
    .maxRank = 8,
    .minPlayerCount = 3,
    .minRatio = 0.20f,
    .maxRatio = 0.80f,
    .weight = 50,
    .maxCount = 2,
    .conditionType = 2,
    .conditionValue = 300.0f};
}

void MagicConfig::InitializeGroupRatios()
{
  for (auto& ratio : groupRatios_)
  {
    ratio = GroupRatio{};
  }

  // Data from MagicGroupTeamRatio.xml (Table 355)
  // Exact values from the XML table
  groupRatios_[static_cast<uint32_t>(MagicGroup::Offensive)].rankWeights = {0, 45, 50, 65, 65, 60, 44, 11};
  groupRatios_[static_cast<uint32_t>(MagicGroup::Defensive)].rankWeights = {60, 15, 0, 0, 0, 0, 0, 0};
  groupRatios_[static_cast<uint32_t>(MagicGroup::SpeedUtility)].rankWeights = {5, 40, 30, 25, 40, 35, 44, 82};
  groupRatios_[static_cast<uint32_t>(MagicGroup::Special)].rankWeights = {35, 0, 0, 0, 0, 0, 0, 0};
  groupRatios_[static_cast<uint32_t>(MagicGroup::RareOffensive)].rankWeights = {0, 0, 5, 3, 3, 3, 0, 0};
  groupRatios_[static_cast<uint32_t>(MagicGroup::RareSpeed)].rankWeights = {0, 0, 0, 0, 0, 0, 4, 11};
  groupRatios_[static_cast<uint32_t>(MagicGroup::RareUtility)].rankWeights = {0, 0, 0, 0, 0, 0, 22, 22};
}

void MagicConfig::InitializeSlotRatios()
{
  // Initialize all to a low default
  SlotRatio defaultRatio;
  defaultRatio.rankWeights = {5, 5, 5, 5, 5, 5, 5, 5};

  for (auto& ratio : slotRatios_)
  {
    ratio = defaultRatio;
  }

  // DEFENSIVE ITEMS (Group 2)
  slotRatios_[ToUnderlying(MagicType::WaterShield)].rankWeights = {60, 15, 0, 0, 0, 0, 0, 0};
  slotRatios_[ToUnderlying(MagicType::WaterShieldCritical)].rankWeights = {60, 15, 0, 0, 0, 0, 0, 0};
  slotRatios_[ToUnderlying(MagicType::IceWall)].rankWeights = {50, 15, 0, 0, 0, 0, 0, 0};
  slotRatios_[ToUnderlying(MagicType::IceWallCritical)].rankWeights = {50, 15, 0, 0, 0, 0, 0, 0};

  // SPEED/UTILITY ITEMS (Group 3)
  // Based on original game: Booster & HotRodding had half weight (2 vs 4)
  // So their slot ratios are ~50% of other items
  // Matching MagicGroupAttackRatio scale for other items
  slotRatios_[ToUnderlying(MagicType::Booster)].rankWeights = {0, 10, 17, 15, 17, 17, 22, 32};
  slotRatios_[ToUnderlying(MagicType::BoosterCritical)].rankWeights = {0, 10, 17, 15, 17, 17, 22, 32};
  slotRatios_[ToUnderlying(MagicType::HotRodding)].rankWeights = {0, 7, 15, 12, 15, 15, 20, 27};
  slotRatios_[ToUnderlying(MagicType::HotRoddingCritical)].rankWeights = {0, 7, 15, 12, 15, 15, 20, 27};
  slotRatios_[ToUnderlying(MagicType::BufGauge)].rankWeights = {0, 0, 0, 0, 0, 0, 5, 5};
  slotRatios_[ToUnderlying(MagicType::BufGaugeCritical)].rankWeights = {0, 0, 0, 0, 0, 0, 5, 5};
  slotRatios_[ToUnderlying(MagicType::BufSpeed)].rankWeights = {0, 0, 0, 0, 0, 0, 20, 20};
  slotRatios_[ToUnderlying(MagicType::BufSpeedCritical)].rankWeights = {0, 0, 0, 0, 0, 0, 20, 20};

  // OFFENSIVE ITEMS (Group 1)
  // Directly from MagicGroupAttackRatio (Table 217):
  // Type 2 (FireBall):  0-15-15-10-10-25-5-10
  // Type 14 (DarkFire): 0-20-17-10-10-15-10-10
  // Type 16 (Summon):   0-5-15-15-15-10-10-5
  // Type 12 (JumpStun): 0-0-0-0-5-25-0-0
  // Type 18 (Lightning): NOT in table -> use legacy reduced weight

  // FireBall - directly from MagicGroupAttackRatio
  slotRatios_[ToUnderlying(MagicType::FireBall)].rankWeights = {0, 15, 15, 10, 10, 25, 5, 10};
  slotRatios_[ToUnderlying(MagicType::FireBallCritical)].rankWeights = {0, 15, 15, 10, 10, 25, 5, 10};

  // DarkFire - directly from MagicGroupAttackRatio
  slotRatios_[ToUnderlying(MagicType::DarkFire)].rankWeights = {0, 20, 17, 10, 10, 15, 10, 10};
  slotRatios_[ToUnderlying(MagicType::DarkFireCritical)].rankWeights = {0, 20, 17, 10, 10, 15, 10, 10};

  // Summon - directly from MagicGroupAttackRatio
  slotRatios_[ToUnderlying(MagicType::Summon)].rankWeights = {0, 5, 15, 15, 15, 10, 10, 5};
  slotRatios_[ToUnderlying(MagicType::SummonCritical)].rankWeights = {0, 5, 15, 15, 15, 10, 10, 5};

  // JumpStun - directly from MagicGroupAttackRatio
  slotRatios_[ToUnderlying(MagicType::JumpStun)].rankWeights = {0, 0, 0, 0, 5, 25, 0, 0};
  slotRatios_[ToUnderlying(MagicType::JumpStunCritical)].rankWeights = {0, 0, 0, 0, 5, 25, 0, 0};

  // Lightning - NOT in MagicGroupAttackRatio
  // Legacy code gives it weight=1 (vs 4 for others), so ~25% of normal offensive items
  // Since it's not in the attack ratio table, use 25% of FireBall/DarkFire ratios
  // FireBall: {0, 15, 15, 10, 10, 25, 5, 10}
  // DarkFire: {0, 20, 17, 10, 10, 15, 10, 10}
  // Lightning (25% of average): {0, 3, 4, 2, 2, 6, 2, 3}
  slotRatios_[ToUnderlying(MagicType::Lightning)].rankWeights = {0, 3, 4, 2, 2, 6, 2, 3};
  slotRatios_[ToUnderlying(MagicType::LightningCritical)].rankWeights = {0, 3, 4, 2, 2, 6, 2, 3};

  // TEAM ASSISTANCE ITEMS
  slotRatios_[ToUnderlying(MagicType::BufPower)].rankWeights = {0, 0, 0, 0, 0, 0, 20, 20};
  slotRatios_[ToUnderlying(MagicType::BufPowerCritical)].rankWeights = {0, 0, 0, 0, 0, 0, 20, 20};
}

void MagicConfig::InitializeTeamModifiers()
{
  for (auto& groupMods : teamModifiers_)
  {
    for (auto& mod : groupMods)
    {
      mod = TeamModifier{};
    }
  }

  teamModifiers_[static_cast<uint32_t>(MagicGroup::Offensive)][0] = {
    .ahead1 = 10, .ahead2 = 10, .ahead3 = 10, .ahead4 = 10, .appliesWhenLeading = false};
  teamModifiers_[static_cast<uint32_t>(MagicGroup::Offensive)][1] = {
    .ahead1 = -10, .ahead2 = -10, .ahead3 = -10, .ahead4 = -10, .appliesWhenLeading = true};
  teamModifiers_[static_cast<uint32_t>(MagicGroup::Defensive)][1] = {
    .ahead1 = 10, .ahead2 = 5, .ahead3 = 0, .ahead4 = 0, .appliesWhenLeading = true};
  teamModifiers_[static_cast<uint32_t>(MagicGroup::SpeedUtility)][1] = {
    .ahead1 = 10, .ahead2 = 5, .ahead3 = 0, .ahead4 = 0, .appliesWhenLeading = true};
  teamModifiers_[static_cast<uint32_t>(MagicGroup::RareOffensive)][1] = {
    .ahead1 = -100, .ahead2 = -100, .ahead3 = -100, .ahead4 = -100, .appliesWhenLeading = true};
  teamModifiers_[static_cast<uint32_t>(MagicGroup::RareUtility)][1] = {
    .ahead1 = -10, .ahead2 = -10, .ahead3 = -20, .ahead4 = -20, .appliesWhenLeading = true};
}

void MagicConfig::InitializeTypeMappings()
{
  typeToGroup_.fill(MagicGroup::Invalid);
  basicTypeToGroup_.fill(MagicGroup::Invalid);

  for (uint32_t t = static_cast<uint32_t>(MagicType::FireBall);
    t < static_cast<uint32_t>(MagicType::MaxTypes);
    ++t)
  {
    allMagicTypes_.push_back(static_cast<MagicType>(t));
  }

  // Offensive Group (1)
  typeToGroup_[ToUnderlying(MagicType::FireBall)] = MagicGroup::Offensive;
  typeToGroup_[ToUnderlying(MagicType::FireBallCritical)] = MagicGroup::Offensive;
  typeToGroup_[ToUnderlying(MagicType::DarkFire)] = MagicGroup::Offensive;
  typeToGroup_[ToUnderlying(MagicType::DarkFireCritical)] = MagicGroup::Offensive;
  typeToGroup_[ToUnderlying(MagicType::Summon)] = MagicGroup::Offensive;
  typeToGroup_[ToUnderlying(MagicType::SummonCritical)] = MagicGroup::Offensive;
  typeToGroup_[ToUnderlying(MagicType::Lightning)] = MagicGroup::Offensive;
  typeToGroup_[ToUnderlying(MagicType::LightningCritical)] = MagicGroup::Offensive;

  // Defensive Group (2)
  typeToGroup_[ToUnderlying(MagicType::WaterShield)] = MagicGroup::Defensive;
  typeToGroup_[ToUnderlying(MagicType::WaterShieldCritical)] = MagicGroup::Defensive;
  typeToGroup_[ToUnderlying(MagicType::IceWall)] = MagicGroup::Defensive;
  typeToGroup_[ToUnderlying(MagicType::IceWallCritical)] = MagicGroup::Defensive;

  // SpeedUtility Group (3)
  typeToGroup_[ToUnderlying(MagicType::Booster)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::BoosterCritical)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::HotRodding)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::HotRoddingCritical)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::BufGauge)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::BufGaugeCritical)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::BufSpeed)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::BufSpeedCritical)] = MagicGroup::SpeedUtility;

  // RareOffensive Group (5)
  typeToGroup_[ToUnderlying(MagicType::JumpStun)] = MagicGroup::RareOffensive;
  typeToGroup_[ToUnderlying(MagicType::JumpStunCritical)] = MagicGroup::RareOffensive;

  // Team buff items
  typeToGroup_[ToUnderlying(MagicType::BufPower)] = MagicGroup::SpeedUtility;
  typeToGroup_[ToUnderlying(MagicType::BufPowerCritical)] = MagicGroup::SpeedUtility;

  // BasicType to Group
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::FireBall)] = MagicGroup::Offensive;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::WaterShield)] = MagicGroup::Defensive;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::Booster)] = MagicGroup::SpeedUtility;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::HotRodding)] = MagicGroup::SpeedUtility;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::IceWall)] = MagicGroup::Defensive;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::JumpStun)] = MagicGroup::RareOffensive;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::DarkFire)] = MagicGroup::Offensive;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::Summon)] = MagicGroup::Offensive;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::Lightning)] = MagicGroup::Offensive;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::BufPower)] = MagicGroup::SpeedUtility;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::BufGauge)] = MagicGroup::SpeedUtility;
  basicTypeToGroup_[static_cast<uint32_t>(BasicType::BufSpeed)] = MagicGroup::SpeedUtility;
}

std::vector<MagicType> MagicConfig::GetValidTypesForContext(const RaceContext& context) const
{
  std::vector<MagicType> validTypes;

  for (MagicType type : allMagicTypes_)
  {
    if (type == MagicType::Invalid)
      continue;

    const AllocationRule* rule = GetAllocationRule(type);

    if (rule && (context.totalPlayers < rule->minPlayerCount ||
                  context.rank < rule->minRank ||
                  context.rank > rule->maxRank))
    {
      continue;
    }

    validTypes.push_back(type);
  }

  return validTypes;
}

} // namespace server::magic
