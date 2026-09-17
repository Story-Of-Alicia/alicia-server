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

#include "server/system/AchievementSystem.hpp"

#include "libserver/registry/AchievementRegistry.hpp"
#include "server/ServerInstance.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <limits>
#include <optional>

namespace server
{

namespace
{

//! Parses the decimal text a property update carries, which is a plain integer
//! for counters and has two decimal places for distances, times and speeds.
//! @param scale Factor bringing the value into the unit of the thresholds.
//! @param text The reported value, at most 16 characters.
//! @returns The rounded value, or nothing when the text is not a number.
std::optional<uint32_t> ParseReportedValue(
  const double scale,
  const std::string& text)
{
  double value = 0.0;
  const auto* const begin = text.data();
  const auto* const end = begin + text.size();

  const auto [parsedUntil, error] = std::from_chars(begin, end, value);
  if (error != std::errc{} or parsedUntil != end)
    return std::nullopt;

  if (value <= 0.0)
    return 0u;

  const auto rounded = std::llround(value * scale);
  if (rounded > std::numeric_limits<uint32_t>::max())
    return std::numeric_limits<uint32_t>::max();

  return static_cast<uint32_t>(rounded);
}

//! Event the race result reports under. No client ever sends it.
constexpr uint16_t RaceResultEvent = 2;

//! Perfect, good and failed landings. The client sorts every landing into at
//! most one of them, so they add up without counting a jump twice.
constexpr std::array<uint16_t, 3> JumpEvents = {29, 30, 31};

//! Event carrying how often the racer slid.
constexpr uint16_t SlidingCountEvent = 26;

//! What a racer did during the race. The client sends no summary at the end,
//! so the reports made while it ran are the only source.
struct RaceStatistics
{
  //! How often the racer left the ground.
  uint32_t jumpCount{};
  //! How often the racer slid.
  uint32_t slidingCount{};
};

//! Event a run is broken by when it has to hold within a single login. The
//! client neither sends it nor names it; the three achievements resetting on it
//! all ask for races finished within one connection.
constexpr uint16_t SessionEndEvent = 4;

//! Finds the stored progress of an achievement, creating it on first contact.
//! @param character The character record.
//! @param tid The achievement.
//! @returns The entry, which lives in the character record.
data::Character::AchievementEntry& FindOrCreateEntry(
  data::Character& character,
  const uint16_t tid)
{
  auto& achievements = character.achievements();
  const auto entry = std::ranges::find(
    achievements, tid, &data::Character::AchievementEntry::tid);

  if (entry != achievements.end())
    return *entry;

  achievements.push_back({.tid = tid});
  return achievements.back();
}

//! Stamps every tier just crossed, pays out its reward and describes the change.
//! @param characterUid The character, for the log.
//! @param character The character record, for the carrot balance.
//! @param info The achievement.
//! @param entry Its stored progress, already brought up to date.
//! @param tiersBefore Count of tiers earned before the change.
//! @returns What to tell the client about this achievement.
AchievementSystem::ProgressUpdate AwardReachedTiers(
  const data::Uid characterUid,
  data::Character& character,
  const registry::AchievementInfo& info,
  data::Character::AchievementEntry& entry,
  const uint8_t tiersBefore)
{
  const auto tiersAfter = info.GetReachedTierCount(entry.progress);

  for (uint8_t tier = tiersBefore; tier < tiersAfter; ++tier)
  {
    entry.tierEarnedAt[tier] = data::Clock::now();
    character.carrots() += static_cast<int32_t>(info.rewards[tier]);

    spdlog::info(
      "Character '{}' reached tier {} of achievement {} (+{} carrots)",
      characterUid,
      tier,
      info.tid,
      info.rewards[tier]);
  }

  // A tier was just reached, which is not the same as the achievement being
  // done. The client marks the reported tier and plays its ceremony only then.
  const bool reachedTier = tiersAfter > tiersBefore;

  return {
    .achievementTid = info.tid,
    .progress = entry.progress,
    .reachedTier = reachedTier,
    .tier = reachedTier ? static_cast<uint8_t>(tiersAfter - 1) : uint8_t{},
    .carrotBalance = character.carrots()};
}

//! Whether an hour of the day falls into a window, the end being exclusive.
//! @param hour The hour the race ended in.
//! @param from First hour of the window.
//! @param until First hour no longer in the window.
//! @returns True when the hour is inside.
bool WithinHours(const uint8_t hour, const uint8_t from, const uint8_t until)
{
  return hour >= from and hour < until;
}

//! Whether a race satisfies a reset condition, so whether the run is broken.
//! @param function Name of the reset condition.
//! @param resetEventId Event the reset is judged on.
//! @param outcome What the race amounted to.
//! @param supported Set to false when the condition cannot be judged yet.
//! @returns True when the run is broken and the progress goes back to zero.
bool RaceOutcomeBreaksRun(
  const std::string& function,
  const uint16_t resetEventId,
  const AchievementSystem::RaceOutcome& outcome,
  bool& supported)
{
  supported = true;

  // A run held within one login is broken by the session ending, not by a race.
  if (resetEventId == SessionEndEvent)
    return false;

  // Anything that is not a win breaks the run, including giving up.
  if (function == "NotWin")
    return not (outcome.finished and outcome.placement == 1);

  // Losing means reaching the goal behind someone, which giving up is not.
  if (function == "Lose")
    return outcome.finished and outcome.placement != 1;

  if (function == "Retire" or function == "RetireOrDisconnect")
    return not outcome.finished;

  if (function == "GoalIn")
    return outcome.finished;

  // TeamLose and NotTeamWin need the team result, which is only settled once
  // every racer has finished.
  supported = false;
  return false;
}

//! Decides whether a race satisfies the named condition. An unlisted condition
//! never fires, because an achievement handed out wrongly cannot be taken back.
//! 47 of the 84 race achievements are still missing, in five groups:
//! - Team result, 11, settled only once every racer has finished: TeamWin,
//!   TeamPerfectWin, TeamPerfectWinNew, TeamWinTheLast, PerfectWin.
//! - History across races, 8, needing state that outlives one race: MaxWinStop,
//!   SerialWin, PainOfLosses, Magic_TripleWin, Revenge, KingOfLosers.
//! - Course and time, 11: the eight Mastery conditions, GoalInAllMap, WinAllMap.
//! - Never reported by the client, 12: NoCollisionGoalIn, NoNPCCollision,
//!   NoPlayerCollisionRanking, NoSpurRanking, UseSpur40, NoSlidingWin,
//!   GoalInWithSpur, GoalInWithSliding, GoalInWithGliding, StartingWinner,
//!   ReversalWinner, OtherRetirePlayerCount.
//! - Modes this server lacks, 5: WinAI, the three Mission conditions,
//!   CarnivalSuccess.
//! @param function Name of the condition.
//! @param info The achievement, for the parameter of the condition.
//! @param outcome What the race amounted to.
//! @returns True when the race counts for the achievement.
bool RaceOutcomeSatisfies(
  const std::string& function,
  const registry::AchievementInfo& info,
  const AchievementSystem::RaceOutcome& outcome,
  const RaceStatistics& statistics)
{
  // Conditions that read what the racer did during the race. The counts come
  // from the reports of this race, which are cleared when it starts.
  if (function == "NoJumpGoalIn")
    return outcome.finished and statistics.jumpCount == 0;

  if (function == "NoJumpWin")
    return outcome.finished and outcome.placement == 1
      and statistics.jumpCount == 0;

  if (function == "NoJumpRanking")
    return outcome.finished
      and info.functionValue != 0
      and outcome.placement <= info.functionValue
      and statistics.jumpCount == 0;

  if (function == "Sliding20")
    return outcome.finished and statistics.slidingCount >= 20;

  if (function == "GoalIn")
    return outcome.finished;

  if (function == "Retire")
    return not outcome.finished;

  if (function == "Win" or function == "MyFirstWin")
    return outcome.finished and outcome.placement == 1;

  if (function == "GoalInRanking")
    return outcome.finished
      and info.functionValue != 0
      and outcome.placement <= info.functionValue;

  if (function == "Playing20Races")
    return true;

  if (function == "GoalIn_8h_13h")
    return outcome.finished and WithinHours(outcome.hourOfDay, 8, 13);

  if (function == "GoalIn_14h_16h")
    return outcome.finished and WithinHours(outcome.hourOfDay, 14, 16);

  if (function == "GoalIn_16h_18h")
    return outcome.finished and WithinHours(outcome.hourOfDay, 16, 18);

  if (function == "GoalIn_19h_22h")
    return outcome.finished and WithinHours(outcome.hourOfDay, 19, 22);

  return false;
}

} // namespace

AchievementSystem::AchievementSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
  // Empty.
}

protocol::AcCmdRCAchievementUpdateNotify AchievementSystem::BuildUpdateNotify(
  const ProgressUpdate& update)
{
  protocol::AcCmdRCAchievementUpdateNotify notify{};
  notify.achievementTid = update.achievementTid;
  notify.objectiveProgress.isCompleted = update.reachedTier;
  notify.objectiveProgress.progress = update.progress;
  notify.objectiveProgress.achievementTier = update.reachedTier
    ? static_cast<protocol::ObjectiveProgress::AchievementTier>(update.tier)
    : protocol::ObjectiveProgress::AchievementTier::None;
  notify.carrotBalance = update.carrotBalance;

  return notify;
}

std::vector<AchievementSystem::ProgressUpdate>
AchievementSystem::ApplyReportedProperty(
  const data::Uid characterUid,
  const uint16_t eventId,
  const std::string& propertyValue)
{
  std::vector<ProgressUpdate> updates;

  const auto& achievementRegistry = _serverInstance.GetAchievementRegistry();

  const auto reported = ParseReportedValue(
    achievementRegistry.GetReportedValueScale(eventId), propertyValue);
  if (not reported.has_value() and not propertyValue.empty())
  {
    // An empty value is normal, anything else is a gap in our reading of it.
    spdlog::warn(
      "Unparsable achievement property value '{}' on event {}",
      propertyValue,
      eventId);
  }

  // Kept even for an event no achievement listens to, because the statistics of
  // a race are read back from here and count the good and failed jumps, which
  // nothing is triggered by.
  uint32_t lastReportedValue = 0;
  {
    const std::scoped_lock lock(_reportedValuesMutex);
    auto& reportedValues = _reportedValues[characterUid];
    lastReportedValue = reportedValues[eventId];

    if (reported.has_value())
      reportedValues[eventId] = *reported;
  }

  const auto matchingAchievements =
    achievementRegistry.GetAchievementsByEvent(eventId);

  if (matchingAchievements.empty())
    return updates;

  const auto characterRecord =
    _serverInstance.GetDataDirector().GetCharacter(characterUid);

  if (not characterRecord.IsAvailable())
    return updates;

  characterRecord.Mutable(
    [&](data::Character& character)
    {
      for (const auto* const achievementInfo : matchingAchievements)
      {
        if (achievementInfo->conditionKind == registry::ConditionKind::Never)
          continue;

        // Without a value only a plain counter can step, a record or a total
        // would be fabricating a result.
        if (not reported.has_value()
          and achievementInfo->compareType != registry::CompareType::Count)
          continue;

        auto& entry = FindOrCreateEntry(character, achievementInfo->tid);

        const auto tiersBefore = GetEarnedTierCount(entry);
        if (tiersBefore >= entry.tierEarnedAt.size())
          continue;

        if (achievementInfo->IsGated())
        {
          // Count the crossings of the bar, not every step above it.
          if (not reported.has_value()
            or not achievementInfo->PassesBar(*reported)
            or achievementInfo->PassesBar(lastReportedValue))
          {
            continue;
          }

          entry.progress += 1;
        }
        else
        {
          entry.progress = reported.has_value()
            ? achievementInfo->CombineProgress(
                entry.progress, *reported, lastReportedValue)
            : entry.progress + 1;
        }

        updates.push_back(AwardReachedTiers(
          characterUid, character, *achievementInfo, entry, tiersBefore));
      }
    });

  return updates;
}

std::vector<AchievementSystem::ProgressUpdate>
AchievementSystem::ApplyRaceOutcome(
  const data::Uid characterUid,
  const RaceOutcome& outcome)
{
  std::vector<ProgressUpdate> updates;

  // Cleared when the race started, so what is left belongs to this race.
  RaceStatistics statistics;
  {
    const std::scoped_lock lock(_reportedValuesMutex);
    const auto reported = _reportedValues.find(characterUid);
    if (reported != _reportedValues.end())
    {
      for (const auto event : JumpEvents)
      {
        const auto value = reported->second.find(event);
        if (value != reported->second.end())
          statistics.jumpCount += value->second;
      }

      const auto sliding = reported->second.find(SlidingCountEvent);
      if (sliding != reported->second.end())
        statistics.slidingCount = sliding->second;
    }
  }

  const auto& achievementRegistry = _serverInstance.GetAchievementRegistry();
  const auto raceAchievements =
    achievementRegistry.GetAchievementsByEvent(RaceResultEvent);

  const auto characterRecord =
    _serverInstance.GetDataDirector().GetCharacter(characterUid);

  if (not characterRecord.IsAvailable())
    return updates;

  characterRecord.Mutable(
    [&](data::Character& character)
    {
      for (const auto* const achievementInfo : raceAchievements)
      {
        if (not achievementInfo->CountsInMode(outcome.mode))
          continue;

        if (outcome.racerCount < achievementInfo->numPlayer)
          continue;

        // A reset that cannot be judged skips the achievement, since without it
        // "three wins in a row" would count as "three wins".
        bool resetSupported = true;
        bool runBroken = false;
        if (not achievementInfo->resetFunction.empty())
        {
          runBroken = RaceOutcomeBreaksRun(
            achievementInfo->resetFunction,
            achievementInfo->resetEventId,
            outcome,
            resetSupported);

          if (not resetSupported)
            continue;
        }

        const bool satisfied = RaceOutcomeSatisfies(
          achievementInfo->function, *achievementInfo, outcome, statistics);

        if (not satisfied and not runBroken)
          continue;

        auto& entry = FindOrCreateEntry(character, achievementInfo->tid);

        const auto tiersBefore = GetEarnedTierCount(entry);
        if (tiersBefore >= entry.tierEarnedAt.size())
          continue;

        if (runBroken)
        {
          // The run starts over. Earned tiers stay, only the count is lost.
          entry.progress = 0;
          continue;
        }

        // A race counts once, so every one of these is a plain step.
        entry.progress += 1;

        updates.push_back(AwardReachedTiers(
          characterUid, character, *achievementInfo, entry, tiersBefore));
      }
    });

  return updates;
}

void AchievementSystem::ApplySessionEnd(const data::Uid characterUid)
{
  const auto characterRecord =
    _serverInstance.GetDataDirector().GetCharacter(characterUid);

  if (not characterRecord.IsAvailable())
    return;

  characterRecord.Mutable(
    [&](data::Character& character)
    {
      const auto& achievementRegistry =
        _serverInstance.GetAchievementRegistry();

      for (auto& entry : character.achievements())
      {
        const auto* const info = achievementRegistry.GetAchievement(entry.tid);
        if (info == nullptr or info->resetEventId != SessionEndEvent)
          continue;

        // Earned tiers stay, only the run towards the next one is lost.
        entry.progress = 0;
      }
    });
}

void AchievementSystem::ForgetReportedValues(const data::Uid characterUid)
{
  const std::scoped_lock lock(_reportedValuesMutex);
  _reportedValues.erase(characterUid);
}

uint8_t AchievementSystem::GetEarnedTierCount(
  const data::Character::AchievementEntry& entry)
{
  uint8_t earned = 0;
  for (const auto earnedAt : entry.tierEarnedAt)
  {
    if (earnedAt == data::Clock::time_point{})
      break;

    ++earned;
  }

  return earned;
}

uint8_t AchievementSystem::GetBookCompletedTierCount(
  const data::Character& character,
  const int8_t bookType) const
{
  const auto bookAchievements =
    _serverInstance.GetAchievementRegistry().GetAchievementsByBook(bookType);

  if (bookAchievements.empty())
    return 0;

  const auto& achievements = character.achievements();

  // Tiers are earned in order, so the count of tiers reached by the weakest
  // achievement of the book is how far the book itself has come.
  uint8_t completedTiers = registry::AchievementTierCount;
  for (const auto* const bookAchievement : bookAchievements)
  {
    const auto entry = std::ranges::find(
      achievements,
      bookAchievement->tid,
      &data::Character::AchievementEntry::tid);

    if (entry == achievements.end())
      return 0;

    completedTiers = std::min(completedTiers, GetEarnedTierCount(*entry));
    if (completedTiers == 0)
      return 0;
  }

  return completedTiers;
}

uint8_t AchievementSystem::GetBookGrade(
  const data::Character& character,
  const int8_t bookType) const
{
  // The grade is the index of the highest earned tier, not their count. The
  // client picks the book icon by indexing a table of four with it and has no
  // icon for "no grade", so a book without an earned tier still reports bronze.
  const auto completedTiers = GetBookCompletedTierCount(character, bookType);
  if (completedTiers == 0)
    return 0;

  return static_cast<uint8_t>(completedTiers - 1);
}

} // namespace server
