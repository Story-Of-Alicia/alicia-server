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

#ifndef FESTIVALSYSTEM_HPP
#define FESTIVALSYSTEM_HPP

#include <libserver/data/DataDefinitions.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace server
{

class ServerInstance;

namespace tracker
{
class RaceTracker;
}

class FestivalSystem
{
public:
  struct AuditionParticipant
  {
    data::Uid characterUid{data::InvalidUid};
    data::Uid horseUid{data::InvalidUid};
  };

  struct ResultEntry
  {
    std::string characterName;
    std::string horseName;
    uint8_t horseGrade{};
    uint32_t totalStats{};
    uint32_t rewardCarrots{};
    uint8_t rank{};
  };

  struct Result
  {
    uint32_t date{};
    data::Uid claimUid{data::InvalidUid};
    uint8_t horseGrade{};
    uint32_t totalStats{};
    std::string characterName;
    std::string horseName;
    uint8_t group{};
    uint8_t round{};
    uint32_t rewardCarrots{};
    bool rewardAvailable{};
    std::vector<ResultEntry> leaderboard;
  };

  using Clock = data::Clock;

  enum class MissionType : uint32_t
  {
    DoNotTimeOut = 3,
    FinishWithinFinalCountdown = 4,
    AcquireFieldMagic = 7,
    FinishWithMagic = 9,
    WinTeamRace = 15,
    EveryoneFinishes = 16,
    EveryoneStaysOnCourse = 17
  };

  explicit FestivalSystem(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

  [[nodiscard]] bool IsEnabled() const;
  [[nodiscard]] std::chrono::seconds GetCycleTimeRemaining() const;

  [[nodiscard]] bool EvaluateServerMission(
    uint32_t missionType,
    tracker::RaceTracker& raceTracker,
    data::Uid characterUid) const;

  [[nodiscard]] data::Uid SelectAuditionParticipant(
    std::span<const AuditionParticipant> qualifiedParticipants);

  [[nodiscard]] std::optional<Result> GetResult(
    data::Uid claimUid,
    data::Uid characterUid) const;

  [[nodiscard]] std::optional<uint32_t> ClaimPrize(
    data::Uid claimUid,
    data::Uid characterUid);

  void HandleResultMailDeleted(data::Uid mailUid);

private:
  static constexpr size_t MaxParticipantsPerGroup = 50;
  static constexpr size_t MaxLeaderboardEntries = 8;
  static constexpr size_t MaxCyclesPerTick = 5;
  static constexpr size_t MaxRewardCreationsPerTick = 10;
  static constexpr size_t MaxMailsPerTick = 10;
  static constexpr size_t MaxCleanupRecordsPerTick = 25;

  struct GradingResult
  {
    uint64_t baseScore{};
    uint32_t varianceBasisPoints{};
    uint64_t finalScore{};
  };

  struct Group
  {
    std::array<data::Uid, MaxParticipantsPerGroup> admissions{};
    std::unordered_set<data::Uid> characterUids;
  };

  void CreateCycle(Clock::time_point now);
  void RotateCycle(Clock::time_point now);
  bool ResolveCycle(
    data::Uid cycleUid,
    Clock::time_point resolvedAt,
    size_t& rewardCreationBudget);
  [[nodiscard]] GradingResult GradeAdmission(
    const data::FestivalAdmission::GradingSnapshot& grading);
  bool EnsureAdmissionReward(data::Uid admissionUid, size_t& rewardCreationBudget);
  void QueueParticipationMail(data::Uid admissionUid);
  void QueueResultMail(data::Uid admissionUid);
  void QueueCleanupAdmission(data::Uid admissionUid);
  bool CleanupAdmission(data::Uid admissionUid);
  bool SendParticipationMail(data::Uid admissionUid);
  bool SendResultMail(data::Uid admissionUid);

  ServerInstance& _serverInstance;
  mutable std::mutex _mutex;
  data::Uid _activeCycleUid{data::InvalidUid};
  Clock::time_point _cycleEndsAt{};
  std::vector<Group> _groups;
  std::unordered_set<data::Uid> _admittedHorseUids;
  std::unordered_map<data::Uid, std::vector<data::Uid>> _admissionsByCycle;
  std::unordered_map<data::Uid, data::Uid> _admissionByClaimUid;
  std::unordered_map<data::Uid, data::Uid> _admissionByResultMailUid;
  std::unordered_map<data::Uid, std::unordered_set<data::Uid>> _deletedAdmissionsByCycle;
  std::vector<data::Uid> _awaitingResultCycleUids;
  std::unordered_set<data::Uid> _rankedCycleUids;
  std::unordered_map<data::Uid, size_t> _rewardCreationOffsets;
  std::deque<data::Uid> _pendingParticipationMails;
  std::unordered_set<data::Uid> _pendingParticipationMailUids;
  std::deque<data::Uid> _pendingResultMails;
  std::unordered_set<data::Uid> _pendingResultMailUids;
  std::deque<data::Uid> _pendingCleanupAdmissions;
  std::unordered_set<data::Uid> _pendingCleanupAdmissionUids;
  std::chrono::steady_clock::time_point _nextMailRetry{};
};

} // namespace server

#endif // FESTIVALSYSTEM_HPP
