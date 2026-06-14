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

#ifndef CHARACTERREGISTRY_HPP
#define CHARACTERREGISTRY_HPP

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace server::registry
{

struct CharacterLevelInfo
{
  //! The level this entry describes.
  uint32_t level{};
  //! Total experience points required to reach this level.
  uint32_t expRequired{};
};

class CharacterRegistry
{
public:
  void ReadConfig(const std::filesystem::path& configPath);

  //! Returns the level info for a given level, or nullopt if not found.
  [[nodiscard]] std::optional<CharacterLevelInfo> GetLevelInfo(uint32_t level) const;

  //! Returns the level a character is at given their total experience.
  [[nodiscard]] uint32_t GetLevelForExp(uint32_t totalExp) const;

  //! Returns the total experience required to reach the given level.
  [[nodiscard]] std::optional<uint32_t> GetExpRequiredForLevel(uint32_t level) const;

private:
  //! Level info sorted by level, indexed from 0 (level 1 = index 0).
  std::vector<CharacterLevelInfo> _levelInfo;
};

} // namespace server::registry

#endif // CHARACTERREGISTRY_HPP
