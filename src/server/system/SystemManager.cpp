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

#include "server/system/SystemManager.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace server
{

void SystemManager::ReadConfig(const std::filesystem::path& configPath)
{
  _configPath = configPath;

  if (not std::filesystem::exists(_configPath))
  {
    return;
  }

  const auto root = YAML::LoadFile(_configPath.string());
  
  std::scoped_lock lock(_mutex);
  _entries.clear();

  if (root.IsSequence())
  {
    for (const auto& node : root)
    {
      SystemEntry entry;
      entry.type = node["type"].as<uint32_t>();
      try
      {
        entry.value = node["value"].as<uint32_t>();
      }
      catch (YAML::TypedBadConversion<uint32_t>&)
      {
        entry.value = node["value"].as<bool>();
      }

      entry.name = node["name"].as<std::string>();
      entry.description = node["description"].as<std::string>();
      
      _entries.push_back(entry);
    }
  }

  spdlog::info("System manager loaded {} parameters", _entries.size());
}

void SystemManager::Save() const
{
  YAML::Node root;

  {
    std::scoped_lock lock(_mutex);
    for (const auto& entry : _entries)
    {
      YAML::Node node;
      node["type"] = entry.type;
      node["value"] = entry.value;
      node["name"] = entry.name;
      node["description"] = entry.description;

      root.push_back(node);
    }
  }

  std::ofstream fout(_configPath);
  fout << root;
}

uint32_t SystemManager::GetValue(uint32_t type) const
{
  std::scoped_lock lock(_mutex);
  const auto it = std::find_if(
    _entries.cbegin(),
    _entries.cend(),
    [type](const SystemEntry& entry)
    {
      return entry.type == type;
    });

  if (it != _entries.cend())
  {
    return it->value;
  }

  return 0;
}

void SystemManager::SetValue(uint32_t type, uint32_t value)
{
  {
    std::scoped_lock lock(_mutex);
    const auto it = std::find_if(
      _entries.begin(),
      _entries.end(),
      [type](const SystemEntry& entry)
      {
        return entry.type == type;
      });

    if (it != _entries.end())
    {
      it->value = value;
    }
    else
    {
      _entries.push_back({.type = type, .value = value});
    }
  }

  Save();
}

const std::unordered_map<uint32_t, uint32_t> SystemManager::GetSystemContent() const
{
  std::unordered_map<uint32_t, uint32_t> content{};
  
  std::scoped_lock lock(_mutex);
  for (const auto& entry : _entries)
  {
    content[entry.type] = entry.value;
  }

  return content;
}

} // namespace server
