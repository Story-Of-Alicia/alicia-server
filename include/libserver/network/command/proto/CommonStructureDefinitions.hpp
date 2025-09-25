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

#include "libserver/util/Stream.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace server
{

//!
enum class Gender : uint8_t
{
  Unspecified = 0x0,
  Boy = 0x1,
  Girl = 0x2
};

//! Item
struct Item
{
  uint32_t uid{};
  uint32_t tid{};
  uint32_t expiresAt{};
  uint32_t count{};

  static void Write(const Item& item, SinkStream& stream);
  static void Read(Item& item, SourceStream& stream);
};

struct StoredItem
{
  enum class Status : uint8_t {
    Unread = 0,
    Expired = 1,
    Read = 2
  };

  uint32_t uid{};
  uint32_t val1{};
  Status status{};
  // hide sender and message.
  uint32_t val3{};
  uint32_t val4{};
  uint32_t val5{};
  uint32_t val6{};
  std::string sender;
  std::string message;
  //! [0000'00][00'0000]'[0000'0000]'[0000]'[0000'0000'0000]
  //! [minute] [hour] [day] [month] [year]
  uint32_t dateAndTime{};

  static void Write(const StoredItem& item, SinkStream& stream);
  static void Read(StoredItem& item, SourceStream& stream);
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

    static void Write(const Option& option, SinkStream& stream);
    static void Read(Option& option, SourceStream& stream);
  };

  std::vector<Option> bindings{};

  static void Write(const KeyboardOptions& value, SinkStream& stream);
  static void Read(KeyboardOptions& value, SourceStream& stream);
};

struct MacroOptions
{
  std::array<std::string, 8> macros;

  static void Write(const MacroOptions& value, SinkStream& stream);
  static void Read(MacroOptions& value, SourceStream& stream);
};

//!
struct Character
{
  //! Used to build character from the _ClientCharDefaultPartInfo table.
  struct Parts
  {
    //!
    uint8_t charId{10};
    //! FaceId
    uint8_t mouthSerialId{};
    //! EyeId
    uint8_t faceSerialId{};
    //!
    uint8_t val0{};

    static void Write(const Parts& value, SinkStream& stream);
    static void Read(Parts& value, SourceStream& stream);
  } parts{};

  //!
  struct Appearance
  {
    //!
    uint16_t voiceId{};
    //! FigFace
    uint16_t headSize{};
    //! FigTall
    uint16_t height{};
    //! FigVolume
    uint16_t thighVolume{};
    //! FigShape
    uint16_t legVolume{};
    //!
    uint16_t emblemId{};

    static void Write(const Appearance& value, SinkStream& stream);
    static void Read(Appearance& value, SourceStream& stream);
  } appearance{};

  static void Write(const Character& value, SinkStream& stream);
  static void Read(Character& value, SourceStream& stream);
};

//!
struct Horse
{
  //!
  uint32_t uid{};
  //!
  uint32_t tid{};
  //! Max length is 255.
  std::string name{};
  //!
  enum class HorseType : uint8_t
  {
    Adult = 0,
    Foal = 1,
    Stallion = 2,
    Rented = 3
  };

  //!
  struct Parts
  {
    //!
    uint8_t skinId{};
    //!
    uint8_t maneId{};
    //!
    uint8_t tailId{};
    //!
    uint8_t faceId{};

    static void Write(const Parts& value, SinkStream& stream);
    static void Read(Parts& value, SourceStream& stream);
  } parts{};

  //! Figure
  struct Appearance
  {
    //!
    uint8_t scale{};
    //!
    uint8_t legLength{};
    //!
    uint8_t legVolume{};
    //!
    uint8_t bodyLength{};
    //!
    uint8_t bodyVolume{};

    static void Write(const Appearance& value, SinkStream& stream);
    static void Read(Appearance& value, SourceStream& stream);
  } appearance{};

  struct Stats
  {
    //! Agility value.
    uint32_t agility{};
    //! Ambition (spirit) value.
    uint32_t ambition{};
    //! Rush (speed) value.
    uint32_t rush{};
    //! Endurance (strength) value.
    uint32_t endurance{};
    //! Courage (control) value.
    uint32_t courage{};

    static void Write(const Stats& value, SinkStream& stream);
    static void Read(Stats& value, SourceStream& stream);
  } stats{};

  uint32_t rating{};
  uint8_t clazz{};
  uint8_t val0{}; // classProgress
  uint8_t grade{};
  uint16_t growthPoints{};

  struct MountCondition
  {
    //! Stamina: range <0, 4000>
    uint16_t stamina{};
    //! Charm (attractiveness, beauty) value in a range of <0, 1000>. 
    uint16_t charmPoint{};
    //! Friendliness (intimacy) value in a range of <0, 1000>. 
    uint16_t friendlyPoint{};
    uint16_t injuryPoint{};

    //! A plenitude value in a range of <0, 1200>.
    //! 910 is a little full, 1200 is full
    uint16_t plenitude{};
    //! A dirty value in a range of <0, 600>. for all body parts.
    //! 600 is fully dirty, 0 is clean.
    uint16_t bodyDirtiness{};
    //! Referred to as `ManeTwisted` by the client.
    uint16_t maneDirtiness{};
    //! Referred to as `TailTwisted` by the client.
    uint16_t tailDirtiness{};

    //! An attachment (trust) value with a possibly RNG thresholds for certain play activities.
    //! >111 - Fish on a rod play activity unlocked
    //! >501 - Bow play activity unlocked
    uint16_t attachment{};  
    //! A boredom value in a range of <0, 21>.
    //! 0 is bored
    //! 1 is a little bored
    //! 11 wants to play a little
    //! 21 wants to play.
    uint16_t boredom{};

    uint16_t bodyPolish{};
    uint16_t manePolish{};
    uint16_t tailPolish{};
    
    uint16_t stopAmendsPoint{};
  } mountCondition{};

  enum class Injury : uint8_t
  {
    None = 0,
    MinorMuscleStrain = 17,
    SevereMuscleStrain = 18,
    MinorWounds = 33,
    SevereWounds = 34,
    MinorFracture = 65,
    SevereFracture = 66,
  };

  struct
  {
    HorseType type{};
    uint32_t val1{};
    uint32_t dateOfBirth{};

    //! The different horse tendencies can be found in the Tendency table
    uint8_t tendency{};
    uint8_t spirit{};
    uint32_t classProgression{};
    uint32_t val5{};

    uint8_t potentialLevel{};
    uint8_t hasPotential{};
    uint8_t potentialValue{};
    uint8_t val9{};

    uint8_t luck{};
    Injury injury{};
    uint8_t val12{};

    //! Fatigue: range <0, 1500>
    uint16_t fatigue{};
    uint16_t val14{};
    uint16_t emblem{};
  } vals1{};

  struct Mastery
  {
    uint32_t spurMagicCount{};
    uint32_t jumpCount{};
    uint32_t slidingTime{};
    //! Divided by 10?
    uint32_t glidingDistance{};

    static void Write(const Mastery& value, SinkStream& stream);
    static void Read(Mastery& value, SourceStream& stream);
  } mastery{};

  uint32_t val16{};
  uint32_t val17{};

  static void Write(const Horse& value, SinkStream& stream);
  static void Read(Horse& value, SourceStream& stream);
};

//!
struct Guild
{
  uint32_t uid{};
  uint8_t val1{};
  uint32_t val2{};
  std::string name{};
  uint8_t val4{};
  uint32_t val5{};
  // ignored by the client?
  uint8_t val6{};

  static void Write(const Guild& value, SinkStream& stream);
  static void Read(Guild& value, SourceStream& stream);
};

//!
struct Rent
{
  uint32_t mountUid{};
  uint32_t val1{};
  uint32_t val2{};

  static void Write(const Rent& value, SinkStream& stream);
  static void Read(Rent& value, SourceStream& stream);
};

//!
struct Pet
{
  uint32_t petId{};
  uint32_t member2{};
  std::string name{};
  uint32_t birthDate{};

  static void Write(const Pet& value, SinkStream& stream);
  static void Read(Pet& value, SourceStream& stream);
};

//!
struct Egg
{
  uint32_t uid{};
  uint32_t itemTid{};
  uint32_t member3{};
  uint8_t member4{};
  uint32_t member5{};
  uint32_t timeRemaining{};
  uint32_t boost{}; //needs further investigation and possibly a rename
  uint32_t totalHatchingTime{};
  uint32_t member9{};

  static void Write(const Egg& value, SinkStream& stream);
  static void Read(Egg& value, SourceStream& stream);
};


//!
struct PetInfo
{
  uint32_t characterUid{};
  uint32_t itemUid{}; //can also be an eggUid
  Pet pet{};
  uint32_t member4{};

  static void Write(const PetInfo& value, SinkStream& stream);
  static void Read(PetInfo& value, SourceStream& stream);
};

//!
struct PetBirthInfo
{
  Item eggItem{};
  uint32_t member2{};
  uint32_t member3{};
  PetInfo petInfo{};

  static void Write(
    const PetBirthInfo& value,
    SinkStream& stream);
  static void Read(
    PetBirthInfo& value,
    SourceStream& stream);
};

//!
struct RanchHorse
{
  uint16_t horseOid{};
  Horse horse{};

  static void Write(const RanchHorse& value, SinkStream& stream);
  static void Read(RanchHorse& value, SourceStream& stream);
};

//!
struct RanchCharacter
{
  uint32_t uid{};
  std::string name{};
  enum class Role : uint8_t
  {
    User = 0x0,
    Op = 0x1, // Assumed, tested but no effect
    GameMaster = 0x2
  } role{Role::User};
  uint8_t age{};
  enum class Gender : uint8_t
  {
    Girl = 0,
    Boy = 1
  } gender{Gender::Girl};
  std::string introduction{};

  Character character{};
  Horse mount{};
  std::vector<Item> characterEquipment{};

  Guild guild{};

  //! Unique ranch object identifier.
  uint16_t oid{};
  uint8_t isBusy{0};
  uint8_t unk3{0};

  Rent rent{};
  Pet pet{};

  uint8_t unk4{0};
  uint8_t unk5{0};

  static void Write(const RanchCharacter& ranchCharacter, SinkStream& stream);
  static void Read(RanchCharacter& value, SourceStream& stream);
};

//!
struct Quest
{
  uint16_t tid{};
  uint32_t member0{};
  uint8_t member1{};
  uint32_t member2{};
  uint8_t member3{};
  uint8_t member4{};

  static void Write(const Quest& value, SinkStream& stream);
  static void Read(Quest& value, SourceStream& stream);
};

//! Housing
struct Housing
{
  uint32_t uid{};
  uint16_t tid{};
  uint32_t durability{};

  static void Write(const Housing& value, SinkStream& stream);
  static void Read(Housing& value, SourceStream& stream);
};

//!
struct League
{
  enum class Type : uint8_t
  {
    None = 0,
    Bronze = 1,
    Silver = 2,
    Gold = 3,
    Platinum = 4
  };

  Type type{};
  //! League rank percentile expressed as a whole number in an interval <0, 100>.
  uint8_t rankingPercentile{};

  static void Write(const League& value, SinkStream& stream);
  static void Read(League& value, SourceStream& stream);
};

enum class GameMode : uint8_t
{
  Nothing = 0,
  Speed = 1,
  Magic = 2,
  Guild = 3,
  Tutorial = 6
};

enum class TeamMode : uint8_t
{
  Single = 1,
  Team = 2
};

} // namespace server

#endif
