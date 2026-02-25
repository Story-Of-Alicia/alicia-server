/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
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

#include "libserver/util/Util.hpp"

#include <ranges>

namespace server::util
{

WinFileTime TimePointToFileTime(const std::chrono::system_clock::time_point& timePoint)
{
  // The time difference between 1970 and 1601 in seconds.
  constexpr uint64_t EpochDifference = 11'644'473'600ULL;
  // The transformation constant to convert seconds to 100ns intervals.
  constexpr uint64_t FileTimeIntervalToSecondsConstant = 10'000'000ull;

  // The total time in seconds since the FILETIME epoch.
  const uint64_t totalTime = std::chrono::ceil<std::chrono::seconds>(
    timePoint.time_since_epoch()).count() + EpochDifference;
  const uint64_t fileTime = totalTime * FileTimeIntervalToSecondsConstant;

  return WinFileTime{
    .dwLowDateTime = static_cast<uint32_t>(fileTime),
    .dwHighDateTime = static_cast<uint32_t>(fileTime >> 32)};
}

uint32_t DateTimeToAliciaTime(const DateTime& dateTime)
{
  // [0000'00] [0[0]'000] [0'0000] [0000]  [0000'0000'0000]
  // [minute]  [hour]     [day]   [month]  [year]
  // <0-63>    <0-15>     <0-31>  <0-15>  <0-4095>
  const uint32_t value = 0
    | std::min(dateTime.years, int32_t{4095}) << 0
    | std::min(dateTime.months, uint32_t{15}) << 12
    | std::min(dateTime.days, uint32_t{31}) << (12 + 4)
    | std::min(dateTime.hours, int32_t{31}) << (12 + 4 + 5)
    | std::min(dateTime.minutes, int32_t{63}) << (12 + 4 + 5 + 5);

  return value;
}

uint32_t TimePointToAliciaTime(const Clock::time_point& timePoint)
{
  const std::chrono::year_month_day date{
    std::chrono::floor<std::chrono::days>(timePoint)};
  const std::chrono::hh_mm_ss time{
    timePoint - std::chrono::floor<std::chrono::days>(timePoint)};

  const DateTime dateTime{
    .years = static_cast<int32_t>(date.year()),
    .months = static_cast<uint32_t>(date.month()),
    .days = static_cast<uint32_t>(date.day()),
    .hours = static_cast<int32_t>(time.hours().count()),
    .minutes = static_cast<int32_t>(time.minutes().count())};
  return DateTimeToAliciaTime(dateTime);
}

uint32_t DurationToAliciaTime(const Clock::duration& duration)
{
  // The extracted date time from the duration.
  DateTime dateTime{};
  // Total time of the duration in seconds.
  auto timeLeft = std::chrono::duration_cast<
    std::chrono::seconds>(duration).count();

  // Convert the remaining time to time unit, subtract equivalent of the time unit in seconds from the time left
  // and store the time unit value.

  // Years
  const auto years = std::chrono::floor<std::chrono::years>(
    std::chrono::seconds(timeLeft));
  timeLeft -= std::chrono::duration_cast<std::chrono::seconds>(years).count();
  dateTime.years = years.count();

  // Months
  const auto months = std::chrono::floor<std::chrono::months>(
    std::chrono::seconds(timeLeft));
  timeLeft -= std::chrono::duration_cast<std::chrono::seconds>(months).count();
  dateTime.months = months.count();

  // Days
  const auto days = std::chrono::floor<std::chrono::days>(
    std::chrono::seconds(timeLeft));
  timeLeft -= std::chrono::duration_cast<std::chrono::seconds>(days).count();
  dateTime.days = days.count();

  // Hours
  const auto hours = std::chrono::floor<std::chrono::hours>(
    std::chrono::seconds(timeLeft));
  timeLeft -= std::chrono::duration_cast<std::chrono::seconds>(hours).count();
  dateTime.hours = hours.count();

  // Minutes
  const auto minutes = std::chrono::floor<std::chrono::minutes>(
    std::chrono::seconds(timeLeft));
  timeLeft -= std::chrono::duration_cast<std::chrono::seconds>(minutes).count();
  dateTime.minutes = minutes.count();

  return DateTimeToAliciaTime(dateTime);
}

DateTime AliciaShopTimeToDateTime(const std::array<uint32_t, 3> timestamp)
{
  // "2025-10-31 23:59:59"
  // 000a07e9 0017001f 003b003b

  // 2025-10 = 0x000a07e9
  // 31 23   = 0x0017001f
  // 59:59   = 0x003b003b

  return DateTime{
    .years = static_cast<uint16_t>(timestamp[0]),
    .months = static_cast<uint16_t>((timestamp[0] >> 16)),
    .days = static_cast<uint16_t>(timestamp[1]),
    .hours = static_cast<uint16_t>((timestamp[1] >> 16)),
    .minutes = static_cast<uint16_t>(timestamp[2]),
    .seconds = static_cast<uint16_t>((timestamp[2] >> 16))
  };
}

std::array<uint32_t, 3> DateTimeToAliciaShopTime(const DateTime& dateTime)
{
  uint32_t monthYear, hourDay, secondMinute;
  monthYear = (dateTime.months << 16) | dateTime.years;
  hourDay = (dateTime.hours << 16) | dateTime.days;
  secondMinute = (dateTime.seconds<< 16) | dateTime.minutes;

  return {monthYear, hourDay, secondMinute};
}

Clock::time_point AliciaShopTimeToTimePoint(const std::array<uint32_t, 3>& timestamp)
{
  // "2025-10-31 23:59:59"
  // 000a07e9 0017001f 003b003b

  // 2025-10 = 0x000a07e9
  // 31 23   = 0x0017001f
  // 59:59   = 0x003b003b

  const uint16_t year = static_cast<uint16_t>(timestamp[0]);
  const uint16_t month = static_cast<uint16_t>(timestamp[0] >> 16);
  const uint16_t day = static_cast<uint16_t>(timestamp[1]);
  const uint16_t hour = static_cast<uint16_t>(timestamp[1] >> 16);
  const uint16_t minute = static_cast<uint16_t>(timestamp[2]);
  const uint16_t second = static_cast<uint16_t>(timestamp[2] >> 16);

  std::chrono::year y{static_cast<int32_t>(year)};
  std::chrono::month m{static_cast<uint32_t>(month)};
  std::chrono::day d{static_cast<uint32_t>(day)};

  std::chrono::year_month_day ymd{y, m, d};
  std::chrono::sys_days days{ymd};
  std::chrono::sys_seconds tp_seconds = 
    days + 
    std::chrono::hours{hour} + 
    std::chrono::minutes{minute} + 
    std::chrono::seconds{second};

  // convert to system_clock::time_point (sys_seconds is based on system_clock on most platforms)
  return std::chrono::system_clock::time_point(tp_seconds);
}

std::array<uint32_t, 3> TimePointToAliciaShopTime(const Clock::time_point& timePoint)
{
  const std::chrono::year_month_day date{
    std::chrono::floor<std::chrono::days>(timePoint)};
  if (not date.ok())
    throw std::runtime_error("Invalid date");

  const std::chrono::hh_mm_ss time{
    timePoint - std::chrono::floor<std::chrono::days>(timePoint)};

  // Pack according to spec:
  // word0: high16 = month, low16 = year
  // word1: high16 = hour,  low16 = day
  // word2: high16 = second, low16 = minute
  uint32_t w0 = 
    (static_cast<uint32_t>(date.month()) << 16) | static_cast<int32_t>(date.year());
  uint32_t w1 = 
    (static_cast<int32_t>(time.hours().count()) << 16) | static_cast<uint32_t>(date.day());
  uint32_t w2 = 
    (static_cast<int32_t>(time.seconds().count()) << 16) | static_cast<int32_t>(time.minutes().count());

  return std::array<uint32_t, 3>{w0, w1, w2};
}

asio::ip::address_v4 ResolveHostName(const std::string& host)
{
  try
  {
    // Try to parse the address directly.
    const auto address = asio::ip::make_address(host);
    return address.to_v4();
  }
  catch (const std::exception&)
  {
  }

  asio::io_context ioContext;
  asio::ip::tcp::resolver resolver(ioContext);
  const auto endpoints = resolver.resolve(host, "");

  for (const auto& endpoint : endpoints)
  {
    const auto& addr = endpoint.endpoint().address();
    if (addr.is_v4())
      return addr.to_v4();
  }

  throw std::runtime_error(
    std::format("Hostname '{}' does not resolve to any valid IPv4 address.", host));
}

std::string GenerateByteDump(const std::span<const std::byte> data)
{
  if (data.empty())
    return "";

  std::string dump;

  for (const auto row : data | std::views::chunk(16))
  {
    std::string bytes;
    std::string ascii;
    for (const auto& byte : row)
    {
      bytes += std::format("{:02X} ", static_cast<uint8_t>(byte));
      ascii += std::format("{:c}", std::isalnum(static_cast<uint8_t>(byte)) ? static_cast<uint8_t>(byte) : '.');
    }

    dump += std::format("{:<48}\t{:<16}\n", bytes, ascii);
  }

  return dump;
}

std::vector<std::string> TokenizeString(const std::string& value, char delimiter)
{
  std::vector<std::string> tokens;
  size_t position = 0;
  size_t idx = std::string::npos;

  while (true)
  {
    idx = value.find(delimiter, position);
    if (idx == std::string::npos)
    {
      tokens.emplace_back(
        value.substr(position));
      break;
    }

    tokens.emplace_back(
      value.substr(position, idx - position));
    position = idx + 1;
  }

  return tokens;
}

} // namespace server
