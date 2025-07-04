#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <utility>

#include <nlohmann/json.hpp>
#include <boost/asio/ip/address.hpp>

namespace server
{

namespace asio = boost::asio;

class Settings
{
public:
  //! Lobby settings.
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

  //! Ranch settings.
  struct RanchSettings
  {
    asio::ip::address_v4 address{
      asio::ip::address_v4::any()
    };
    uint16_t port = 10031;
  } _ranchSettings;

  //! Race settings.
  struct RaceSettings
  {
    asio::ip::address_v4 address{
      asio::ip::address_v4::any()
    };
    uint16_t port = 10032;
  } _raceSettings;

  //! Messenger settings.
  struct MessengerSettings
  {
    asio::ip::address_v4 address{
      asio::ip::address_v4::any()
    };
    uint16_t port = 10033;
  } _messengerSettings;

  //! Data source settings.
  struct DataSource
  {
    std::string connectionString;
  } _dataSourceSettings;

  //
  void LoadFromEnvironment();
  // Updates settings from json configuration file
  void LoadFromFile(const std::filesystem::path& filePath);

  // Parse address and port from json object
  std::pair<asio::ip::address_v4, uint16_t> ParseAddressAndPort(const nlohmann::json& jsonObject);
};

} // namespace server

#endif // SETTINGS_HPP