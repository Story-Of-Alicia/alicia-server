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

#include "server/system/FestivalSystem.hpp"

#include "server/ServerInstance.hpp"
#include "server/tracker/RaceTracker.hpp"

#include <libserver/util/Util.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iterator>
#include <random>
#include <ranges>
#include <stdexcept>
#include <thread>
#include <spdlog/spdlog.h>

namespace server
{

namespace
{

constexpr uint32_t NeutralVarianceBasisPoints = 10000;
constexpr uint32_t MinimumVarianceBasisPoints = 9700;
constexpr uint32_t MaximumVarianceBasisPoints = 10300;
constexpr uint32_t VisibleStatBase = 100;
constexpr auto FestivalCycleTestDuration = std::chrono::minutes(10); // Set to zero for production.

uint64_t GetRarityScore(const uint32_t tier, const uint32_t maximumTier)
{
  if (tier > maximumTier)
    return 0;

  switch (tier)
  {
    case 1:
      return 500;
    case 2:
      return 1500;
    case 3:
      return 2500;
    default:
      return 0;
  }
}

data::Clock::time_point GetNextLocalMidnight(const data::Clock::time_point now)
{
  const std::time_t nowTime = data::Clock::to_time_t(now);
  std::tm localTime{};
#ifdef _WIN32
  if (localtime_s(&localTime, &nowTime) != 0)
    throw std::runtime_error("Failed to resolve the local festival cycle time");
#else
  if (localtime_r(&nowTime, &localTime) == nullptr)
    throw std::runtime_error("Failed to resolve the local festival cycle time");
#endif

  localTime.tm_mday += 1;
  localTime.tm_hour = 0;
  localTime.tm_min = 0;
  localTime.tm_sec = 0;
  localTime.tm_isdst = -1;

  const std::time_t midnight = std::mktime(&localTime);
  if (midnight == std::time_t{-1})
    throw std::runtime_error("Failed to calculate the next festival cycle time");
  return data::Clock::from_time_t(midnight);
}

data::Clock::time_point GetCycleEnd(const data::Clock::time_point now)
{
  if constexpr (FestivalCycleTestDuration > std::chrono::minutes::zero())
    return now + FestivalCycleTestDuration;
  return GetNextLocalMidnight(now);
}

uint32_t GetFestivalDate(const data::Clock::time_point timePoint)
{
  const std::time_t time = data::Clock::to_time_t(timePoint);
  std::tm localTime{};
#ifdef _WIN32
  if (localtime_s(&localTime, &time) != 0)
    throw std::runtime_error("Failed to resolve the local festival result date");
#else
  if (localtime_r(&time, &localTime) == nullptr)
    throw std::runtime_error("Failed to resolve the local festival result date");
#endif

  return static_cast<uint32_t>(
    (localTime.tm_year + 1900) * 10000
    + (localTime.tm_mon + 1) * 100
    + localTime.tm_mday);
}

uint32_t GetRewardCarrots(const uint32_t rank)
{
  if (rank == 1)
    return 170000;
  if (rank <= 3)
    return 150000;
  if (rank <= 8)
    return 120000;
  return 80000;
}

} // anon namespace

FestivalSystem::FestivalSystem(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void FestivalSystem::Initialize()
{
  std::scoped_lock lock(_mutex);

  _activeCycleUid = data::InvalidUid;
  _cycleEndsAt = {};
  _groups.clear();
  _admittedHorseUids.clear();
  _admissionsByCycle.clear();
  _admissionByClaimUid.clear();
  _admissionByResultMailUid.clear();
  _deletedAdmissionsByCycle.clear();
  _awaitingResultCycleUids.clear();
  _rankedCycleUids.clear();
  _rewardCreationOffsets.clear();
  _pendingParticipationMails.clear();
  _pendingParticipationMailUids.clear();
  _pendingResultMails.clear();
  _pendingResultMailUids.clear();
  _pendingCleanupAdmissions.clear();
  _pendingCleanupAdmissionUids.clear();

  auto& dataDirector = _serverInstance.GetDataDirector();
  std::vector<data::Uid> collectingCycleUids;
  std::unordered_set<data::Uid> resolvedCycleUids;
  std::unordered_set<data::Uid> knownCycleUids;
  auto cycleUids = dataDirector.ListFestivalCycles();
  auto cycleIterator = cycleUids.begin();
  while (not cycleUids.empty())
  {
    if (cycleIterator == cycleUids.end())
      cycleIterator = cycleUids.begin();

    const auto cycleUid = *cycleIterator;
    const auto cycleRecord = dataDirector.GetFestivalCycleCache().Get(cycleUid);
    if (not cycleRecord)
    {
      if (dataDirector.GetFestivalCycleCache().GetRetrieveFailureCount(cycleUid) != 0)
      {
        spdlog::error("Skipping unavailable festival cycle '{}'", cycleUid);
        cycleIterator = cycleUids.erase(cycleIterator);
        continue;
      }
      ++cycleIterator;
      std::this_thread::yield();
      continue;
    }

    knownCycleUids.insert(cycleUid);
    cycleRecord->Immutable(
      [this, &collectingCycleUids, &resolvedCycleUids](const data::FestivalCycle& cycle)
      {
        if (cycle.state() == data::FestivalCycle::State::Collecting
          && cycle.uid() > _activeCycleUid)
        {
          _activeCycleUid = cycle.uid();
          _cycleEndsAt = cycle.endsAt();
        }
        if (cycle.state() == data::FestivalCycle::State::Collecting)
          collectingCycleUids.push_back(cycle.uid());
        else if (cycle.state() == data::FestivalCycle::State::AwaitingResults)
          _awaitingResultCycleUids.push_back(cycle.uid());
        else if (cycle.state() == data::FestivalCycle::State::Resolved)
          resolvedCycleUids.insert(cycle.uid());
      });
    cycleIterator = cycleUids.erase(cycleIterator);
  }

  const auto now = Clock::now();
  if (_activeCycleUid == data::InvalidUid)
  {
    CreateCycle(now);
  }
  else if (_cycleEndsAt == Clock::time_point{}
    || (FestivalCycleTestDuration > std::chrono::minutes::zero()
      && now < _cycleEndsAt
      && _cycleEndsAt - now > FestivalCycleTestDuration))
  {
    _cycleEndsAt = GetCycleEnd(now);
    dataDirector.GetFestivalCycle(_activeCycleUid).Mutable([this](data::FestivalCycle& cycle)
    {
      cycle.endsAt() = _cycleEndsAt;
    });
  }

  for (const auto cycleUid : collectingCycleUids)
  {
    if (cycleUid == _activeCycleUid)
      continue;

    dataDirector.GetFestivalCycle(cycleUid).Mutable([](data::FestivalCycle& cycle)
    {
      cycle.state() = data::FestivalCycle::State::AwaitingResults;
    });
    _awaitingResultCycleUids.push_back(cycleUid);
  }

  auto admissionUids = dataDirector.ListFestivalAdmissions();
  const auto maximumGroupCount = admissionUids.size();
  std::vector<data::Uid> rewardClaimUids;
  auto admissionIterator = admissionUids.begin();
  while (not admissionUids.empty())
  {
    if (admissionIterator == admissionUids.end())
      admissionIterator = admissionUids.begin();

    const auto admissionUid = *admissionIterator;
    const auto admissionRecord = dataDirector.GetFestivalAdmissionCache().Get(admissionUid);
    if (not admissionRecord)
    {
      if (dataDirector.GetFestivalAdmissionCache().GetRetrieveFailureCount(admissionUid) != 0)
      {
        spdlog::error("Skipping unavailable festival admission '{}'", admissionUid);
        admissionIterator = admissionUids.erase(admissionIterator);
        continue;
      }
      ++admissionIterator;
      std::this_thread::yield();
      continue;
    }

    data::Uid cycleUid{data::InvalidUid};
    data::Uid characterUid{data::InvalidUid};
    data::Uid horseUid{data::InvalidUid};
    data::Uid participationMailUid{data::InvalidUid};
    data::Uid rewardClaimUid{data::InvalidUid};
    data::Uid resultMailUid{data::InvalidUid};
    uint32_t groupIndex{};
    uint32_t slotIndex{};
    admissionRecord->Immutable(
      [&cycleUid,
       &characterUid,
       &horseUid,
       &participationMailUid,
       &rewardClaimUid,
       &resultMailUid,
       &groupIndex,
       &slotIndex](
        const data::FestivalAdmission& admission)
      {
        cycleUid = admission.cycleUid();
        characterUid = admission.characterUid();
        horseUid = admission.horseUid();
        participationMailUid = admission.participationMailUid();
        rewardClaimUid = admission.rewardClaimUid();
        resultMailUid = admission.resultMailUid();
        groupIndex = admission.groupIndex();
        slotIndex = admission.slotIndex();
      });

    if (cycleUid == data::InvalidUid || not knownCycleUids.contains(cycleUid))
    {
      spdlog::warn(
        "Ignoring festival admission '{}' with unavailable cycle '{}'",
        admissionUid,
        cycleUid);
      admissionIterator = admissionUids.erase(admissionIterator);
      continue;
    }

    _admissionsByCycle[cycleUid].push_back(admissionUid);
    if (rewardClaimUid != data::InvalidUid)
    {
      rewardClaimUids.push_back(rewardClaimUid);
      _admissionByClaimUid[rewardClaimUid] = admissionUid;
      if (resultMailUid == data::InvalidUid && resolvedCycleUids.contains(cycleUid))
        QueueResultMail(admissionUid);
    }
    if (resultMailUid != data::InvalidUid)
      _admissionByResultMailUid[resultMailUid] = admissionUid;
    if (resolvedCycleUids.contains(cycleUid))
      QueueCleanupAdmission(admissionUid);

    if (cycleUid == _activeCycleUid)
    {
      const bool invalidPosition = groupIndex >= maximumGroupCount
        || slotIndex >= MaxParticipantsPerGroup;
      if (characterUid == data::InvalidUid
        || horseUid == data::InvalidUid
        || invalidPosition)
      {
        spdlog::warn("Ignoring invalid festival admission '{}'", admissionUid);
      }
      else
      {
        if (_groups.size() <= groupIndex)
          _groups.resize(groupIndex + 1);

        auto& group = _groups[groupIndex];
        auto& slot = group.admissions[slotIndex];
        if (slot != data::InvalidUid
          || group.characterUids.contains(characterUid)
          || _admittedHorseUids.contains(horseUid))
        {
          spdlog::warn("Ignoring duplicate festival admission '{}'", admissionUid);
        }
        else
        {
          slot = admissionUid;
          group.characterUids.insert(characterUid);
          _admittedHorseUids.insert(horseUid);
          if (participationMailUid == data::InvalidUid)
            QueueParticipationMail(admissionUid);
        }
      }
    }
    else if (participationMailUid == data::InvalidUid)
    {
      QueueParticipationMail(admissionUid);
    }
    admissionIterator = admissionUids.erase(admissionIterator);
  }

  for (const auto claimUid : rewardClaimUids)
    dataDirector.GetRewardCache().Get(claimUid);

  if (now >= _cycleEndsAt)
    RotateCycle(now);

  _nextMailRetry = std::chrono::steady_clock::now();
}

void FestivalSystem::Terminate()
{
  std::scoped_lock lock(_mutex);
  _activeCycleUid = data::InvalidUid;
  _cycleEndsAt = {};
  _groups.clear();
  _admittedHorseUids.clear();
  _admissionsByCycle.clear();
  _admissionByClaimUid.clear();
  _admissionByResultMailUid.clear();
  _deletedAdmissionsByCycle.clear();
  _awaitingResultCycleUids.clear();
  _rankedCycleUids.clear();
  _rewardCreationOffsets.clear();
  _pendingParticipationMails.clear();
  _pendingParticipationMailUids.clear();
  _pendingResultMails.clear();
  _pendingResultMailUids.clear();
  _pendingCleanupAdmissions.clear();
  _pendingCleanupAdmissionUids.clear();
}

void FestivalSystem::Tick()
{
  std::scoped_lock lock(_mutex);
  const auto systemNow = Clock::now();
  if (_activeCycleUid != data::InvalidUid && systemNow >= _cycleEndsAt)
    RotateCycle(systemNow);

  size_t rewardCreationBudget = MaxRewardCreationsPerTick;
  size_t processedCycleCount{};
  auto cycleIterator = _awaitingResultCycleUids.begin();
  while (cycleIterator != _awaitingResultCycleUids.end()
    && processedCycleCount < MaxCyclesPerTick)
  {
    ++processedCycleCount;
    if (ResolveCycle(*cycleIterator, systemNow, rewardCreationBudget))
      cycleIterator = _awaitingResultCycleUids.erase(cycleIterator);
    else
      ++cycleIterator;

    if (rewardCreationBudget == 0)
      break;
  }

  const auto steadyNow = std::chrono::steady_clock::now();
  if (steadyNow < _nextMailRetry)
    return;

  const size_t participationMailCount = std::min(
    MaxMailsPerTick,
    _pendingParticipationMails.size());
  for (size_t index = 0; index < participationMailCount; ++index)
  {
    const auto admissionUid = _pendingParticipationMails.front();
    _pendingParticipationMails.pop_front();
    _pendingParticipationMailUids.erase(admissionUid);
    if (not SendParticipationMail(admissionUid))
      QueueParticipationMail(admissionUid);
  }

  const size_t resultMailCount = std::min(MaxMailsPerTick, _pendingResultMails.size());
  for (size_t index = 0; index < resultMailCount; ++index)
  {
    const auto admissionUid = _pendingResultMails.front();
    _pendingResultMails.pop_front();
    _pendingResultMailUids.erase(admissionUid);
    if (not SendResultMail(admissionUid))
      QueueResultMail(admissionUid);
  }

  const size_t cleanupCount = std::min(
    MaxCleanupRecordsPerTick,
    _pendingCleanupAdmissions.size());
  for (size_t index = 0; index < cleanupCount; ++index)
  {
    const auto admissionUid = _pendingCleanupAdmissions.front();
    _pendingCleanupAdmissions.pop_front();
    _pendingCleanupAdmissionUids.erase(admissionUid);
    if (not CleanupAdmission(admissionUid))
      QueueCleanupAdmission(admissionUid);
  }
  _nextMailRetry = steadyNow + std::chrono::seconds(1);
}

bool FestivalSystem::IsEnabled() const
{
  return _serverInstance
    .GetSystemContentRegistry()
    .GetValue(registry::SystemContentRegistry::Key::FestivalActivation)
    .value_or(1) != 0;
}

std::chrono::seconds FestivalSystem::GetCycleTimeRemaining() const
{
  std::scoped_lock lock(_mutex);
  if (_activeCycleUid == data::InvalidUid)
    return std::chrono::seconds::zero();

  return std::max(
    std::chrono::ceil<std::chrono::seconds>(_cycleEndsAt - Clock::now()),
    std::chrono::seconds::zero());
}

bool FestivalSystem::EvaluateServerMission(
  const uint32_t missionType,
  tracker::RaceTracker& raceTracker,
  const data::Uid characterUid) const
{
  switch (static_cast<MissionType>(missionType))
  {
    case MissionType::DoNotTimeOut:
    {
      const auto& racer = raceTracker.GetRacer(characterUid);
      return racer.state != tracker::RaceTracker::Racer::State::Disconnected
        && racer.courseTime != tracker::InvalidCourseTime;
    }
    // TODO: Implement the remaining server-checked festival missions.
    case MissionType::FinishWithinFinalCountdown:
    case MissionType::AcquireFieldMagic:
    case MissionType::FinishWithMagic:
    case MissionType::WinTeamRace:
    case MissionType::EveryoneFinishes:
    case MissionType::EveryoneStaysOnCourse:
    default:
      return false;
  }
}

data::Uid FestivalSystem::SelectAuditionParticipant(
  const std::span<const AuditionParticipant> qualifiedParticipants)
{
  std::scoped_lock lock(_mutex);
  const auto now = Clock::now();
  if (_activeCycleUid != data::InvalidUid && now >= _cycleEndsAt)
    RotateCycle(now);
  if (_activeCycleUid == data::InvalidUid)
    return data::InvalidUid;

  std::vector<AuditionParticipant> eligibleParticipants;
  std::ranges::copy_if(
    qualifiedParticipants,
    std::back_inserter(eligibleParticipants),
    [this](const AuditionParticipant& participant)
    {
      return participant.characterUid != data::InvalidUid
        && participant.horseUid != data::InvalidUid
        && not _admittedHorseUids.contains(participant.horseUid);
    });
  std::ranges::shuffle(eligibleParticipants, server::util::GetRandomEngine());

  auto& dataDirector = _serverInstance.GetDataDirector();
  for (const auto& participant : eligibleParticipants)
  {
    const auto characterRecord = dataDirector.GetCharacter(participant.characterUid);
    const auto horseRecord = dataDirector.GetHorse(participant.horseUid);
    if (not characterRecord || not horseRecord)
      continue;

    std::string characterName;
    bool horseIsMounted = false;
    characterRecord.Immutable(
      [&characterName, &horseIsMounted, &participant](const data::Character& character)
      {
        characterName = character.name();
        horseIsMounted = character.mountUid() == participant.horseUid;
      });
    if (not horseIsMounted)
      continue;

    std::string horseName;
    uint32_t horseGrade{};
    data::FestivalAdmission::GradingSnapshot grading;
    horseRecord.Immutable(
      [this, &horseName, &horseGrade, &grading](const data::Horse& horse)
      {
        horseName = horse.name();
        horseGrade = horse.grade();
        grading.totalStats() = horse.stats.agility()
          + horse.stats.courage()
          + horse.stats.rush()
          + horse.stats.endurance()
          + horse.stats.ambition();
        grading.bodyDirtiness() = horse.mountCondition.bodyDirtiness();
        grading.maneDirtiness() = horse.mountCondition.maneDirtiness();
        grading.tailDirtiness() = horse.mountCondition.tailDirtiness();
        grading.friendliness() = horse.mountCondition.friendliness();
        grading.charm() = horse.mountCondition.charm();
        grading.skinTier() = static_cast<uint32_t>(
          _serverInstance.GetHorseRegistry().GetCoatInfo(horse.parts.skinTid()).tier);
        grading.maneTier() = static_cast<uint32_t>(
          _serverInstance.GetHorseRegistry().GetMane(horse.parts.maneTid()).tier);
        grading.tailTier() = static_cast<uint32_t>(
          _serverInstance.GetHorseRegistry().GetTail(horse.parts.tailTid()).tier);
      });

    uint32_t groupIndex{};
    uint32_t slotIndex{};
    bool foundSlot = false;
    for (size_t currentGroupIndex = 0;
      currentGroupIndex < _groups.size() && not foundSlot;
      ++currentGroupIndex)
    {
      for (size_t currentSlotIndex = 0;
        currentSlotIndex < MaxParticipantsPerGroup;
        ++currentSlotIndex)
      {
        auto& group = _groups[currentGroupIndex];
        if (not group.characterUids.contains(participant.characterUid)
          && group.admissions[currentSlotIndex] == data::InvalidUid)
        {
          groupIndex = static_cast<uint32_t>(currentGroupIndex);
          slotIndex = static_cast<uint32_t>(currentSlotIndex);
          foundSlot = true;
          break;
        }
      }
    }

    if (not foundSlot)
    {
      _groups.emplace_back();
      groupIndex = static_cast<uint32_t>(_groups.size() - 1);
      slotIndex = 0;
    }

    auto admissionRecord = dataDirector.CreateFestivalAdmission();
    if (not admissionRecord)
      continue;

    data::Uid admissionUid{data::InvalidUid};
    admissionRecord.Mutable(
      [this,
       &admissionUid,
       groupIndex,
       slotIndex,
       &participant,
       &characterName,
       &horseName,
       horseGrade,
       &grading](data::FestivalAdmission& admission)
      {
        admissionUid = admission.uid();
        admission.cycleUid() = _activeCycleUid;
        admission.groupIndex() = groupIndex;
        admission.slotIndex() = slotIndex;
        admission.characterUid() = participant.characterUid;
        admission.characterName() = characterName;
        admission.horseUid() = participant.horseUid;
        admission.horseName() = horseName;
        admission.horseGrade() = horseGrade;
        admission.grading.totalStats() = grading.totalStats();
        admission.grading.bodyDirtiness() = grading.bodyDirtiness();
        admission.grading.maneDirtiness() = grading.maneDirtiness();
        admission.grading.tailDirtiness() = grading.tailDirtiness();
        admission.grading.friendliness() = grading.friendliness();
        admission.grading.charm() = grading.charm();
        admission.grading.skinTier() = grading.skinTier();
        admission.grading.maneTier() = grading.maneTier();
        admission.grading.tailTier() = grading.tailTier();
      });

    _admissionsByCycle[_activeCycleUid].push_back(admissionUid);

    _groups[groupIndex].admissions[slotIndex] = admissionUid;
    _groups[groupIndex].characterUids.insert(participant.characterUid);
    _admittedHorseUids.insert(participant.horseUid);
    QueueParticipationMail(admissionUid);
    _nextMailRetry = std::chrono::steady_clock::now();

    return participant.characterUid;
  }

  return data::InvalidUid;
}

FestivalSystem::GradingResult FestivalSystem::GradeAdmission(
  const data::FestivalAdmission::GradingSnapshot& grading)
{
  uint64_t baseScore = static_cast<uint64_t>(
    grading.totalStats() + VisibleStatBase) * 105;
  baseScore += static_cast<uint64_t>(grading.friendliness()) * 10;
  baseScore += static_cast<uint64_t>(grading.charm()) * 10;

  if (grading.bodyDirtiness() == 0)
    baseScore += 1000;
  if (grading.maneDirtiness() == 0)
    baseScore += 1000;
  if (grading.tailDirtiness() == 0)
    baseScore += 1000;

  baseScore += GetRarityScore(grading.skinTier(), 3);
  baseScore += GetRarityScore(grading.maneTier(), 2);
  baseScore += GetRarityScore(grading.tailTier(), 2);

  std::uniform_int_distribution<uint32_t> varianceDistribution(
    MinimumVarianceBasisPoints,
    MaximumVarianceBasisPoints);
  const uint32_t varianceBasisPoints = varianceDistribution(
    server::util::GetRandomEngine());
  const uint64_t finalScore =
    (baseScore * varianceBasisPoints + NeutralVarianceBasisPoints / 2)
    / NeutralVarianceBasisPoints;

  return {
    .baseScore = baseScore,
    .varianceBasisPoints = varianceBasisPoints,
    .finalScore = finalScore};
}

void FestivalSystem::CreateCycle(const Clock::time_point now)
{
  auto cycleRecord = _serverInstance.GetDataDirector().CreateFestivalCycle();
  if (not cycleRecord)
    throw std::runtime_error("Failed to create the active festival cycle");

  _cycleEndsAt = GetCycleEnd(now);
  cycleRecord.Mutable([this](data::FestivalCycle& cycle)
  {
    cycle.state() = data::FestivalCycle::State::Collecting;
    cycle.endsAt() = _cycleEndsAt;
    _activeCycleUid = cycle.uid();
  });
  _admissionsByCycle.try_emplace(_activeCycleUid);
}

void FestivalSystem::RotateCycle(const Clock::time_point now)
{
  const auto cycleRecord = _serverInstance
    .GetDataDirector()
    .GetFestivalCycle(_activeCycleUid);
  if (not cycleRecord)
    throw std::runtime_error("Active festival cycle record is unavailable");

  const auto completedCycleUid = _activeCycleUid;
  const bool hasAdmissions = not _admissionsByCycle[completedCycleUid].empty();
  if (not hasAdmissions)
  {
    _cycleEndsAt = GetCycleEnd(now);
    cycleRecord.Mutable([this](data::FestivalCycle& cycle)
    {
      cycle.endsAt() = _cycleEndsAt;
    });
    return;
  }

  cycleRecord.Mutable([](data::FestivalCycle& cycle)
  {
    cycle.state() = data::FestivalCycle::State::AwaitingResults;
  });

  if (not std::ranges::contains(_awaitingResultCycleUids, completedCycleUid))
  {
    _awaitingResultCycleUids.push_back(completedCycleUid);
  }

  _groups.clear();
  _admittedHorseUids.clear();
  CreateCycle(now);
}

bool FestivalSystem::ResolveCycle(
  const data::Uid cycleUid,
  const Clock::time_point resolvedAt,
  size_t& rewardCreationBudget)
{
  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto cycleRecord = dataDirector.GetFestivalCycle(cycleUid);
  if (not cycleRecord)
    return false;

  Clock::time_point resultTime{};
  cycleRecord.Mutable([resolvedAt, &resultTime](data::FestivalCycle& cycle)
  {
    if (cycle.resolvedAt() == Clock::time_point{})
      cycle.resolvedAt() = resolvedAt;
    resultTime = cycle.resolvedAt();
  });

  const auto cycleAdmissions = _admissionsByCycle.find(cycleUid);
  if (cycleAdmissions == _admissionsByCycle.end()
    || cycleAdmissions->second.empty())
  {
    dataDirector.GetFestivalCycleCache().Delete(cycleUid);
    _admissionsByCycle.erase(cycleUid);
    return true;
  }

  if (not _rankedCycleUids.contains(cycleUid))
  {
    struct RankedAdmission
    {
      data::Uid admissionUid{data::InvalidUid};
      uint64_t baseScore{};
      uint64_t finalScore{};
    };

    std::unordered_map<uint32_t, std::vector<RankedAdmission>> rankedGroups;
    for (const auto admissionUid : cycleAdmissions->second)
    {
      const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionUid);
      if (not admissionRecord)
      {
        if (dataDirector.GetFestivalAdmissionCache()
          .GetRetrieveFailureCount(admissionUid) == 0)
        {
          return false;
        }
        spdlog::error(
          "Skipping unavailable festival admission '{}' while resolving cycle '{}'",
          admissionUid,
          cycleUid);
        continue;
      }

      uint32_t groupIndex{};
      uint32_t varianceBasisPoints{};
      uint64_t baseScore{};
      uint64_t finalScore{};
      data::FestivalAdmission::GradingSnapshot grading;
      admissionRecord.Immutable(
        [&groupIndex,
         &varianceBasisPoints,
         &baseScore,
         &finalScore,
         &grading](const data::FestivalAdmission& admission)
        {
          groupIndex = admission.groupIndex();
          varianceBasisPoints = admission.varianceBasisPoints();
          baseScore = admission.baseScore();
          finalScore = admission.finalScore();
          grading.totalStats() = admission.grading.totalStats();
          grading.bodyDirtiness() = admission.grading.bodyDirtiness();
          grading.maneDirtiness() = admission.grading.maneDirtiness();
          grading.tailDirtiness() = admission.grading.tailDirtiness();
          grading.friendliness() = admission.grading.friendliness();
          grading.charm() = admission.grading.charm();
          grading.skinTier() = admission.grading.skinTier();
          grading.maneTier() = admission.grading.maneTier();
          grading.tailTier() = admission.grading.tailTier();
        });

      if (varianceBasisPoints == 0)
      {
        const auto gradingResult = GradeAdmission(grading);
        baseScore = gradingResult.baseScore;
        finalScore = gradingResult.finalScore;
        admissionRecord.Mutable([gradingResult](data::FestivalAdmission& admission)
        {
          admission.baseScore() = gradingResult.baseScore;
          admission.varianceBasisPoints() = gradingResult.varianceBasisPoints;
          admission.finalScore() = gradingResult.finalScore;
        });
      }

      rankedGroups[groupIndex].push_back({
        .admissionUid = admissionUid,
        .baseScore = baseScore,
        .finalScore = finalScore});
    }

    if (rankedGroups.empty())
    {
      cycleRecord.Mutable([resultTime](data::FestivalCycle& cycle)
      {
        cycle.state() = data::FestivalCycle::State::Resolved;
        cycle.resolvedAt() = resultTime;
      });
      return true;
    }

    for (auto& group : rankedGroups | std::views::values)
    {
      std::ranges::sort(group, [](const RankedAdmission& lhs, const RankedAdmission& rhs)
      {
        if (lhs.finalScore != rhs.finalScore)
          return lhs.finalScore > rhs.finalScore;
        if (lhs.baseScore != rhs.baseScore)
          return lhs.baseScore > rhs.baseScore;
        return lhs.admissionUid < rhs.admissionUid;
      });

      for (size_t index = 0; index < group.size(); ++index)
      {
        const uint32_t rank = static_cast<uint32_t>(index + 1);
        const uint32_t rewardCarrots = GetRewardCarrots(rank);
        dataDirector.GetFestivalAdmission(group[index].admissionUid).Mutable(
          [rank, rewardCarrots](data::FestivalAdmission& admission)
          {
            admission.rank() = rank;
            admission.rewardCarrots() = rewardCarrots;
          });
      }
    }

    _rankedCycleUids.insert(cycleUid);
    _rewardCreationOffsets[cycleUid] = 0;
  }

  auto& rewardCreationOffset = _rewardCreationOffsets[cycleUid];
  while (rewardCreationOffset < cycleAdmissions->second.size())
  {
    const auto admissionUid = cycleAdmissions->second[rewardCreationOffset];
    const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionUid);
    if (not admissionRecord)
    {
      if (dataDirector.GetFestivalAdmissionCache()
        .GetRetrieveFailureCount(admissionUid) == 0)
      {
        return false;
      }

      spdlog::error(
        "Skipping unavailable festival admission '{}' while creating rewards",
        admissionUid);
      ++rewardCreationOffset;
      continue;
    }

    if (not EnsureAdmissionReward(admissionUid, rewardCreationBudget))
      return false;

    ++rewardCreationOffset;
    if (rewardCreationBudget == 0
      && rewardCreationOffset < cycleAdmissions->second.size())
    {
      return false;
    }
  }

  cycleRecord.Mutable([resultTime](data::FestivalCycle& cycle)
  {
    cycle.state() = data::FestivalCycle::State::Resolved;
    cycle.resolvedAt() = resultTime;
  });

  for (const auto admissionUid : cycleAdmissions->second)
  {
    const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionUid);
    if (not admissionRecord)
      continue;

    data::Uid resultMailUid{data::InvalidUid};
    admissionRecord.Immutable([&resultMailUid](const data::FestivalAdmission& admission)
    {
      resultMailUid = admission.resultMailUid();
    });
    if (resultMailUid == data::InvalidUid)
      QueueResultMail(admissionUid);
    else
      QueueCleanupAdmission(admissionUid);
  }

  _rankedCycleUids.erase(cycleUid);
  _rewardCreationOffsets.erase(cycleUid);
  spdlog::info(
    "Resolved festival cycle '{}' with {} admissions",
    cycleUid,
    cycleAdmissions->second.size());
  return true;
}

bool FestivalSystem::EnsureAdmissionReward(
  const data::Uid admissionUid,
  size_t& rewardCreationBudget)
{
  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionUid);
  if (not admissionRecord)
    return false;

  data::Uid characterUid{data::InvalidUid};
  data::Uid claimUid{data::InvalidUid};
  uint32_t rewardCarrots{};
  bool rewardCreationPending{};
  admissionRecord.Immutable(
    [&characterUid, &claimUid, &rewardCarrots, &rewardCreationPending](
      const data::FestivalAdmission& admission)
    {
      characterUid = admission.characterUid();
      claimUid = admission.rewardClaimUid();
      rewardCarrots = admission.rewardCarrots();
      rewardCreationPending = admission.rewardCreationPending();
    });

  if (claimUid != data::InvalidUid)
  {
    const auto rewardRecord = dataDirector.GetReward(claimUid);
    if (rewardRecord)
    {
      _admissionByClaimUid[claimUid] = admissionUid;
      if (rewardCreationPending)
      {
        admissionRecord.Mutable([](data::FestivalAdmission& admission)
        {
          admission.rewardCreationPending() = false;
        });
      }
      return true;
    }

    if (dataDirector.GetRewardCache().GetRetrieveFailureCount(claimUid) == 0)
      return false;

    spdlog::error(
      "Festival admission '{}' references unavailable reward '{}'",
      admissionUid,
      claimUid);
    rewardCreationPending = true;
  }

  if (rewardCreationPending)
  {
    const auto existingClaimUid = dataDirector.FindRewardClaimUid(
      data::Reward::Type::Carnival,
      admissionUid);
    if (existingClaimUid != data::InvalidUid)
    {
      admissionRecord.Mutable([existingClaimUid](data::FestivalAdmission& admission)
      {
        admission.rewardClaimUid() = existingClaimUid;
        admission.rewardCreationPending() = false;
      });
      if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
        return false;

      _admissionByClaimUid[existingClaimUid] = admissionUid;
      dataDirector.GetRewardCache().Get(existingClaimUid);
      return false;
    }
  }

  if (rewardCreationBudget == 0)
    return false;

  admissionRecord.Mutable([](data::FestivalAdmission& admission)
  {
    admission.rewardClaimUid() = data::InvalidUid;
    admission.rewardCreationPending() = true;
  });
  if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
    return false;

  --rewardCreationBudget;
  claimUid = _serverInstance.GetRewardSystem().CreateReward(
    characterUid,
    data::Reward::Type::Carnival,
    rewardCarrots,
    admissionUid);
  if (claimUid == data::InvalidUid)
    return false;

  admissionRecord.Mutable([claimUid](data::FestivalAdmission& admission)
  {
    admission.rewardClaimUid() = claimUid;
    admission.rewardCreationPending() = false;
  });
  if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
    return false;

  _admissionByClaimUid[claimUid] = admissionUid;
  return true;
}

void FestivalSystem::QueueParticipationMail(const data::Uid admissionUid)
{
  if (admissionUid != data::InvalidUid
    && _pendingParticipationMailUids.insert(admissionUid).second)
  {
    _pendingParticipationMails.push_back(admissionUid);
  }
}

void FestivalSystem::QueueResultMail(const data::Uid admissionUid)
{
  if (admissionUid != data::InvalidUid
    && _pendingResultMailUids.insert(admissionUid).second)
  {
    _pendingResultMails.push_back(admissionUid);
  }
}

void FestivalSystem::QueueCleanupAdmission(const data::Uid admissionUid)
{
  if (admissionUid != data::InvalidUid
    && _pendingCleanupAdmissionUids.insert(admissionUid).second)
  {
    _pendingCleanupAdmissions.push_back(admissionUid);
  }
}

bool FestivalSystem::CleanupAdmission(const data::Uid admissionUid)
{
  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionUid);
  if (not admissionRecord)
  {
    return dataDirector.GetFestivalAdmissionCache()
      .GetRetrieveFailureCount(admissionUid) != 0;
  }

  data::Uid cycleUid{data::InvalidUid};
  data::Uid claimUid{data::InvalidUid};
  data::Uid resultMailUid{data::InvalidUid};
  admissionRecord.Immutable(
    [&cycleUid, &claimUid, &resultMailUid](const data::FestivalAdmission& admission)
    {
      cycleUid = admission.cycleUid();
      claimUid = admission.rewardClaimUid();
      resultMailUid = admission.resultMailUid();
    });

  if (resultMailUid == data::InvalidUid)
    return true;

  const auto mailRecord = dataDirector.GetMail(resultMailUid);
  if (not mailRecord)
  {
    if (dataDirector.GetMailCache().GetRetrieveFailureCount(resultMailUid) == 0)
      return false;

    spdlog::error(
      "Festival admission '{}' references unavailable result mail '{}'",
      admissionUid,
      resultMailUid);
    return true;
  }

  bool isDeleted{};
  mailRecord.Immutable([&isDeleted](const data::Mail& mail)
  {
    isDeleted = mail.isDeleted();
  });
  if (not isDeleted)
    return true;

  _admissionByResultMailUid.erase(resultMailUid);
  if (claimUid != data::InvalidUid)
  {
    _admissionByClaimUid.erase(claimUid);
    dataDirector.GetRewardCache().Delete(claimUid);
  }

  const auto cycleAdmissions = _admissionsByCycle.find(cycleUid);
  if (cycleAdmissions == _admissionsByCycle.end())
    return true;

  auto& deletedAdmissions = _deletedAdmissionsByCycle[cycleUid];
  deletedAdmissions.insert(admissionUid);
  if (deletedAdmissions.size() < cycleAdmissions->second.size())
    return true;

  for (const auto cycleAdmissionUid : cycleAdmissions->second)
  {
    const auto cycleAdmissionRecord = dataDirector.GetFestivalAdmission(cycleAdmissionUid);
    if (cycleAdmissionRecord)
    {
      data::Uid cycleClaimUid{data::InvalidUid};
      data::Uid cycleResultMailUid{data::InvalidUid};
      cycleAdmissionRecord.Immutable(
        [&cycleClaimUid, &cycleResultMailUid](const data::FestivalAdmission& admission)
        {
          cycleClaimUid = admission.rewardClaimUid();
          cycleResultMailUid = admission.resultMailUid();
        });
      _admissionByClaimUid.erase(cycleClaimUid);
      _admissionByResultMailUid.erase(cycleResultMailUid);
      if (cycleClaimUid != data::InvalidUid)
        dataDirector.GetRewardCache().Delete(cycleClaimUid);
    }
    dataDirector.GetFestivalAdmissionCache().Delete(cycleAdmissionUid);
  }

  dataDirector.GetFestivalCycleCache().Delete(cycleUid);
  _admissionsByCycle.erase(cycleAdmissions);
  _deletedAdmissionsByCycle.erase(cycleUid);
  _rankedCycleUids.erase(cycleUid);
  _rewardCreationOffsets.erase(cycleUid);
  std::erase(_awaitingResultCycleUids, cycleUid);
  return true;
}

void FestivalSystem::HandleResultMailDeleted(const data::Uid mailUid)
{
  std::scoped_lock lock(_mutex);
  const auto admission = _admissionByResultMailUid.find(mailUid);
  if (admission != _admissionByResultMailUid.end())
    QueueCleanupAdmission(admission->second);
}

std::optional<FestivalSystem::Result> FestivalSystem::GetResult(
  const data::Uid claimUid,
  const data::Uid characterUid) const
{
  std::scoped_lock lock(_mutex);

  const auto admissionIterator = _admissionByClaimUid.find(claimUid);
  if (admissionIterator == _admissionByClaimUid.end())
    return std::nullopt;

  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionIterator->second);
  if (not admissionRecord)
    return std::nullopt;

  Result result;
  data::Uid cycleUid{data::InvalidUid};
  uint32_t groupIndex{};
  admissionRecord.Immutable(
    [&result, &cycleUid, &groupIndex, characterUid](const data::FestivalAdmission& admission)
    {
      if (admission.characterUid() != characterUid)
        return;

      cycleUid = admission.cycleUid();
      groupIndex = admission.groupIndex();
      result.claimUid = admission.rewardClaimUid();
      result.horseGrade = static_cast<uint8_t>(
        std::clamp(admission.horseGrade(), uint32_t{1}, uint32_t{255}));
      result.totalStats = admission.grading.totalStats();
      result.characterName = admission.characterName();
      result.horseName = admission.horseName();
      result.group = static_cast<uint8_t>(std::min(groupIndex + 1, uint32_t{255}));
      result.round = 1;
      result.rewardCarrots = admission.rewardCarrots();
    });

  if (cycleUid == data::InvalidUid || result.claimUid != claimUid)
    return std::nullopt;

  const auto cycleRecord = dataDirector.GetFestivalCycle(cycleUid);
  if (not cycleRecord)
    return std::nullopt;

  bool isResolved = false;
  Clock::time_point resultTime{};
  cycleRecord.Immutable([&isResolved, &resultTime](const data::FestivalCycle& cycle)
  {
    isResolved = cycle.state() == data::FestivalCycle::State::Resolved;
    resultTime = cycle.resolvedAt() == Clock::time_point{}
      ? cycle.endsAt()
      : cycle.resolvedAt();
  });
  if (not isResolved)
    return std::nullopt;
  result.date = GetFestivalDate(resultTime);

  const auto rewardRecord = dataDirector.GetReward(claimUid);
  if (rewardRecord)
  {
    rewardRecord.Immutable([&result, characterUid](const data::Reward& reward)
    {
      result.rewardAvailable = reward.characterUid() == characterUid
        && reward.type() == data::Reward::Type::Carnival
        && not reward.isClaimed();
    });
  }

  const auto cycleAdmissions = _admissionsByCycle.find(cycleUid);
  if (cycleAdmissions == _admissionsByCycle.end())
    return result;

  for (const auto admissionUid : cycleAdmissions->second)
  {
    const auto entryRecord = dataDirector.GetFestivalAdmission(admissionUid);
    if (not entryRecord)
      continue;

    ResultEntry entry;
    uint32_t entryGroupIndex{};
    uint32_t entryRank{};
    entryRecord.Immutable(
      [&entry, &entryGroupIndex, &entryRank](const data::FestivalAdmission& admission)
      {
        entryGroupIndex = admission.groupIndex();
        entryRank = admission.rank();
        entry.characterName = admission.characterName();
        entry.horseName = admission.horseName();
        entry.horseGrade = static_cast<uint8_t>(
          std::clamp(admission.horseGrade(), uint32_t{1}, uint32_t{255}));
        entry.totalStats = admission.grading.totalStats();
        entry.rewardCarrots = admission.rewardCarrots();
        entry.rank = static_cast<uint8_t>(std::min(entryRank, uint32_t{255}));
      });

    if (entryGroupIndex == groupIndex
      && entryRank > 0
      && entryRank <= MaxLeaderboardEntries)
    {
      result.leaderboard.push_back(std::move(entry));
    }
  }

  std::ranges::sort(result.leaderboard, {}, &ResultEntry::rank);
  return result;
}

std::optional<uint32_t> FestivalSystem::ClaimPrize(
  const data::Uid claimUid,
  const data::Uid characterUid)
{
  std::scoped_lock lock(_mutex);

  const auto admissionIterator = _admissionByClaimUid.find(claimUid);
  if (admissionIterator == _admissionByClaimUid.end())
    return std::nullopt;

  const auto admissionRecord = _serverInstance
    .GetDataDirector()
    .GetFestivalAdmission(admissionIterator->second);
  if (not admissionRecord)
    return std::nullopt;

  bool ownsResult = false;
  admissionRecord.Immutable(
    [&ownsResult, characterUid, claimUid](const data::FestivalAdmission& admission)
    {
      ownsResult = admission.characterUid() == characterUid
        && admission.rewardClaimUid() == claimUid
        && admission.rank() > 0;
    });
  if (not ownsResult)
    return std::nullopt;

  const auto rewardRecord = _serverInstance.GetDataDirector().GetReward(claimUid);
  if (not rewardRecord)
    return std::nullopt;

  bool isCarnivalReward = false;
  uint32_t rewardCarrots{};
  rewardRecord.Immutable([&isCarnivalReward, &rewardCarrots](const data::Reward& reward)
  {
    isCarnivalReward = reward.type() == data::Reward::Type::Carnival;
    rewardCarrots = reward.carrots();
  });
  if (not isCarnivalReward)
    return std::nullopt;

  if (not _serverInstance.GetRewardSystem().ClaimReward(claimUid, characterUid))
    return std::nullopt;

  QueueCleanupAdmission(admissionIterator->second);
  return rewardCarrots;
}

bool FestivalSystem::SendParticipationMail(const data::Uid admissionUid)
{
  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionUid);
  if (not admissionRecord)
    return false;

  data::Uid characterUid{data::InvalidUid};
  data::Uid mailUid{data::InvalidUid};
  bool mailCreationPending{};
  std::string horseName;
  admissionRecord.Immutable(
    [&characterUid, &mailUid, &mailCreationPending, &horseName](
      const data::FestivalAdmission& admission)
    {
      characterUid = admission.characterUid();
      mailUid = admission.participationMailUid();
      mailCreationPending = admission.participationMailCreationPending();
      horseName = admission.horseName();
    });

  if (mailUid != data::InvalidUid)
    return true;

  const auto characterRecord = dataDirector.GetCharacter(characterUid);
  if (not characterRecord)
    return false;

  if (mailCreationPending)
    mailUid = dataDirector.FindMailUid(data::Mail::MailType::NoReply, admissionUid);
  if (mailUid != data::InvalidUid)
  {
    const auto mailRecord = dataDirector.GetMail(mailUid);
    if (not mailRecord)
      return false;

    bool isDeleted{};
    mailRecord.Immutable([&isDeleted](const data::Mail& mail)
    {
      isDeleted = mail.isDeleted();
    });
    if (not isDeleted)
    {
      characterRecord.Mutable([mailUid](data::Character& character)
      {
        if (not std::ranges::contains(character.mailbox.inbox(), mailUid))
          character.mailbox.inbox().insert(character.mailbox.inbox().begin(), mailUid);
        character.mailbox.hasNewMail() = true;
      });
      if (not dataDirector.GetCharacterCache().StoreNow(characterUid))
        return false;
    }

    admissionRecord.Mutable([mailUid](data::FestivalAdmission& admission)
    {
      admission.participationMailUid() = mailUid;
      admission.participationMailCreationPending() = false;
    });
    return dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid);
  }

  if (not mailCreationPending)
  {
    admissionRecord.Mutable([](data::FestivalAdmission& admission)
    {
      admission.participationMailCreationPending() = true;
    });
    if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
      return false;
  }

  try
  {
    mailUid = _serverInstance
      .GetMessengerDirector()
      .SendFestivalParticipation(characterUid, horseName, admissionUid);
    if (mailUid == data::InvalidUid)
      return false;

    admissionRecord.Mutable([mailUid](data::FestivalAdmission& admission)
    {
      admission.participationMailUid() = mailUid;
      admission.participationMailCreationPending() = false;
    });
    return dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid);
  }
  catch (const std::exception& x)
  {
    spdlog::error(
      "Failed to send participation mail for festival admission '{}': {}",
      admissionUid,
      x.what());
    return false;
  }
}

bool FestivalSystem::SendResultMail(const data::Uid admissionUid)
{
  auto& dataDirector = _serverInstance.GetDataDirector();
  const auto admissionRecord = dataDirector.GetFestivalAdmission(admissionUid);
  if (not admissionRecord)
    return false;

  data::Uid characterUid{data::InvalidUid};
  data::Uid claimUid{data::InvalidUid};
  data::Uid mailUid{data::InvalidUid};
  bool mailCreationPending{};
  std::string horseName;
  admissionRecord.Immutable(
    [&characterUid, &claimUid, &mailUid, &mailCreationPending, &horseName](
      const data::FestivalAdmission& admission)
    {
      characterUid = admission.characterUid();
      claimUid = admission.rewardClaimUid();
      mailUid = admission.resultMailUid();
      mailCreationPending = admission.resultMailCreationPending();
      horseName = admission.horseName();
    });

  if (mailUid != data::InvalidUid)
    return true;
  if (characterUid == data::InvalidUid || claimUid == data::InvalidUid)
    return false;

  const auto characterRecord = dataDirector.GetCharacter(characterUid);
  if (not characterRecord)
    return false;

  if (mailCreationPending)
  {
    mailUid = dataDirector.FindMailUid(
      data::Mail::MailType::CarnivalReward,
      admissionUid);
  }
  if (mailUid != data::InvalidUid)
  {
    const auto mailRecord = dataDirector.GetMail(mailUid);
    if (not mailRecord)
      return false;

    bool isDeleted{};
    mailRecord.Immutable([&isDeleted](const data::Mail& mail)
    {
      isDeleted = mail.isDeleted();
    });
    if (not isDeleted)
    {
      characterRecord.Mutable([mailUid](data::Character& character)
      {
        if (not std::ranges::contains(character.mailbox.inbox(), mailUid))
          character.mailbox.inbox().insert(character.mailbox.inbox().begin(), mailUid);
        character.mailbox.hasNewMail() = true;
      });
      if (not dataDirector.GetCharacterCache().StoreNow(characterUid))
        return false;
    }

    admissionRecord.Mutable([mailUid](data::FestivalAdmission& admission)
    {
      admission.resultMailUid() = mailUid;
      admission.resultMailCreationPending() = false;
    });
    if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
      return false;
    _admissionByResultMailUid[mailUid] = admissionUid;
    if (isDeleted)
      QueueCleanupAdmission(admissionUid);
    return true;
  }

  std::vector<data::Uid> inbox;
  characterRecord.Immutable([&inbox](const data::Character& character)
  {
    inbox.assign(
      character.mailbox.inbox().begin(),
      character.mailbox.inbox().end());
  });

  for (const auto inboxMailUid : inbox)
  {
    const auto mailRecord = dataDirector.GetMail(inboxMailUid);
    if (not mailRecord)
    {
      if (dataDirector.GetMailCache().GetRetrieveFailureCount(inboxMailUid) == 0)
        return false;
      spdlog::warn(
        "Skipping unavailable mail '{}' while recovering festival admission '{}'",
        inboxMailUid,
        admissionUid);
      continue;
    }

    bool isResultMail = false;
    mailRecord.Immutable([&isResultMail, claimUid](const data::Mail& mail)
    {
      isResultMail = mail.type() == data::Mail::MailType::CarnivalReward
        && mail.claimUid() == claimUid;
    });
    if (not isResultMail)
      continue;

    admissionRecord.Mutable([inboxMailUid](data::FestivalAdmission& admission)
    {
      admission.resultMailUid() = inboxMailUid;
      admission.resultMailCreationPending() = false;
    });
    if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
      return false;
    _admissionByResultMailUid[inboxMailUid] = admissionUid;
    return true;
  }

  if (not mailCreationPending)
  {
    admissionRecord.Mutable([](data::FestivalAdmission& admission)
    {
      admission.resultMailCreationPending() = true;
    });
    if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
      return false;
  }

  try
  {
    mailUid = _serverInstance
      .GetMessengerDirector()
      .SendFestivalResult(characterUid, horseName, claimUid, admissionUid);
    if (mailUid == data::InvalidUid)
      return false;

    admissionRecord.Mutable([mailUid](data::FestivalAdmission& admission)
    {
      admission.resultMailUid() = mailUid;
      admission.resultMailCreationPending() = false;
    });
    if (not dataDirector.GetFestivalAdmissionCache().StoreNow(admissionUid))
      return false;
    _admissionByResultMailUid[mailUid] = admissionUid;
    return true;
  }
  catch (const std::exception& x)
  {
    spdlog::error(
      "Failed to send result mail for festival admission '{}': {}",
      admissionUid,
      x.what());
    return false;
  }
}

} // namespace server
