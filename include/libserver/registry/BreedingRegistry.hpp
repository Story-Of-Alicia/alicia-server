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

private:
  std::vector<FailureCardProbEntry> _failureCardProbs;
  FailureCardTable _normalCard;
  FailureCardTable _chanceCard;
};

} // namespace server::registry

#endif // BREEDINGREGISTRY_HPP
