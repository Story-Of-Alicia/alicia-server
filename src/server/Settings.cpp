#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>
#include <libserver/Util.hpp>
#include <server/Settings.hpp>

namespace alicia
{

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
        if (!address.is_unspecified()
          && port != 0)
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

    const auto resolvedAddress = ResolveHostName(address);
    if (resolvedAddress.is_unspecified())
    {
      throw std::runtime_error(
        std::format("Couldn't resolve '{}' to a valid IPv4 address.", address));
    }

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

} // namespace alicia
