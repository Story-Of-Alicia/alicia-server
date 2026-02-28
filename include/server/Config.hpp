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
  struct General
  {
    std::string brand;
    std::string notice;
  } general{};

  //!
  struct Authentication
  {
    std::string backend;

    struct Postgres
    {
      std::string connectionUri;
    } postgres;

  } authentication{};

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
      Listen allChat{
        .address = asio::ip::address_v4::loopback(),
        .port = 10034};
      Listen privateChat{
        .address = asio::ip::address_v4::loopback(),
        .port = 10035};
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
  struct AllChat
  {
    bool enabled{true};
    Listen listen{
      .port = 10034};
  } allChat{};

  struct PrivateChat
  {
    bool enabled{true};
    Listen listen{
      .port = 10035};
  } privateChat{};

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