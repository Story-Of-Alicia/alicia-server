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

#ifndef ALICIA_SERVER_TELEMETRY_HPP
#define ALICIA_SERVER_TELEMETRY_HPP

#include <libserver/util/Scheduler.hpp>
#include <libserver/util/TimeSeriesData.hpp>

#include <optional>

#include <pqxx/pqxx>

namespace server
{

class ServerInstance;

class Telemetry
{
public:
  explicit Telemetry(ServerInstance& serverInstance);

  void Initialize();
  void Terminate();
  void Tick();

private:
  //! Time series data tracking the player count.
  TimeSeriesData<size_t, 3600> _playerCountMetric;
  //! Time series data tracking the race count.
  TimeSeriesData<size_t, 3600> _roomCountMetric;

  //! Flag indicating whether telemetry is enabled.
  bool enabled = false;

  void ConnectPostgresBackend();

  void CollectData();
  void ScheduleCollectData();

  void SynchronizeData();
  void ScheduleSynchronizeData();

  //! Reference to server instance.
  ServerInstance& _serverInstance;
  //! Job scheduler.
  Scheduler _scheduler;

  // telemetry data source
  std::optional<pqxx::connection> _connection;
};

} // namespace server

#endif // ALICIA_SERVER_TELEMETRY_HPP
