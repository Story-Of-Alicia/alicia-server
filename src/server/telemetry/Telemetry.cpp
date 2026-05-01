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

  tx.commit();
}

} // anon namespace

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
  if (settings.telemetry.backend == "postgres")
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