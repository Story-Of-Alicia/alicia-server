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

#ifndef HORSEREGISTRY_HPP
#define HORSEREGISTRY_HPP

#include <array>
#include <filesystem>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "libserver/data/DataDefinitions.hpp"

namespace server::registry
{

enum class Color
{
  White = 1,
  LightBrown = 2,
  Brown = 3,
  DarkBrown = 4,
  Grey = 5,
  Black = 6
};

struct PotentialGrowth
{
  uint32_t type{0};
  std::array<float, 10> weights{};
};

struct PotentialLevel
{
  uint32_t level{0};
  int32_t exp{0};
};

struct PotentialInfo
{
  uint32_t type{0};
  std::string name;
};

struct MasteryParams
{
  uint32_t spurMagicCount{600};
  uint32_t jumpCount{1200};
  uint32_t slidingTime{900};
  uint32_t glidingDistance{8000};
};

struct MasteryReward
{
  uint32_t percent{0};
  uint32_t userExpAdd{0};
  uint32_t gameMoneyAdd{0};
};

struct TendencyRatio
{
  uint32_t tendency{0};
  int32_t buyRatio{0};
  int32_t breedingRatio{0};
};

struct GroupForceCondition
{
  int32_t tendency{0};
  int32_t count{0};
};

struct GroupForceEffect
{
  int32_t effect{0};
  std::string param;
};

struct GroupForce
{
  uint32_t id{0};
  bool affectToAll{false};
  int32_t action{0};
  std::vector<GroupForceCondition> conditions;
  std::vector<GroupForceEffect> effects;
};

struct LevelUpPoints
{
  int32_t base{6650};
  int32_t exp10{13250};
  int32_t exp20{16550};
};

struct GradeInfo
{
  uint32_t grade{0};
  int32_t minStatSum{0};
  int32_t pregnantValue{0};
};

struct EmblemInfo
{
  uint32_t id{0};
  uint32_t odds{0};
};

struct EmblemRatio
{
  uint32_t odds{0};
  int32_t ratio{0};
};

struct Coat
{
  enum class Tier
  {
    Common = 1,
    Uncommon = 2,
    Rare = 3
  };

  data::Tid tid{data::InvalidTid};
  //! -1 and 0
  int32_t faceType{0};
  int32_t minGrade{1};
  Tier tier{Tier::Common};
  //! Base probability weight for this coat (from DNA_SkinInfo)
  float inheritanceRate{1.0f};
  //! Color group index (0-3) from MountSkinInfo/MountColorGroupInfo
  int32_t allowedColorGroups{0};
};

struct Face
{
  data::Tid tid{data::InvalidTid};
  int32_t type{0};
};

struct Mane
{
  data::Tid tid{data::InvalidTid};
  Color color{Color::White};
  int32_t shape{0};
  float inheritanceRate{1.0f};
  int32_t minGrade{1};
  int32_t tier{1};
};

struct Tail
{
  data::Tid tid{data::InvalidTid};
  Color color{Color::White};
  int32_t shape{0};
  float inheritanceRate{1.0f};
  int32_t minGrade{1};
  int32_t tier{1};
};

//! Per-shape inheritance info, aggregated across every colour variant of a shape.
//! Used to roll a grade-eligible mane/tail shape weighted by its inheritance rate,
//! without assuming any particular TID layout.
struct ShapeInheritance
{
  int32_t shape{0};
  int32_t minGrade{1};
  float inheritanceRate{1.0f};
};


class HorseRegistry
{
public:
  HorseRegistry();

  void ReadConfig(const std::filesystem::path& configPath);

  void BuildRandomHorse(
    data::Horse::Parts& parts,
    data::Horse::Appearance& appearance);

  void SetHorsePotential(
    data::Horse::Potential& potential,
    uint8_t type,
    uint8_t level,
    uint8_t value);

  void GiveHorseRandomPotential(
    data::Horse::Potential& potential);

  //! Gets coat information for a given coat TID.
  //! @param coatTid Coat template ID.
  //! @returns Reference to Coat, with fallback to default if not found.
  const Coat& GetCoatInfo(data::Tid coatTid) const;

  //! Gets a random mane TID from the specified color group and shape.
  //! @param colorGroupId Color group ID.
  //! @param shape Mane shape (0-7).
  //! @returns Mane TID, or InvalidTid if not found.
  data::Tid GetRandomManeFromColorAndShape(int32_t colorGroupId, int32_t shape);

  //! Gets a random tail TID from the specified color group and shape.
  //! @param colorGroupId Color group ID.
  //! @param shape Tail shape (0-5).
  //! @returns Tail TID, or InvalidTid if not found.
  data::Tid GetRandomTailByColorGroupAndShape(int32_t colorGroupId, int32_t shape);

  //! Gets the color group ID for a mane TID.
  //! @param maneTid Mane template ID.
  //! @returns Color group ID, or 0 if not found.
  int32_t GetManeColorGroupId(data::Tid maneTid) const;

  //! Gets the color group ID for a tail TID.
  //! @param tailTid Tail template ID.
  //! @returns Color group ID, or 0 if not found.
  int32_t GetTailColorGroupId(data::Tid tailTid) const;

  //! Finds a tail TID with a specific color and shape.
  //! @param color Desired color.
  //! @param shape Desired shape (0-5).
  //! @returns Tail TID, or InvalidTid if not found.
  data::Tid FindTailByColorAndShape(Color color, int32_t shape) const;

  //! Gets mane by TID (for accessing inheritance rate/minGrade).
  //! @param tid Mane TID.
  //! @returns Reference to Mane, with fallback to default if not found.
  const Mane& GetMane(data::Tid tid) const;

  //! Gets tail by TID (for accessing inheritance rate/minGrade).
  //! @param tid Tail TID.
  //! @returns Reference to Tail, with fallback to default if not found.
  const Tail& GetTail(data::Tid tid) const;

  //! Returns every configured coat TID (for weighted random coat selection).
  const std::vector<data::Tid>& GetPossibleCoats() const;

  //! Returns one entry per distinct mane shape, ordered by shape, with the
  //! shape's lowest minGrade and a representative inheritance rate.
  std::vector<ShapeInheritance> GetManeShapeInheritance() const;

  //! Returns one entry per distinct tail shape, ordered by shape, with the
  //! shape's lowest minGrade and a representative inheritance rate.
  std::vector<ShapeInheritance> GetTailShapeInheritance() const;

  //! Gets potential info for a given type ID.
  //! @returns Pointer to PotentialInfo, or nullptr if type not found.
  const PotentialInfo* GetPotentialInfo(uint32_t type) const;

  //! Gets all valid potential type IDs.
  const std::vector<uint32_t>& GetPotentialTypes() const;

  //! Gets potential growth entry by type.
  //! @returns Pointer to PotentialGrowth, or nullptr if not found.
  const PotentialGrowth* GetPotentialGrowth(uint32_t type) const;

  const std::vector<PotentialLevel>& GetPotentialLevels() const;
  const MasteryParams& GetMasteryParams() const;
  const std::vector<MasteryReward>& GetMasteryRewards() const;

  //! Gets tendency ratio for a given tendency ID.
  //! @returns Pointer to TendencyRatio, or nullptr if not found.
  const TendencyRatio* GetTendencyRatio(uint32_t tendency) const;

  //! Gets group force entry by ID.
  //! @returns Pointer to GroupForce, or nullptr if not found.
  const GroupForce* GetGroupForce(uint32_t id) const;

  const LevelUpPoints& GetLevelUpPoints() const;

  //! Total class exp required to reach a given class level (level 1 == 0).
  //! Per-level costs come from LevelUpPoints: levels 1-10 use `base`,
  //! 11-20 use `exp10`, 21-30 use `exp20`.
  //! @param level Class level (1-30).
  //! @returns Cumulative exp required to be at that level.
  uint32_t GetCumulativeClassExp(uint32_t level) const;

  //! Applies gained class exp to a horse. `clazzProgress` stores the lifetime
  //! total and `clazz` the cached level; the level is advanced incrementally
  //! and one growth point is awarded per level gained. Caps at class 30.
  //! @param horse Horse to mutate.
  //! @param gainedExp Class exp gained.
  void ApplyClassProgress(data::Horse& horse, uint32_t gainedExp) const;

  //! Gets grade info by grade number.
  //! @returns Pointer to GradeInfo, or nullptr if not found.
  const GradeInfo* GetGradeInfo(uint32_t grade) const;

  //! Gets emblem info by ID.
  //! @returns Pointer to EmblemInfo, or nullptr if not found.
  const EmblemInfo* GetEmblemInfo(uint32_t id) const;

  //! Gets emblem ratio entry by odds key.
  //! @returns Pointer to EmblemRatio, or nullptr if not found.
  const EmblemRatio* GetEmblemRatio(uint32_t odds) const;

  //! Returns every emblem rarity tier (odds + ratio weight).
  std::vector<EmblemRatio> GetEmblemRatios() const;

  //! Returns the ids of every emblem in the given odds tier.
  std::vector<uint32_t> GetEmblemsByOdds(uint32_t odds) const;

private:
  std::random_device _randomDevice;
  mutable std::mt19937 _randomEngine;

  std::vector<std::vector<Color>> _colorGroups;

  std::unordered_map<data::Tid, Coat> _coats;
  std::unordered_map<data::Tid, Face> _faces;

  std::unordered_map<data::Tid, Mane> _manes;
  std::unordered_map<data::Tid, Tail> _tails;

  std::vector<data::Tid> _possibleCoats;
  std::vector<data::Tid> _possibleFaces;
  std::vector<data::Tid> _possibleManes;
  std::vector<data::Tid> _possibleTails;

  std::unordered_map<uint32_t, PotentialGrowth> _potentialGrowth;
  std::vector<PotentialLevel> _potentialLevels;
  std::unordered_map<uint32_t, PotentialInfo> _potentials;
  std::vector<uint32_t> _potentialTypes;
  MasteryParams _masteryParams;
  std::vector<MasteryReward> _masteryRewards;
  std::unordered_map<uint32_t, TendencyRatio> _tendencies;
  std::unordered_map<uint32_t, GroupForce> _groupForces;
  LevelUpPoints _levelUpPoints;
  std::unordered_map<uint32_t, GradeInfo> _grades;
  std::unordered_map<uint32_t, EmblemInfo> _emblems;
  std::unordered_map<uint32_t, EmblemRatio> _emblemRatios;

  struct ManeTailColorGroup
  {
    data::Tid maneTid{data::InvalidTid};
    data::Tid tailTid{data::InvalidTid};
  };

  //! A vector of manes and tails with a matching color group.
  std::vector<ManeTailColorGroup> maneTailColorGroups;

  // Lookup tables for efficient querying: [colorGroupId][shape] -> vector of TIDs
  std::unordered_map<int32_t, std::unordered_map<int32_t, std::vector<data::Tid>>> _manesByColorAndShape;
  std::unordered_map<int32_t, std::unordered_map<int32_t, std::vector<data::Tid>>> _tailsByColorAndShape;
};

} // namespace server::registry

#endif //HORSEREGISTRY_HPP
