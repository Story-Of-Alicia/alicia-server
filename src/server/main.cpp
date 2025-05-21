#include "Version.hpp"

#include "server/lobby/LobbyDirector.hpp"
#include "server/race/RaceDirector.hpp"
#include "server/ranch/RanchDirector.hpp"
#include "server/Settings.hpp"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <iostream>
#include <thread>

#ifdef WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

namespace
{

using Clock = std::chrono::steady_clock;

std::atomic_bool shouldRun = true;

std::shared_ptr<spdlog::logger> g_logger;

std::unique_ptr<soa::DataDirector> g_dataDirector;
std::unique_ptr<alicia::LobbyDirector> g_lobbyDirector;
std::unique_ptr<alicia::RanchDirector> g_ranchDirector;
std::unique_ptr<alicia::RaceDirector> g_raceDirector;

Clock::time_point serverStartupTime;

void TickLoop(
  const uint64_t ticksPerSecond,
  const std::function<void(void)>& task)
{
  Clock::time_point lastTick;
  const uint64_t millisPerTick = 1000ull / ticksPerSecond;

  while (shouldRun)
  {
    const auto timeNow = Clock::now();
    // Time delta between ticks [ms].
    const auto tickDelta = std::chrono::duration_cast<
      std::chrono::milliseconds>(timeNow - lastTick);

    if (tickDelta < std::chrono::milliseconds(millisPerTick))
    {
      const auto sleepMs = millisPerTick - tickDelta.count();
      std::this_thread::sleep_for(
        std::chrono::milliseconds(sleepMs));
      continue;
    }

    lastTick = timeNow;

    try
    {
      task();
    }
    catch (const std::exception& x)
    {
      spdlog::error("Exception in tick loop: {}", x.what());
      break;
    }
  }
}

#ifdef WIN32

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
  switch (fdwCtrlType)
  {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    {
      spdlog::debug("Shutting down because of CTRL+C");
      shouldRun = false;
      return TRUE;
    }

    default:
      return FALSE;
  }
}
#else

void
handler(int sig, siginfo_t *info, void *ucontext)
{
  if (sig == SIGTERM)
  {
    spdlog::debug("SIGTERM caught, exiting");
    shouldRun = false;
  }
}

#endif

} // namespace

int main()
{
#ifdef WIN32
  // Register the control handler.
  SetConsoleCtrlHandler(CtrlHandler, TRUE);
#else
  struct sigaction act {};

  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = &handler;
  if (sigaction(SIGTERM, &act, nullptr) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
#endif

  serverStartupTime = std::chrono::steady_clock::now();

  // Daily file sink.
  const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
    "logs/log.log", 0, 0);

  // Console sink.
  const auto consoleSink = std::make_shared<
    spdlog::sinks::stdout_color_sink_mt>();

  // Initialize the application logger with file sink and console sink.
  g_logger = std::make_shared<spdlog::logger>(
    "server",
    spdlog::sinks_init_list{fileSink, consoleSink});

  g_logger->set_level(spdlog::level::debug);
  g_logger->set_pattern("%H:%M:%S:%e [%^%l%$] [Thread %t] %v");

  // Set is as the default logger for the application.
  spdlog::set_default_logger(g_logger);

  spdlog::info("Running Alicia server v{}.", alicia::BuildVersion);

  // Parsing settings file
  alicia::Settings settings;
  settings.LoadFromFile("resources/settings.json5");

  // Data director.
  g_dataDirector = std::make_unique<soa::DataDirector>();

  // Lobby director.
  g_lobbyDirector = std::make_unique<alicia::LobbyDirector>(
    *g_dataDirector,
    settings._lobbySettings);
  // Ranch director.
  g_ranchDirector = std::make_unique<alicia::RanchDirector>(
    *g_dataDirector,
    settings._ranchSettings);
  // Race director.
  g_raceDirector = std::make_unique<alicia::RaceDirector>(
    *g_dataDirector,
    settings._raceSettings);

  const std::jthread dataThread([]()
  {
    g_dataDirector->Initialize();
    TickLoop(50, []()
    {
      g_dataDirector->Tick();
    });
    g_dataDirector->Terminate();
  });

  const std::jthread lobbyThread([]()
  {
    g_lobbyDirector->Initialize();
    TickLoop(50, []()
    {
      g_lobbyDirector->Tick();
    });
    g_lobbyDirector->Terminate();
  });

  const std::jthread ranchThread([]()
  {
    g_ranchDirector->Initialize();
    TickLoop(50, []()
    {
      g_ranchDirector->Tick();
    });
    g_ranchDirector->Terminate();
  });

  const std::jthread raceThread([]()
  {
    g_raceDirector->Initialize();
    TickLoop(50, []()
    {
      g_raceDirector->Tick();
    });
    g_raceDirector->Terminate();
  });

  const std::jthread messengerThread([]()
  {
    // TODO: Messenger
    alicia::CommandServer messengerServer;
    //messengerServer.Host(boost::asio::ip::address_v4::any(), 10032);
  });

  spdlog::info(
    "Server started up in {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(
      Clock::now() - serverStartupTime).count());

  while (shouldRun)
  {
    std::string commandLine;
    std::getline(std::cin, commandLine);

    std::vector<std::string> command;
    size_t position = 0;
    size_t idx = std::string::npos;
    while (true)
    {
      idx = commandLine.find(' ', position);
      if (idx == std::string::npos)
      {
        command.emplace_back(
          commandLine.substr(position));
        break;
      }

      command.emplace_back(
        commandLine.substr(position, idx - position));
      position = idx + 1;
    }

    if (command[0] == "exit")
    {
      shouldRun = false;
    }
    else if (command[0] == "register")
    {
      if (command.size() < 3)
      {
        printf("Please specify name and token\n");
        continue;
      }

      const auto name = command[1];
      const auto token = command[2];

      auto user = g_dataDirector->GetUsers().Create(name);
      if (not user)
      {
        printf("User already exists.\n");
        continue;
      }

      user->Mutable([&name, &token](auto& user)
      {
        user.name = name;
        user.token = token;
      });
      printf("Created user %s (token: %s)\n", name.c_str(), token.c_str());
    }
  }

  return 0;
}

