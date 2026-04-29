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

#ifndef RANCHHORSEAI_HPP
#define RANCHHORSEAI_HPP

#include "server/tracker/RanchTracker.hpp"
#include "libserver/network/command/CommandServer.hpp"
#include "libserver/data/DataDefinitions.hpp"

#include <array>
#include <random>
#include <unordered_set>

namespace server
{

// Wire values for EVENT_CALL_NPC_RESULT carried in calledOid.
// Actual values need verification against the game client.
enum class HorseCallResult : uint32_t
{
  Ignored    = 0, // TODO: verify
  NotIgnored = 1, // TODO: verify
  Following  = 2, // TODO: verify
};

//! Server-side AI for a single ranch horse.
//! Implements ai_ranch_horse.lua (idleState / playState / calledState / leaveState).
class RanchHorseAI
{
public:
  enum class State : uint8_t
  {
    CreateIdle, // Initial short pause after spawn (idleState / STATE_IDLE)
    Play,       // Main free-roaming state (playState / STATE_ATTACK1)
    Called,     // Responding to a player call (calledState / STATE_CALLED)
    Leave,      // Returning to spawn and leaving (STATE_DEAD)
  };

  //! Advance this horse's AI by one 50 Hz tick.
  void Tick(
    tracker::RanchTracker& tracker,
    data::Uid horseUid,
    const std::unordered_set<ClientId>& clients,
    CommandServer& commandServer,
    std::random_device& rng);

  //! Triggered by EVENT_CALL_NPC from a player. Transitions to Called state.
  void TriggerCall(
    uint16_t callerOid,
    uint16_t horseOid,
    const std::unordered_set<ClientId>& clients,
    CommandServer& commandServer,
    std::random_device& rng);

  [[nodiscard]] State GetState() const { return _state; }

private:
  using Clients = std::unordered_set<ClientId>;

  // ---- time ----
  float _curTime{0.0f};
  float _procDelay{0.0f};

  // ---- core ----
  State _state{State::CreateIdle};
  bool _spawned{false};
  std::array<float, 3> _pivotPos{};   // anchor point (spawn or last trace target)
  std::array<float, 3> _position{};
  std::array<float, 3> _targetPos{};
  float _facingX{1.0f};
  float _facingZ{0.0f};
  float _velocity{1.4f};
  bool _isMoving{false};
  float _travelTimer{0.0f};
  uint16_t _gazeTargetOid{0};

  // ---- CreateIdle ----
  float _createIdleEndTime{0.0f};

  // ---- Play ----
  bool _playMoving{false};
  bool _playForceNewMove{false};
  float _playMoveTiming{0.0f};
  float _playCheckFriendlyTime{0.0f};
  float _playGazeEndTime{-1.0f};
  float _playCautionTime{-1.0f};
  int   _playFeedCount{0};
  float _playFeedMotionTiming{0.0f};
  float _playTraceHorseTime{0.0f};
  bool  _firstIdle{true};
  int   _angryPoint{0};
  float _angryResetTime{0.0f};

  // ---- Called ----
  uint16_t _callerOid{0};
  bool  _callApproachChecked{false};
  bool  _callLookedAt{false};
  bool  _callActionDone{false};
  bool  _callMovePath{false};
  bool  _callRetryPath{true};
  int   _callFindPathCount{0};
  float _callLoveMotionTime{-1.0f};
  float _callFollowTime{-1.0f};
  bool  _callFastMove{true};
  float _callGazeEndTime{-1.0f};
  std::array<float, 3> _callLastApproachPlayerPos{}; // player pos when last MobMove was issued

  // ---- movement helpers (modify internal state) ----
  void StartMove(
    uint16_t horseOid, float x, float y, float z,
    const Clients&, CommandServer&);
  void SetHorseVelocity(
    uint16_t horseOid, float v,
    const Clients&, CommandServer&);
  void SetGaze(
    uint16_t horseOid, uint16_t targetOid, uint8_t gazeType,
    const Clients&, CommandServer&);
  void ClearGaze(uint16_t horseOid, const Clients&, CommandServer&);

  // ---- state transitions ----
  void EnterCreateIdle(uint16_t horseOid, const Clients&, CommandServer&, std::random_device&);
  void EnterPlay(uint16_t horseOid, const Clients&, CommandServer&, std::random_device&);
  void LeaveCalledState(uint16_t horseOid, const Clients&, CommandServer&);
  void EnterCalled(uint16_t callerOid, uint16_t horseOid, const Clients&, CommandServer&, std::random_device&);
  void EnterLeave(uint16_t horseOid, const Clients&, CommandServer&);

  // ---- per-state tick ----
  void TickCreateIdle(uint16_t horseOid, const Clients&, CommandServer&, std::random_device&);
  void TickPlay(tracker::RanchTracker&, data::Uid, uint16_t horseOid, const Clients&, CommandServer&, std::random_device&);
  void TickCalled(tracker::RanchTracker&, uint16_t horseOid, const Clients&, CommandServer&, std::random_device&);
  void TickLeave(uint16_t horseOid, const Clients&, CommandServer&, std::random_device&);

  // ---- tracker helpers ----
  [[nodiscard]] float Rand(std::random_device& rng, float min, float max) const;
  [[nodiscard]] float Dist2D(float ax, float az, float bx, float bz) const;
  [[nodiscard]] const tracker::RanchTracker::Entity* FindNearPlayer(const tracker::RanchTracker&, float maxDist) const;
  [[nodiscard]] const tracker::RanchTracker::Entity* GetPlayerByOid(const tracker::RanchTracker&, uint16_t oid) const;
  [[nodiscard]] const tracker::RanchTracker::Entity* FindHighestUidHorse(const tracker::RanchTracker&, data::Uid selfUid) const;
};

} // namespace server

#endif // RANCHHORSEAI_HPP
