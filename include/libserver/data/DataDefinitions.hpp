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

#ifndef DATADEFINITIONS_HPP
#define DATADEFINITIONS_HPP

#include <atomic>
#include <cstdint>
#include <chrono>
#include <string>
#include <vector>

namespace server
{

namespace dao
{

template <typename T>
struct Field
{
  //! Constructs a field with an initialized value.
  //! @param value Value.
  Field(T value) noexcept
    : _value(std::move(value))
  {
  }

  //! Constructs field with an initialized value.
  Field()
    : _value()
  {
  }

  //! Deleted copy constructor.
  Field(const Field& field) = delete;
  //!  Deleted copy assignment operator.
  Field& operator=(const Field& field) = delete;

  Field(Field&& field) noexcept
    : _modified(field.IsModified())
    , _value(field._value)
  {
  }

  Field& operator=(Field&& field) noexcept
  {
    _modified = field.IsModified();
    _value = std::move(field._value);

    return *this;
  }


  [[nodiscard]] bool IsModified() const noexcept
  {
    return _modified;
  }

  T& operator()(const T& value) noexcept
  {
    _modified = true;
    _value = value;
    return value;
  }

  T& operator()(T&& value) noexcept
  {
    _modified = true;
    _value = std::move(value);
    return value;
  }

  const T& operator()() const noexcept
  {
    return _value;
  }

  T& operator()() noexcept
  {
    return _value;
  }

private:
  std::atomic_bool _modified{false};
  T _value;
};

} // namespace dao

namespace data
{

//! Unique identifier.
using Uid = uint32_t;
//! Type identifier.
using Tid = uint32_t;
//! Value of an invalid unique identifier.
constexpr Uid InvalidUid = 0;
//! Value of an invalid type identifier.
constexpr Tid InvalidTid = 0;

using Clock = std::chrono::system_clock;

//! User
struct User
{
  //! An UID of the user.
  dao::Field<Uid> uid{InvalidUid};
  //! A name of the user.
  dao::Field<std::string> name{};
  //! An authorization token of the user.
  dao::Field<std::string> token{};
  //! A character UID of the user.
  dao::Field<Uid> characterUid{InvalidUid};
};

//! Item
struct Item
{
  //! A unique identifier.
  dao::Field<Uid> uid{InvalidUid};
  //! A type identifier.
  dao::Field<Tid> tid{InvalidTid};
  //! A time point of when the item expires.
  dao::Field<Clock::time_point> expiresAt{};
  //! Amount of an item.
  dao::Field<uint32_t> count{};
};

//! Pet
struct Pet
{
  //! A unique identifier.
  dao::Field<Uid> uid{InvalidUid};
  //! A Item tied to the pet.
  dao::Field<Uid> itemUid{InvalidUid};
  //! A pet identifier.
  dao::Field<Uid> petId{0};
  //! A name of the pet.
  dao::Field<std::string> name{};
};

//! Stored item
struct StorageItem
{
  //! A unique identifier.
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<std::vector<Uid>> items{};
  dao::Field<std::string> sender{};
  dao::Field<std::string> message{};
  dao::Field<Clock::time_point> created{};
  dao::Field<bool> checked{false};
  dao::Field<bool> expired{false};
};

//! Guild
struct Guild
{
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<std::string> name{};
};

//! User
struct Character
{
  //! An UID of the character.
  dao::Field<Uid> uid{InvalidUid};
  //! A name of the character.
  dao::Field<std::string> name{};

  dao::Field<std::string> introduction{};

  dao::Field<uint32_t> level{};
  dao::Field<int32_t> carrots{};
  dao::Field<uint32_t> cash{};

  enum class Role
  {
    User,
    Op,
    GameMaster
  };
  dao::Field<Role> role{};

  struct Parts
  {
    //! An ID of the character model.
    dao::Field<uint32_t> modelId{0u};
    //! An ID of the mouth part.
    dao::Field<uint32_t> mouthId{0u};
    //! An ID of the face part.
    dao::Field<uint32_t> faceId{0u};
  } parts{};

  struct Appearance
  {
    //! An ID of the Voice model.
    dao::Field<uint32_t> voiceId{0u};
    dao::Field<uint32_t> headSize{0u};
    dao::Field<uint32_t> height{0u};
    dao::Field<uint32_t> thighVolume{0u};
    dao::Field<uint32_t> legVolume{0u};
    //! An ID of the emblem.
    dao::Field<uint32_t> emblemId{0u};
  } appearance{};

  dao::Field<Uid> guildUid{InvalidUid};
  
  dao::Field<std::vector<Uid>> gifts{};
  dao::Field<std::vector<Uid>> purchases{};
  
  dao::Field<std::vector<Uid>> items{};
  dao::Field<std::vector<Uid>> characterEquipment{};
  dao::Field<std::vector<Uid>> mountEquipment{};
  
  dao::Field<std::vector<Uid>> horses{};
  dao::Field<std::vector<Uid>> pets{};
  dao::Field<Uid> mountUid{InvalidUid};
  dao::Field<Uid> petUid{InvalidUid};

  dao::Field<std::vector<Uid>> eggs{};

  dao::Field<std::vector<Uid>> housing{};

  dao::Field<bool> isRanchLocked{};
};

struct Horse
{
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<Tid> tid{InvalidTid};
  dao::Field<std::string> name{};

  struct Parts
  {
    dao::Field<Tid> skinTid{0u};
    dao::Field<Tid> faceTid{0u};
    dao::Field<Tid> maneTid{0u};
    dao::Field<Tid> tailTid{0u};
  } parts{};

  struct Appearance
  {
    dao::Field<uint32_t> scale{0u};
    dao::Field<uint32_t> legLength{0u};
    dao::Field<uint32_t> legVolume{0u};
    dao::Field<uint32_t> bodyLength{0u};
    dao::Field<uint32_t> bodyVolume{0u};
  } appearance{};

  struct Stats
  {
    dao::Field<uint32_t> agility{0u};
    dao::Field<uint32_t> courage{0u};
    dao::Field<uint32_t> rush{0u};
    dao::Field<uint32_t> endurance{0u};
    dao::Field<uint32_t> ambition{0u};
  } stats{};

  struct Mastery
  {
    dao::Field<uint32_t> spurMagicCount{0u};
    dao::Field<uint32_t> jumpCount{0u};
    dao::Field<uint32_t> slidingTime{0u};
    dao::Field<uint32_t> glidingDistance{0u};
  } mastery{};

  dao::Field<uint32_t> rating{0u};
  dao::Field<uint32_t> clazz{0u};
  dao::Field<uint32_t> clazzProgress{0u};
  dao::Field<uint32_t> grade{0u};
  dao::Field<uint32_t> growthPoints{0u};

  dao::Field<Tid> potentialType{0u};
  dao::Field<uint32_t> potentialLevel{0u};

  dao::Field<uint32_t> luckState{0u};
  dao::Field<uint32_t> emblemUid{0u};
  dao::Field<Clock::time_point> dateOfBirth{};

  struct MountCondition
  {
    dao::Field<uint16_t> stamina{};
    dao::Field<uint16_t> charm{};
    dao::Field<uint16_t> friendliness{};
    dao::Field<uint16_t> injury{};
    dao::Field<uint16_t> plenitude{};
    dao::Field<uint16_t> bodyDirtiness{};
    dao::Field<uint16_t> maneDirtiness{};
    dao::Field<uint16_t> tailDirtiness{};
    dao::Field<uint16_t> bodyPolish{};
    dao::Field<uint16_t> manePolish{};
    dao::Field<uint16_t> tailPolish{};
    dao::Field<uint16_t> attachment{};
    dao::Field<uint16_t> boredom{};
    dao::Field<uint16_t> stopAmendsPoint{};
  } mountCondition{};
};

struct Housing
{
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<uint16_t> housingId{};
  dao::Field<Clock::time_point> expiresAt{};
  dao::Field<uint32_t> durability{};
};

struct Egg
{
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<Uid> itemUid{InvalidUid};
  dao::Field<Tid> itemTid{InvalidTid};
  dao::Field<Clock::time_point> incubatedAt{};
  dao::Field<uint32_t> incubatorSlot{};
  dao::Field<uint32_t> boostsUsed;
};

} // namespace data

} // namespace server

#endif // DATADEFINITIONS_HPP
