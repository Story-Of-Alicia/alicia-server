#include "Version.hpp"

#include "server/Scheduler.hpp"

#include "lobby/LobbyDirector.hpp"
#include "ranch/RanchDirector.hpp"

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

std::unique_ptr<alicia::LoginDirector> g_loginDirector;
std::unique_ptr<alicia::RanchDirector> g_ranchDirector;

} // namespace

// Adds task to queue

Scheduler g_scheduler;
void EnqueueTask(std::function<void()> task)
{
    g_scheduler.EnqueueTask(std::move(task));
}

int main()
{

  std::thread taskProcessorThread(&Scheduler::ProcessTasks, &g_scheduler);

  // Daily file sink.
  const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/log.log", 0, 0);

  // Console sink.
  const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // Initialize the application logger with file sink and console sink.
  auto applicationLogger =
    std::make_shared<spdlog::logger>("abc", spdlog::sinks_init_list{fileSink, consoleSink});
  applicationLogger->set_level(spdlog::level::debug);
  applicationLogger->set_pattern("%H:%M:%S:%e [%^%l%$] [Thread %t] %v");

  // Set is as the default logger for the application.
  spdlog::set_default_logger(applicationLogger);

  spdlog::info("Running Alicia server v{}.", alicia::BuildVersion);

  // Parsing settings file
  alicia::Settings settings;
  settings.LoadFromFile("../../resources/settings.json");

  // Lobby thread.
  std::jthread lobbyThread(
    [&settings]()
    {
      alicia::CommandServer lobbyServer("Lobby");
      g_loginDirector = std::make_unique<alicia::LoginDirector>(lobbyServer, settings);

      // Handlers

      //! Login handler
      lobbyServer.RegisterCommandHandler<alicia::LobbyCommandLogin>(
        alicia::CommandId::LobbyLogin,
        [](alicia::ClientId clientId, const auto& message)
        
        {
          EnqueueTask([clientId, message]() {
            g_loginDirector->HandleUserLogin(clientId, message);
          });
        });

      // Heartbeat handler
      lobbyServer.RegisterCommandHandler(
        alicia::CommandId::LobbyHeartbeat,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::LobbyCommandHeartbeat heartbeat;
          alicia::LobbyCommandHeartbeat::Read(heartbeat, buffer);

          EnqueueTask([clientId, heartbeat]() {
            g_loginDirector->HandleHeartbeat(clientId, heartbeat);
          });
        });

      // ShowInventory handler
      lobbyServer.RegisterCommandHandler(
        alicia::CommandId::LobbyShowInventory,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::LobbyCommandShowInventory showInventoryCommand;
          alicia::LobbyCommandShowInventory::Read(showInventoryCommand, buffer);

          EnqueueTask([clientId, showInventoryCommand]() {
            g_loginDirector->HandleShowInventory(clientId, showInventoryCommand);
          });
        });

      // AchievementCompleteList handler
      lobbyServer.RegisterCommandHandler(
        alicia::CommandId::LobbyAchievementCompleteList,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::LobbyCommandAchievementCompleteList achievementCompleteList;
          alicia::LobbyCommandAchievementCompleteList::Read(achievementCompleteList, buffer);

          EnqueueTask([clientId, achievementCompleteList]() {
            g_loginDirector->HandleAchievementCompleteList(clientId, achievementCompleteList);
          });
        });

      // RequestLeagueInfo
      lobbyServer.RegisterCommandHandler(
        alicia::CommandId::LobbyRequestLeagueInfo,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::LobbyCommandRequestLeagueInfo requestLeagueInfo;
          alicia::LobbyCommandRequestLeagueInfo::Read(requestLeagueInfo, buffer);

          EnqueueTask([clientId, requestLeagueInfo]() {
            g_loginDirector->HandleRequestLeagueInfo(clientId, requestLeagueInfo);
          });
        });

      // RequestQuestList handler
      lobbyServer.RegisterCommandHandler(
        alicia::CommandId::LobbyRequestQuestList,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::LobbyCommandRequestQuestList requestDailyQuestList;
          alicia::LobbyCommandRequestQuestList::Read(requestDailyQuestList, buffer);
          
          EnqueueTask([clientId, requestDailyQuestList]() {
            g_loginDirector->HandleRequestQuestList(clientId, requestDailyQuestList);
          });
        });

      lobbyServer.RegisterCommandHandler(
        alicia::CommandId::LobbyRequestSpecialEventList,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::LobbyCommandRequestSpecialEventList requestSpecialEventList;
          alicia::LobbyCommandRequestSpecialEventList::Read(requestSpecialEventList, buffer);

          EnqueueTask([clientId, requestSpecialEventList]() {
            g_loginDirector->HandleRequestSpecialEventList(clientId, requestSpecialEventList);
          });
        });

      lobbyServer.RegisterCommandHandler(
        alicia::CommandId::LobbyEnterRanch,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::LobbyCommandEnterRanch enterRanch;
          alicia::LobbyCommandEnterRanch::Read(enterRanch, buffer);

          EnqueueTask([clientId, enterRanch]() {
            g_loginDirector->HandleEnterRanch(clientId, enterRanch);
          });
        });

      lobbyServer.RegisterCommandHandler<alicia::LobbyCommandGetMessengerInfo>(
        alicia::CommandId::LobbyGetMessengerInfo,
        [](alicia::ClientId clientId, const auto& message)
        {
            EnqueueTask([clientId, message]() {
                g_loginDirector->HandleGetMessengerInfo(clientId, message);
            });
        });

      // Host
      lobbyServer.Host(settings._lobbySettings.address, settings._lobbySettings.port);
    });

  // Ranch thread.
  std::jthread ranchThread(
    [&settings]()
    {
      alicia::CommandServer ranchServer("Ranch");
      g_ranchDirector = std::make_unique<alicia::RanchDirector>(ranchServer, settings);

      ranchServer.RegisterCommandHandler(
        alicia::CommandId::RanchEnterRanch,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::RanchCommandEnterRanch enterRanch;
          alicia::RanchCommandEnterRanch::Read(enterRanch, buffer);

          EnqueueTask([clientId, enterRanch]() {
            g_ranchDirector->HandleEnterRanch(clientId, enterRanch);
          });
        });

      ranchServer.RegisterCommandHandler(
        alicia::CommandId::RanchSnapshot,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::RanchCommandRanchSnapshot snapshot;
          alicia::RanchCommandRanchSnapshot::Read(snapshot, buffer);

          EnqueueTask([clientId, snapshot]() {
            g_ranchDirector->HandleSnapshot(clientId, snapshot);
          });
        });

      ranchServer.RegisterCommandHandler(
        alicia::CommandId::RanchCmdAction,
        [](alicia::ClientId clientId, auto& buffer)
        {
          alicia::RanchCommandRanchCmdAction cmdAction;
          alicia::RanchCommandRanchCmdAction::Read(cmdAction, buffer);

          EnqueueTask([clientId, cmdAction]() {
            g_ranchDirector->HandleCmdAction(clientId, cmdAction);
          });
        });

      ranchServer.RegisterCommandHandler<alicia::RanchCommandRanchStuff>(
        alicia::CommandId::RanchStuff,
        [](alicia::ClientId clientId, auto& command)
        
        {
          EnqueueTask([clientId, command]() {
            g_ranchDirector->HandleRanchStuff(clientId, command); 
          });
        });

      // Host
      ranchServer.Host(settings._ranchSettings.address, settings._ranchSettings.port);
    });

  // Messenger thread.
  std::jthread messengerThread(
    [&settings]()
    {
      alicia::CommandServer messengerServer("Messenger");
      // TODO: Messenger
      messengerServer.Host("0.0.0.0", 10032);
    });

  taskProcessorThread.join();
  
  return 0;
}
