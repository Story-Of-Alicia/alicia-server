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

#ifndef SYSTEMCONTENTREGISTRY_HPP
#define SYSTEMCONTENTREGISTRY_HPP

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

namespace server::registry
{

class SystemContentRegistry
{
public:
  struct SystemEntry
  {
    uint32_t type{};
    uint32_t value{};
    std::string name{};
    std::string description{};
  };

  /**
   * Reads the system configuration from a YAML file.
   * @param configPath Path to the system.yaml file.
   */
  void ReadConfig(const std::filesystem::path& configPath);

  /**
   * Saves the current system configuration back to the YAML file.
   */
  void Save() const;

  /**
   * Gets the value for a specific system type.
   * @param type The type key.
   * @return The current value, if any.
   */
  [[nodiscard]] std::optional<uint32_t> GetValue(uint32_t type) const;

  /**
   * Sets the value for a specific system type and persists the change.
   * @param type The type key.
   * @param value The new value.
   */
  void SetValue(uint32_t type, uint32_t value);

  /**
   * Returns the system content for protocol responses.
   * @return SystemContent struct containing all values.
   */
  [[nodiscard]] const std::unordered_map<uint32_t, uint32_t> GetSystemContent() const;

private:
  mutable std::mutex _mutex;
  std::filesystem::path _configPath;
  std::vector<SystemEntry> _entries;
};

} // namespace server::registry

#endif // SYSTEMCONTENTREGISTRY_HPP
