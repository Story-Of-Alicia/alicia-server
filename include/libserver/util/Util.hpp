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

namespace server
{

namespace util
{

namespace asio = boost::asio;

//! Windows file-time represents number of 100 nanosecond intervals since January 1, 1601 (UTC).
struct WinFileTime
{
  uint32_t dwLowDateTime = 0;
  uint32_t dwHighDateTime = 0;
};

//! Converts a time point to the Windows file time.
//! @param timePoint Point in time.
//! @return Windows file time representing specified point in time.
WinFileTime UnixTimeToFileTime(const std::chrono::system_clock::time_point& timePoint);

asio::ip::address_v4 ResolveHostName(const std::string& host);

std::string GenerateByteDump(std::span<const std::byte> data);

std::vector<std::string> TokenizeString(const std::string& value, char delimiter);

} // namespace util

} // namespace server

#endif // UTIL_HPP
