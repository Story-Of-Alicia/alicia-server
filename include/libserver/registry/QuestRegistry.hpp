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

#ifndef QUEST_REGISTRY_HPP
#define QUEST_REGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace server::registry
{

//! An item awarded as part of a quest reward.
struct QuestRewardItem
{
  //! Template ID of the item.
  uint32_t tid{};
  //! Count of the item.
  uint32_t count{};
};

//! Quest reward data loaded from quests.yaml.
struct QuestReward
{
  //! Reward ID.
  uint32_t id{};
  //! Description of the reward.
  std::string desc{};
  //! Carrot currency reward.
  uint32_t carrots{};
  //! Experience reward.
  uint32_t exp{};
  //! Key NPC dress ID.
  uint32_t keyNpcDress{};
  //! Items included in the reward.
  std::vector<QuestRewardItem> items{};
};

//! A single entry in the QuestRewardPoint table.
//! Defines the items awarded when a player's accumulated quest points reach
//! the corresponding threshold.
struct QuestRewardPoint
{
  //! Point threshold (key). The reward is granted at this point value.
  uint32_t point{};
  //! Display name of the reward.
  std::string name{};
  //! Items included in the reward (all non-zero TID entries from the table).
  std::vector<QuestRewardItem> items{};
};

//! Quest data loaded from quests.yaml. Complete Table from  client libconfig_c.dat
struct Quest
{
  //! Classification of a quest by its gameplay role.
  enum class Type : uint32_t
  {
    //! Main story quest
    Main = 0,
    //! Repeatable / Daily Reward
    Repeatable = 1,
    //! Daily quest (resets every day).
    Daily = 2,
    //! Seasonal or limited-time event quest.
    Event = 7,
  };

  //! TID of the quest.
  uint32_t tid{};
  //! Display name of the quest.
  std::string name{};
  //! Description of the quest.
  std::string desc{};
  //! Quest type / group classification.
  Type type{};
  //! Difficulty level.
  uint32_t difficult{};
  //! Required player level.
  uint32_t level{};
  //! Game mode flag (bitmask of applicable race modes for this quest)
  //! Matches DailyQuestInfo::Type values.
  enum class GameModeFlag : uint32_t
  {
    None           = 0,
    SpeedTeam      = 2,
    MagicTeam      = 8,
    WinSpeedSolo   = 33,
    SpeedSoloAction = 35,  //!< Perfect jumps, boosts
    WinMagicSolo   = 68,
    MagicSoloAction = 76,  //!< Bolt attack
    Any            = 111,
  };

  //! Game mode flag (bitmask of applicable race modes for this quest).
  GameModeFlag gameModeFlag{};
  //! NPC ID that starts the quest.
  uint32_t startNpcId{};
  //! NPC ID that ends the quest.
  uint32_t endNpcId{};
  //! Preceding quest TIDs that must be completed first.
  std::vector<uint32_t> preceding{};
  //! Success condition type.
  uint32_t successType{};
  //! Success condition value.
  uint32_t successValue{};
  //! Quest completion function / condition type.
  enum class Function
  {
    Unknown,
    True,                    //!< Used by "complete N races" quests.
    RunMap,                  //!< Complete a specific map (matched against functionValue).
    TeamWin,                 //!< Win a team race.
    PerfectJump,             //!< Land a perfect jump over a hurdle.
    FireballAttack,          //!< Hit an opponent with a fireball.
    CollectDropItem,         //!< Collect a drop item during a race.
    GlidingDistanceValue,    //!< Accumulate gliding distance.
    ClearMission,            //!< Clear a mission stage.
    PrizeWinnerForLowLevel,          //!< Place in the top 3 (low-level variant).
    PrizeWinnerInMapForLowLevel,     //!< Place in the top 3 on a specific map.
  };

  //! Quest completion function / condition type.
  Function function{};
  //! Parameter value for the function (e.g. map ID, count, etc.).
  uint32_t functionValue{};
  //! Linked reward ID (references a QuestReward).
  uint32_t rewardId{};
  //! horse Exp reward.
  uint32_t rewardExp{};
  //! Direct game money reward.
  uint32_t rewardGameMoney{};
  //! Direct point reward.
  uint32_t rewardPoint{};
};

class QuestRegistry
{
public:
  void ReadConfig(const std::filesystem::path& configPath);
  [[nodiscard]] std::optional<Quest> GetQuest(uint32_t tid) const;
  [[nodiscard]] std::optional<QuestReward> GetQuestReward(uint32_t id) const;
  [[nodiscard]] std::optional<QuestRewardPoint> GetQuestRewardPoint(uint32_t point) const;
  [[nodiscard]] const std::unordered_map<uint32_t, Quest>& GetQuests() const;
  [[nodiscard]] const std::unordered_map<uint32_t, QuestReward>& GetQuestRewards() const;
  [[nodiscard]] const std::unordered_map<uint32_t, QuestRewardPoint>& GetQuestRewardPoints() const;

private:
  std::unordered_map<uint32_t, Quest> _quests{};
  std::unordered_map<uint32_t, QuestReward> _rewards{};
  std::unordered_map<uint32_t, QuestRewardPoint> _rewardPoints{};
};

} // namespace server::registry

#endif // QUEST_REGISTRY_HPP
