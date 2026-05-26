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

#ifndef ALICIA_SERVER_TIMESERIESDATA_HPP
#define ALICIA_SERVER_TIMESERIESDATA_HPP

#include <array>
#include <chrono>
#include <functional>
#include <shared_mutex>

namespace server
{

//! Time series numeric data tracker for purpose of tracking chronological data points.
//! @tparam T Numeric datum type.
//! @tparam HistorySize Data point size.
template <typename T, size_t HistorySize>
class TimeSeriesData
{
public:
  using Clock = std::chrono::system_clock;

  struct Datum
  {
    Clock::time_point timePoint{};
    T value{};
  };

  using Data = std::array<Datum, HistorySize>;

  TimeSeriesData()
  {
    for (auto& data : _data)
    {
      data.timePoint = Clock::time_point::min();
      data.value = T{};
    }
  }

  void Collect(T value) noexcept
  {
    std::scoped_lock lock(_mutex);
    _data[_dataIndex] = Datum{
      .timePoint = Clock::now(),
      .value = value};

    if (++_dataIndex >= HistorySize)
      _dataIndex = 0;
  }

  void GetAndClearData(const std::function<void(Data& data)>& access)
  {
    std::scoped_lock lock(_mutex);
    access(_data);

    _dataIndex = 0;

    for (auto& datum : _data)
    {
      datum.timePoint = Clock::time_point::min();
      datum.value = T{};
    }
  }

private:
  std::shared_mutex _mutex{};

  size_t _dataIndex{};
  Data _data{};
};

}

#endif // ALICIA_SERVER_TIMESERIESDATA_HPP
