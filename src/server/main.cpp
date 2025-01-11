#include "Version.hpp"

#include "server/lobby/LobbyDirector.hpp"
#include "server/race/RaceDirector.hpp"
#include "server/ranch/RanchDirector.hpp"

#include <libserver/base/Server.hpp>
#include <libserver/command/CommandServer.hpp>
#include <libserver/Util.hpp>
#include <server/Settings.hpp>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <thread>

#include <iostream>
namespace
{

std::unique_ptr<alicia::DataDirector> g_dataDirector;
std::unique_ptr<alicia::LobbyDirector> g_loginDirector;
std::unique_ptr<alicia::RanchDirector> g_ranchDirector;
std::unique_ptr<alicia::RaceDirector> g_raceDirector;

} // namespace

int main()
{
  // Daily file sink.
  const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/log.log", 0, 0);

  // Console sink.
  const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // Initialize the application logger with file sink and console sink.
  auto applicationLogger = std::make_shared<spdlog::logger>(
    "abc",
    spdlog::sinks_init_list{fileSink, consoleSink});

  applicationLogger->set_level(spdlog::level::debug);
  applicationLogger->set_pattern("%H:%M:%S:%e [%^%l%$] [Thread %t] %v");

  // Set is as the default logger for the application.
  spdlog::set_default_logger(applicationLogger);

  spdlog::info("Running Alicia server v{}.", alicia::BuildVersion);

  // Parsing settings file
  alicia::Settings settings;
  settings.LoadFromFile("resources/settings.json");

  g_dataDirector = std::make_unique<alicia::DataDirector>();

  // Lobby director thread.
  std::jthread lobbyThread(
    [&settings]()
    {
      g_loginDirector = std::make_unique<alicia::LobbyDirector>(
        *g_dataDirector,
        settings._lobbySettings);
    });

  // Ranch director thread.
  std::jthread ranchThread(
    [&settings]()
    {
      g_ranchDirector = std::make_unique<alicia::RanchDirector>(
        *g_dataDirector,
        settings._ranchSettings);
    });

  // Race director thread.
  std::jthread raceThread(
    [&settings]()
    {
      g_raceDirector = std::make_unique<alicia::RaceDirector>(
        *g_dataDirector,
        settings._raceSettings);
    });

  // Messenger thread.
  std::jthread messengerThread(
    [&settings]()
    {
      alicia::CommandServer messengerServer("Messenger");
      // TODO: Messenger
      messengerServer.Host(boost::asio::ip::address_v4::any(), 10032);
    });

  return 0;
}
