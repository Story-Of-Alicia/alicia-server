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

#include "libserver/registry/PetRegistry.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <regex>

namespace server::registry
{

namespace
{

data::Tid ReadEggInfo(
  const YAML::Node& node,
  EggInfo& info)
{
  info.deckItemId = node["deckId"].as<decltype(info.deckItemId)>();

  const auto hatchSection = node["hatch"];

  const std::regex secondPattern(R"((\d+)s)");
  const std::regex minutePattern(R"((\d+)m)");
  const std::regex hourPattern(R"((\d+)h)");
  const std::regex dayPattern(R"((\d+)d)");
  std::smatch match;

  const auto durationInput = hatchSection["duration"].as<std::string>();

  if (std::regex_search(durationInput, match, secondPattern))
    info.hatchDuration += std::chrono::seconds(std::stoi(match[1].str()));
  if (std::regex_search(durationInput, match, minutePattern))
    info.hatchDuration += std::chrono::minutes(std::stoi(match[1].str()));
  if (std::regex_search(durationInput, match, hourPattern))
    info.hatchDuration += std::chrono::hours(std::stoi(match[1].str()));
  if (std::regex_search(durationInput, match, dayPattern))
    info.hatchDuration += std::chrono::days(std::stoi(match[1].str()));

  info.hatchablePets = hatchSection["pets"].as<std::vector<uint32_t>>();

  return node["eggItemTid"].as<data::Tid>();
}

data::Tid ReadPetInfo(
  const YAML::Node& node,
  PetInfo& info)
{
  info.petId = node["petId"].as<decltype(info.petId)>();
  return node["itemTid"].as<data::Tid>();
}

} // anon namespace

void PetRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto petsSection = root["pets"];
  const auto eggsSection = petsSection["eggs"];

  for (const auto& eggSection : eggsSection["collection"])
  {
    EggInfo info;
    const auto eggItemTid = ReadEggInfo(eggSection, info);
    _eggs.try_emplace(eggItemTid, info);
  }

  for (const auto& petSection : petsSection["collection"])
  {
    PetInfo info;
    const auto petItemTid = ReadPetInfo(petSection, info);
    _pets.try_emplace(petItemTid, info);
  }

  spdlog::info("Pet registry loaded {} pets and {} eggs", _pets.size(), _eggs.size());
}

EggInfo PetRegistry::GetEggInfo(server::data::Tid tid)
{
  auto it = _eggs.find(tid);
  if (it != _eggs.end())
  {
    return it->second;
  }

  throw std::runtime_error("Egg with given TID not found.");
}

PetInfo PetRegistry::GetPetInfo(server::data::Tid tid)
{
  auto it = _pets.find(tid);
  if (it != _pets.end())
    return it->second;

  throw std::runtime_error("Pet with given TID not found");
}

} // namespace server::registry
