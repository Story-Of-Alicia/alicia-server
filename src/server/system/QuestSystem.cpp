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

#include "server/system/QuestSystem.hpp"

#include "server/ServerInstance.hpp"

#include <libserver/data/DataDirector.hpp>
#include <libserver/registry/QuestRegistry.hpp>

#include <spdlog/spdlog.h>

namespace server
{

QuestSystem::QuestSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

bool QuestSystem::IsModeMatch(
  const registry::Quest::GameModeFlag questFlag,
  const registry::Quest::GameModeFlag eventMode)
{
  // Flag None (0): no mode restriction — matches everything (ranch activities, etc.)
  if (questFlag == registry::Quest::GameModeFlag::None)
    return true;
  // Flag Any (111): explicitly matches all race modes
  if (questFlag == registry::Quest::GameModeFlag::Any)
    return true;
  return questFlag == eventMode;
}

registry::Quest::GameModeFlag QuestSystem::ToGameModeFlag(
  const protocol::GameMode gameMode,
  const protocol::TeamMode teamMode)
{
  using GameModeFlag = registry::Quest::GameModeFlag;
  const bool isTeam = teamMode == protocol::TeamMode::Team;

  switch (gameMode)
  {
    case protocol::GameMode::Speed:
      return isTeam ? GameModeFlag::SpeedTeam : GameModeFlag::SpeedSoloAction;
    case protocol::GameMode::Magic:
      return isTeam ? GameModeFlag::MagicTeam : GameModeFlag::MagicSoloAction;
    default:
      return GameModeFlag::None;
  }
}

bool QuestSystem::IsEventMatch(
  const registry::Quest::Function function,
  const QuestEvent event,
  const uint32_t questFunctionValue,
  const uint32_t eventValue)
{
  switch (event)
  {
    case QuestEvent::Any:
      return function == registry::Quest::Function::True;
    case QuestEvent::PrizeWinner:
      return function == registry::Quest::Function::PrizeWinnerForLowLevel ||
             function == registry::Quest::Function::PrizeWinnerInMapForLowLevel;
    case QuestEvent::PerfectJump:
      return function == registry::Quest::Function::PerfectJump;
    case QuestEvent::FireballAttack:
      return function == registry::Quest::Function::FireballAttack;
    case QuestEvent::RunMap:
      return function == registry::Quest::Function::RunMap && questFunctionValue == eventValue;
    case QuestEvent::TeamWin:
      return function == registry::Quest::Function::TeamWin;
    case QuestEvent::GlidingDistance:
      return function == registry::Quest::Function::GlidingDistanceValue;
    case QuestEvent::CollectDropItem:
      return function == registry::Quest::Function::CollectDropItem;
    default:
      return false;
  }
}

std::vector<protocol::AcCmdRCUpdateDailyQuestNotify> QuestSystem::OnQuestEvent(
  const data::Uid characterUid,
  const QuestEvent event,
  const registry::Quest::GameModeFlag gameMode,
  const uint32_t value)
{
  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto& questRegistry = _serverInstance.GetQuestRegistry();

  const auto characterRecord = dataDirector.GetCharacter(characterUid);
  if (!characterRecord)
    return {};

  // Read the character's daily quest group UID
  data::Uid dailyQuestGroupUid = data::InvalidUid;
  characterRecord.Immutable([&dailyQuestGroupUid](const data::Character& character)
  {
    dailyQuestGroupUid = character.dailyQuestGroupUid();
  });

  if (dailyQuestGroupUid == data::InvalidUid)
    return {};

  auto questGroupRecord = dataDirector.GetDailyQuestGroup(dailyQuestGroupUid);
  if (!questGroupRecord)
    return {};

  std::vector<protocol::AcCmdRCUpdateDailyQuestNotify> notifies;

  questGroupRecord.Mutable([&](data::DailyQuestGroup& group)
  {
    auto questSlots = group.quests();
    for (auto& entry : questSlots)
    {
      if (entry.questId == 0)
        continue;

      const auto questDef = questRegistry.GetQuest(entry.questId);
      if (!questDef)
        continue;

      // Already satisfied
      if (entry.progress >= questDef->successValue)
        continue;

      // Check game mode and function match
      if (!IsModeMatch(questDef->gameModeFlag, gameMode))
        continue;
      if (!IsEventMatch(questDef->function, event, questDef->functionValue, value))
        continue;

      // Advance progress by 1 (all quest functions are count-based)
      entry.progress = std::min(entry.progress + 1, questDef->successValue);

      const bool completed = entry.progress >= questDef->successValue;

      // Determine reward params — only populated on completion
      const auto rewardType = completed
        ? static_cast<protocol::AcCmdRCUpdateDailyQuestNotify::RewardType>(group.rewardType())
        : protocol::AcCmdRCUpdateDailyQuestNotify::RewardType::None;
      const uint32_t carrotsReward =
        rewardType == protocol::AcCmdRCUpdateDailyQuestNotify::RewardType::Carrots
          ? questDef->rewardGameMoney
          : 0;
      const uint32_t mountExp =
        rewardType == protocol::AcCmdRCUpdateDailyQuestNotify::RewardType::Exp
          ? questDef->rewardExp
          : 0;

      notifies.push_back({
        .characterUid = static_cast<uint32_t>(characterUid),
        .questId = static_cast<uint16_t>(entry.questId),
        .objectiveProgress = {
          .isCompleted = completed,
          .progress = entry.progress,
        },
        .carrotsReward = carrotsReward,
        .rewardType = rewardType,
        .unk2 = 0,
        .mountExp = mountExp,
      });

      if (completed)
      {
        group.rewardPoints = group.rewardPoints() + questDef->rewardPoint;

        spdlog::info(
          "QuestSystem: Character {} completed daily quest {} (rewardPoints now {})",
          characterUid,
          entry.questId,
          group.rewardPoints());
      }
    }

    // Mark quests field as modified so it gets persisted
    group.quests = questSlots;
  });

  return notifies;
}

} // namespace server
