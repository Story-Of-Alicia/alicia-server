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

#include "libserver/registry/HorseRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <map>
#include <ranges>

namespace server::registry
{

namespace
{

constexpr uint32_t FigureScaleMin = 2;
constexpr uint32_t FigureScaleMax = 10;

Color ParseColor(const std::string& str)
{
  if (str == "White")      return Color::White;
  if (str == "LightBrown") return Color::LightBrown;
  if (str == "Brown")      return Color::Brown;
  if (str == "DarkBrown")  return Color::DarkBrown;
  if (str == "Grey")       return Color::Grey;
  if (str == "Black")      return Color::Black;
  throw std::runtime_error("Unknown color: " + str);
}

// Maps ManeTailColor IDs (from MountColorGroupInfo) to Color enum.
// 1=Black, 2=White, 3=Brown, 4=DarkBrown, 5=Grey
Color ParseManeTailColorId(int id)
{
  switch (id)
  {
    case 1: return Color::Black;
    case 2: return Color::White;
    case 3: return Color::Brown;
    case 4: return Color::DarkBrown;
    case 5: return Color::Grey;
    default: throw std::runtime_error("Unknown ManeTailColor ID: " + std::to_string(id));
  }
}

} // anon namespace

HorseRegistry::HorseRegistry()
  : _randomEngine(_randomDevice())
{
}

void HorseRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  // Horse tables are split across category files under the config directory.
  // Merge their top-level sections into a single node so the parsing below
  // stays uniform regardless of which file a section lives in.
  YAML::Node root;
  for (const auto* file : {"appearance.yaml", "potential.yaml", "progression.yaml", "emblems.yaml"})
  {
    const auto node = YAML::LoadFile((configPath / file).string());
    for (const auto& entry : node)
      root[entry.first.as<std::string>()] = entry.second;
  }

  _colorGroups.clear();
  for (const auto& node : root["colorGroups"])
  {
    std::vector<Color> colors;
    for (const auto& idNode : node["colorIds"])
    {
      const Color c = ParseManeTailColorId(idNode.as<int32_t>());
      if (std::find(colors.begin(), colors.end(), c) == colors.end())
        colors.push_back(c);
    }
    _colorGroups.push_back(std::move(colors));
  }

  _coats.clear();
  _possibleCoats.clear();
  for (const auto& node : root["coats"])
  {
    const auto tid = node["tid"].as<data::Tid>();
    _coats[tid] = Coat{
      .tid = tid,
      .faceType = node["faceType"].as<int32_t>(),
      .minGrade = node["minGrade"].as<int32_t>(),
      .tier = static_cast<Coat::Tier>(node["tier"].as<int32_t>()),
      .inheritanceRate = node["inheritanceRate"].as<float>(),
      .allowedColorGroups = node["allowedColorGroups"].as<int32_t>(),
    };
    _possibleCoats.emplace_back(tid);
  }

  _faces.clear();
  _possibleFaces.clear();
  for (const auto& node : root["faces"])
  {
    const auto tid = node["tid"].as<data::Tid>();
    _faces[tid] = Face{
      .tid = tid,
      .type = node["type"].as<int32_t>(),
    };
    _possibleFaces.emplace_back(tid);
  }

  _manes.clear();
  _possibleManes.clear();
  for (const auto& node : root["manes"])
  {
    const auto tid = node["tid"].as<data::Tid>();
    _manes[tid] = Mane{
      .tid = tid,
      .color = ParseColor(node["color"].as<std::string>()),
      .shape = node["shape"].as<int32_t>(),
      .inheritanceRate = node["inheritanceRate"].as<float>(),
      .minGrade = node["minGrade"].as<int32_t>(),
      .tier = node["tier"].as<int32_t>(),
    };
    _possibleManes.emplace_back(tid);
  }

  _tails.clear();
  _possibleTails.clear();
  for (const auto& node : root["tails"])
  {
    const auto tid = node["tid"].as<data::Tid>();
    _tails[tid] = Tail{
      .tid = tid,
      .color = ParseColor(node["color"].as<std::string>()),
      .shape = node["shape"].as<int32_t>(),
      .inheritanceRate = node["inheritanceRate"].as<float>(),
      .minGrade = node["minGrade"].as<int32_t>(),
      .tier = node["tier"].as<int32_t>(),
    };
    _possibleTails.emplace_back(tid);
  }

  _potentialGrowth.clear();
  for (const auto& node : root["potentialGrowth"])
  {
    const auto type = node["type"].as<uint32_t>();
    PotentialGrowth pg{ .type = type };
    const auto weightsNode = node["weights"];
    for (size_t i = 0; i < pg.weights.size(); ++i)
      pg.weights[i] = weightsNode[i].as<float>();
    _potentialGrowth[type] = pg;
  }

  _potentialLevels.clear();
  for (const auto& node : root["potentialLevels"])
  {
    _potentialLevels.push_back(PotentialLevel{
      .level = node["level"].as<uint32_t>(),
      .exp = node["exp"].as<int32_t>(),
    });
  }

  _potentials.clear();
  _potentialTypes.clear();
  for (const auto& node : root["potentials"])
  {
    const auto type = node["type"].as<uint32_t>();
    _potentials[type] = PotentialInfo{
      .type = type,
      .name = node["name"].as<std::string>(),
    };
    _potentialTypes.push_back(type);
  }

  {
    const auto& params = root["mastery"]["params"];
    _masteryParams = MasteryParams{
      .spurMagicCount = params["spurMagicCount"].as<uint32_t>(),
      .jumpCount = params["jumpCount"].as<uint32_t>(),
      .slidingTime = params["slidingTime"].as<uint32_t>(),
      .glidingDistance = params["glidingDistance"].as<uint32_t>(),
    };
  }

  _masteryRewards.clear();
  for (const auto& node : root["mastery"]["rewards"])
  {
    _masteryRewards.push_back(MasteryReward{
      .percent = node["percent"].as<uint32_t>(),
      .userExpAdd = node["userExpAdd"].as<uint32_t>(),
      .gameMoneyAdd = node["gameMoneyAdd"].as<uint32_t>(),
    });
  }

  _tendencies.clear();
  for (const auto& node : root["tendencies"])
  {
    const auto tendency = node["tendency"].as<uint32_t>();
    _tendencies[tendency] = TendencyRatio{
      .tendency = tendency,
      .buyRatio = node["buyRatio"].as<int32_t>(),
      .breedingRatio = node["breedingRatio"].as<int32_t>(),
    };
  }

  _groupForces.clear();
  for (const auto& node : root["groupForces"])
  {
    const auto id = node["id"].as<uint32_t>();
    GroupForce gf{
      .id = id,
      .affectToAll = node["affectToAll"].as<bool>(),
      .action = node["action"].as<int32_t>(),
    };
    for (const auto& cNode : node["conditions"])
      gf.conditions.push_back({
        .tendency = cNode["tendency"].as<int32_t>(),
        .count = cNode["count"].as<int32_t>(),
      });
    for (const auto& eNode : node["effects"])
      gf.effects.push_back({
        .effect = eNode["effect"].as<int32_t>(),
        .param = eNode["param"].as<std::string>(),
      });
    _groupForces[id] = std::move(gf);
  }

  {
    const auto& lup = root["levelUpPoints"];
    _levelUpPoints = LevelUpPoints{
      .base = lup["base"].as<int32_t>(),
      .exp10 = lup["exp10"].as<int32_t>(),
      .exp20 = lup["exp20"].as<int32_t>(),
    };
  }

  _grades.clear();
  for (const auto& node : root["grades"])
  {
    const auto grade = node["grade"].as<uint32_t>();
    _grades[grade] = GradeInfo{
      .grade = grade,
      .minStatSum = node["minStatSum"].as<int32_t>(),
      .pregnantValue = node["pregnantValue"].as<int32_t>(),
    };
  }

  _emblems.clear();
  for (const auto& node : root["emblems"])
  {
    const auto id = node["id"].as<uint32_t>();
    _emblems[id] = EmblemInfo{
      .id = id,
      .odds = node["odds"].as<uint32_t>(),
    };
  }

  _emblemRatios.clear();
  for (const auto& node : root["emblemRatios"])
  {
    const auto odds = node["odds"].as<uint32_t>();
    _emblemRatios[odds] = EmblemRatio{
      .odds = odds,
      .ratio = node["ratio"].as<int32_t>(),
    };
  }

  _manesByColorAndShape.clear();
  _tailsByColorAndShape.clear();

  for (const auto& [tid, mane] : _manes)
  {
    for (int32_t groupId = 0; groupId < static_cast<int32_t>(_colorGroups.size()); ++groupId)
    {
      const auto& colors = _colorGroups[groupId];
      if (std::find(colors.begin(), colors.end(), mane.color) != colors.end())
        _manesByColorAndShape[groupId][mane.shape].push_back(tid);
    }
  }

  for (const auto& [tid, tail] : _tails)
  {
    for (int32_t groupId = 0; groupId < static_cast<int32_t>(_colorGroups.size()); ++groupId)
    {
      const auto& colors = _colorGroups[groupId];
      if (std::find(colors.begin(), colors.end(), tail.color) != colors.end())
        _tailsByColorAndShape[groupId][tail.shape].push_back(tid);
    }
  }
}

void HorseRegistry::BuildRandomHorse(
  data::Horse::Parts& parts,
  data::Horse::Appearance& appearance)
{
  // Pick a random coat.
  std::uniform_int_distribution<size_t> coatRandomDist(
    0, _possibleCoats.size() - 1);

  const Coat& coat = _coats[_possibleCoats[coatRandomDist(_randomDevice)]];
  parts.skinTid = coat.tid;

  // If the coat has a face available, pick a random face.
  if (coat.faceType != 0)
  {
    std::uniform_int_distribution<size_t> faceRandomDist(
      0, _possibleFaces.size() - 1);

    const Face& face = _faces[_possibleFaces[faceRandomDist(_randomDevice)]];
    parts.faceTid = face.tid;
  }

  {
    // Pick a random mane.
    std::uniform_int_distribution<size_t> maneRandomDist(
      0, _possibleManes.size() - 1);

    const Mane& mane = _manes[_possibleManes[maneRandomDist(_randomDevice)]];
    parts.maneTid = mane.tid;
  }

  {
    // Pick a random tail.
    std::uniform_int_distribution<size_t> tailRandomDist(
      0, _possibleFaces.size() - 1);

    const Tail& tail = _tails[_possibleFaces[tailRandomDist(_randomDevice)]];
    parts.tailTid = tail.tid;
  }

  std::uniform_int_distribution figureScaleDist(FigureScaleMin, FigureScaleMax);
  const uint32_t scale = figureScaleDist(_randomDevice);
  appearance.scale =  scale;
  appearance.legLength = scale;
  appearance.legVolume = scale;
  appearance.bodyLength = scale;
  appearance.bodyVolume = scale;
}

void HorseRegistry::GiveHorseRandomPotential(
  data::Horse::Potential& potential)
{
  std::uniform_int_distribution<size_t> typeDist(0, _potentialTypes.size() - 1);
  std::uniform_int_distribution<uint32_t> randomDist(0, 255);
  potential.type = _potentialTypes[typeDist(_randomDevice)];
  potential.level = randomDist(_randomDevice);
  potential.value = randomDist(_randomDevice);
}

const Coat& HorseRegistry::GetCoatInfo(data::Tid coatTid) const
{
  auto it = _coats.find(coatTid);
  if (it != _coats.end())
  {
    return it->second;
  }
  
  // Fallback to Chestnut (coat 1) if not found
  static const Coat invalidCoat{.tid = 1, .faceType = 0, .minGrade = 1, .tier = Coat::Tier::Common, .allowedColorGroups = 0};
  return invalidCoat;
}

data::Tid HorseRegistry::GetRandomManeFromColorAndShape(int32_t colorGroupId, int32_t shape)
{
  auto groupIt = _manesByColorAndShape.find(colorGroupId);
  if (groupIt == _manesByColorAndShape.end())
  {
    return data::InvalidTid;
  }

  auto shapeIt = groupIt->second.find(shape);
  if (shapeIt == groupIt->second.end() || shapeIt->second.empty())
  {
    return data::InvalidTid;
  }

  const auto& candidates = shapeIt->second;
  std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
  return candidates[dist(_randomEngine)];
}

data::Tid HorseRegistry::GetRandomTailByColorGroupAndShape(int32_t colorGroupId, int32_t shape)
{
  auto groupIt = _tailsByColorAndShape.find(colorGroupId);
  if (groupIt == _tailsByColorAndShape.end())
  {
    return data::InvalidTid;
  }

  auto shapeIt = groupIt->second.find(shape);
  if (shapeIt == groupIt->second.end() || shapeIt->second.empty())
  {
    return data::InvalidTid;
  }

  const auto& candidates = shapeIt->second;
  std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
  return candidates[dist(_randomEngine)];
}

int32_t HorseRegistry::GetManeColorGroupId(data::Tid maneTid) const
{
  auto it = _manes.find(maneTid);
  if (it == _manes.end())
    return -1;

  const Color maneColor = it->second.color;
  for (int32_t groupId = 0; groupId < static_cast<int32_t>(_colorGroups.size()); ++groupId)
  {
    const auto& colors = _colorGroups[groupId];
    if (std::find(colors.begin(), colors.end(), maneColor) != colors.end())
      return groupId;
  }

  return -1;
}

int32_t HorseRegistry::GetTailColorGroupId(data::Tid tailTid) const
{
  auto it = _tails.find(tailTid);
  if (it == _tails.end())
    return -1;

  const Color tailColor = it->second.color;
  for (int32_t groupId = 0; groupId < static_cast<int32_t>(_colorGroups.size()); ++groupId)
  {
    const auto& colors = _colorGroups[groupId];
    if (std::find(colors.begin(), colors.end(), tailColor) != colors.end())
      return groupId;
  }

  return -1;
}

data::Tid HorseRegistry::FindTailByColorAndShape(Color color, int32_t shape) const
{
  for (const auto& [tailTid, tailInfo] : _tails)
  {
    if (tailInfo.color == color && tailInfo.shape == shape)
    {
      return tailTid;
    }
  }
  return data::InvalidTid;
}

const Mane& HorseRegistry::GetMane(data::Tid tid) const
{
  auto it = _manes.find(tid);
  if (it != _manes.end())
  {
    return it->second;
  }
  
  // Fallback to white short mane (TID 1) if not found
  static const Mane invalidMane{.tid = 1, .color = Color::White, .shape = 1, .inheritanceRate = 30.0f, .minGrade = 1, .tier = 1};
  return invalidMane;
}

const Tail& HorseRegistry::GetTail(data::Tid tid) const
{
  auto it = _tails.find(tid);
  if (it != _tails.end())
  {
    return it->second;
  }
  
  // Fallback to white medium tail (TID 1) if not found
  static const Tail invalidTail{.tid = 1, .color = Color::White, .shape = 1, .inheritanceRate = 30.0f, .minGrade = 1, .tier = 1};
  return invalidTail;
}

const std::vector<data::Tid>& HorseRegistry::GetPossibleCoats() const
{
  return _possibleCoats;
}

namespace
{

template <typename PartMap>
std::vector<ShapeInheritance> AggregateShapeInheritance(const PartMap& parts)
{
  struct Aggregate
  {
    data::Tid representativeTid{data::InvalidTid};
    int32_t minGrade{0};
    float inheritanceRate{0.0f};
  };

  std::map<int32_t, Aggregate> byShape;
  for (const auto& [tid, part] : parts)
  {
    auto [it, inserted] = byShape.try_emplace(
      part.shape, Aggregate{tid, part.minGrade, part.inheritanceRate});
    if (inserted)
      continue;

    it->second.minGrade = std::min(it->second.minGrade, part.minGrade);
    if (tid < it->second.representativeTid)
    {
      it->second.representativeTid = tid;
      it->second.inheritanceRate = part.inheritanceRate;
    }
  }

  std::vector<ShapeInheritance> result;
  result.reserve(byShape.size());
  for (const auto& [shape, aggregate] : byShape)
    result.push_back({shape, aggregate.minGrade, aggregate.inheritanceRate});
  return result;
}

} // namespace

std::vector<ShapeInheritance> HorseRegistry::GetManeShapeInheritance() const
{
  return AggregateShapeInheritance(_manes);
}

std::vector<ShapeInheritance> HorseRegistry::GetTailShapeInheritance() const
{
  return AggregateShapeInheritance(_tails);
}

const PotentialInfo* HorseRegistry::GetPotentialInfo(uint32_t type) const
{
  auto it = _potentials.find(type);
  return it != _potentials.end() ? &it->second : nullptr;
}

const std::vector<uint32_t>& HorseRegistry::GetPotentialTypes() const
{
  return _potentialTypes;
}

const PotentialGrowth* HorseRegistry::GetPotentialGrowth(uint32_t type) const
{
  auto it = _potentialGrowth.find(type);
  return it != _potentialGrowth.end() ? &it->second : nullptr;
}

const std::vector<PotentialLevel>& HorseRegistry::GetPotentialLevels() const
{
  return _potentialLevels;
}

const MasteryParams& HorseRegistry::GetMasteryParams() const
{
  return _masteryParams;
}

const std::vector<MasteryReward>& HorseRegistry::GetMasteryRewards() const
{
  return _masteryRewards;
}

const TendencyRatio* HorseRegistry::GetTendencyRatio(uint32_t tendency) const
{
  auto it = _tendencies.find(tendency);
  return it != _tendencies.end() ? &it->second : nullptr;
}

const GroupForce* HorseRegistry::GetGroupForce(uint32_t id) const
{
  auto it = _groupForces.find(id);
  return it != _groupForces.end() ? &it->second : nullptr;
}

const LevelUpPoints& HorseRegistry::GetLevelUpPoints() const
{
  return _levelUpPoints;
}

uint32_t HorseRegistry::GetCumulativeClassExp(uint32_t level) const
{
  if (level <= 1)
    return 0;

  const auto base = static_cast<uint32_t>(_levelUpPoints.base);
  const auto exp10 = static_cast<uint32_t>(_levelUpPoints.exp10);
  const auto exp20 = static_cast<uint32_t>(_levelUpPoints.exp20);

  // Number of level-ups needed to reach `level` from level 1.
  const uint32_t transitions = level - 1;

  // Levels 1-10 cost `base`, 11-20 cost `exp10`, 21-30 cost `exp20`.
  uint32_t total = std::min<uint32_t>(transitions, 10) * base;
  if (transitions > 10)
    total += std::min<uint32_t>(transitions - 10, 10) * exp10;
  if (transitions > 20)
    total += (transitions - 20) * exp20;

  return total;
}

void HorseRegistry::ApplyClassProgress(data::Horse& horse, uint32_t gainedExp) const
{
  constexpr uint32_t MaxClass = 30;

  uint32_t level = std::max<uint32_t>(horse.clazz(), 1);
  if (level >= MaxClass)
    return;

  // clazzProgress is the lifetime total; advance the cached class while the
  // next level's cumulative requirement is met.
  horse.clazzProgress() += gainedExp;

  while (level < MaxClass
    && horse.clazzProgress() >= GetCumulativeClassExp(level + 1))
  {
    level += 1;
    horse.growthPoints() += 1;
  }

  horse.clazz() = level;
}

uint32_t HorseRegistry::ApplyPotentialGrowth(data::Horse& horse) const
{
  constexpr uint32_t MaxTransitionLevel = 10;
  constexpr uint32_t MaxPotentialValue = 100;
  constexpr uint32_t MaxPotentialPoints = 15;

  if (horse.potential.type() == 0)
    return 0;

  uint32_t targetLevel = 1;
  for (const auto& potentialLevel : _potentialLevels)
  {
    if (static_cast<int32_t>(horse.clazzProgress()) >= potentialLevel.exp)
      targetLevel = std::max(targetLevel, potentialLevel.level);
  }

  uint32_t level = std::max<uint32_t>(horse.potential.level(), 1);

  uint32_t gained = 0;
  while (level < targetLevel && level <= MaxTransitionLevel)
  {
    const size_t columnIndex = level - 1;

    std::vector<uint32_t> points;
    std::vector<float> weights;
    points.reserve(_potentialGrowth.size());
    weights.reserve(_potentialGrowth.size());
    for (const auto& [pointAmount, growth] : _potentialGrowth)
    {
      if (pointAmount > MaxPotentialPoints)
        continue;
      points.emplace_back(pointAmount);
      weights.emplace_back(growth.weights[columnIndex]);
    }

    std::discrete_distribution<size_t> raffle(weights.begin(), weights.end());
    gained += points[raffle(_randomEngine)];

    level += 1;
  }

  if (gained == 0)
    return 0;

  horse.potential.value() = std::min(horse.potential.value() + gained, MaxPotentialValue);
  horse.potential.level() = level;
  return gained;
}

const GradeInfo* HorseRegistry::GetGradeInfo(uint32_t grade) const
{
  auto it = _grades.find(grade);
  return it != _grades.end() ? &it->second : nullptr;
}

const EmblemInfo* HorseRegistry::GetEmblemInfo(uint32_t id) const
{
  auto it = _emblems.find(id);
  return it != _emblems.end() ? &it->second : nullptr;
}

const EmblemRatio* HorseRegistry::GetEmblemRatio(uint32_t odds) const
{
  auto it = _emblemRatios.find(odds);
  return it != _emblemRatios.end() ? &it->second : nullptr;
}

std::vector<EmblemRatio> HorseRegistry::GetEmblemRatios() const
{
  std::vector<EmblemRatio> result;
  result.reserve(_emblemRatios.size());
  for (const auto& [odds, ratio] : _emblemRatios)
    result.push_back(ratio);
  return result;
}

std::vector<uint32_t> HorseRegistry::GetEmblemsByOdds(uint32_t odds) const
{
  std::vector<uint32_t> result;
  for (const auto& [id, emblem] : _emblems)
  {
    if (emblem.odds == odds)
      result.push_back(id);
  }
  return result;
}

} // namespace server