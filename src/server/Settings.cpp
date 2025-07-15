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

#include "server/Settings.hpp"

#include <libserver/util/Util.hpp>

#include <stdlib.h>
#include <charconv>
#include <format>
#include <fstream>
#include <iostream>

#include <spdlog/spdlog.h>

namespace server
{

void Settings::LoadFromEnvironment()
{
  const auto getEnvValue = [](const std::string& key)
  {
    std::string value;
    // muslc does not support getenv_s,
    // we need it for Alpine :(
    const char* envValue = getenv(key.data());
    if (envValue == nullptr)
      return value;

    value = std::string_view(envValue);
    return value;
  };

  const auto getAddressAndPortVariables = [&getEnvValue](
    const std::string& addressVariableName,
    const std::string& portVariableName,
    asio::ip::address_v4& address,
    uint16_t& port)
  {
    try
    {
      // Get the address.
      const auto addressValue = getEnvValue(addressVariableName);
      if (not addressValue.empty())
      {
        address = util::ResolveHostName(addressValue);
      }
    }
    catch (const std::exception& x)
    {
      spdlog::error(" Couldn't resolve the host for '{}'", addressVariableName);
    }

    // Get the port.
    const std::string portValue = getEnvValue(portVariableName);
    if (not portValue.empty())
    {
      const auto result = std::from_chars(
        portValue.c_str(),
        portValue.c_str() + portValue.length(),
        port);
      if (result.ec != std::errc{})
      {
        spdlog::error("Couldn't resolve the port for '{}'.", portVariableName);
      }
    }
  };

  // Lobby address and port.
  getAddressAndPortVariables(
        std::format("LOBBY_SERVER_ADDRESS"),
        std::format("LOBBY_SERVER_PORT"),
        _lobbySettings.address,
        _lobbySettings.port);

  // Lobby advertised address and port for ranch.
  getAddressAndPortVariables(
      std::format("LOBBY_ADVERTISED_RANCH_ADDRESS"),
      std::format("LOBBY_ADVERTISED_RANCH_PORT"),
      _lobbySettings.ranchAdvAddress,
      _lobbySettings.ranchAdvPort);

  // Lobby advertised address and port for race.
  getAddressAndPortVariables(
      std::format("LOBBY_ADVERTISED_RACE_ADDRESS"),
      std::format("LOBBY_ADVERTISED_RACE_PORT"),
      _lobbySettings.ranchAdvAddress,
      _lobbySettings.ranchAdvPort);

  // Ranch address and port.
  getAddressAndPortVariables(
        std::format("RANCH_SERVER_ADDRESS"),
        std::format("RANCH_SERVER_PORT"),
        _ranchSettings.address,
        _ranchSettings.port);

  // Race address and port.
  getAddressAndPortVariables(
        std::format("RACE_SERVER_ADDRESS"),
        std::format("RACE_SERVER_PORT"),
        _raceSettings.address,
        _raceSettings.port);
}

void Settings::LoadFromFile(const std::filesystem::path& filePath)
{
  auto fullPath = std::filesystem::absolute(filePath);
  std::ifstream file(fullPath);

  if (!file.is_open())
  {
    spdlog::error("Could not open configuration file at '{}'", fullPath.string());
    return;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string jsonContent = buffer.str();

  // Parse the JSON file
  try
  {
    nlohmann::json jsonConfig = nlohmann::json::parse(jsonContent, nullptr, false, true);

    // Extract lobby settings
    if (jsonConfig.contains("lobby"))
    {
      const auto& lobby = jsonConfig["lobby"];
      // Bind address and port of the lobby host
      if (lobby.contains("bind"))
      {
        auto [address, port] = ParseAddressAndPort(lobby["bind"]);
        // If parsing succeeded, update values
        if (!address.is_unspecified() && port != 0)
        {
          _lobbySettings.address = address;
          _lobbySettings.port = port;
        }
      }

      if (lobby.contains("advertisement"))
      {
        const auto& advertisement = lobby["advertisement"];
        // Advertised address and port of the ranch host
        if (advertisement.contains("ranch"))
        {
          auto [address, port] = ParseAddressAndPort(advertisement["ranch"]);
          // If parsing succeeded, update values
          if (!address.is_unspecified() && port != 0)
          {
            _lobbySettings.ranchAdvAddress = address;
            _lobbySettings.ranchAdvPort = port;
          }
        }

        if (advertisement.contains("messenger"))
        {
          // Advertised address and port of the messenger host
          auto [address, port] = ParseAddressAndPort(advertisement["messenger"]);
          // If parsing succeeded, update values
          if (!address.is_unspecified() && port != 0)
          {
            _lobbySettings.messengerAdvAddress = address;
            _lobbySettings.messengerAdvPort = port;
          }
        }

        if (advertisement.contains("race"))
        {
          // Advertised address and port of the race host
          auto [address, port] = ParseAddressAndPort(advertisement["race"]);
          // If parsing succeeded, update values
          if (!address.is_unspecified() && port != 0)
          {
            _lobbySettings.raceAdvAddress = address;
            _lobbySettings.raceAdvPort = port;
          }
        }
      }
    }

    // Extract ranch settings
    if (jsonConfig.contains("ranch"))
    {
      const auto& ranch = jsonConfig["ranch"];
      if (ranch.contains("bind"))
      {
        auto [address, port] = ParseAddressAndPort(ranch["bind"]);
        // If parsing succeeded, update values
        if (!address.is_unspecified() && port != 0)
        {
          _ranchSettings.address = address;
          _ranchSettings.port = port;
        }
      }
    }

    // Extract messenger settings
    if (jsonConfig.contains("messenger"))
    {
      const auto& messenger = jsonConfig["messenger"];
      if (messenger.contains("bind"))
      {
        auto [address, port] = ParseAddressAndPort(messenger["bind"]);
        // If parsing succeeded, update values
        if (!address.is_unspecified() && port != 0)
        {
          _messengerSettings.address = address;
          _messengerSettings.port = port;
        }
      }
    }

    // Extract race settings
    if (jsonConfig.contains("race"))
    {
      const auto& race = jsonConfig["race"];
      if (race.contains("bind"))
      {
        auto [address, port] = ParseAddressAndPort(race["bind"]);
        // If parsing succeeded, update values
        if (!address.is_unspecified() && port != 0)
        {
          _raceSettings.address = address;
          _raceSettings.port = port;
        }
      }
    }

    // Extract data source settings.
    if (jsonConfig.contains("data_source"))
    {
      const auto& dataSource = jsonConfig["data_source"];
      if (dataSource.contains("connection_string"))
      {
        _dataSourceSettings.connectionString = dataSource["connection_string"];
      }
    }
  }
  catch (const nlohmann::json::parse_error& e)
  {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
    throw e;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
    throw e;
  }
}

std::pair<asio::ip::address_v4, uint16_t> Settings::ParseAddressAndPort(const nlohmann::json& jsonObject)
{
  // Both "address" and "port" properties must exist.
  if (!jsonObject.contains("address") && !jsonObject.contains("port"))
  {
    // Return empty pair if keys are missing
    return std::make_pair(asio::ip::address_v4{}, 0);
  }

  try
  {
    const std::string address = jsonObject.at("address").get<std::string>();
    const uint16_t port = jsonObject.at("port").get<uint16_t>();

    const auto resolvedAddress = util::ResolveHostName(address);
    return std::make_pair(
      resolvedAddress,
      port);
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error parsing address/port: " << e.what() << std::endl;
    return std::make_pair(asio::ip::address_v4{}, 0); // Return empty pair if parsing fails
  }
}

} // namespace server
