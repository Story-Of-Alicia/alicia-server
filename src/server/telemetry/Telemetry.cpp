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
  tx.exec("create table player_count_time_series(time timestamp, value int);");
  tx.exec("create table room_count_time_series(time timestamp, value int);");
}

} // anon namespace

Telemetry::Telemetry(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void Telemetry::Initialize()
{
  const auto& settings = _serverInstance.GetSettings();

  if (settings.telemetry.backend == "postgres")
  {
    _connection = std::make_unique<pqxx::connection>(settings.telemetry.backend);

    PrepareTables(*_connection);

    ScheduleCollectData();
    ScheduleSynchronizeData();

    spdlog::info("Telemetry is using PostgreSQL backend");
  }
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

void Telemetry::CollectData()
{
  const auto playerCount = _serverInstance.GetLobbyDirector().GetUserCount();
  const auto roomCount = _serverInstance.GetRaceDirector().GetRoomCount();

  _playerCountTimeSeries.Collect(playerCount);
  _roomCountTimeSeries.Collect(roomCount);
}

void Telemetry::ScheduleCollectData()
{
  _scheduler.Queue(
    [this]()
    {
      CollectData();
      ScheduleCollectData();
    },
    Scheduler::Clock::now() + std::chrono::seconds(5));
}

void Telemetry::SynchronizeData()
{
  pqxx::work tx(*_connection);

  _playerCountTimeSeries.GetAndClearData([&tx](auto& data)
    {
      for (const auto& [timePoint, value] : data)
      {
        tx.exec(std::format(
            "insert into player_count_time_series (%ld, %ld)",
            timePoint.time_since_epoch(),
            value));
      }
    });

  _roomCountTimeSeries.GetAndClearData([&tx](auto& data)
    {
      for (const auto& [timePoint, value] : data)
      {
        tx.exec(std::format(
            "insert into room_count_time_series (%ld, %ld)",
            timePoint.time_since_epoch(),
            value));
      }
    });
}

void Telemetry::ScheduleSynchronizeData()
{
  _scheduler.Queue(
    [this]()
    {
      SynchronizeData();
      ScheduleSynchronizeData();
    },
    Scheduler::Clock::now() + std::chrono::minutes(1));
}

} // namespace server