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
#include <functional>
#include <unordered_set>
#include <string>

namespace soa
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

  Field(const Field& field) = delete;

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

  //! Constructs field with an initialized value.
  Field()
    : _value()
  {
  }

  //! Constructs a field with an uninitialized value.
  explicit Field(std::nullptr_t t)
  {
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
  //! Slot type of the item.
  enum class Slot
  {
    CharacterEquipment,
    HorseEquipment,
    Storage,
  };

  //! A unique identifier.
  dao::Field<Uid> uid{InvalidUid};
  //! A type identifier.
  dao::Field<Tid> tid{InvalidTid};
  //! Amount of an item.
  dao::Field<uint32_t> count{};
  //! Slot of the item.
  dao::Field<Slot> slot{Slot::Storage};
};

struct StoredItem
{
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<std::vector<Uid>> items{};
  dao::Field<std::string> sender;
  dao::Field<std::string> message;
  dao::Field<bool> checked;
  dao::Field<bool> expired;
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
    dao::Field<uint32_t> headSize{0u};
    dao::Field<uint32_t> height{0u};
    dao::Field<uint32_t> thighVolume{0u};
    dao::Field<uint32_t> legVolume{0u};
  } appearance{};

  dao::Field<std::vector<Uid>> gifts;
  dao::Field<std::vector<Uid>> purchases;

  dao::Field<std::vector<Uid>> inventory;
  dao::Field<std::vector<Uid>> characterEquipment;
  dao::Field<std::vector<Uid>> mountEquipment;

  dao::Field<std::vector<Uid>> horses;
  dao::Field<Uid> mountUid{InvalidUid};

  dao::Field<Uid> ranchUid{InvalidUid};
};

struct Horse
{
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<Uid> tid{InvalidTid};
  dao::Field<std::string> name{};

  struct Parts
  {
    dao::Field<uint32_t> skinId{0u};
    dao::Field<uint32_t> faceId{0u};
    dao::Field<uint32_t> maneId{0u};
    dao::Field<uint32_t> tailId{0u};
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
    dao::Field<uint32_t> control{0u};
    dao::Field<uint32_t> speed{0u};
    dao::Field<uint32_t> strength{0u};
    dao::Field<uint32_t> spirit{0u};
  } stats{};

  struct Mastery
  {
    dao::Field<uint32_t> spurMagicCount{0u};
    dao::Field<uint32_t> jumpCount{0u};
    dao::Field<uint32_t> slidingTime{0u};
    dao::Field<uint32_t> glidingDistance{0u};
  } mastery;

  dao::Field<uint32_t> rating{0u};
  dao::Field<uint32_t> clazz{0u};
  dao::Field<uint32_t> clazzProgress{0u};
  dao::Field<uint32_t> grade{0u};
  dao::Field<uint32_t> growthPoints{0u};

  dao::Field<uint32_t> potentialType{0u};
  dao::Field<uint32_t> potentialLevel{0u};

  dao::Field<uint32_t> luckState{0u};
  dao::Field<uint32_t> emblem{0u};
};

struct Ranch
{
  dao::Field<Uid> uid{InvalidUid};
  dao::Field<std::string> name{};
};

} // namespace data

} // namespace soa

#endif // DATADEFINITIONS_HPP
