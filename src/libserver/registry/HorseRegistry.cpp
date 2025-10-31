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
{
  // Color codes: 1=Black, 2=White, 3=Brown, 4=Dark Brown, 5=Grey
  _coats = {
  // Tier 1 (Common): TID 1-7
  {1, Coat{.tid = 1, .faceType = 0, .minGrade = 1, .tier = Coat::Tier::Common, .allowedManeColors = {1,2,3}}},   // Chestnut: White, light brown, dark brown
  {2, Coat{.tid = 2, .faceType = -1, .minGrade = 1, .tier = Coat::Tier::Common, .allowedManeColors = {1,2,3}}},  // Bay: White, light brown, dark brown
  {3, Coat{.tid = 3, .faceType = 0, .minGrade = 1, .tier = Coat::Tier::Common, .allowedManeColors = {5,1,4}}},   // Champagne Sabino: Grey, white, black
  {4, Coat{.tid = 4, .faceType = 0, .minGrade = 2, .tier = Coat::Tier::Common, .allowedManeColors = {1,2,3}}},   // Chestnut Stockings: White, light brown, dark brown
  {5, Coat{.tid = 5, .faceType = -1, .minGrade = 3, .tier = Coat::Tier::Common, .allowedManeColors = {5,1,4}}},  // Buckskin: Grey, white, black
  {6, Coat{.tid = 6, .faceType = 0, .minGrade = 3, .tier = Coat::Tier::Common, .allowedManeColors = {5,1,2}}},   // Champagne: Grey, white, brown (no dark brown)
  {7, Coat{.tid = 7, .faceType = 0, .minGrade = 5, .tier = Coat::Tier::Common, .allowedManeColors = {1,2,3}}},   // Leopard Appaloosa: White, light brown, dark brown
  // Tier 2 (Uncommon): TID 8-14
  {8, Coat{.tid = 8, .faceType = -1, .minGrade = 6, .tier = Coat::Tier::Uncommon, .allowedManeColors = {5,1,4}}},  // Blanket Appaloosa: Grey, white, black
  {9, Coat{.tid = 9, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .allowedManeColors = {5,1,4}}},   // White Grey: Grey, white, black
  {10, Coat{.tid = 10, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .allowedManeColors = {5,1,4}}}, // Dapple Grey: Grey, white, black
  {11, Coat{.tid = 11, .faceType = -1, .minGrade = 6, .tier = Coat::Tier::Uncommon, .allowedManeColors = {2,3,1}}}, // Chestnut Pinto: Light brown, dark brown, white
  {12, Coat{.tid = 12, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .allowedManeColors = {1,2,3}}}, // Sooty Bay: White, light brown, dark brown
  {13, Coat{.tid = 13, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Uncommon, .allowedManeColors = {1,2,3}}}, // Palomino: White, light brown, dark brown
  {14, Coat{.tid = 14, .faceType = 0, .minGrade = 7, .tier = Coat::Tier::Uncommon, .allowedManeColors = {5,1,4}}}, // Black Pinto: Grey, white, black
  // Tier 3 (Rare): TID 15-20
  {15, Coat{.tid = 15, .faceType = -1, .minGrade = 7, .tier = Coat::Tier::Rare, .allowedManeColors = {5,1,4}}}, // Black: Grey, white, black
  {16, Coat{.tid = 16, .faceType = 0, .minGrade = 6, .tier = Coat::Tier::Rare, .allowedManeColors = {1,2,3}}}, // Brown Sabino (Chestnut Sabino): White, light brown, dark brown
  {17, Coat{.tid = 17, .faceType = 0, .minGrade = 8, .tier = Coat::Tier::Rare, .allowedManeColors = {1,2,3}}}, // Amber Cream: White, light brown, dark brown
  {18, Coat{.tid = 18, .faceType = 0, .minGrade = 7, .tier = Coat::Tier::Rare, .allowedManeColors = {1,2,3}}}, // Mealy Bay: White, light brown, dark brown
  {19, Coat{.tid = 19, .faceType = -1, .minGrade = 8, .tier = Coat::Tier::Rare, .allowedManeColors = {5,1,4}}}, // Black Sabino: Grey, white, black
  {20, Coat{.tid = 20, .faceType = 0, .minGrade = 8, .tier = Coat::Tier::Rare, .allowedManeColors = {1,2,3}}}, // Dapple Bay: White, light brown, dark brown
  };

  _faces = {
    {1, Face{.tid = 1, .type = -1}},
    {2, Face{.tid = 2, .type = -1}},
    {3, Face{.tid = 3, .type = -1}},
    {5, Face{.tid = 5, .type = -1}},
    {7, Face{.tid = 7, .type = -1}},
  };

  _manes = {
    {1, Mane{.tid = 1, .colorGroup = 1}},
    {2, Mane{.tid = 2, .colorGroup = 2}},
    {3, Mane{.tid = 3, .colorGroup = 3}},
    {4, Mane{.tid = 4, .colorGroup = 4}},
    {5, Mane{.tid = 5, .colorGroup = 5}},
    {6, Mane{.tid = 6, .colorGroup = 1}},
    {7, Mane{.tid = 7, .colorGroup = 2}},
    {8, Mane{.tid = 8, .colorGroup = 3}},
    {9, Mane{.tid = 9, .colorGroup = 4}},
    {10, Mane{.tid = 10, .colorGroup = 5}},
    {11, Mane{.tid = 11, .colorGroup = 1}},
    {12, Mane{.tid = 12, .colorGroup = 2}},
    {13, Mane{.tid = 13, .colorGroup = 3}},
    {14, Mane{.tid = 14, .colorGroup = 4}},
    {15, Mane{.tid = 15, .colorGroup = 5}},
    {16, Mane{.tid = 16, .colorGroup = 1}},
    {17, Mane{.tid = 17, .colorGroup = 2}},
    {18, Mane{.tid = 18, .colorGroup = 3}},
    {19, Mane{.tid = 19, .colorGroup = 4}},
    {20, Mane{.tid = 20, .colorGroup = 5}},
    {21, Mane{.tid = 21, .colorGroup = 1}},
    {22, Mane{.tid = 22, .colorGroup = 2}},
    {23, Mane{.tid = 23, .colorGroup = 3}},
    {24, Mane{.tid = 24, .colorGroup = 4}},
    {25, Mane{.tid = 25, .colorGroup = 5}},
    {26, Mane{.tid = 26, .colorGroup = 1}},
    {27, Mane{.tid = 27, .colorGroup = 2}},
    {28, Mane{.tid = 28, .colorGroup = 3}},
    {29, Mane{.tid = 29, .colorGroup = 4}},
    {30, Mane{.tid = 30, .colorGroup = 5}},
    {31, Mane{.tid = 31, .colorGroup = 1}},
    {32, Mane{.tid = 32, .colorGroup = 2}},
    {33, Mane{.tid = 33, .colorGroup = 3}},
    {34, Mane{.tid = 34, .colorGroup = 4}},
    {35, Mane{.tid = 35, .colorGroup = 5}},
    {36, Mane{.tid = 36, .colorGroup = 1}},
    {37, Mane{.tid = 37, .colorGroup = 2}},
    {38, Mane{.tid = 38, .colorGroup = 3}},
    {39, Mane{.tid = 39, .colorGroup = 4}},
    {40, Mane{.tid = 40, .colorGroup = 5}},
  };

  _tails = {
    {1, Tail{.tid = 1, .colorGroup = 1}},
    {2, Tail{.tid = 2, .colorGroup = 2}},
    {3, Tail{.tid = 3, .colorGroup = 3}},
    {4, Tail{.tid = 4, .colorGroup = 4}},
    {5, Tail{.tid = 5, .colorGroup = 5}},
    {6, Tail{.tid = 6, .colorGroup = 1}},
    {7, Tail{.tid = 7, .colorGroup = 2}},
    {8, Tail{.tid = 8, .colorGroup = 3}},
    {9, Tail{.tid = 9, .colorGroup = 4}},
    {10, Tail{.tid = 10, .colorGroup = 5}},
    {11, Tail{.tid = 11, .colorGroup = 1}},
    {12, Tail{.tid = 12, .colorGroup = 2}},
    {13, Tail{.tid = 13, .colorGroup = 3}},
    {14, Tail{.tid = 14, .colorGroup = 4}},
    {15, Tail{.tid = 15, .colorGroup = 5}},
    {16, Tail{.tid = 16, .colorGroup = 1}},
    {17, Tail{.tid = 17, .colorGroup = 2}},
    {18, Tail{.tid = 18, .colorGroup = 3}},
    {19, Tail{.tid = 19, .colorGroup = 4}},
    {20, Tail{.tid = 20, .colorGroup = 5}},
    {21, Tail{.tid = 21, .colorGroup = 1}},
    {22, Tail{.tid = 22, .colorGroup = 2}},
    {23, Tail{.tid = 23, .colorGroup = 3}},
    {24, Tail{.tid = 24, .colorGroup = 4}},
    {25, Tail{.tid = 25, .colorGroup = 5}},
    {26, Tail{.tid = 26, .colorGroup = 1}},
    {27, Tail{.tid = 27, .colorGroup = 2}},
    {28, Tail{.tid = 28, .colorGroup = 3}},
    {29, Tail{.tid = 29, .colorGroup = 4}},
    {30, Tail{.tid = 30, .colorGroup = 5}},
  };
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
  static const Coat invalidCoat{.tid = 1, .faceType = 0, .minGrade = 1, .tier = Coat::Tier::Common, .allowedManeColors = {2,3,4,1}};
  return invalidCoat;
}

int32_t HorseRegistry::GetManeColorGroup(data::Tid maneTid) const
{
  auto it = _manes.find(maneTid);
  if (it != _manes.end())
  {
    return it->second.colorGroup;
  }
  return 0;
}

int32_t HorseRegistry::GetTailColorGroup(data::Tid tailTid) const
{
  auto it = _tails.find(tailTid);
  if (it != _tails.end())
  {
    return it->second.colorGroup;
  }
  return 0;
}

} // namespace server