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

#ifndef RELAY_MESSAGE_DEFINES_HPP
#define RELAY_MESSAGE_DEFINES_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <chrono>

namespace server::protocol::relay
{

enum class PayloadType : uint16_t
{
  Snapshot = 0x3,
  SyncProgress = 0x7,
  SetTargetStateEnabled = 0x9,  // Magic effect target locked
  SetTargetStateDisabled = 0xa, // Magic effect target unlocked
  NetSetState = 0xc,            // Racer animations (especially prominent in magic)
  NetSetLayerAnimation = 0xd,   // Braking/Stopping horse
  SyncGoalIn = 0x12,            // Crossing finish line or DNF
  BroadcastCharacterUid = 0x13, // Sends self character UID
  SpurLevel = 0x14,
  ResetPosOther = 0x15,
  SlidingMotion = 0x16
};

struct Snapshot
{
  uint16_t racerOid{};
  uint32_t networkTickCounter{};

  // Seems to be stuff like IDLE, MOVE_4TH_START, GLIDE etc
  uint8_t animationState{};

  // TODO: confirm if this is really a byte
  enum class MountState : uint8_t
  {
    Reversing = 0x00,
    Standing = 0x10,
    MovingForward = 0x20,
    Jumping = 0x80
  } mountState{};

  // TODO: identify this data
  std::vector<uint8_t> unidentifiedData{};

  //! Position vector in XYZ.
  // TODO: extract this into a common place so others can use this
  struct Vector3
  {
    float X{};
    float Y{};
    float Z{};
  } position{};

  //! Rotation vector in XYZW.
  // TODO: extract this into a common place so others can use this
  struct Quarternion
  {
    float X{};
    float Y{};
    float Z{};
    float W{};
  } rotation{};

  //! In meters per second.
  float forwardSpeed{};
  //! Likely the same as forward speed, in m/s.
  float reverseSpeed{};
  //! No turning - 0.5f
  //! Turning left - < 0.5f
  //! Turning right - > 0.5f
  float turningRate{};
};

struct SyncProgress
{
  //! The OID of the affected racer.
  uint16_t racerOid{};
  //! The lap count of the racer.
  uint32_t lapCount{};
  //! The lap progress of the racer.
  uint32_t lapProgress{};
};

struct SlidingMotion
{
  //! The OID of the affected racer.
  uint16_t racerOid{};
  //! Is the racer sliding?
  bool isSliding{};
  //! The angle of the slide, in degrees.
  float slidingAngle{};
};

struct SpurLevel
{
  //! The OID of the affected racer.
  uint16_t racerOid{};
  //! The amount of successive spurs by the racer.
  uint8_t successiveSpurCount{};
};

struct SyncGoalIn
{
  //! The OID of the affected racer.
  uint16_t racerOid{};
  //! Race time, in milliseconds.
  std::chrono::milliseconds raceTimeMs{}; 
  //! The race track progress of the racer upon finish.
  //! -1 if crossed the finish line, else 0 <= x < 1.0 on DNF.
  float raceTrackProgress{};
};

struct NetSetLayerAnimation
{
  //! The OID of the affected racer.
  uint16_t racerOid{};
  //! Animation layer.
  //! Usually 0x0001 (true).
  uint16_t layerAnimation{};
};

struct BroadcastCharacterUid
{
  uint32_t selfCharacterUid{data::InvalidUid};
};

struct ResetPosOther
{
  uint16_t affectedOid{};    // Bytes 0-1: Affected object id

  // Structure and names assumed
  PackedVector3 right;       // Bytes 2-17: Right Direction (w is always 0.0)
  PackedVector3 up;          // Bytes 18-33: Up Direction (w is always 0.0)
  PackedVector3 forward;     // Bytes 34-49: Forward Direction + Packed State in 'w'

  PackedVector3 position;    // Bytes 50-65: World Coordinates + Packed State in 'w'
};

struct SetTargetState
{
  //! Indicates whether the target is locked.
  bool targetLocked{false};
  //! The ID of the magic effect.
  uint32_t magicEffectId{};
  //! The OID of the invoker.
  uint16_t invokerRacerOid{};
  //! The OID of the target.
  uint16_t targetRacerOid{};
};

struct NetSetState
{
  //! The OID of the affected racer.
  uint16_t racerOid{};

  //! State of the racer.
  //! This is read as a 64 bit value internally,
  //! but broken into 2 32-bit values.
  struct State
  {
    uint32_t val1{};
    uint32_t val2{};
  } state{};
};

} // namespace server::protocol::relay

#endif // RELAY_MESSAGE_DEFINES_HPP
