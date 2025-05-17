#include "Version.hpp"

#include "server/lobby/LobbyDirector.hpp"
#include "server/race/RaceDirector.hpp"
#include "server/ranch/RanchDirector.hpp"
#include "server/Settings.hpp"

#include <libserver/base/Server.hpp>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <thread>

namespace
{

std::unique_ptr<soa::DataDirector> g_dataDirector;
std::unique_ptr<alicia::LobbyDirector> g_lobbyDirector;
std::unique_ptr<alicia::RanchDirector> g_ranchDirector;
std::unique_ptr<alicia::RaceDirector> g_raceDirector;

void TickLoop(
  const uint64_t ticksPerSecond,
  const std::function<void(void)>& task)
{
  using Clock = std::chrono::steady_clock;

  Clock::time_point lastTick;
  const uint64_t millisPerTick = 1000ull / ticksPerSecond;

  while (true)
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

    //try
    {
      task();
    }
    //catch (const std::exception& x)
    //{
    //  spdlog::error("Exception in tick loop: {}", x.what());
    //  break;
    //}
  }
}

} // namespace

int main()
{
  // Daily file sink.
  const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
    "logs/log.log", 0, 0);

  // Console sink.
  const auto consoleSink = std::make_shared<
    spdlog::sinks::stdout_color_sink_mt>();

  // Initialize the application logger with file sink and console sink.
  auto applicationLogger = std::make_shared<spdlog::logger>(
    "server",
    spdlog::sinks_init_list{fileSink, consoleSink});

  applicationLogger->set_level(spdlog::level::debug);
  applicationLogger->set_pattern("%H:%M:%S:%e [%^%l%$] [Thread %t] %v");

  // Set is as the default logger for the application.
  spdlog::set_default_logger(applicationLogger);

  spdlog::info("Running Alicia server v{}.", alicia::BuildVersion);

  // Parsing settings file
  alicia::Settings settings;
  settings.LoadFromFile("resources/settings.json5");

  // Data director.
  g_dataDirector = std::make_unique<soa::DataDirector>();

  const std::jthread dataThread([]()
  {
    TickLoop(50, []()
    {
      g_dataDirector->Tick();
    });
  });

  const std::jthread lobbyThread([&settings]()
  {
    // Lobby director.
    g_lobbyDirector = std::make_unique<alicia::LobbyDirector>(
      *g_dataDirector,
      settings._lobbySettings);

    TickLoop(50, []()
    {
      g_lobbyDirector->Tick();
    });
  });

  const std::jthread ranchThread([&settings]()
  {
    // Ranch director.
    g_ranchDirector = std::make_unique<alicia::RanchDirector>(
      *g_dataDirector,
      settings._ranchSettings);
  });

  const std::jthread raceThread([&settings]()
  {
    // Race director.
    g_raceDirector = std::make_unique<alicia::RaceDirector>(
      *g_dataDirector,
      settings._raceSettings);
  });

  const std::jthread messengerThread([&settings]()
  {
    // TODO: Messenger
    alicia::CommandServer messengerServer("Messenger");
    messengerServer.Host(boost::asio::ip::address_v4::any(), 10032);
  });

  return 0;
}

