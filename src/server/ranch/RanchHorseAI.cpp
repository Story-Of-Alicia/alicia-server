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

#include "server/ranch/RanchHorseAI.hpp"

#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>

namespace server
{

namespace
{

constexpr float kDt = 1.0f / 50.0f;

// ---- Lua onInit constants ----
constexpr float kDefaultVelocity        = 1.4f;
constexpr float kFirstIdleTimeMin       = 5.0f;
constexpr float kFirstIdleTimeMax       = 25.0f;
constexpr float kIdleTimeMin            = 35.0f;
constexpr float kIdleTimeMax            = 55.0f;
constexpr float kCreateIdleDelayTime    = 5.0f;
constexpr float kCalledVelocity         = 6.0f;
constexpr float kCalledSlowVelocity     = 4.0f;
constexpr float kCalledSlowNearVelocity = 2.0f;
constexpr float kCalledSlowNearDist     = 5.0f;
constexpr float kLoveMotionDist         = 8.0f;
constexpr float kLookPlayerDist         = 15.0f;
constexpr float kLookHorseDist          = 10.0f;
constexpr float kFriendlyCheckDist      = 15.0f;
constexpr float kFriendlyCheckMinDelay  = 10.0f;
constexpr float kFriendlyCheckMaxDelay  = 20.0f;
constexpr float kFriendlyActionRatio    = 10.0f;
constexpr float kFriendlyAutoCallRatio  = 20.0f;
constexpr float kTraceHorseMinDist      = 12.0f;
constexpr float kFeedMotionDelay        = 5.0f;
constexpr float kSlowApproachDist       = 5.0f;
constexpr float kCalledApproachDist     = 3.0f;
constexpr float kFeedRatio              = 30.0f;
constexpr int   kFeedCountMin           = 3;
constexpr int   kFeedCountMax           = 5;
constexpr float kLookNearRatio          = 50.0f;
constexpr float kLookPlayerRatio        = 40.0f;
constexpr float kRandomMoveDist         = 10.0f;
constexpr float kAngryResetDelay        = 60.0f;

// Gaze types
constexpr uint8_t kGazeNone  = 0;
constexpr uint8_t kGazeFavor = 1;

// Mob state IDs are hashes of w@ animation state strings in the game client.
// kMobStateWander (5) is the existing working wander state; all others are TODO.
constexpr uint32_t kMobStateWander     = 5;
constexpr uint32_t kMobStateGrazeIdle  = 0; // TODO: GRAZE_IDLE
constexpr uint32_t kMobStateEatFood    = 0; // TODO: EAT_FOOD
constexpr uint32_t kMobStateEatFoodEnd = 0; // TODO: EAT_FOOD_END
constexpr uint32_t kMobStateDispleased = 0; // TODO: DISPLEASED
constexpr uint32_t kMobStateWayLost    = 0; // TODO: WAY_LOST (SearchFailed/ByeLast)

// Paddock fence bounds, inset by 1 unit to keep horses away from the edge.
constexpr float kPaddockMinX = -5.1686106f + 1.0f;
constexpr float kPaddockMaxX =  8.701556f  - 1.0f;
constexpr float kPaddockMinZ = -52.76547f  + 1.0f;
constexpr float kPaddockMaxZ = -18.826422f - 1.0f;

template<typename Cmd>
void Broadcast(
  const std::unordered_set<ClientId>& clients,
  CommandServer& cs,
  Cmd cmd)
{
  for (const ClientId id : clients)
    cs.QueueCommand<Cmd>(id, [cmd]() { return cmd; });
}

void BroadcastMissionEvent(
  protocol::AcCmdRCMissionEvent::Event event,
  uint32_t callerOid, uint32_t calledOid,
  const std::unordered_set<ClientId>& clients,
  CommandServer& cs)
{
  Broadcast(clients, cs, protocol::AcCmdRCMissionEvent{
    .event     = event,
    .callerOid = callerOid,
    .calledOid = calledOid});
}

void BroadcastMotion(
  uint16_t horseOid, uint32_t stateId,
  const std::unordered_set<ClientId>& clients,
  CommandServer& cs)
{
  Broadcast(clients, cs, protocol::AcCmdRCMobSetState{
    .mobOid   = horseOid,
    .state    = stateId,
    .subState = 0});
}

} // namespace

// =============================================================================
// Helpers
// =============================================================================

float RanchHorseAI::Rand(std::random_device& rng, float min, float max) const
{
  std::uniform_real_distribution<float> d(min, max);
  return d(rng);
}

float RanchHorseAI::Dist2D(float ax, float az, float bx, float bz) const
{
  const float dx = bx - ax;
  const float dz = bz - az;
  return std::sqrt(dx * dx + dz * dz);
}

const tracker::RanchTracker::Entity* RanchHorseAI::FindNearPlayer(
  const tracker::RanchTracker& tracker, float maxDist) const
{
  const tracker::RanchTracker::Entity* nearest = nullptr;
  float nearestDist = maxDist;
  for (const auto& [uid, entity] : tracker.GetCharacters())
  {
    const float d = Dist2D(_position[0], _position[2], entity.position[0], entity.position[2]);
    if (d < nearestDist)
    {
      nearestDist = d;
      nearest = &entity;
    }
  }
  return nearest;
}

const tracker::RanchTracker::Entity* RanchHorseAI::GetPlayerByOid(
  const tracker::RanchTracker& tracker, uint16_t oid) const
{
  for (const auto& [uid, entity] : tracker.GetCharacters())
    if (entity.oid == oid) return &entity;
  return nullptr;
}

const tracker::RanchTracker::Entity* RanchHorseAI::FindHighestUidHorse(
  const tracker::RanchTracker& tracker, data::Uid selfUid) const
{
  const tracker::RanchTracker::Entity* best = nullptr;
  data::Uid bestUid = 0;
  for (const auto& [uid, entity] : tracker.GetHorses())
  {
    if (uid == selfUid) continue;
    if (uid > bestUid) { bestUid = uid; best = &entity; }
  }
  return best;
}

// =============================================================================
// Movement / gaze helpers (mutate AI state)
// =============================================================================

void RanchHorseAI::StartMove(
  uint16_t horseOid, float x, float y, float z,
  const Clients& clients, CommandServer& cs)
{
  const float dx   = x - _position[0];
  const float dz   = z - _position[2];
  const float dist = Dist2D(_position[0], _position[2], x, z);

  _targetPos   = {x, y, z};
  _isMoving    = true;
  _travelTimer = (dist > 0.001f) ? (dist / _velocity) : 0.01f;

  if (dist > 0.001f)
  {
    _facingX = dx / dist;
    _facingZ = dz / dist;
  }

  Broadcast(clients, cs, protocol::AcCmdRCMobMove{
    .type     = protocol::AcCmdRCMobMove::Type::Partial,
    .mobOid   = horseOid,
    .position = {.X = x, .Y = y, .Z = z},
    .unk2     = 2});
}

void RanchHorseAI::SetHorseVelocity(
  uint16_t horseOid, float v,
  const Clients& clients, CommandServer& cs)
{
  _velocity = v;
  Broadcast(clients, cs, protocol::AcCmdRCMobSetVelocity{
    .mobOid   = horseOid,
    .velocity = v});
}

void RanchHorseAI::SetGaze(
  uint16_t horseOid, uint16_t targetOid, uint8_t gazeType,
  const Clients& clients, CommandServer& cs)
{
  _gazeTargetOid = targetOid;
  Broadcast(clients, cs, protocol::AcCmdRCMobGazeAt{
    .mobOid    = horseOid,
    .targetOid = targetOid,
    .gazeType  = gazeType});
}

void RanchHorseAI::ClearGaze(
  uint16_t horseOid, const Clients& clients, CommandServer& cs)
{
  if (_gazeTargetOid == 0) return;
  _gazeTargetOid  = 0;
  _playGazeEndTime = -1.0f;
  Broadcast(clients, cs, protocol::AcCmdRCMobGazeAt{
    .mobOid    = horseOid,
    .targetOid = 0,
    .gazeType  = kGazeNone});
}

// =============================================================================
// State transitions
// =============================================================================

void RanchHorseAI::EnterCreateIdle(
  uint16_t horseOid,
  const Clients& clients, CommandServer& cs, std::random_device& rng)
{
  _state              = State::CreateIdle;
  _createIdleEndTime  = _curTime + kCreateIdleDelayTime + Rand(rng, 0.0f, 1.5f);

  std::uniform_real_distribution<float> angleDist(0.0f, 6.283185f);
  const float angle = angleDist(rng);
  _facingX = std::cos(angle);
  _facingZ = std::sin(angle);

  BroadcastMotion(horseOid, kMobStateGrazeIdle, clients, cs);
}

void RanchHorseAI::EnterPlay(
  uint16_t horseOid,
  const Clients& clients, CommandServer& cs, std::random_device& rng)
{
  _state                 = State::Play;
  _playMoving            = true;   // pretend we "just arrived" so timing setup runs first tick
  _playForceNewMove      = false;
  _playMoveTiming        = _curTime;
  _playCheckFriendlyTime = _curTime + Rand(rng, kFriendlyCheckMinDelay, kFriendlyCheckMaxDelay);
  _playGazeEndTime       = -1.0f;
  _playCautionTime       = -1.0f;
  _playFeedCount         = 0;
  _playTraceHorseTime    = _curTime;

  ClearGaze(horseOid, clients, cs);
  SetHorseVelocity(horseOid, kDefaultVelocity, clients, cs);
}

void RanchHorseAI::LeaveCalledState(
  uint16_t horseOid, const Clients& clients, CommandServer& cs)
{
  using Event = protocol::AcCmdRCMissionEvent::Event;
  BroadcastMissionEvent(Event::EVENT_NPC_FOLLOWING_END, horseOid, _callerOid, clients, cs);
  SetHorseVelocity(horseOid, kDefaultVelocity, clients, cs);
  ClearGaze(horseOid, clients, cs);
}

void RanchHorseAI::EnterCalled(
  uint16_t callerOid, uint16_t horseOid,
  const Clients& clients, CommandServer& cs, std::random_device& rng)
{
  _state               = State::Called;
  _callerOid           = callerOid;
  _callApproachChecked = false;
  _callLookedAt        = false;
  _callActionDone      = false;
  _callMovePath        = false;
  _callRetryPath       = true;
  _callFindPathCount   = 0;
  _callLoveMotionTime  = -1.0f;
  _callFollowTime      = -1.0f;
  _callFastMove        = true;
  _callGazeEndTime     = -1.0f;

  // Initial look-at delay
  _procDelay = Rand(rng, 0.0f, 1.0f);

  using Event = protocol::AcCmdRCMissionEvent::Event;
  BroadcastMissionEvent(Event::EVENT_CALLED_NPC, horseOid, callerOid, clients, cs);
}

void RanchHorseAI::EnterLeave(
  uint16_t horseOid, const Clients& clients, CommandServer& cs)
{
  _state = State::Leave;
  ClearGaze(horseOid, clients, cs);
  SetHorseVelocity(horseOid, kDefaultVelocity, clients, cs);

  // Walk back to pivot (spawn) position
  StartMove(horseOid, _pivotPos[0], _pivotPos[1], _pivotPos[2], clients, cs);
}

// =============================================================================
// Main tick
// =============================================================================

void RanchHorseAI::Tick(
  tracker::RanchTracker& tracker,
  data::Uid horseUid,
  const Clients& clients,
  CommandServer& cs,
  std::random_device& rng)
{
  _curTime += kDt;
  auto& entity        = tracker.GetHorseEntity(horseUid);
  const uint16_t oid  = entity.oid;

  // First tick: place horse, push initial commands
  if (!_spawned)
  {
    _spawned = true;

    std::uniform_real_distribution<float> xDist(kPaddockMinX, kPaddockMaxX);
    std::uniform_real_distribution<float> zDist(kPaddockMinZ, kPaddockMaxZ);

    _pivotPos       = {xDist(rng), 0.0f, zDist(rng)};
    _position       = _pivotPos;
    entity.position = _position;

    Broadcast(clients, cs, protocol::AcCmdRCMobResetPos{
      .mobOid   = oid,
      .position = {.X = _position[0], .Y = _position[1], .Z = _position[2]}});

    SetHorseVelocity(oid, kDefaultVelocity, clients, cs);

    Broadcast(clients, cs, protocol::AcCmdRCMobSetState{
      .mobOid   = oid,
      .state    = kMobStateWander,
      .subState = 2});

    EnterCreateIdle(oid, clients, cs, rng);
    return;
  }

  // Simulate movement at current velocity
  if (_isMoving)
  {
    _travelTimer -= kDt;
    const float dx   = _targetPos[0] - _position[0];
    const float dz   = _targetPos[2] - _position[2];
    const float dist = std::sqrt(dx * dx + dz * dz);
    const float step = _velocity * kDt;

    if (dist <= step || _travelTimer <= 0.0f)
    {
      _position        = _targetPos;
      _isMoving        = false;
      entity.hasTarget = false;
    }
    else
    {
      _position[0] += (dx / dist) * step;
      _position[2] += (dz / dist) * step;
    }
    entity.position = _position;
  }

  // Process delay (mirrors Lua SET_DELAY / procDelay)
  if (_procDelay > 0.0f)
  {
    _procDelay -= kDt;
    return;
  }

  switch (_state)
  {
    case State::CreateIdle: TickCreateIdle(oid, clients, cs, rng); break;
    case State::Play:       TickPlay(tracker, horseUid, oid, clients, cs, rng); break;
    case State::Called:     TickCalled(tracker, oid, clients, cs, rng); break;
    case State::Leave:      TickLeave(oid, clients, cs, rng); break;
  }
}

// =============================================================================
// CreateIdle (idleState)
// =============================================================================

void RanchHorseAI::TickCreateIdle(
  uint16_t horseOid, const Clients& clients, CommandServer& cs, std::random_device& rng)
{
  if (_curTime < _createIdleEndTime) return;

  BroadcastMotion(horseOid, kMobStateGrazeIdle, clients, cs);
  EnterPlay(horseOid, clients, cs, rng);
}

// =============================================================================
// Play (playState / STATE_ATTACK1)
// =============================================================================

void RanchHorseAI::TickPlay(
  tracker::RanchTracker& tracker,
  data::Uid horseUid,
  uint16_t horseOid,
  const Clients& clients, CommandServer& cs, std::random_device& rng)
{
  // Angry-point periodic reset
  if (_curTime >= _angryResetTime)
  {
    _angryPoint     = 0;
    _angryResetTime = _curTime + kAngryResetDelay;
  }

  // Player proximity: if arrived and player is within 2 units, force a new move
  if (!_isMoving)
  {
    const auto* nearPlayer = FindNearPlayer(tracker, 2.0f);
    if (nearPlayer != nullptr)
    {
      _playMoveTiming   = _curTime;
      _playForceNewMove = true;
    }
  }

  // Gaze timeout
  if (_playGazeEndTime >= 0.0f && _curTime >= _playGazeEndTime)
    ClearGaze(horseOid, clients, cs);

  if (!_isMoving) // mob:IsArrived()
  {
    // Feeding loop (skip if forceNewMove)
    if (!_playForceNewMove && _playFeedCount > 0)
    {
      if (_curTime >= _playFeedMotionTiming)
      {
        BroadcastMotion(horseOid, kMobStateEatFood, clients, cs);
        _playFeedMotionTiming = _curTime + Rand(rng, 5.0f, 7.0f);
        _playFeedCount--;
        if (_playFeedCount == 0)
          BroadcastMotion(horseOid, kMobStateEatFoodEnd, clients, cs);
      }
      return;
    }

    // Caution / angry animation (skip if forceNewMove; stubbed: feeling always good)
    if (!_playForceNewMove && _playCautionTime >= 0.0f && _curTime >= _playCautionTime)
    {
      BroadcastMotion(horseOid, kMobStateDispleased, clients, cs);
      _playCautionTime = -1.0f;
    }

    // "Just arrived" — set up next idle/feed/gaze timing
    if (!_playForceNewMove && _playMoving)
    {
      _playMoving = false;

      if (Rand(rng, 0.0f, 100.0f) < kFeedRatio)
      {
        // Feed N times, then resume moving
        _playFeedCount        = static_cast<int>(Rand(rng, (float)kFeedCountMin, (float)kFeedCountMax));
        const float startDelay = Rand(rng, 1.0f, 2.0f);
        _playFeedMotionTiming  = _curTime + startDelay;
        _playMoveTiming        = _curTime + startDelay
                                 + _playFeedCount * kFeedMotionDelay
                                 + Rand(rng, 1.0f, 5.0f);
      }
      else
      {
        // Maybe look at something nearby
        if (Rand(rng, 0.0f, 100.0f) < kLookNearRatio)
        {
          if (Rand(rng, 0.0f, 100.0f) < kLookPlayerRatio)
          {
            // Look at nearest player within lookPlayerDist (friendlyStep always 3 >= 2)
            const auto* player = FindNearPlayer(tracker, kLookPlayerDist);
            if (player != nullptr)
            {
              SetGaze(horseOid, player->oid, kGazeFavor, clients, cs);
              _playGazeEndTime = _curTime + Rand(rng, 5.0f, 15.0f);
            }
          }
          else
          {
            // Look at nearest other horse
            const auto* horse = FindHighestUidHorse(tracker, horseUid);
            if (horse != nullptr
              && Dist2D(_position[0], _position[2], horse->position[0], horse->position[2]) <= kLookHorseDist)
            {
              SetGaze(horseOid, horse->oid, kGazeFavor, clients, cs);
              _playGazeEndTime = _curTime + Rand(rng, 10.0f, 20.0f);
            }
          }
        }

        // Idle timer
        if (_firstIdle)
        {
          _firstIdle      = false;
          _playMoveTiming = _curTime + Rand(rng, kFirstIdleTimeMin, kFirstIdleTimeMax);
        }
        else
        {
          _playMoveTiming = _curTime + Rand(rng, kIdleTimeMin, kIdleTimeMax);
        }
      }
    }

    // Drop gaze if gazing player has wandered too far
    if (_gazeTargetOid != 0)
    {
      const auto* player = GetPlayerByOid(tracker, _gazeTargetOid);
      if (player == nullptr
        || Dist2D(_position[0], _position[2], player->position[0], player->position[2]) > kLookPlayerDist + 5.0f)
      {
        ClearGaze(horseOid, clients, cs);
      }
    }

    // When forceNewMove is set, skip idle timers and move now
    if (_playForceNewMove)
      _playMoveTiming = _curTime;

    if (_curTime < _playMoveTiming) return;

    // Clear gaze before starting to move
    ClearGaze(horseOid, clients, cs);

    // Friendly check: 10% chance per interval, must be within friendlyCheckDist
    if (!_playForceNewMove && _curTime >= _playCheckFriendlyTime
      && Rand(rng, 0.0f, 100.0f) <= kFriendlyActionRatio)
    {
      const auto* player = FindNearPlayer(tracker, kFriendlyCheckDist);
      if (player != nullptr)
      {
        // friendlyStep always 3 (stubbed)
        if (Rand(rng, 0.0f, 100.0f) < kFriendlyAutoCallRatio)
        {
          // Auto-approach: transition to Called
          EnterCalled(player->oid, horseOid, clients, cs, rng);
          return;
        }
        // Step 2 fallback: just gaze
        SetGaze(horseOid, player->oid, kGazeFavor, clients, cs);
        _playGazeEndTime = _curTime + Rand(rng, 5.0f, 15.0f);
      }
      _playCheckFriendlyTime = _curTime + Rand(rng, kFriendlyCheckMinDelay, kFriendlyCheckMaxDelay);
      return;
    }

    // Trace highest-UID horse if it is far enough away
    if (!_playForceNewMove && _curTime >= _playTraceHorseTime)
    {
      _playTraceHorseTime = _curTime + 10.0f;
      const auto* highHorse = FindHighestUidHorse(tracker, horseUid);
      if (highHorse != nullptr
        && Dist2D(_position[0], _position[2], highHorse->position[0], highHorse->position[2]) >= kTraceHorseMinDist)
      {
        const float tx = std::clamp(highHorse->position[0] + Rand(rng, -10.0f, 10.0f), kPaddockMinX, kPaddockMaxX);
        const float tz = std::clamp(highHorse->position[2] + Rand(rng, -10.0f, 10.0f), kPaddockMinZ, kPaddockMaxZ);
        _pivotPos = {tx, _position[1], tz};
        StartMove(horseOid, tx, _position[1], tz, clients, cs);
        auto& horseEntity             = tracker.GetHorseEntity(horseUid);
        horseEntity.targetPosition    = {tx, _position[1], tz};
        horseEntity.hasTarget         = true;
        _playMoving       = true;
        _playForceNewMove = false;
        _procDelay        = 1.0f;
        return;
      }
    }

    // Pick a biased directional destination from the horse's *current* position.
    // Using the pivot (spawn) as the base caused U-turns: once the horse had moved
    // past pivot+forward, the next "front" target landed behind it.
    // Try up to 5 times to find a target that is inside the paddock AND not
    // occupied by another horse. Each failed attempt rotates the direction ~60°
    // so repeated retries spread out rather than piling onto the same spot.
    constexpr float kHorseCheckDist  = 4.0f;
    constexpr float kMaxWanderRadius = 18.0f;

    float px = _position[0], pz = _position[2];
    bool validTarget = false;

    for (int attempt = 0; attempt < 5; ++attempt)
    {
      const int moveType = static_cast<int>(Rand(rng, 1.0f, 3.99f));
      const float sideDist  = Rand(rng, 4.0f, 10.0f);
      const float frontDist = Rand(rng, 4.0f, 12.0f);

      float cx = _position[0];
      float cz = _position[2];

      // Rotate facing slightly on each retry to spread candidate directions.
      const float angle = attempt * 1.047f; // ~60° per attempt
      const float fx = _facingX * std::cos(angle) - _facingZ * std::sin(angle);
      const float fz = _facingX * std::sin(angle) + _facingZ * std::cos(angle);

      if (moveType == 1)      { cx += fx * frontDist;  cz += fz * frontDist; }
      else if (moveType == 2) { cx += -fz * sideDist;  cz += fx * sideDist;  }
      else                    { cx +=  fz * sideDist;  cz -= fx * sideDist;  }

      cx += Rand(rng, -kRandomMoveDist * 0.5f, kRandomMoveDist * 0.5f);
      cz += Rand(rng, -kRandomMoveDist * 0.5f, kRandomMoveDist * 0.5f);

      // Soft pull back toward spawn so the horse stays near its home area.
      const float spawnDx   = cx - _pivotPos[0];
      const float spawnDz   = cz - _pivotPos[2];
      const float spawnDist = std::sqrt(spawnDx * spawnDx + spawnDz * spawnDz);
      if (spawnDist > kMaxWanderRadius)
      {
        const float t = kMaxWanderRadius / spawnDist;
        cx = _pivotPos[0] + spawnDx * t;
        cz = _pivotPos[2] + spawnDz * t;
      }

      // Reject if outside the paddock.
      if (cx < kPaddockMinX || cx > kPaddockMaxX || cz < kPaddockMinZ || cz > kPaddockMaxZ)
        continue;

      // Reject if another horse is at or heading toward this position.
      bool occupied = false;
      for (const auto& [uid, otherEntity] : tracker.GetHorses())
      {
        if (uid == horseUid) continue;
        if (Dist2D(cx, cz, otherEntity.position[0], otherEntity.position[2]) < kHorseCheckDist)
        {
          occupied = true;
          break;
        }
        if (otherEntity.hasTarget
          && Dist2D(cx, cz, otherEntity.targetPosition[0], otherEntity.targetPosition[2]) < kHorseCheckDist)
        {
          occupied = true;
          break;
        }
      }
      if (occupied) continue;

      px          = cx;
      pz          = cz;
      validTarget = true;
      break;
    }

    // Hard fallback: clamp to paddock. May still collide but avoids getting stuck.
    if (!validTarget)
    {
      px = std::clamp(px, kPaddockMinX, kPaddockMaxX);
      pz = std::clamp(pz, kPaddockMinZ, kPaddockMaxZ);
    }

    StartMove(horseOid, px, _position[1], pz, clients, cs);
    auto& horseEntity          = tracker.GetHorseEntity(horseUid);
    horseEntity.targetPosition = {px, _position[1], pz};
    horseEntity.hasTarget      = true;
    _playMoving       = true;
    _playForceNewMove = false;
    _procDelay        = 1.0f; // SET_DELAY(1) — short pause before processing again
  }
  // mid-move: nothing to do — target selection already avoids occupied positions
}

// =============================================================================
// Called (calledState / STATE_CALLED)
// =============================================================================

void RanchHorseAI::TickCalled(
  tracker::RanchTracker& tracker,
  uint16_t horseOid,
  const Clients& clients, CommandServer& cs, std::random_device& rng)
{
  using Event = protocol::AcCmdRCMissionEvent::Event;

  // Follow timer expired → stop MobLead and return to Play
  if (_callFollowTime >= 0.0f && _curTime >= _callFollowTime)
  {
    LeaveCalledState(horseOid, clients, cs);
    Broadcast(clients, cs, protocol::AcCmdRCMobSetDefaultState{
      .mobOid        = horseOid,
      .defaultState  = kMobStateWander,
      .fallbackState = 0});
    EnterPlay(horseOid, clients, cs, rng);
    return;
  }

  // Gaze-only hold (friendlyStep == 2 path; never in our stub)
  if (_callGazeEndTime >= 0.0f)
  {
    if (_curTime >= _callGazeEndTime)
    {
      LeaveCalledState(horseOid, clients, cs);
      EnterPlay(horseOid, clients, cs, rng);
    }
    return;
  }

  // Step 1: approach-value check (runs once, first tick after enter delay)
  if (!_callApproachChecked)
  {
    _callApproachChecked = true;
    // friendlyStep always 3 (stubbed) → not ignored
    BroadcastMissionEvent(
      Event::EVENT_CALL_NPC_RESULT,
      _callerOid,
      static_cast<uint32_t>(HorseCallResult::NotIgnored),
      clients, cs);
    return;
  }

  // Step 2: look at player (once)
  if (!_callLookedAt)
  {
    _callLookedAt = true;
    _isMoving = false; // stop
    // friendlyStep >= 3 → stop + look + send FOLLOW_START
    BroadcastMissionEvent(Event::EVENT_NPC_FOLLOW_START, horseOid, _callerOid, clients, cs);
    _procDelay = Rand(rng, 0.0f, 0.5f);
    return;
  }

  // Step 3: call action (once)
  if (!_callActionDone)
  {
    _callActionDone = true;
    // callSuccessRatio = 100 → always succeeds
    BroadcastMissionEvent(
      Event::EVENT_CALL_NPC_RESULT,
      _callerOid,
      static_cast<uint32_t>(HorseCallResult::Following),
      clients, cs);
    return;
  }

  // Love motion timer: 1 second after arriving near player.
  // On expiry, hand movement over to MobLead so the client drives following
  // natively — no more periodic MobMove commands that caused circling.
  if (_callLoveMotionTime >= 0.0f)
  {
    if (_curTime >= _callLoveMotionTime)
    {
      _callLoveMotionTime = -1.0f;
      _callFollowTime     = _curTime + 20.0f;

      Broadcast(clients, cs, protocol::AcCmdRCMobSetState{
        .mobOid   = horseOid,
        .state    = kMobStateWander, // TODO: kMobStateFollowing
        .subState = 0});
      Broadcast(clients, cs, protocol::AcCmdRCMobSetMoveType{
        .mobOid   = horseOid,
        .moveType = 0}); // walk
      Broadcast(clients, cs, protocol::AcCmdRCMobLead{
        .mobOid      = horseOid,
        .targetOid   = _callerOid,
        .leadType    = 0,
        .destination = {.X = 2.0f, .Y = 0.0f, .Z = 0.0f}});
      SetGaze(horseOid, _callerOid, kGazeFavor, clients, cs);

      // _callMovePath stays true — approach loop must not restart during follow
    }
    return;
  }

  // Once following via MobLead, skip the approach loop entirely —
  // the client drives movement and we just wait for the follow timer.
  if (_callFollowTime >= 0.0f)
    return;

  // Approach loop: move toward player until we are within loveMotionDist.
  if (!_callMovePath)
  {
    const auto* player = GetPlayerByOid(tracker, _callerOid);
    if (player != nullptr)
    {
      SetHorseVelocity(horseOid, kCalledVelocity, clients, cs);

      const float dx   = _position[0] - player->position[0];
      const float dz   = _position[2] - player->position[2];
      const float dist = std::sqrt(dx * dx + dz * dz);
      float tx = player->position[0];
      float tz = player->position[2];
      if (dist > 0.001f)
      {
        tx += (dx / dist) * kCalledApproachDist;
        tz += (dz / dist) * kCalledApproachDist;
      }

      _callFindPathCount++;
      if (_callFindPathCount <= 5)
      {
        StartMove(horseOid, tx, player->position[1], tz, clients, cs);
        _callLastApproachPlayerPos = player->position;
        _callFastMove = true;
      }
      else
      {
        _callRetryPath = false;
      }
    }
    _callMovePath = true;
  }
  else if (!_isMoving) // arrived at approach target
  {
    const auto* player = GetPlayerByOid(tracker, _callerOid);
    if (player != nullptr
      && Dist2D(_position[0], _position[2], player->position[0], player->position[2]) <= kLoveMotionDist)
    {
      _callLoveMotionTime = _curTime + 1.0f;
    }
    else if (_callRetryPath)
    {
      // Only re-approach if the player has actually moved since the last attempt,
      // otherwise we'd spam MobMove commands at a stationary player.
      constexpr float kMinReapproachDist = 2.0f;
      const bool playerMoved = player == nullptr
        || Dist2D(player->position[0], player->position[2],
                  _callLastApproachPlayerPos[0], _callLastApproachPlayerPos[2]) >= kMinReapproachDist;

      if (playerMoved)
        _callMovePath = false; // player moved — retry approach
      else
        _procDelay = 1.0f;    // player standing still — wait before checking again
    }
    else
    {
      BroadcastMotion(horseOid, kMobStateWayLost, clients, cs);
      LeaveCalledState(horseOid, clients, cs);
      EnterPlay(horseOid, clients, cs, rng);
    }
  }
  else // still moving toward player — adjust speed near destination
  {
    const float remainDist = Dist2D(_position[0], _position[2], _targetPos[0], _targetPos[2]);
    if (remainDist < kSlowApproachDist)
    {
      if (_callFastMove)
      {
        const auto* player = GetPlayerByOid(tracker, _callerOid);
        const bool veryNear = player != nullptr
          && Dist2D(_position[0], _position[2], player->position[0], player->position[2]) <= kCalledSlowNearDist;
        SetHorseVelocity(
          horseOid,
          veryNear ? kCalledSlowNearVelocity : kCalledSlowVelocity,
          clients, cs);
        _callFastMove = false;
      }
    }
    else if (!_callFastMove)
    {
      SetHorseVelocity(horseOid, kCalledVelocity, clients, cs);
      _callFastMove = true;
    }
  }
}

// =============================================================================
// Leave (LeaveRanch / STATE_DEAD)
// =============================================================================

void RanchHorseAI::TickLeave(
  uint16_t horseOid, const Clients& clients, CommandServer& cs, std::random_device& rng)
{
  if (_isMoving) return;

  // Arrived at spawn — play goodbye animation then cycle back to CreateIdle
  BroadcastMotion(horseOid, kMobStateWayLost, clients, cs); // TODO: MOTION_ANI8 (ByeLast)
  EnterCreateIdle(horseOid, clients, cs, rng);
}

// =============================================================================
// TriggerCall (public — called by RanchDirector on EVENT_CALL_NPC)
// =============================================================================

void RanchHorseAI::TriggerCall(
  uint16_t callerOid,
  uint16_t horseOid,
  const Clients& clients,
  CommandServer& cs,
  std::random_device& rng)
{
  EnterCalled(callerOid, horseOid, clients, cs, rng);
}

} // namespace server
