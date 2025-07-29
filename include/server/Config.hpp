#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <nlohmann/json.hpp>
#include <boost/asio/ip/address.hpp>

namespace server
{

namespace asio = boost::asio;

class Config
{
public:
  //! Generic listen section consisting of address and port fields.
  struct Listen
  {
    //! An IPv4 or a hostname.
    asio::ip::address_v4 address{
      asio::ip::address_v4::any()};
    //! A port.
    uint16_t port{0};
  };

  //!
  struct Lobby
  {
    bool enabled{true};
    Listen listen{
      .port = 10030};

    struct Advertisement
    {
      Listen ranch{
        .address = asio::ip::address_v4::loopback(),
        .port = 10031};
      Listen race{
        .address = asio::ip::address_v4::loopback(),
        .port = 10032};
      Listen messenger{
        .address = asio::ip::address_v4::loopback(),
        .port = 10033};
    } advertisement{};
  } lobby{};

  //!
  struct Ranch
  {
    bool enabled{true};
    Listen listen{
      .port = 10031};
  } ranch{};

  //!
  struct Race
  {
    bool enabled{true};
    Listen listen{
      .port = 10032};
  } race{};

  //!
  struct Messenger
  {
    bool enabled{true};
    Listen listen{
      .port = 10033};
  } messenger{};

  //!
  struct Data
  {
    enum class Source
    {
      File, Postgres
    } source{Source::File};

    struct File
    {
      std::string basePath = "./data";
    } file{};

    struct Postgres
    {

    } postgres{};
  } data{};

  //! Loads the config from the environment.
  void LoadFromEnvironment();
  //! Loads the config from the specified file.
  //! @param  filePath Path to the config file.
  void LoadFromFile(const std::filesystem::path& filePath);
};

} // namespace server

#endif // CONFIG_HPP