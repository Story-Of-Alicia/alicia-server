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
#include "server/race/RaceNetworkHandler.hpp"

#include "server/ServerInstance.hpp"

#include <chrono>
#include <format>
#include <vector>
#include <map>

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
  tx.exec("create table if not exists metrics.privatechat_send_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.privatechat_receive_time_series(time bigint, value int);");

  tx.exec("create table if not exists metrics.lobby_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.ranch_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.race_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.messenger_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.allchat_processing_time_series(time bigint, value int);");
  tx.exec("create table if not exists metrics.privatechat_processing_time_series(time bigint, value int);");

  tx.commit();
}

  //! Remove metrics older than 30 days on startup
void CleanOldData(pqxx::connection& connection)
{
  pqxx::work tx(connection);

  const auto cutoff = std::chrono::duration_cast<std::chrono::seconds>(
    (std::chrono::system_clock::now() - std::chrono::hours(24 * 30)).time_since_epoch())
                        .count();

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
  tx.exec(std::format("delete from metrics.privatechat_send_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.privatechat_receive_time_series where time < {};", cutoff));

  tx.exec(std::format("delete from metrics.lobby_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.ranch_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.race_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.messenger_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.allchat_processing_time_series where time < {};", cutoff));
  tx.exec(std::format("delete from metrics.privatechat_processing_time_series where time < {};", cutoff));

  tx.commit();
}

  //! Group samples from the same second and write their average as one row.
  template <typename Range>
  void WriteTimingStatistics(
    pqxx::work& tx,
    const char* tableName,
    const Range& statistics)
{
  std::map<int64_t, std::pair<int64_t, size_t>> buckets;

  for (const auto& [timePoint, value] : statistics)
  {
    if (timePoint == TimeStatistics::Clock::time_point::min())
      continue;

    const auto second =
      std::chrono::duration_cast<std::chrono::seconds>(
        timePoint.time_since_epoch()).count();

    auto& [sum, count] = buckets[second];
    sum += value;
    ++count;
  }

  auto stream = pqxx::stream_to::raw_table(tx, tableName);

  for (const auto& [second, bucket] : buckets)
  {
    const auto& [sum, count] = bucket;
    stream.write_values(second, sum / static_cast<int64_t>(count));
  }

  stream.complete();
}

} // anon namespace

  namespace
{
  //! Flatten the populated samples from each client's buffer into one batch.
  std::vector<TimeStatistics::Datum> MergeClientTimeStatistics(
    const std::vector<TimeStatistics::Data>& clientStatistics)
  {
    std::vector<TimeStatistics::Datum> mergedStatistics;

    for (const auto& statistics : clientStatistics)
    {
      for (const auto& datum : statistics)
      {
        if (datum.timePoint == TimeStatistics::Clock::time_point::min())
          continue;

        mergedStatistics.emplace_back(datum);
      }
    }

    return mergedStatistics;
  }
}

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
  const auto roomCount = _serverInstance.GetRoomSystem().GetRoomCount();

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
          if (timePoint == decltype(_roomCountMetric)::Clock::time_point::min())
            continue;

          roomCountStream.write_values(
            std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count(),
            value);
        }
      });
    roomCountStream.complete();

    //! Pull each server once before collecting its metrics.
    auto& lobbyCommandServer = _serverInstance.GetLobbyDirector().GetNetworkHandler().GetCommandServer();
    auto& ranchCommandServer = _serverInstance.GetRanchDirector().GetCommandServer();
    auto& raceCommandServer = _serverInstance.GetRaceDirector().GetNetworkHandler().GetCommandServer();
    auto& messengerChatterServer = _serverInstance.GetMessengerDirector().GetChatterServer();
    auto& allChatChatterServer = _serverInstance.GetAllChatDirector().GetChatterServer();
    auto& privateChatChatterServer = _serverInstance.GetPrivateChatDirector().GetChatterServer();

    WriteTimingStatistics(
      tx,
      "metrics.lobby_send_time_series",
      MergeClientTimeStatistics(lobbyCommandServer.GetServer().GetSendTimeStatistics()));
    WriteTimingStatistics(
      tx,
      "metrics.lobby_receive_time_series",
      MergeClientTimeStatistics(lobbyCommandServer.GetServer().GetReceiveTimeStatistics()));

    WriteTimingStatistics(
      tx,
      "metrics.ranch_send_time_series",
      MergeClientTimeStatistics(ranchCommandServer.GetServer().GetSendTimeStatistics()));
    WriteTimingStatistics(
      tx,
      "metrics.ranch_receive_time_series",
      MergeClientTimeStatistics(ranchCommandServer.GetServer().GetReceiveTimeStatistics()));

    WriteTimingStatistics(
      tx,
      "metrics.race_send_time_series",
      MergeClientTimeStatistics(raceCommandServer.GetServer().GetSendTimeStatistics()));
    WriteTimingStatistics(
      tx,
      "metrics.race_receive_time_series",
      MergeClientTimeStatistics(raceCommandServer.GetServer().GetReceiveTimeStatistics()));

    WriteTimingStatistics(
      tx,
      "metrics.messenger_send_time_series",
      MergeClientTimeStatistics(messengerChatterServer.GetServer().GetSendTimeStatistics()));
    WriteTimingStatistics(
      tx,
      "metrics.messenger_receive_time_series",
      MergeClientTimeStatistics(messengerChatterServer.GetServer().GetReceiveTimeStatistics()));

    WriteTimingStatistics(
      tx,
      "metrics.allchat_send_time_series",
      MergeClientTimeStatistics(allChatChatterServer.GetServer().GetSendTimeStatistics()));
    WriteTimingStatistics(
      tx,
      "metrics.allchat_receive_time_series",
      MergeClientTimeStatistics(allChatChatterServer.GetServer().GetReceiveTimeStatistics()));

    WriteTimingStatistics(
      tx,
      "metrics.privatechat_send_time_series",
      MergeClientTimeStatistics(privateChatChatterServer.GetServer().GetSendTimeStatistics()));
    WriteTimingStatistics(
      tx,
      "metrics.privatechat_receive_time_series",
      MergeClientTimeStatistics(privateChatChatterServer.GetServer().GetReceiveTimeStatistics()));

    WriteTimingStatistics(
      tx,
      "metrics.lobby_processing_time_series",
      lobbyCommandServer.GetProcessingTimeStatistics().GetAndClearData());
    WriteTimingStatistics(
      tx,
      "metrics.ranch_processing_time_series",
      ranchCommandServer.GetProcessingTimeStatistics().GetAndClearData());
    WriteTimingStatistics(
      tx,
      "metrics.race_processing_time_series",
      raceCommandServer.GetProcessingTimeStatistics().GetAndClearData());
    WriteTimingStatistics(
      tx,
      "metrics.messenger_processing_time_series",
      messengerChatterServer.GetProcessingTimeStatistics().GetAndClearData());
    WriteTimingStatistics(
      tx,
      "metrics.allchat_processing_time_series",
      allChatChatterServer.GetProcessingTimeStatistics().GetAndClearData());
    WriteTimingStatistics(
      tx,
      "metrics.privatechat_processing_time_series",
      privateChatChatterServer.GetProcessingTimeStatistics().GetAndClearData());

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