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

#ifndef ACHIEVEMENT_REGISTRY_HPP
#define ACHIEVEMENT_REGISTRY_HPP

#include <libserver/registry/Registry.hpp>
#include <libserver/registry/RegistryDefinitions.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <unordered_map>

namespace server::registry
{

struct Achievement
{
  //! TID of the achievement.
  uint32_t tid{};
  //! Number of players involved in the condition (0 for solo conditions).
  uint32_t numPlayer{};
  //! Game mode flag bitmask.
  GameModeFlag gameModeFlag{};
  //! Success condition type.
  uint32_t successType{};
  //! Comparison type for the success values.
  uint32_t compareType{};
  //! Up to 4 success condition values (client SuccessValue1-4).
  std::array<uint32_t, 4> successValues{};
  //! Client-side achievement event category (UserAchvEvent in libconfig).
  uint32_t userAchvEvent{};
  //! Achievement completion function / condition type.
  Function function{};
  //! Up to 4 parameter values for the function (client FunctionValue1-4).
  std::array<uint32_t, 4> functionValues{};
  //! Event category that resets progress, if any.
  uint32_t resetUserAchvEvent{};
  //! Function that resets progress, if any.
  Function resetFunction{};
  //! Parameter value for the reset function.
  uint32_t resetFunctionValue{};
  //! Game money rewards (client GameMoney, GameMoney1-3).
  std::array<uint32_t, 4> gameMoney{};
  //! Achievement points awarded.
  uint32_t achvPoint{};
};

class AchievementRegistry : public Registry
{
public:
  void ReadConfig(const std::filesystem::path& configPath) override;
  void Clear() override;

  [[nodiscard]] std::optional<Achievement> GetAchievement(uint32_t tid) const;
  [[nodiscard]] const std::unordered_map<uint32_t, Achievement>& GetAchievements() const;

private:
  std::unordered_map<uint32_t, Achievement> _achievements{};
};

} // namespace server::registry

#endif // ACHIEVEMENT_REGISTRY_HPP
