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

#ifndef AIRIDERREGISTRY_HPP
#define AIRIDERREGISTRY_HPP

#include "libserver/registry/Registry.hpp"

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
  uint32_t aiType{};
};

class AiRiderRegistry : public Registry
{
public:
  AiRiderRegistry() = default;
  ~AiRiderRegistry() override = default;

  void ReadConfig(const std::filesystem::path& configPath) override;
  void Clear() override;

  //! Returns the preset pool for the given difficulty level.
  [[nodiscard]] const std::vector<AiRiderPreset>& GetPresetsForDifficulty(uint8_t difficulty) const;

private:
  std::unordered_map<uint32_t, std::vector<AiRiderPreset>> _presets;
};

} // namespace server::registry

#endif // AIRIDERREGISTRY_HPP
