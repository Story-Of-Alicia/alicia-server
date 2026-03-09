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

#include "libserver/registry/QuestRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace server::registry
{

namespace
{

void ReadQuestRewardItem(QuestRewardItem& item, const YAML::Node& yaml)
{
  item.tid = yaml["tid"].as<decltype(QuestRewardItem::tid)>(0);
  item.count = yaml["count"].as<decltype(QuestRewardItem::count)>(0);
}

void ReadQuestReward(QuestReward& reward, const YAML::Node& yaml)
{
  reward.id = yaml["id"].as<decltype(QuestReward::id)>(0);
  reward.desc = yaml["desc"].as<decltype(QuestReward::desc)>("");
  reward.carrots = yaml["carrots"].as<decltype(QuestReward::carrots)>(0);
  reward.exp = yaml["exp"].as<decltype(QuestReward::exp)>(0);
  reward.keyNpcDress = yaml["keyNpcDress"].as<decltype(QuestReward::keyNpcDress)>(0);

  if (const auto itemsNode = yaml["items"])
  {
    for (const auto& itemNode : itemsNode)
    {
      QuestRewardItem rewardItem{};
      ReadQuestRewardItem(rewardItem, itemNode);
      reward.items.push_back(rewardItem);
    }
  }
}

void ReadQuest(Quest& quest, const YAML::Node& yaml)
{
  quest.tid = yaml["tid"].as<decltype(Quest::tid)>(0);
  quest.name = yaml["name"].as<decltype(Quest::name)>("");
  quest.desc = yaml["desc"].as<decltype(Quest::desc)>("");
  quest.type = static_cast<Quest::Type>(yaml["groupType"].as<uint32_t>(0));
  quest.difficult = yaml["difficult"].as<decltype(Quest::difficult)>(0);
  quest.level = yaml["level"].as<decltype(Quest::level)>(0);
  quest.gameModeFlag = yaml["gameModeFlag"].as<decltype(Quest::gameModeFlag)>(0);
  quest.startNpcId = yaml["startNpcId"].as<decltype(Quest::startNpcId)>(0);
  quest.endNpcId = yaml["endNpcId"].as<decltype(Quest::endNpcId)>(0);
  quest.successType = yaml["successType"].as<decltype(Quest::successType)>(0);
  quest.successValue = yaml["successValue"].as<decltype(Quest::successValue)>(0);
  quest.function = yaml["function"].as<decltype(Quest::function)>("");
  quest.functionValue = yaml["functionValue"].as<decltype(Quest::functionValue)>(0);
  quest.rewardId = yaml["rewardId"].as<decltype(Quest::rewardId)>(0);
  quest.rewardExp = yaml["rewardExp"].as<decltype(Quest::rewardExp)>(0);
  quest.rewardGameMoney = yaml["rewardGameMoney"].as<decltype(Quest::rewardGameMoney)>(0);
  quest.rewardPoint = yaml["rewardPoint"].as<decltype(Quest::rewardPoint)>(0);

  if (const auto precedingNode = yaml["preceding"])
  {
    for (const auto& entry : precedingNode)
      quest.preceding.push_back(entry.as<uint32_t>(0));
  }
}

} // anonymous namespace

void QuestRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto questsSection = root["quests"];
  if (not questsSection)
    throw std::runtime_error("Missing quests section");

  const auto rewardsSection = questsSection["rewards"];
  if (not rewardsSection)
    throw std::runtime_error("Missing quests.rewards section");

  const auto collectionSection = questsSection["collection"];
  if (not collectionSection)
    throw std::runtime_error("Missing quests.collection section");

  _rewards.clear();
  _quests.clear();

  for (const auto& rewardNode : rewardsSection)
  {
    QuestReward reward{};
    ReadQuestReward(reward, rewardNode);
    _rewards.try_emplace(reward.id, reward);
  }

  for (const auto& questNode : collectionSection)
  {
    Quest quest{};
    ReadQuest(quest, questNode);
    _quests.try_emplace(quest.tid, quest);
  }

  spdlog::info(
    "Quest registry loaded {} quests and {} rewards",
    _quests.size(),
    _rewards.size());
}

std::optional<Quest> QuestRegistry::GetQuest(uint32_t tid) const
{
  const auto iter = _quests.find(tid);
  if (iter == _quests.cend())
    return std::nullopt;
  return iter->second;
}

std::optional<QuestReward> QuestRegistry::GetQuestReward(uint32_t id) const
{
  const auto iter = _rewards.find(id);
  if (iter == _rewards.cend())
    return std::nullopt;
  return iter->second;
}

const std::unordered_map<uint32_t, Quest>& QuestRegistry::GetQuests() const
{
  return _quests;
}

const std::unordered_map<uint32_t, QuestReward>& QuestRegistry::GetQuestRewards() const
{
  return _rewards;
}

} // namespace server::registry
