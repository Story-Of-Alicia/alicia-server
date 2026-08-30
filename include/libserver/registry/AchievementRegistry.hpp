/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
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

#ifndef ACHIEVEMENTREGISTRY_HPP
#define ACHIEVEMENTREGISTRY_HPP

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace server::registry
{

//! Count of tiers an achievement can be earned at,
//! bronze, silver, gold and platinum.
constexpr size_t AchievementTierCount = 4;

//! Count of book types the client renders, so the types 0 to 8. The books the
//! libconfig table defines outside that range belong to no page row.
constexpr size_t AchievementBookTypeCount = 9;

//! Game modes an achievement can be restricted to, as a bit set. The client
//! numbers the four normal modes in this order in its room dialog, and the
//! quest table encodes the same modes, see Quest::GameModeFlag.
enum class GameModeFlag : uint8_t
{
  SpeedSingle = 1 << 0,
  SpeedTeam = 1 << 1,
  MagicSingle = 1 << 2,
  MagicTeam = 1 << 3,
  Mission = 1 << 4,
  SpeedAgainstComputer = 1 << 5,
  MagicAgainstComputer = 1 << 6
};

//! Turns the way a race is set up into the bit the achievements are filtered
//! by, so that callers do not have to know the bit order.
//! @param isMagic Whether the race is a magic race rather than a speed one.
//! @param isTeam Whether it is raced in teams.
//! @param isMission Whether it is a mission, which is a mode of its own.
//! @returns The matching bit.
[[nodiscard]] GameModeFlag ToGameModeFlag(
  bool isMagic,
  bool isTeam,
  bool isMission);

//! How a newly reported progress value combines with the stored one. The client
//! never reads this column, so the meanings are read off the shipped data: all
//! but four achievements use Count, and those four are named after their rule.
enum class CompareType : uint8_t
{
  //! Number of times the event happened.
  Count = 0,
  //! Best result, lower being better. Used by the single lap time.
  Minimum = 1,
  //! Best result, higher being better. Used by top speed and spur count.
  Maximum = 2,
  //! Running total. Used by the carrots spent on purchases.
  Sum = 3
};

//! How the reported value turns into progress. Read from
//! achievement-conditions.yaml, which explains where each reading comes from.
enum class ConditionKind : uint8_t
{
  //! The reported value is the progress.
  Direct,
  //! The value has to reach a bar, and the progress counts how often it did.
  AtLeast,
  //! The value has to stay below a bar, counted the same way.
  AtMost,
  //! Nothing ever satisfies this condition.
  Never
};

struct AchievementInfo
{
  //! Achievement TID. References libconfig `Achievements` table.
  uint16_t tid{};
  //! Event whose value the condition measures, which is how an achievement is
  //! found when a report arrives. Defaults to the triggering event; two are
  //! measured by another one, named in achievement-conditions.yaml.
  uint16_t measuredEventId{};
  //! How the value turns into progress.
  ConditionKind conditionKind{ConditionKind::Direct};
  //! Value the measured quantity is held against, for a gated condition.
  uint32_t conditionBar{};
  //! Name of the condition the event has to satisfy, "TRUE" when the event
  //! itself is the condition.
  std::string function{};
  //! Parameter of the condition, zero when it takes none. Used by 22 of them.
  uint32_t functionValue{};
  //! Condition putting the progress back to zero, empty when it accumulates.
  //! The 29 achievements carrying one ask for a run rather than a total.
  std::string resetFunction{};
  //! Event the reset is judged on. Zero means the achievement's own event.
  uint16_t resetEventId{};
  //! Game modes the achievement counts in, as a bit set, see GameModeFlag.
  //! Zero means every mode counts.
  uint8_t gameModeFlag{};
  //! Smallest field the race must have, zero when the size does not matter.
  uint8_t numPlayer{};
  //! Progress threshold of each tier, rising except where less is better, see
  //! IsLowerBetter. A zero marks the tier as unused by this achievement.
  std::array<uint32_t, AchievementTierCount> successValues{};
  //! Carrot reward granted when the tier of the same index is reached.
  std::array<uint32_t, AchievementTierCount> rewards{};
  //! How progress accumulates, see CompareType.
  CompareType compareType{};
  //! Achievement book this belongs to. 1 to 8 are shown in the UI, 0 is the
  //! system only book, -1 the book of completed challenges that no achievement
  //! uses, and -2 no book at all.
  int8_t bookType{};

  //! Whether the achievement counts in the given game mode.
  //! @param mode The mode the race runs in.
  //! @returns True when the mode counts, which it always does for an
  //!          achievement that names no mode at all.
  [[nodiscard]] bool CountsInMode(GameModeFlag mode) const;

  //! Whether the condition holds a reported value against a bar instead of
  //! taking the value as the progress.
  //! @returns True for a gated condition.
  [[nodiscard]] bool IsGated() const;

  //! Whether a reported value satisfies a gated condition.
  //! @param value The measured value.
  //! @returns True when the value passes the bar.
  [[nodiscard]] bool PassesBar(uint32_t value) const;

  //! Whether a lower progress value is the better one, which is the case for
  //! the single lap time achievement and shows in its falling thresholds.
  //! @returns True when less is better.
  [[nodiscard]] bool IsLowerBetter() const;

  //! Returns the count of tiers reached with the given progress,
  //! so zero when not even the first threshold is met.
  //! @param progress The accumulated progress.
  //! @returns The count of tiers reached, at most AchievementTierCount.
  [[nodiscard]] uint8_t GetReachedTierCount(uint32_t progress) const;

  //! Combines a stored progress with a newly reported one. Counts and totals
  //! grow by what happened since the previous report, records keep the better
  //! of the two values.
  //! @param stored The value held so far, zero when there is none.
  //! @param reported The value the client just reported.
  //! @param lastReported The value reported before, zero when there is none.
  //! @returns The value to store.
  [[nodiscard]] uint32_t CombineProgress(
    uint32_t stored,
    uint32_t reported,
    uint32_t lastReported) const;
};

//! Reward for completing a book at a grade. The shipped data grants exactly one
//! item per grade and no money, which is why this carries item ids only.
struct AchievementBookRewardInfo
{
  //! Book type the reward belongs to, in an interval <1, 8>.
  int8_t bookType{};
  //! Character model the reward is meant for, 10 for the boy, 20 for the girl.
  uint8_t characterModelId{};
  //! Item granted for the grade of the same index.
  std::array<uint32_t, AchievementTierCount> itemTids{};
};

class AchievementRegistry
{
public:
  AchievementRegistry();

  //! Reads the generated achievement data.
  //! @param configPath Path of achievements.yaml.
  void ReadConfig(const std::filesystem::path& configPath);

  //! Reads the hand written reading of the conditions and applies it to the
  //! achievements already loaded, so it has to run after ReadConfig.
  //! @param configPath Path of achievement-conditions.yaml.
  void ReadConditions(const std::filesystem::path& configPath);

  //! Get all achievements measured by a given event. The lists are built once
  //! when the configuration is read, since every progress report passes here.
  //! @param eventId The event a report arrived for.
  //! @returns The achievements of that event, empty when there are none.
  [[nodiscard]] std::span<const AchievementInfo* const>
  GetAchievementsByEvent(uint16_t eventId) const;

  //! Get a specific achievement by TID.
  [[nodiscard]] const AchievementInfo* GetAchievement(uint16_t tid) const;

  //! Get all achievements belonging to a book.
  //! @param bookType The book type the achievements reference.
  //! @returns The achievements of that book, empty when there are none.
  [[nodiscard]] std::span<const AchievementInfo* const>
  GetAchievementsByBook(int8_t bookType) const;

  //! Factor a reported value has to be multiplied by to reach the unit its
  //! thresholds use.
  //! @param eventId The event the value was reported for.
  //! @returns The factor, one when the value already arrives in the right unit.
  [[nodiscard]] double GetReportedValueScale(uint16_t eventId) const;

  //! Get the reward of a book for a character model.
  //! @param bookType The book type.
  //! @param characterModelId The character model, 10 or 20.
  //! @returns The reward, or nullptr when the book grants none.
  [[nodiscard]] const AchievementBookRewardInfo* GetBookReward(
    int8_t bookType,
    uint8_t characterModelId) const;

private:
  //! Rebuilds the event index from the measured event of each achievement.
  void RebuildEventIndex();

  //! All achievements keyed by TID.
  std::unordered_map<uint16_t, AchievementInfo> _achievements;
  //! Achievements measured by an event, built once so the hot path allocates
  //! nothing.
  std::unordered_map<uint16_t, std::vector<const AchievementInfo*>> _byEvent;
  //! Achievements of a book, likewise prebuilt.
  std::unordered_map<int8_t, std::vector<const AchievementInfo*>> _byBook;
  //! Factor per event for the values that do not arrive in the unit their
  //! thresholds use.
  std::unordered_map<uint16_t, double> _reportedValueScales;
  //! The book rewards, keyed by book type and character model.
  std::vector<AchievementBookRewardInfo> _bookRewards;
};

} // namespace server::registry

#endif // ACHIEVEMENTREGISTRY_HPP
