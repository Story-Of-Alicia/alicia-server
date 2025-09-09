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

#include "server/Config.hpp"

#include <libserver/util/Util.hpp>

#include <charconv>
#include <format>
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

#include <spdlog/spdlog.h>

namespace server
{

void Config::LoadFromEnvironment()
{
  const auto getEnvValue = [](const std::string& key)
  {
    std::string value;

    // Unfortunately musl standard implementation does not support getenv_s,
    // so we have to stick with the unsafe version.
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
    lobby.listen.address,
    lobby.listen.port);

  // Lobby advertised address and port for ranch.
  getAddressAndPortVariables(
    std::format("LOBBY_ADVERTISED_RANCH_ADDRESS"),
    std::format("LOBBY_ADVERTISED_RANCH_PORT"),
    lobby.advertisement.ranch.address,
    lobby.advertisement.ranch.port);

  // Lobby advertised address and port for race.
  getAddressAndPortVariables(
    std::format("LOBBY_ADVERTISED_RACE_ADDRESS"),
    std::format("LOBBY_ADVERTISED_RACE_PORT"),
    lobby.advertisement.race.address,
    lobby.advertisement.race.port);

  // Ranch address and port.
  getAddressAndPortVariables(
    std::format("RANCH_SERVER_ADDRESS"),
    std::format("RANCH_SERVER_PORT"),
    ranch.listen.address,
    ranch.listen.port);

  // Race address and port.
  getAddressAndPortVariables(
    std::format("RACE_SERVER_ADDRESS"),
    std::format("RACE_SERVER_PORT"),
    race.listen.address,
    race.listen.port);
}

void Config::LoadFromFile(const std::filesystem::path& filePath)
{
  std::ifstream file(filePath);

  if (not file.is_open())
  {
    spdlog::error(
      "Could not open configuration file at '{}'",
      filePath.string());
    return;
  }

  const auto parseListenSection = [](const YAML::Node& node)
  {
    try
    {
      return Listen{
        .address = util::ResolveHostName(node["address"].as<std::string>()),
        .port = node["port"].as<uint16_t>()
      };
    }
    catch (const std::exception& e)
    {
      spdlog::error("Failed parsing address or port: {}", e.what());
    }

    return Listen{};
  };

  try
  {
    const YAML::Node yamlConfig = YAML::Load(file);
    const auto serverYaml = yamlConfig["server"];

    // General config
    try
    {
      const auto generalYaml = serverYaml["general"];
      general.brand = generalYaml["brand"].as<std::string>("<not set>");
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unhandled exception parsing the general config: {}", e.what());
    }

    // Lobby config
    try
    {
      const auto lobbyYaml = serverYaml["lobby"];
      lobby.enabled = lobbyYaml["enabled"].as<bool>();
      lobby.listen = parseListenSection(lobbyYaml["listen"]);

      const auto lobbyAdvertisementYaml = lobbyYaml["advertisement"];
      lobby.advertisement.ranch = parseListenSection(lobbyAdvertisementYaml["ranch"]);
      lobby.advertisement.race = parseListenSection(lobbyAdvertisementYaml["race"]);
      lobby.advertisement.messenger = parseListenSection(lobbyAdvertisementYaml["messenger"]);
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unhandled exception parsing the lobby config: {}", e.what());
    }

    // Ranch config
    try
    {
      const auto ranchYaml = serverYaml["ranch"];
      ranch.enabled = ranchYaml["enabled"].as<bool>();
      ranch.listen = parseListenSection(ranchYaml["listen"]);
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unhandled exception parsing the ranch config: {}", e.what());
    }

    // Race config
    try
    {
      const auto raceYaml = serverYaml["race"];
      race.enabled = raceYaml["enabled"].as<bool>();
      race.listen = parseListenSection(raceYaml["listen"]);
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unhandled exception parsing the race config: {}", e.what());
    }

    // Messenger config
    try
    {
      const auto messengerYaml = serverYaml["messenger"];
      messenger.enabled = messengerYaml["enabled"].as<bool>();
      messenger.listen = parseListenSection(messengerYaml["listen"]);
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unhandled exception parsing the messenger config: {}", e.what());
    }

    // Messenger config
    try
    {
      const auto dataYaml = serverYaml["data"];

      const auto dataSourceName = dataYaml["source"].as<std::string>();
      if (dataSourceName == "file")
      {
        const auto fileYaml = dataYaml["file"];
        data.file.basePath = fileYaml["basePath"].as<std::string>();
      }
      else
      {
        spdlog::error("Unsupported data source type: {}", dataSourceName);
      }
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unhandled exception parsing the dat config: {}", e.what());
    }
  }
  catch (const std::exception& e)
  {
    spdlog::error("Unhandled exception parsing the config: {}", e.what());
  }
}

} // namespace server
