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

#include <libserver/util/Util.hpp>

#include <array>
#include <chrono>
#include <cassert>

namespace
{

void TestAliciaShopTimeToTimePoint()
{
  // 2025-10-31 23:59:58
  constexpr std::array<uint32_t, 3> actualTimestamp{0x000a07e9, 0x0017001f, 0x003a003b};
  const auto& actualTimePoint = server::util::AliciaShopTimeToTimePoint(actualTimestamp);

  // 2025-10-31 23:59:58
  constexpr auto expectedTimePoint =
    std::chrono::sys_days{std::chrono::year{2025} / 10 / 31} +
    std::chrono::hours{23} +
    std::chrono::minutes{59} +
    std::chrono::seconds{58};

  assert(actualTimePoint == expectedTimePoint);
}

void TestTimePointToAliciaShopTime()
{
  // 2026-01-23 01:23:45
  constexpr auto actualTimePoint =
    std::chrono::sys_days{std::chrono::year{2026} / 01 / 23} +
    std::chrono::hours{01} +
    std::chrono::minutes{23} +
    std::chrono::seconds{45};
  const auto& actualTimestamp = server::util::TimePointToAliciaShopTime(actualTimePoint);

  // 2026-01-23 01:23:45
  constexpr std::array<uint32_t, 3> expectedTimestamp{0x000107ea, 0x00010017, 0x002d0017};

  bool success = false;
  for (auto i = 0; i < 3; ++i)
  {
    success = actualTimestamp[i] == expectedTimestamp[i];
    if (not success)
      break;
  }

  assert(success);
}

} // namespace

int main()
{
  TestAliciaShopTimeToTimePoint();
  TestTimePointToAliciaShopTime();
}
