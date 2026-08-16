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

#ifndef HOUSINGREGISTRY_HPP
#define HOUSINGREGISTRY_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace server::registry
{

//! The number of housing slots a ranch has, one per category.
constexpr uint32_t HousingCategoryCount = 13;

//! The slot the incubators occupy. Incubators never expire: they are spent by
//! hatching instead, and rebuilding one tops its uses back up.
constexpr uint32_t IncubatorCategory = 12;

//! The faction a housing belongs to, from HousingPropertyInfo.
enum class HousingProperty : uint32_t
{
  None = 0,
  CrystalKnights = 1,
  StatoTradingCompany = 2,
  AliciaRidingClub = 3
};

//! An item cost, used for both building and repairing.
struct HousingResource
{
  //! The TID of the item consumed.
  data::Tid itemTid{data::InvalidTid};
  //! How many of that item are consumed.
  uint32_t quantity{};
};

//! One row of HousingInfo.
struct HousingInfo
{
  //! The housing ID, matching `data::Housing::housingId`.
  uint32_t id{};
  //! The ranch slot this housing occupies. A ranch holds one housing per
  //! category, so building here replaces whatever shares the category.
  uint32_t category{};
  //! The faction this housing belongs to.
  HousingProperty property{HousingProperty::None};
  //! Whether the ranch starts with this housing already placed.
  bool isDefault{};
  //! Whether this housing counts as a basic one.
  bool isBasic{};
  //! The character level required before this housing may be built.
  uint32_t minLevel{1};
  //! The `Life` column. 28800 for everything buildable, 0 for the defaults and
  //! the incubators. The unit is unconfirmed - it reads as 8 hours if seconds,
  //! but may be a decaying durability value; nothing consumes it yet.
  std::chrono::seconds lifetime{0};
  //! The items consumed to build this housing.
  std::vector<HousingResource> buildResources;
  //! The item consumed to repair this housing, if it is repairable.
  HousingResource repairResource{};
  //! Ranch experience awarded for building this housing.
  uint32_t buildExp{};
  //! Ranch experience paid out by the recurring ranch bonus.
  uint32_t regularExp{};
  //! Carrots paid out by the recurring ranch bonus.
  uint32_t regularMoney{};
  //! Only meaningful for the incubators (id 51 and 52). Zero everywhere else.
  //! The number of egg slots the incubator provides.
  uint32_t value{};
  //! Hatches granted per build. Zero means the incubator never runs out, which
  //! is how the level 1 incubator works.
  uint32_t value2{};
  //! The most uses the incubator can hold. Rebuilding tops it up by `value2`
  //! and stops here. Zero means no cap.
  uint32_t value3{};
};

//! One row of RanchLevelInfo. Ranch experience is earned by building housing
//! and from the recurring ranch bonus the placed housing pays out.
struct RanchLevelInfo
{
  //! The ranch level this entry describes.
  uint32_t level{};
  //! Total ranch experience required to reach this level.
  uint32_t expRequired{};
};

//! One row of HousingSetInfo. Owning every housing in the set grants its bonus.
struct HousingSetInfo
{
  //! The set ID.
  uint32_t id{};
  //! The housing IDs that make up the set.
  std::vector<uint32_t> housingIds;
  //! Percentage added to the ranch regular bonus while the set is complete.
  uint32_t regularBonusPercent{};
};

class HousingRegistry final
{
public:
  void ReadConfig(const std::filesystem::path& configPath);

  //! @param housingId Housing ID.
  //! @returns The housing, or nullptr if no such ID is configured.
  [[nodiscard]] const HousingInfo* GetHousing(uint32_t housingId) const;

  //! @returns Every configured housing, ordered by ID.
  [[nodiscard]] const std::vector<HousingInfo>& GetAllHousing() const;

  //! @param category Ranch slot.
  //! @returns Every housing that occupies the given slot.
  [[nodiscard]] std::vector<const HousingInfo*> GetHousingByCategory(
    uint32_t category) const;

  //! @returns The housing a ranch starts with, one per category that has a
  //!          default configured.
  [[nodiscard]] std::vector<const HousingInfo*> GetDefaultHousing() const;

  //! @param setId Set ID.
  //! @returns The set, or nullptr if no such ID is configured.
  [[nodiscard]] const HousingSetInfo* GetSet(uint32_t setId) const;

  //! @returns Every configured housing set.
  [[nodiscard]] const std::vector<HousingSetInfo>& GetSets() const;

  //! Totals the bonus of every set the given housing completes.
  //! @param ownedHousingIds The housing IDs currently built on a ranch.
  //! @returns Combined percentage to add to the ranch regular bonus.
  [[nodiscard]] uint32_t GetSetBonusPercent(
    const std::unordered_set<uint32_t>& ownedHousingIds) const;

  //! @param level Ranch level.
  //! @returns The level info, or nullopt if the level is not configured.
  [[nodiscard]] std::optional<RanchLevelInfo> GetRanchLevelInfo(uint32_t level) const;

  //! @param totalExp Total ranch experience earned.
  //! @returns The ranch level that experience reaches, at least 1.
  [[nodiscard]] uint32_t GetRanchLevelForExp(uint32_t totalExp) const;

  //! @param level Ranch level.
  //! @returns Total ranch experience needed to reach it, or nullopt if the
  //!          level is not configured.
  [[nodiscard]] std::optional<uint32_t> GetExpRequiredForRanchLevel(uint32_t level) const;

private:
  std::vector<HousingInfo> _housing;
  //! Index into `_housing` by housing ID.
  std::unordered_map<uint32_t, size_t> _housingById;
  std::vector<HousingSetInfo> _sets;
  //! Ranch level info, sorted by level.
  std::vector<RanchLevelInfo> _ranchLevels;
};

} // namespace server::registry

#endif // HOUSINGREGISTRY_HPP
