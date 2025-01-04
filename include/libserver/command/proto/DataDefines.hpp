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

#ifndef DATA_DEFINES_HPP
#define DATA_DEFINES_HPP

#include <array>
#include <cstdint>
#include <vector>
#include <string>

namespace alicia
{

//!
enum class Gender : uint8_t
{
  Unspecified = 0x0,
  Boy = 0x1,
  Girl = 0x2
};

//!
enum class AgeGroup : uint8_t
{
  //! Age <12
  Kid = 0x0C,
  //! Age 13-15
  Teenager = 0x0D,
  //! Age 16-18
  Highschooler = 0x10,
  //! Age 19+
  Adult = 0x13,
};

//! Item
struct Item
{
  uint32_t uid{};
  uint32_t tid{};
  uint32_t val{};
  uint32_t count{};
};

//!
enum class OptionType : uint32_t
{
  Keyboard = 1 << 0,
  Macros = 1 << 3,
  Value = 1 << 4,
  Gamepad = 1 << 5,
};

//!
struct KeyboardOptions
{
  struct Option
  {
    uint16_t index{};
    uint8_t type{};
    uint8_t key{};
  };

  std::vector<Option> bindings{};
};

struct MacroOptions
{
  std::array<std::string, 8> macros;
};

struct Character
{
  //! Used to build character from the _ClientCharDefaultPartInfo table.
  struct CharacterParts
  {
    uint8_t charId{ 10 };
    //! FaceId
    uint8_t mouthSerialId{};
    //! EyeId
    uint8_t faceSerialId{};

    uint8_t val0{};
  } parts{};

  //! Seems to not be based on any physical units...
  struct CharacterAppearance
  {
    uint16_t val0{};
    //! FigFace
    uint16_t headSize{};
    //! FigTall
    uint16_t height{};
    //! FigVolume
    uint16_t thighVolume{};
    //! FigShape
    uint16_t legVolume{};

    uint16_t val1{};
  } appearance{};
};

struct Horse
{
  uint32_t uid{};
  uint32_t tid{};
  std::string name{};

  struct Parts
  {
    uint8_t skinId{};
    uint8_t maneId{};
    uint8_t tailId{};
    uint8_t faceId{};
  } parts{};

  //! Figure
  struct Appearance
  {
    uint8_t scale{};
    uint8_t legLength{};
    uint8_t legVolume{};
    uint8_t bodyLength{};
    uint8_t bodyVolume{};
  } appearance{};

  struct Stats
  {
    uint32_t agility{};
    uint32_t spirit{};
    uint32_t speed{};
    uint32_t strength{};
    uint32_t ambition{};
  } stats{};

  uint32_t rating{};
  uint8_t clazz{};
  uint8_t val0{}; // classProgress
  uint8_t grade{};
  uint16_t growthPoints{};

  struct
  {
    uint16_t stamina{};
    uint16_t attractiveness{};

    uint16_t hunger{};
    uint16_t val0{};

    uint16_t val1{};
    uint16_t val2{};

    uint16_t val3{};
    uint16_t val4{};

    uint16_t val5{};
    uint16_t val6{};

    uint16_t val7{};
    uint16_t val8{};

    uint16_t val9{};
    uint16_t val10{};
  } vals0{};

  struct
  {
    uint8_t val0{};
    uint32_t val1{};
    uint32_t val2{};

    uint8_t val3{};
    uint8_t val4{};
    uint32_t classProgression{};
    uint32_t val5{};

    uint8_t val6{};
    uint8_t val7{};
    uint8_t val8{};
    uint8_t val9{};

    uint8_t val10{};
    uint8_t val11{};
    uint8_t val12{};

    uint16_t val13{};
    uint16_t val14{};
    uint16_t val15{};
  } vals1{};

  struct Mastery
  {
    uint32_t magic{};
    uint32_t jumping{};
    uint32_t sliding{};
    //! Divided by 10?
    uint32_t gliding{};
  } mastery{};

  uint32_t val16{};
  uint32_t val17{};
};

//!
struct Struct5
{
  uint32_t val0{};
  uint8_t val1{};
  uint32_t val2{};
  std::string val3{};
  uint8_t val4{};
  uint32_t val5{};
  // ignored by the client?
  uint8_t val6{};
};

//!
struct Struct6
{
  uint32_t mountUid{};
  uint32_t val1{};
  uint32_t val2{};
};

//!
struct Struct7
{
  uint32_t val0{};
  uint32_t val1{};
  std::string val2{};
  uint32_t val3{};
};

//!
struct RanchHorse {
  uint16_t ranchIndex{};
  Horse horse{};
};

//!
struct RanchPlayer
{
  uint32_t userUid{};
  std::string name{};
  Gender gender{};
  uint8_t unk0{};
  uint8_t unk1{};
  std::string description{};

  Character character{};
  Horse horse{};
  std::vector<Item> characterEquipment{};

  Struct5 playerRelatedThing{};

  uint16_t ranchIndex{};
  uint8_t unk2{};
  uint8_t unk3{};

  Struct6 anotherPlayerRelatedThing{};
  Struct7 yetAnotherPlayerRelatedThing{};

  uint8_t unk4{};
  uint8_t unk5{};
};


} // namespace alicia

#endif