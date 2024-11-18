#include "Version.hpp"

#include "LobbyDirector.hpp"

#include <libserver/base/server.hpp>
#include <libserver/command/CommandServer.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <thread>

namespace
{

std::unique_ptr<alicia::LoginDirector> g_loginDirector;

} // anon namespace

int main()
{
  // Daily file sink.
  const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
    "logs/log.log",
    0, 0);
  // Console sink.
  const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // Initialize the application logger with file sink and console sink.
  auto applicationLogger = std::make_shared<spdlog::logger>(
    "abc",
    spdlog::sinks_init_list{
      fileSink,
      consoleSink});
  applicationLogger->set_level(spdlog::level::debug);
  applicationLogger->set_pattern("%H:%M:%S:%e [%^%l%$] [Thread %t] %v");

  // Set is as the default logger for the application.
  spdlog::set_default_logger(applicationLogger);

  spdlog::info("Running Alicia server v{}.", alicia::BuildVersion);

  // Lobby thread.
  std::jthread lobbyThread([]()
  {
    alicia::CommandServer lobbyServer;
    g_loginDirector = std::make_unique<alicia::LoginDirector>(lobbyServer);

    // Handlers
<<<<<<< HEAD

    // Login handler
=======
>>>>>>> parent of d81bd53 (show inventory impl)
    lobbyServer.RegisterCommandHandler(
      alicia::CommandId::LobbyLogin,
      [](alicia::ClientId clientId, auto& buffer)
      {
        alicia::LobbyCommandLogin loginCommand;
        alicia::LobbyCommandLogin::Read(
          loginCommand, buffer);

        g_loginDirector->HandleUserLogin(clientId, loginCommand);
      });

<<<<<<< HEAD
    // Heartbeat handler
    lobbyServer.RegisterCommandHandler(
      alicia::CommandId::LobbyHeartbeat,
      [](alicia::ClientId clientId, auto& buffer)
      {
        alicia::LobbyCommandHeartbeat heartbeat;
        alicia::LobbyCommandHeartbeat::Read(heartbeat, buffer);

        g_loginDirector->HandleHeartbeat(clientId, heartbeat);
      });

    // ShowInventory handler
    lobbyServer.RegisterCommandHandler(
      alicia::CommandId::LobbyShowInventory,
      [](alicia::ClientId clientId, auto& buffer)
      {
        alicia::LobbyCommandShowInventory showInventoryCommand;
        alicia::LobbyCommandShowInventory::Read(
          showInventoryCommand, buffer);

        g_loginDirector->HandleShowInventory(clientId, showInventoryCommand);
      });

    // AchievementCompleteList handler
    lobbyServer.RegisterCommandHandler(
      alicia::CommandId::LobbyAchievementCompleteList,
      [](alicia::ClientId clientId, auto& buffer)
      {
        alicia::LobbyCommandAchievementCompleteList achievementCompleteList;
        alicia::LobbyCommandAchievementCompleteList::Read(
          achievementCompleteList, buffer);

        g_loginDirector->HandleAchievementCompleteList(clientId, achievementCompleteList);
      });

    // RequestLeagueInfo
    lobbyServer.RegisterCommandHandler(
      alicia::CommandId::LobbyRequestLeagueInfo,
      [](alicia::ClientId clientId, auto& buffer)
      {
        alicia::LobbyCommandRequestLeagueInfo requestLeagueInfo;
        alicia::LobbyCommandRequestLeagueInfo::Read(
          requestLeagueInfo, buffer);

        g_loginDirector->HandleRequestLeagueInfo(clientId, requestLeagueInfo);
      });

    // RequestQuestList handler
    lobbyServer.RegisterCommandHandler(
      alicia::CommandId::LobbyRequestQuestList,
      [](alicia::ClientId clientId, auto& buffer)
      {
        alicia::LobbyCommandRequestQuestList requestDailyQuestList;
        alicia::LobbyCommandRequestQuestList::Read(
          requestDailyQuestList, buffer);

        g_loginDirector->HandleRequestQuestList(clientId, requestDailyQuestList);
      });

=======
>>>>>>> parent of d81bd53 (show inventory impl)
    // Host
    spdlog::debug("Lobby server hosted on 127.0.0.1:{}", 10030);
    lobbyServer.Host("127.0.0.1", 10030);
  });

  // Ranch thread.
  std::jthread ranchThread([]()
  {
    alicia::CommandServer ranchServer;

    spdlog::debug("Ranch server hosted on 127.0.0.1:{}", 10031);
    ranchServer.Host("127.0.0.1", 10031);
  });

  return 0;
}