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
    std::cerr << "Error: Could not open configuration file at: " << fullPath << std::endl;
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
        if (!address.empty() && port != 0)
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
          if (!address.empty() && port != 0)
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
          if (!address.empty() && port != 0)
          {
            _lobbySettings.messengerAdvAddress = address;
            _lobbySettings.messengerAdvPort = port;
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
        if (!address.empty() && port != 0)
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
        if (!address.empty() && port != 0)
        {
          _messengerSettings.address = address;
          _messengerSettings.port = port;
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

std::pair<std::string, uint16_t> Settings::ParseAddressAndPort(const nlohmann::json& jsonObject)
{
  // Check if both "address" and "port" exist
  if (jsonObject.contains("address") && jsonObject.contains("port"))
  {
    try
    {
      std::string address = jsonObject.at("address").get<std::string>();
      uint16_t port = jsonObject.at("port").get<uint16_t>();
      address = alicia::ResolveAddress(address, std::to_string(port));
      return std::make_pair(address, port);
    }
    catch (const std::exception& e)
    {
      std::cerr << "Error parsing address/port: " << e.what() << std::endl;
      return std::make_pair("", 0); // Return empty pair if parsing fails
    }
  }
  return std::make_pair("", 0); // Return empty pair if keys are missing
}

} // namespace alicia
