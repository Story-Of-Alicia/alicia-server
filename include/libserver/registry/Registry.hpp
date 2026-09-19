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

#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <filesystem>

namespace server::registry
{

class Registry
{
public:
  virtual ~Registry() = default;

  //! Reads and parses configuration from the specified path.
  //! @param configPath Path to the configuration file or directory.
  virtual void ReadConfig(const std::filesystem::path& configPath) = 0;

  //! Clears all loaded registry data and resets state.
  virtual void Clear() = 0;
};

} // namespace server::registry

#endif // REGISTRY_HPP
