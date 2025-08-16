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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <boost/asio.hpp>
#include <chrono>
#include <span>

namespace server::util
{

namespace asio = boost::asio;

using Clock = std::chrono::system_clock;

//! Windows file-time represents number of 100 nanosecond intervals since January 1, 1601 (UTC).
struct WinFileTime
{
  uint32_t dwLowDateTime = 0;
  uint32_t dwHighDateTime = 0;
};

//! A zero-cost struct to represent a date and a time.
struct DateTime
{
  int32_t years = 0;
  uint32_t months = 0;
  uint32_t days = 0;
  int32_t hours = 0;
  int32_t minutes = 0;
};

//! Converts a time point to the Windows file time.
//! @param timePoint Point in time.
//! @return Windows file time representing specified point in time.
WinFileTime TimePointToFileTime(const Clock::time_point& timePoint);

//! Converts date time to alicia time.
//! @param dateTime Date and time.
//! @returns Alicia time representing the date and time.
uint32_t DateTimeToAliciaTime(const DateTime& dateTime);

//! Converts time point to alicia time.
//! @param timePoint Time point.
//! @returns Alicia time representing the date and time of the time point.
uint32_t TimePointToAliciaTime(const Clock::time_point& timePoint);

//! Converts duration to alicia time.
//! @param duration Duration.
//! @returns Alicia time representing the duration.
uint32_t DurationToAliciaTime(const Clock::duration& duration);

asio::ip::address_v4 ResolveHostName(const std::string& host);

std::string GenerateByteDump(std::span<const std::byte> data);

std::vector<std::string> TokenizeString(const std::string& value, char delimiter);

} // namespace server::util

#endif // UTIL_HPP
