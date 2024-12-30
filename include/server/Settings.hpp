#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <utility>

#include <nlohmann/json.hpp>
#include <boost/asio/ip/address.hpp>

namespace alicia
{

namespace asio = boost::asio;

class Settings
{
public:
  struct LobbySettings
  {
    // Bind address and port of the lobby host.
    asio::ip::address_v4 address {
      asio::ip::address_v4::any()
    };
    uint16_t port = 10030;

    // Advertised address and port of the ranch host.
    asio::ip::address_v4 ranchAdvAddress {
      asio::ip::make_address_v4("127.0.0.1")
    };
    uint16_t ranchAdvPort = 10031;

    // Advertised address and port of the messenger host.
    asio::ip::address_v4 messengerAdvAddress {
     asio::ip::make_address_v4("127.0.0.1")
    };
    uint16_t messengerAdvPort = 10032;

    // Advertised address and port of the race host.
    asio::ip::address_v4 raceAdvAddress {
      asio::ip::make_address_v4("127.0.0.1")
    };
    uint16_t raceAdvPort = 10033;
  } _lobbySettings;

  // Bind address and port of the ranch host.
  struct RanchSettings
  {
    asio::ip::address_v4 address{
      asio::ip::address_v4::any()
    };
    uint16_t port = 10031;
  } _ranchSettings;

  // Bind address and port of the messenger host.
  struct MessengerSettings
  {
    asio::ip::address_v4 address{
      asio::ip::address_v4::any()
    };
    uint16_t port = 10032;
  } _messengerSettings;

  // Bind address and port of the race host.
  struct RaceSettings
  {
    asio::ip::address_v4 address{
      asio::ip::address_v4::any()
    };
    uint16_t port = 10033;
  } _raceSettings;

  // Updates settings from json configuration file
  void LoadFromFile(const std::filesystem::path& filePath);

  // Parse address and port from json object
  std::pair<asio::ip::address_v4, uint16_t> ParseAddressAndPort(const nlohmann::json& jsonObject);
};

} // namespace alicia

#endif // SETTINGS_HPP