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

#ifndef BREEDINGREGISTRY_HPP
#define BREEDINGREGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct FailureCardProbEntry
{
  uint32_t moneySpent{0};
  int32_t probA{0};
  int32_t probB{0};
  int32_t probC{0};
};

struct FailureCardGradeRange
{
  uint32_t grade{0};
  uint32_t minId{0};
  uint32_t maxId{0};
};

struct FailureCardReward
{
  uint32_t itemTid{0};
  uint32_t itemCount{0};
  uint32_t gameMoney{0};
};

struct FailureCardTable
{
  std::vector<FailureCardGradeRange> gradeRanges;
  std::unordered_map<uint32_t, FailureCardReward> rewards;
};

//! Foal grade outcome distribution for a given grade distance between parents.
//! See breeding.yaml -> breeding.genetics.gradeProbabilities.
struct GradeProbabilityRow
{
  //! abs(mareGrade - stallionGrade) this row applies to.
  uint32_t gradeDistance{0};
  //! Probability (%) the foal is 3/2/1 grades below the lower parent.
  float minus3{0.0f};
  float minus2{0.0f};
  float minus1{0.0f};
  //! Probability (%) weights for plus0 (same as lower parent) upwards.
  std::vector<float> plus;
};

//! Tunable parameters for the breeding success roll. See breeding.yaml -> breeding.params.
struct BreedingParams
{
  //! Foal grade is capped at this value.
  int32_t childGradeLimit{8};
  //! Success rate (%) lost per prior breeding of the stallion.
  int32_t successDecayPerBreeding{2};
  //! Minimum breeding success rate (%) floor.
  int32_t minSuccessRate{4};
  //! Probability (%) of a Chance (yellow) failure card.
  int32_t chanceCardChance{15};
  //! +/- variation (%) applied when inheriting foal appearance.
  int32_t appearanceVariation{20};
  //! Coat inheritance-rate bonus (%) granted per breeding-combo success.
  int32_t inheritanceRateBonusUnit{2};
};

//! A grade band for the breeding bonus roll (e.g. small/big grades).
struct BreedingBonusBand
{
  uint32_t minGrade{0};
  uint32_t maxGrade{0};
  //! Probability (%) that a bonus activates for a stallion in this band.
  int32_t activationChance{0};
};

//! A single breeding bonus entry from the BonusProbInfo table.
struct BreedingBonusEntry
{
  uint32_t id{0};
  //! 0 = pregnancy success % increase, 1 = fertility peak level.
  uint32_t type{0};
  uint32_t value{0};
  //! Selection weight when the small grade band activates.
  int32_t ratioSmall{0};
  //! Selection weight when the big grade band activates.
  int32_t ratioBig{0};
};

class BreedingRegistry
{
public:
  void ReadConfig(const std::filesystem::path& configPath);

  //! Returns the probability entry for the given cumulative money spent.
  //! Finds the first entry whose moneySpent >= the given value, or the last entry.
  const FailureCardProbEntry& GetFailureCardProb(uint32_t moneySpent) const;

  //! Returns the grade range for a given grade in the normal card table.
  //! @returns Pointer to FailureCardGradeRange, or nullptr if not found.
  const FailureCardGradeRange* GetNormalCardGradeRange(uint32_t grade) const;

  //! Returns the grade range for a given grade in the chance card table.
  //! @returns Pointer to FailureCardGradeRange, or nullptr if not found.
  const FailureCardGradeRange* GetChanceCardGradeRange(uint32_t grade) const;

  //! Returns the normal card reward for a given reward ID.
  //! @returns Pointer to FailureCardReward, or nullptr if not found.
  const FailureCardReward* GetNormalCardReward(uint32_t rewardId) const;

  //! Returns the chance card reward for a given reward ID.
  //! @returns Pointer to FailureCardReward, or nullptr if not found.
  const FailureCardReward* GetChanceCardReward(uint32_t rewardId) const;

  //! Returns the breeding success/roll tuning parameters.
  const BreedingParams& GetBreedingParams() const;

  //! Returns the foal-grade probability row for the given grade distance.
  //! Clamps to the largest configured distance when out of range.
  const GradeProbabilityRow& GetGradeProbability(uint32_t gradeDistance) const;

  //! Returns the small-grade breeding bonus band.
  const BreedingBonusBand& GetSmallGradeBonusBand() const;

  //! Returns the big-grade breeding bonus band.
  const BreedingBonusBand& GetBigGradeBonusBand() const;

  //! Returns all breeding bonus entries (BonusProbInfo table).
  const std::vector<BreedingBonusEntry>& GetBonusEntries() const;

private:
  std::vector<FailureCardProbEntry> _failureCardProbs;
  FailureCardTable _normalCard;
  FailureCardTable _chanceCard;

  BreedingParams _params;
  std::vector<GradeProbabilityRow> _gradeProbabilities;
  BreedingBonusBand _smallGradeBand;
  BreedingBonusBand _bigGradeBand;
  std::vector<BreedingBonusEntry> _bonusEntries;
};

} // namespace server::registry

#endif // BREEDINGREGISTRY_HPP
