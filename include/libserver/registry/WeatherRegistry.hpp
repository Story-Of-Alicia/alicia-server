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

#ifndef WEATHERREGISTRY_HPP
#define WEATHERREGISTRY_HPP

#include <libserver/registry/Registry.hpp>

#include <cstdint>
#include <filesystem>
#include <map>

namespace server::registry
{

//! A weather forecast level, mirroring a row of the client's
//! WeatherSnowRatio table.
struct WeatherForecastLevel
{
  //! A forecast level (1: no snow, 2: light snow 01, 3: light snow 02,
  //! 4: moderate snow, 5: heavy snow).
  uint32_t level{};
  //! A weight of this level being picked as the daily forecast.
  uint32_t ratio{};
  //! Weights of the weather in a race at this level,
  //! keyed by weather type (MapWeatherInfo type).
  std::map<uint32_t, uint32_t> raceChances{};
  //! Increments for the race chances from the client data.
  //! @note Not used yet, their meaning is not known.
  std::map<uint32_t, uint32_t> raceChanceAdds{};
};

class WeatherRegistry : public Registry
{
public:
  void ReadConfig(const std::filesystem::path& configPath) override;
  void Clear() override;

  //! Rolls the forecast level for a date the same way the client does,
  //! so the forecast shown in the client matches the server.
  //! @param date A date in the YYYYMMDD format.
  //! @returns A forecast level, or 0 if no levels are configured.
  [[nodiscard]] uint32_t RollForecastLevel(uint32_t date) const;

  //! Rolls the weather for a race at the given forecast level.
  //! @param level A forecast level.
  //! @returns A weather type (MapWeatherInfo type), or 0 for no weather.
  [[nodiscard]] uint32_t RollRaceWeather(uint32_t level) const;

  //! Rolls the weather for a race from the "random every race" weights.
  //! @returns A weather type (MapWeatherInfo type), or 0 for no weather.
  [[nodiscard]] uint32_t RollRandomRaceWeather() const;

private:
  //! Forecast levels, ordered by level like the client's table.
  std::map<uint32_t, WeatherForecastLevel> _forecastLevels;
  //! Weights of each weather type when every race picks at random.
  std::map<uint32_t, uint32_t> _randomEveryRace;
};

} // namespace server::registry

#endif // WEATHERREGISTRY_HPP
