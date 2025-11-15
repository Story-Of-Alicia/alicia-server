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

namespace server::registry
{

namespace
{

constexpr uint32_t FigureScaleMin = 2;
constexpr uint32_t FigureScaleMax = 10;

} // anon namespace

HorseRegistry::HorseRegistry()
  : _randomEngine(_randomDevice())
{
  // Define color groups
  // Color group 1: White, Light Brown, Dark Brown
  // Color group 2: Grey, White, Black  
  // Color group 3: Grey, White, Light Brown (Champagne special)
  _colorGroups = {
    {1, ColorGroup{.id = 1, .colors = {Color::White, Color::LightBrown, Color::DarkBrown}}},
    {2, ColorGroup{.id = 2, .colors = {Color::Grey, Color::White, Color::Black}}},
    {3, ColorGroup{.id = 3, .colors = {Color::Grey, Color::White, Color::Brown}}},
  };

  _coats = {
  // 1 Star Coats: Chestnut, Bay, Champagne Sabino, Chestnut Stockings, Buckskin, Champagne, Leopard Appaloosa
  {1, Coat{.tid = 1, .faceType = 0, .minGrade = 1, .tier = Coat::Tier::Common, .stars = 1, .inheritanceRate = 30.0f, .allowedColorGroups = {1}}},   // Chestnut
  {2, Coat{.tid = 2, .faceType = -1, .minGrade = 1, .tier = Coat::Tier::Common, .stars = 1, .inheritanceRate = 25.0f, .allowedColorGroups = {1}}},  // Bay
  {3, Coat{.tid = 3, .faceType = 0, .minGrade = 1, .tier = Coat::Tier::Common, .stars = 1, .inheritanceRate = 30.0f, .allowedColorGroups = {2}}},   // Champagne Sabino
  {4, Coat{.tid = 4, .faceType = 0, .minGrade = 2, .tier = Coat::Tier::Common, .stars = 1, .inheritanceRate = 21.0f, .allowedColorGroups = {1}}},   // Chestnut Stockings
  {5, Coat{.tid = 5, .faceType = -1, .minGrade = 3, .tier = Coat::Tier::Common, .stars = 1, .inheritanceRate = 23.0f, .allowedColorGroups = {2}}},  // Buckskin
  {6, Coat{.tid = 6, .faceType = 0, .minGrade = 3, .tier = Coat::Tier::Common, .stars = 1, .inheritanceRate = 21.0f, .allowedColorGroups = {3}}},   // Champagne
  {7, Coat{.tid = 7, .faceType = 0, .minGrade = 5, .tier = Coat::Tier::Common, .stars = 1, .inheritanceRate = 21.0f, .allowedColorGroups = {1}}},   // Leopard Appaloosa
  // 2 Star Coats: Blanket Appaloosa, Dapple Grey, Chestnut Pinto, Palomino, Black Pinto, Sooty Bay
  {8, Coat{.tid = 8, .faceType = -1, .minGrade = 6, .tier = Coat::Tier::Uncommon, .stars = 2, .inheritanceRate = 15.0f, .allowedColorGroups = {2}}},  // Blanket Appaloosa
  {10, Coat{.tid = 10, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .stars = 2, .inheritanceRate = 15.0f, .allowedColorGroups = {2}}}, // Dapple Grey
  {11, Coat{.tid = 11, .faceType = -1, .minGrade = 6, .tier = Coat::Tier::Uncommon, .stars = 2, .inheritanceRate = 18.0f, .allowedColorGroups = {1}}}, // Chestnut Pinto
  {13, Coat{.tid = 13, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .stars = 2, .inheritanceRate = 15.0f, .allowedColorGroups = {1}}}, // Palomino
  {14, Coat{.tid = 14, .faceType = 0, .minGrade = 7, .tier = Coat::Tier::Uncommon, .stars = 2, .inheritanceRate = 19.0f, .allowedColorGroups = {2}}}, // Black Pinto
  {12, Coat{.tid = 12, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .stars = 2, .inheritanceRate = 11.0f, .allowedColorGroups = {1}}}, // Sooty Bay
  // 3 Star Coats: White Grey, Chestnut Sabino, Black, Mealy Bay, Amber Cream, Black Sabino, Dapple Bay
  {9, Coat{.tid = 9, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .stars = 3, .inheritanceRate = 7.0f, .allowedColorGroups = {2}}},   // White Grey
  {16, Coat{.tid = 16, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Rare, .stars = 3, .inheritanceRate = 12.0f, .allowedColorGroups = {1}}}, // Chestnut Sabino
  {15, Coat{.tid = 15, .faceType = -1, .minGrade = 7, .tier = Coat::Tier::Rare, .stars = 3, .inheritanceRate = 7.0f, .allowedColorGroups = {2}}}, // Black
  {18, Coat{.tid = 18, .faceType = 0, .minGrade = 7, .tier = Coat::Tier::Rare, .stars = 3, .inheritanceRate = 10.0f, .allowedColorGroups = {1}}}, // Mealy Bay
  {17, Coat{.tid = 17, .faceType = 0, .minGrade = 8, .tier = Coat::Tier::Rare, .stars = 3, .inheritanceRate = 10.0f, .allowedColorGroups = {1}}}, // Amber Cream
  {19, Coat{.tid = 19, .faceType = -1, .minGrade = 8, .tier = Coat::Tier::Rare, .stars = 3, .inheritanceRate = 10.0f, .allowedColorGroups = {2}}}, // Black Sabino
  {20, Coat{.tid = 20, .faceType = 0, .minGrade = 8, .tier = Coat::Tier::Rare, .stars = 3, .inheritanceRate = 5.0f, .allowedColorGroups = {1}}}, // Dapple Bay
  };

  _faces = {
    {1, Face{.tid = 1, .type = -1}},
    {2, Face{.tid = 2, .type = -1}},
    {3, Face{.tid = 3, .type = -1}},
    {5, Face{.tid = 5, .type = -1}},
    {7, Face{.tid = 7, .type = -1}},
  };

  // Mane TIDs 1-40: 8 shapes x 5 colors
  // Pattern: TID = (shape * 5) + color

  _manes = {
    // Shape 0 (Medium short): 30% inheritance, grade 1+
    {1, Mane{.tid = 1, .color = Color::White, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {2, Mane{.tid = 2, .color = Color::LightBrown, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {3, Mane{.tid = 3, .color = Color::Brown, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {4, Mane{.tid = 4, .color = Color::DarkBrown, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {5, Mane{.tid = 5, .color = Color::Grey, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 1 (Medium): 20% inheritance, grade 6+
    {6, Mane{.tid = 6, .color = Color::White, .shape = 1, .inheritanceRate = 20.0f, .minGrade = 6}},
    {7, Mane{.tid = 7, .color = Color::LightBrown, .shape = 1, .inheritanceRate = 20.0f, .minGrade = 6}},
    {8, Mane{.tid = 8, .color = Color::Brown, .shape = 1, .inheritanceRate = 20.0f, .minGrade = 6}},
    {9, Mane{.tid = 9, .color = Color::DarkBrown, .shape = 1, .inheritanceRate = 20.0f, .minGrade = 6}},
    {10, Mane{.tid = 10, .color = Color::Grey, .shape = 1, .inheritanceRate = 20.0f, .minGrade = 6}},
    // Shape 2 (Shaved): 30% inheritance, grade 1+
    {11, Mane{.tid = 11, .color = Color::White, .shape = 2, .inheritanceRate = 30.0f, .minGrade = 1}},
    {12, Mane{.tid = 12, .color = Color::LightBrown, .shape = 2, .inheritanceRate = 30.0f, .minGrade = 1}},
    {13, Mane{.tid = 13, .color = Color::Brown, .shape = 2, .inheritanceRate = 30.0f, .minGrade = 1}},
    {14, Mane{.tid = 14, .color = Color::DarkBrown, .shape = 2, .inheritanceRate = 30.0f, .minGrade = 1}},
    {15, Mane{.tid = 15, .color = Color::Grey, .shape = 2, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 3 (Short): 30% inheritance, grade 1+
    {16, Mane{.tid = 16, .color = Color::White, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {17, Mane{.tid = 17, .color = Color::LightBrown, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {18, Mane{.tid = 18, .color = Color::Brown, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {19, Mane{.tid = 19, .color = Color::DarkBrown, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {20, Mane{.tid = 20, .color = Color::Grey, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 4 (Extremely short): 30% inheritance, grade 1+
    {21, Mane{.tid = 21, .color = Color::White, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {22, Mane{.tid = 22, .color = Color::LightBrown, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {23, Mane{.tid = 23, .color = Color::Brown, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {24, Mane{.tid = 24, .color = Color::DarkBrown, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {25, Mane{.tid = 25, .color = Color::Grey, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 5 (Spiky): 30% inheritance, grade 4+
    {26, Mane{.tid = 26, .color = Color::White, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 4}},
    {27, Mane{.tid = 27, .color = Color::LightBrown, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 4}},
    {28, Mane{.tid = 28, .color = Color::Brown, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 4}},
    {29, Mane{.tid = 29, .color = Color::DarkBrown, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 4}},
    {30, Mane{.tid = 30, .color = Color::Grey, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 4}},
    // Shape 6 (Long): 15% inheritance, grade 6+
    {31, Mane{.tid = 31, .color = Color::White, .shape = 6, .inheritanceRate = 15.0f, .minGrade = 6}},
    {32, Mane{.tid = 32, .color = Color::LightBrown, .shape = 6, .inheritanceRate = 15.0f, .minGrade = 6}},
    {33, Mane{.tid = 33, .color = Color::Brown, .shape = 6, .inheritanceRate = 15.0f, .minGrade = 6}},
    {34, Mane{.tid = 34, .color = Color::DarkBrown, .shape = 6, .inheritanceRate = 15.0f, .minGrade = 6}},
    {35, Mane{.tid = 35, .color = Color::Grey, .shape = 6, .inheritanceRate = 15.0f, .minGrade = 6}},
    // Shape 7 (Curly): 5% inheritance, grade 7+
    {36, Mane{.tid = 36, .color = Color::White, .shape = 7, .inheritanceRate = 5.0f, .minGrade = 7}},
    {37, Mane{.tid = 37, .color = Color::LightBrown, .shape = 7, .inheritanceRate = 5.0f, .minGrade = 7}},
    {38, Mane{.tid = 38, .color = Color::Brown, .shape = 7, .inheritanceRate = 5.0f, .minGrade = 7}},
    {39, Mane{.tid = 39, .color = Color::DarkBrown, .shape = 7, .inheritanceRate = 5.0f, .minGrade = 7}},
    {40, Mane{.tid = 40, .color = Color::Grey, .shape = 7, .inheritanceRate = 5.0f, .minGrade = 7}},
  };

  // Tail TIDs 1-30: 6 shapes x 5 colors
  // Pattern: TID = (shape * 5) + color
  _tails = {
    // Shape 0 (Medium): 30% inheritance
    {1, Tail{.tid = 1, .color = Color::White, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {2, Tail{.tid = 2, .color = Color::LightBrown, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {3, Tail{.tid = 3, .color = Color::Brown, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {4, Tail{.tid = 4, .color = Color::DarkBrown, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    {5, Tail{.tid = 5, .color = Color::Grey, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 1 (Long Thick): 30% inheritance
    {6, Tail{.tid = 6, .color = Color::White, .shape = 1, .inheritanceRate = 30.0f, .minGrade = 1}},
    {7, Tail{.tid = 7, .color = Color::LightBrown, .shape = 1, .inheritanceRate = 30.0f, .minGrade = 1}},
    {8, Tail{.tid = 8, .color = Color::Brown, .shape = 1, .inheritanceRate = 30.0f, .minGrade = 1}},
    {9, Tail{.tid = 9, .color = Color::DarkBrown, .shape = 1, .inheritanceRate = 30.0f, .minGrade = 1}},
    {10, Tail{.tid = 10, .color = Color::Grey, .shape = 1, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 2 (Cropped): 20% inheritance
    {11, Tail{.tid = 11, .color = Color::White, .shape = 2, .inheritanceRate = 20.0f, .minGrade = 1}},
    {12, Tail{.tid = 12, .color = Color::LightBrown, .shape = 2, .inheritanceRate = 20.0f, .minGrade = 1}},
    {13, Tail{.tid = 13, .color = Color::Brown, .shape = 2, .inheritanceRate = 20.0f, .minGrade = 1}},
    {14, Tail{.tid = 14, .color = Color::DarkBrown, .shape = 2, .inheritanceRate = 20.0f, .minGrade = 1}},
    {15, Tail{.tid = 15, .color = Color::Grey, .shape = 2, .inheritanceRate = 20.0f, .minGrade = 1}},
    // Shape 3 (Long Thin): 30% inheritance
    {16, Tail{.tid = 16, .color = Color::White, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {17, Tail{.tid = 17, .color = Color::LightBrown, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {18, Tail{.tid = 18, .color = Color::Brown, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {19, Tail{.tid = 19, .color = Color::DarkBrown, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    {20, Tail{.tid = 20, .color = Color::Grey, .shape = 3, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 4 (Short Thin): 30% inheritance
    {21, Tail{.tid = 21, .color = Color::White, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {22, Tail{.tid = 22, .color = Color::LightBrown, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {23, Tail{.tid = 23, .color = Color::Brown, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {24, Tail{.tid = 24, .color = Color::DarkBrown, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    {25, Tail{.tid = 25, .color = Color::Grey, .shape = 4, .inheritanceRate = 30.0f, .minGrade = 1}},
    // Shape 5 (Long Curly): 30% inheritance, grade 7+
    {26, Tail{.tid = 26, .color = Color::White, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 7}},
    {27, Tail{.tid = 27, .color = Color::LightBrown, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 7}},
    {28, Tail{.tid = 28, .color = Color::Brown, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 7}},
    {29, Tail{.tid = 29, .color = Color::DarkBrown, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 7}},
    {30, Tail{.tid = 30, .color = Color::Grey, .shape = 5, .inheritanceRate = 30.0f, .minGrade = 7}},
  };

  // Build lookup tables for efficient querying
  for (const auto& [tid, mane] : _manes)
  {
    // Map each color to its color group(s)
    for (const auto& [groupId, group] : _colorGroups)
    {
      if (std::find(group.colors.begin(), group.colors.end(), mane.color) != group.colors.end())
      {
        _manesByColorAndShape[groupId][mane.shape].push_back(tid);
      }
    }
  }

  for (const auto& [tid, tail] : _tails)
  {
    // Map each color to its color group(s)
    for (const auto& [groupId, group] : _colorGroups)
    {
      if (std::find(group.colors.begin(), group.colors.end(), tail.color) != group.colors.end())
      {
        _tailsByColorAndShape[groupId][tail.shape].push_back(tid);
      }
    }
  }
}

void HorseRegistry::ReadConfig()
{
  const YAML::Node config = YAML::Load("./config/game/horses.yaml");
  const auto root = config["horses"];

  const auto facesConfig = root["faces"];
  const auto coatsConfig = root["coats"];
  const auto manesConfig = root["manes"];
  const auto tailsConfig = root["tails"];
}

void HorseRegistry::BuildRandomHorse(
  data::Horse::Parts& parts,
  data::Horse::Appearance& appearance)
{
  // Pick a random coat.
  std::uniform_int_distribution<data::Tid> coatRandomDist(
    1, _coats.size());

  const Coat& coat = _coats[coatRandomDist(_randomDevice)];
  assert(coat.tid != 0);
  parts.skinTid = coat.tid;

  // If the coat has a face available, pick a random face.
  if (coat.faceType != 0)
  {
    std::uniform_int_distribution<data::Tid> faceRandomDist(
      1, _faces.size());

    const Face& face = _faces[faceRandomDist(_randomDevice)];
    parts.faceTid = face.tid;
  }

  {
    // Pick a random mane.
    std::uniform_int_distribution<data::Tid> maneRandomDist(
      1, _manes.size());

    const Mane& mane = _manes[maneRandomDist(_randomDevice)];
    assert(mane.tid != 0);
    parts.maneTid = mane.tid;
  }

  {
    // Pick a random tail.
    std::uniform_int_distribution<data::Tid> tailRandomDist(
      1, _tails.size());

    const Tail& tail = _tails[tailRandomDist(_randomDevice)];
    assert(tail.tid != 0);
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

void HorseRegistry::SetHorsePotential(
  data::Horse::Potential& potential,
  uint8_t type,
  uint8_t level,
  uint8_t value)
{
  potential.type = type;
  potential.level = level;
  potential.value = value;
}

void HorseRegistry::GiveHorseRandomPotential(
  data::Horse::Potential& potential)
{
  uint8_t type;
  std::uniform_int_distribution<uint32_t> typeDist(1, 15);
  // Horse type cannot be 12 as it does not exist in original Alicia
  do
  {
    type = typeDist(_randomDevice);
  } while (type == 12);

  std::uniform_int_distribution<uint32_t> randomDist(0, 255);
  SetHorsePotential(
    potential,
    type,
    randomDist(_randomDevice),
    randomDist(_randomDevice));
}

const Coat& HorseRegistry::GetCoatInfo(data::Tid coatTid) const
{
  auto it = _coats.find(coatTid);
  if (it != _coats.end())
  {
    return it->second;
  }
  
  // Fallback to Chestnut (coat 1) if not found
  static const Coat invalidCoat{.tid = 1, .faceType = 0, .minGrade = 1, .tier = Coat::Tier::Common, .allowedColorGroups = {1}};
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
  {
    return 0;
  }

  const Color maneColor = it->second.color;
  
  // Find which color group(s) contain this color
  for (const auto& [groupId, group] : _colorGroups)
  {
    if (std::find(group.colors.begin(), group.colors.end(), maneColor) != group.colors.end())
    {
      return groupId;
    }
  }
  
  return 0;
}

int32_t HorseRegistry::GetTailColorGroupId(data::Tid tailTid) const
{
  auto it = _tails.find(tailTid);
  if (it == _tails.end())
  {
    return 0;
  }

  const Color tailColor = it->second.color;
  
  // Find which color group(s) contain this color
  for (const auto& [groupId, group] : _colorGroups)
  {
    if (std::find(group.colors.begin(), group.colors.end(), tailColor) != group.colors.end())
    {
      return groupId;
    }
  }
  
  return 0;
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
  static const Mane invalidMane{.tid = 1, .color = Color::White, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1};
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
  static const Tail invalidTail{.tid = 1, .color = Color::White, .shape = 0, .inheritanceRate = 30.0f, .minGrade = 1};
  return invalidTail;
}

} // namespace server