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

#ifndef ALICIA_SERVER_RACE_MAGIC_CONFIG_HPP
#define ALICIA_SERVER_RACE_MAGIC_CONFIG_HPP

#include "MagicTypes.hpp"

namespace server::race::magic
{

//! Magic Configuration
//! Static data loaded from XML config files (MagicAllocInfo, MagicGroupRatio, etc.)
class MagicConfig
{
public:
  // Singleton access
  static const MagicConfig& GetInstance()
  {
    static MagicConfig instance;
    return instance;
  }

  // Get allocation rule for a magic type
  const AllocationRule* GetAllocationRule(MagicType type) const;

  // Get group ratio for a magic group
  const GroupRatio* GetGroupRatio(MagicGroup group, bool isTeamMode) const;

  // Get slot ratio for a magic type
  const SlotRatio* GetSlotRatio(MagicType type) const;

  // Get team modifier for a group and lead status
  const TeamModifier* GetTeamModifier(MagicGroup group, bool isLeading) const;

  // Get slot team modifier for a magic type
  const SlotTeamModifier* GetSlotTeamModifier(MagicType type) const;

  // Map magic type to group
  MagicGroup GetGroupForType(MagicType type) const;

  // Map basic type to group
  MagicGroup GetGroupForBasicType(BasicType basicType) const;

  // Determine primary group for a rank
  MagicGroup GetPrimaryGroupForRank(uint32_t rank) const;

  // Get all magic types
  const std::vector<MagicType>& GetAllMagicTypes() const;

  // Get all magic types valid for a given context
  std::vector<MagicType> GetValidTypesForContext(const RaceContext& context) const;

private:
  MagicConfig();
  MagicConfig(const MagicConfig&) = delete;
  MagicConfig& operator=(const MagicConfig&) = delete;

  // Initialize all static data
  void InitializeAllocationRules();
  void InitializeGroupRatios();
  void InitializeSlotRatios();
  void InitializeTeamModifiers();
  void InitializeTypeMappings();

  //! Static data from tables in XML config

  // All magic types in the game
  std::vector<MagicType> allMagicTypes_;

  // MagicAllocInfo (Table 216): Allocation rules per magic type
  std::array<AllocationRule, 26> allocationRules_; // Indexed by MagicType

  // MagicGroupRatio (Table 214): Group probabilities by rank (Solo)
  std::array<GroupRatio, 8> soloGroupRatios_; // Indexed by MagicGroup
  // MagicGroupTeamRatio (Table 355): Group probabilities by rank (Team)
  std::array<GroupRatio, 8> teamGroupRatios_; // Indexed by MagicGroup

  // MagicSlotRatio (Table 199): Slot probabilities by rank
  std::array<SlotRatio, 26> slotRatios_; // Indexed by MagicType

  // MagicGroupTeamModifier (Table 270): Dynamic modifiers
  std::array<std::array<TeamModifier, 2>, 8> teamModifiers_;
  // [group][isLeading] -> TeamModifier

  // MagicSlotTeamModifier (Table 271): Dynamic slot modifiers
  std::array<SlotTeamModifier, 26> slotTeamModifiers_; // Indexed by MagicType

  // Type to group mapping
  std::array<MagicGroup, 26> typeToGroup_;      // [MagicType] -> MagicGroup
  std::array<MagicGroup, 25> basicTypeToGroup_; // [BasicType] -> MagicGroup
};

// Inline implementations
inline const AllocationRule* MagicConfig::GetAllocationRule(MagicType type) const
{
  uint32_t index = ToUnderlying(type);
  if (index < allocationRules_.size())
  {
    return &allocationRules_[index];
  }
  return nullptr;
}

inline const GroupRatio* MagicConfig::GetGroupRatio(MagicGroup group, bool isTeamMode) const
{
  uint32_t index = static_cast<uint32_t>(group);
  if (isTeamMode)
  {
    if (index < teamGroupRatios_.size())
    {
      return &teamGroupRatios_[index];
    }
  }
  else
  {
    if (index < soloGroupRatios_.size())
    {
      return &soloGroupRatios_[index];
    }
  }
  return nullptr;
}

inline const SlotRatio* MagicConfig::GetSlotRatio(MagicType type) const
{
  uint32_t index = ToUnderlying(type);
  if (index < slotRatios_.size())
  {
    return &slotRatios_[index];
  }
  return nullptr;
}

inline const TeamModifier* MagicConfig::GetTeamModifier(MagicGroup group, bool isLeading) const
{
  uint32_t g = static_cast<uint32_t>(group);
  uint32_t l = isLeading ? 1 : 0;
  if (g < teamModifiers_.size() && l < teamModifiers_[g].size())
  {
    return &teamModifiers_[g][l];
  }
  return nullptr;
}

inline const SlotTeamModifier* MagicConfig::GetSlotTeamModifier(MagicType type) const
{
  uint32_t index = ToUnderlying(type);
  if (index < slotTeamModifiers_.size())
  {
    return &slotTeamModifiers_[index];
  }
  return nullptr;
}

inline MagicGroup MagicConfig::GetGroupForType(MagicType type) const
{
  uint32_t index = ToUnderlying(type);
  if (index < typeToGroup_.size())
  {
    return typeToGroup_[index];
  }
  return MagicGroup::Invalid;
}

inline MagicGroup MagicConfig::GetGroupForBasicType(BasicType basicType) const
{
  uint32_t index = static_cast<uint32_t>(basicType);
  if (index < basicTypeToGroup_.size())
  {
    return basicTypeToGroup_[index];
  }
  return MagicGroup::Invalid;
}

inline MagicGroup MagicConfig::GetPrimaryGroupForRank(uint32_t rank) const
{
  if (rank < 1 || rank > 8)
    return MagicGroup::Invalid;

  // Determine primary group based on MagicGroupRatio table
  // For each group, check the weight at this rank
  // Return the group with highest weight
  MagicGroup bestGroup = MagicGroup::Invalid;
  uint32_t bestWeight = 0;

  for (uint32_t g = 1; g < static_cast<uint32_t>(MagicGroup::MaxGroups); ++g)
  {
    const auto* ratio = GetGroupRatio(static_cast<MagicGroup>(g), false);
    if (ratio && ratio->rankWeights[rank - 1] > bestWeight)
    {
      bestWeight = ratio->rankWeights[rank - 1];
      bestGroup = static_cast<MagicGroup>(g);
    }
  }

  return bestGroup;
}

inline const std::vector<MagicType>& MagicConfig::GetAllMagicTypes() const
{
  return allMagicTypes_;
}

} // namespace server::race::magic

#endif // ALICIA_SERVER_RACE_MAGIC_CONFIG_HPP
