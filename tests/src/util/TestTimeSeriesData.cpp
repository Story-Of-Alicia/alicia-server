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

#include <libserver/util/TimeSeriesData.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <thread>
#include <vector>

namespace
{
  //! Strip the unused slots before checking the returned samples.
template <typename Series>
std::vector<typename Series::Datum> ValidData(const typename Series::Data& data)
{
  std::vector<typename Series::Datum> result;

  for (const auto& datum : data)
  {
    if (datum.timePoint != Series::Clock::time_point::min())
      result.emplace_back(datum);
  }

  return result;
}

  //! Add two samples, pull them once, then make sure the next pull is empty.
  void TestGetAndClearReturnsDataOnce()
{
  using Series = server::TimeSeriesData<int, 4>;

  Series series;
  series.Collect(11);
  series.Collect(22);

  const auto first = ValidData<Series>(series.GetAndClearData());
  assert(first.size() == 2);
  assert(first[0].value == 11);
  assert(first[1].value == 22);

  const auto second = ValidData<Series>(series.GetAndClearData());
  assert(second.empty());
}

  //! Push five samples into a four-slot buffer and make sure the oldest one gets replaced.
void TestHistoryWraps()
{
  using Series = server::TimeSeriesData<int, 4>;

  Series series;
  for (int value = 1; value <= 5; ++value)
    series.Collect(value);

  auto data = ValidData<Series>(series.GetAndClearData());
  assert(data.size() == 4);

  std::array<int, 4> values{};
  std::ranges::transform(data, values.begin(), &Series::Datum::value);
  //! Only the retained values matter here, not their slot order.
  std::ranges::sort(values);

  assert((values == std::array{2, 3, 4, 5}));
}

  //! Collect 100 samples from four threads at once, similar to several handlers
  //! writing timing data in parallel, and make sure all 400 samples are kept.
void TestConcurrentCollection()
{
  //! Keep the buffer above 400 so wrapping doesn't affect this test.
  using Series = server::TimeSeriesData<int, 512>;

  Series series;
  std::array<std::thread, 4> threads;

  for (size_t threadIndex = 0; threadIndex < threads.size(); ++threadIndex)
  {
    threads[threadIndex] = std::thread(
      [&series, threadIndex]()
      {
        for (int sample = 0; sample < 100; ++sample)
          series.Collect(static_cast<int>(threadIndex * 100 + sample));
      });
  }

  for (auto& thread : threads)
    thread.join();

  const auto data = ValidData<Series>(series.GetAndClearData());
  assert(data.size() == 400);
}

} // namespace

int main()
{
  TestGetAndClearReturnsDataOnce();
  TestHistoryWraps();
  TestConcurrentCollection();
}
