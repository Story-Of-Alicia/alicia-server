/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#include "libserver/registry/WeatherRegistry.hpp"
#include "libserver/util/Util.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <random>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace server::registry
{

namespace
{

//! Picks a key at random, weighted by its value.
//! @returns The picked key, or 0 if all weights are 0.
uint32_t RollWeighted(const std::map<uint32_t, uint32_t>& weights)
{
  uint32_t total = 0;
  for (const auto& weight : weights | std::views::values)
    total += weight;

  if (total == 0)
    return 0;

  std::uniform_int_distribution<uint32_t> distribution(0, total - 1);
  uint32_t roll = distribution(util::GetRandomEngine());

  for (const auto& [key, weight] : weights)
  {
    if (roll < weight)
      return key;
    roll -= weight;
  }

  return 0;
}

} // anonymous namespace

void WeatherRegistry::Clear()
{
  _forecastLevels.clear();
  _randomEveryRace.clear();
}

void WeatherRegistry::ReadConfig(const std::filesystem::path& configPath)
{
  const auto root = YAML::LoadFile(configPath.string());

  const auto weatherSection = root["weather"];
  if (not weatherSection)
    throw std::runtime_error("Missing weather section");

  const auto levelsSection = weatherSection["forecastLevels"];
  if (not levelsSection)
    throw std::runtime_error("Missing weather forecastLevels section");

  Clear();

  for (const auto& entry : levelsSection)
  {
    WeatherForecastLevel level{
      .level = entry["level"].as<uint32_t>(),
      .ratio = entry["ratio"].as<uint32_t>()};

    for (const auto& chance : entry["chances"])
      level.raceChances[chance.first.as<uint32_t>()] = chance.second.as<uint32_t>();

    if (const auto adds = entry["add"])
    {
      for (const auto& add : adds)
        level.raceChanceAdds[add.first.as<uint32_t>()] = add.second.as<uint32_t>();
    }

    _forecastLevels[level.level] = level;
  }

  if (const auto randomSection = weatherSection["randomEveryRace"])
  {
    for (const auto& weight : randomSection)
      _randomEveryRace[weight.first.as<uint32_t>()] = weight.second.as<uint32_t>();
  }

  spdlog::info("Weather registry loaded {} forecast levels", _forecastLevels.size());
}

uint32_t WeatherRegistry::RollForecastLevel(const uint32_t date) const
{
  // Cumulative weights in level order, skipping levels that can't be picked.
  std::vector<std::pair<uint32_t, uint32_t>> cumulative;
  uint32_t total = 0;
  for (const auto& [level, info] : _forecastLevels)
  {
    if (info.ratio == 0)
      continue;
    total += info.ratio;
    cumulative.emplace_back(total, level);
  }

  if (total == 0)
    return 0;

  // Two steps of the MSVC rand() generator seeded with the date,
  // exactly like the client: the roll is FUN_0064d430 in Alicia-tag10.exe,
  // called from InitWeatherRatioInfo (FUN_00651ea0) at startup.
  uint32_t state = date;
  state = state * 0x343FDu + 0x269EC3u;
  state = state * 0x343FDu + 0x269EC3u;
  const uint32_t roll = ((state >> 16) & 0x7FFFu) % total;

  // The first level whose cumulative weight is above the roll.
  for (const auto& [threshold, level] : cumulative)
  {
    if (roll < threshold)
      return level;
  }

  return 0;
}

uint32_t WeatherRegistry::RollRaceWeather(const uint32_t level) const
{
  const auto levelIter = _forecastLevels.find(level);
  if (levelIter == _forecastLevels.cend())
    return 0;

  return RollWeighted(levelIter->second.raceChances);
}

uint32_t WeatherRegistry::RollRandomRaceWeather() const
{
  return RollWeighted(_randomEveryRace);
}

} // namespace server::registry
