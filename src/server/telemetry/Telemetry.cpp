/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

#include "server/telemetry/Telemetry.hpp"

#include "server/lobby/LobbyNetworkHandler.hpp"
#include "server/ServerInstance.hpp"

namespace server
{

namespace
{

void PrepareTables(pqxx::connection& connection)
{
  pqxx::work tx(connection);
  tx.exec("create schema if not exists metrics");
  tx.exec("create table if not exists metrics.player_count_time_series(time bigint primary key, value int);");
  tx.exec("create table if not exists metrics.room_count_time_series(time bigint primary key, value int);");

  tx.exec("create table if not exists metrics.lobby_send_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.lobby_receive_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.ranch_send_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.ranch_receive_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.race_send_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.race_receive_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.allchat_send_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.allchat_receive_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.messenger_send_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.messenger_receive_time_series(time bigint, value int);");

  tx.exec("create table if not exists metrics.lobby_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.ranch_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.race_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.messenger_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.allchat_processing_time_series(time bigint, value int);");

  tx.commit();
}

//! Deletes metric rows older than 30 days. Run once on startup.
void CleanOldData(pqxx::connection& connection)
{
  pqxx::work tx(connection);

  const auto cutoff = std::chrono::duration_cast<std::chrono::seconds>(
    (std::chrono::system_clock::now() - std::chrono::hours(24 * 30)).time_since_epoch())
                        .count();

  spdlog::info("Cleaning up metric data older than 30 days");

  tx.exec(std::format("delete from metrics.player_count_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.room_count_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.lobby_send_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.lobby_receive_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.ranch_send_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.ranch_receive_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.race_send_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.race_receive_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.allchat_send_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.allchat_receive_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.messenger_send_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.messenger_receive_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.lobby_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.ranch_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.race_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.messenger_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.allchat_processing_time_series where time < {};", cutoff));

  tx.commit();
}

} // namespace

Telemetry::Telemetry(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void Telemetry::Initialize()
{
  const auto& settings = _serverInstance.GetSettings();

  if (settings.telemetry.backend == "none")
  {
    spdlog::info("Telemetry is not using any backend");
    ScheduleCollectData();
  }
  else if (settings.telemetry.backend == "postgres")
  {
    spdlog::info("Telemetry is using PostgreSQL backend");

    ConnectPostgresBackend();

    ScheduleCollectData();
    ScheduleSynchronizeData();
  }
  else
  {
    spdlog::warn("Telemetry is using an unknown backend");
  }

  spdlog::info("Telemetry is collecting metrics");
}

void Telemetry::Terminate()
{
  CollectData();
  SynchronizeData();
}

void Telemetry::Tick()
{
  _scheduler.Tick();
}

void Telemetry::ConnectPostgresBackend()
{
  const auto& settings = _serverInstance.GetSettings();

  try
  {
    const auto timerBegin = std::chrono::steady_clock::now();

    _connection.emplace(
      settings.telemetry.postgres.connectionUri);
    PrepareTables(*_connection);
    CleanOldData(*_connection);

    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - timerBegin);
    spdlog::info("Connection to telemetry backend established in {}ms", time.count());
  }
  catch (const std::exception& x)
  {
    spdlog::warn("Telemetry backend exception: {}", x.what());
    spdlog::error("Telemetry backend is not functional, data are not synchronized");
  }
}

void Telemetry::CollectData()
{
  const auto playerCount = _serverInstance.GetLobbyDirector().GetUserCount();
  const auto roomCount = _serverInstance.GetRaceDirector().GetRoomCount();

  _playerCountMetric.Collect(playerCount);
  _roomCountMetric.Collect(roomCount);
}

void Telemetry::ScheduleCollectData()
{
  _scheduler.Queue(
    [this]()
    {
      try
      {
        CollectData();
        ScheduleCollectData();
      }
      catch (const std::exception& x)
      {
        spdlog::error("Exception occurred while collecting metrics: {}", x.what());
      }
    },
    Scheduler::Clock::now() + std::chrono::seconds(1));
}

void Telemetry::SynchronizeData()
{
  if (not _connection)
    return;

  try
  {
    pqxx::work tx(*_connection);

    auto playerCountStream = pqxx::stream_to::raw_table(tx, "metrics.player_count_time_series");
    _playerCountMetric.GetAndClearData([&playerCountStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == decltype(_playerCountMetric)::Clock::time_point::min())
            continue;

          playerCountStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    playerCountStream.complete();

    auto roomCountStream = pqxx::stream_to::raw_table(tx, "metrics.room_count_time_series");
    _roomCountMetric.GetAndClearData([&roomCountStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == decltype(_playerCountMetric)::Clock::time_point::min())
            continue;

          roomCountStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    roomCountStream.complete();

    // Drain per-server network and processing timing statistics to their tables.
    auto lobbySendStream = pqxx::stream_to::raw_table(tx, "metrics.lobby_send_time_series");
    _serverInstance.GetLobbyDirector().GetNetworkHandler().GetCommandServer().GetServer().GetSendTimeStatistics().GetAndClearData([&lobbySendStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          // TimeSeriesData uses system_clock, so the sentinel must match that clock.
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;

          lobbySendStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    lobbySendStream.complete();

    auto lobbyReceiveStream = pqxx::stream_to::raw_table(tx, "metrics.lobby_receive_time_series");
    _serverInstance.GetLobbyDirector().GetNetworkHandler().GetCommandServer().GetServer().GetReceiveTimeStatistics().GetAndClearData([&lobbyReceiveStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;

          lobbyReceiveStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    lobbyReceiveStream.complete();

    auto ranchSendStream = pqxx::stream_to::raw_table(tx, "metrics.ranch_send_time_series");
    _serverInstance.GetRanchDirector().GetCommandServer().GetServer().GetSendTimeStatistics().GetAndClearData([&ranchSendStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;

          ranchSendStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    ranchSendStream.complete();

    auto ranchReceiveStream = pqxx::stream_to::raw_table(tx, "metrics.ranch_receive_time_series");
    _serverInstance.GetRanchDirector().GetCommandServer().GetServer().GetReceiveTimeStatistics().GetAndClearData([&ranchReceiveStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;

          ranchReceiveStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    ranchReceiveStream.complete();

    auto raceSendStream = pqxx::stream_to::raw_table(tx, "metrics.race_send_time_series");
    _serverInstance.GetRaceDirector().GetCommandServer().GetServer().GetSendTimeStatistics().GetAndClearData([&raceSendStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;

          raceSendStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    raceSendStream.complete();

    auto raceReceiveStream = pqxx::stream_to::raw_table(tx, "metrics.race_receive_time_series");
    _serverInstance.GetRaceDirector().GetCommandServer().GetServer().GetReceiveTimeStatistics().GetAndClearData([&raceReceiveStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;

          raceReceiveStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    raceReceiveStream.complete();

    auto allchatSendStream = pqxx::stream_to::raw_table(tx, "metrics.allchat_send_time_series");
    _serverInstance.GetAllChatDirector().GetChatterServer().GetServer().GetSendTimeStatistics().GetAndClearData([&allchatSendStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          allchatSendStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    allchatSendStream.complete();

    auto allchatRecieveStream = pqxx::stream_to::raw_table(tx, "metrics.allchat_receive_time_series");
    _serverInstance.GetAllChatDirector().GetChatterServer().GetServer().GetReceiveTimeStatistics().GetAndClearData([&allchatRecieveStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          allchatRecieveStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    allchatRecieveStream.complete();

    auto messengerSendStream = pqxx::stream_to::raw_table(tx, "metrics.messenger_send_time_series");
    _serverInstance.GetMessengerDirector().GetChatterServer().GetServer().GetSendTimeStatistics().GetAndClearData([&messengerSendStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          messengerSendStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    messengerSendStream.complete();

    auto messengerRecieveStream = pqxx::stream_to::raw_table(tx, "metrics.messenger_receive_time_series");
    _serverInstance.GetMessengerDirector().GetChatterServer().GetServer().GetReceiveTimeStatistics().GetAndClearData([&messengerRecieveStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          messengerRecieveStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    messengerRecieveStream.complete();

    auto lobbyProcessingStream = pqxx::stream_to::raw_table(tx, "metrics.lobby_processing_time_series");
    _serverInstance.GetLobbyDirector().GetNetworkHandler().GetCommandServer().GetProcessingTimeStatistics().GetAndClearData([&lobbyProcessingStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          lobbyProcessingStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    lobbyProcessingStream.complete();

    auto ranchProcessingStream = pqxx::stream_to::raw_table(tx, "metrics.ranch_processing_time_series");
    _serverInstance.GetRanchDirector().GetCommandServer().GetProcessingTimeStatistics().GetAndClearData([&ranchProcessingStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          ranchProcessingStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    ranchProcessingStream.complete();

    auto raceProcessingStream = pqxx::stream_to::raw_table(tx, "metrics.race_processing_time_series");
    _serverInstance.GetRaceDirector().GetCommandServer().GetProcessingTimeStatistics().GetAndClearData([&raceProcessingStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          raceProcessingStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    raceProcessingStream.complete();

    auto messengerProcessingStream = pqxx::stream_to::raw_table(tx, "metrics.messenger_processing_time_series");
    _serverInstance.GetMessengerDirector().GetChatterServer().GetProcessingTimeStatistics().GetAndClearData([&messengerProcessingStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          messengerProcessingStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    messengerProcessingStream.complete();

    auto allchatProcessingStream = pqxx::stream_to::raw_table(tx, "metrics.allchat_processing_time_series");
    _serverInstance.GetAllChatDirector().GetChatterServer().GetProcessingTimeStatistics().GetAndClearData([&allchatProcessingStream](auto& data)
      {
        for (const auto& [timePoint, value] : data)
        {
          if (timePoint == std::chrono::system_clock::time_point::min())
            continue;
          allchatProcessingStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    allchatProcessingStream.complete();

    tx.commit();
  }
  catch (const pqxx::broken_connection&)
  {
    spdlog::warn("Lost connection to telemetry backend, attempting to perform a reconnect");
    ConnectPostgresBackend();
  }
}

void Telemetry::ScheduleSynchronizeData()
{
  _scheduler.Queue(
    [this]()
    {
      try
      {
        SynchronizeData();
        ScheduleSynchronizeData();
      }
      catch (const std::exception& x)
      {
        spdlog::error("Exception occurred while synchronizing data with telemetry backend: {}", x.what());
      }
    },
    Scheduler::Clock::now() + std::chrono::minutes(1));
}

} // namespace server
