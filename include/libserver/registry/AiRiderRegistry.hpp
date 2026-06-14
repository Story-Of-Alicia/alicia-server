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

#ifndef AIRIDERREGISTRY_HPP
#define AIRIDERREGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace server::registry
{

struct AiRiderPreset
{
  uint32_t id{};
  std::string name;
  //! AIParam.Type — determines AI racing behavior on the client.
  uint32_t aiType{5};
};

class AiRiderRegistry
{
public:
  AiRiderRegistry() = default;

  void ReadConfig(const std::filesystem::path& configPath);

  //! Returns the preset pool for the given difficulty level, or nullptr if none.
  [[nodiscard]] const std::vector<AiRiderPreset>* GetPresetsForDifficulty(uint32_t difficulty) const;

private:
  //! Presets keyed by difficultyLevel (1=easy, 2=normal, 3=hard, 4=expert).
  std::unordered_map<uint32_t, std::vector<AiRiderPreset>> _presets;
};

} // namespace server::registry

#endif // AIRIDERREGISTRY_HPP
