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

#include <libserver/data/DataStorage.hpp>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace
{

//! Checks the condition in every build configuration, unlike `assert`.
void Check(const bool condition, const char* description)
{
  if (condition)
    return;

  std::fprintf(stderr, "Check failed: %s\n", description);
  std::abort();
}

using Storage = server::DataStorage<uint32_t, uint32_t>;

//! A data source which fails a number of stores before it succeeds.
struct FailingDataSource
{
  uint32_t failuresLeft{0};
  uint32_t storeAttempts{0};
  uint32_t storedValue{0};

  Storage MakeStorage()
  {
    return Storage(
      [](const uint32_t&, uint32_t&)
      {
        return false;
      },
      [this](const uint32_t&, uint32_t& value)
      {
        ++storeAttempts;
        if (failuresLeft > 0)
        {
          --failuresLeft;
          return false;
        }
        storedValue = value;
        return true;
      },
      [](const uint32_t&)
      {
        return true;
      });
  }
};

void TestFailedStoreIsRetried()
{
  FailingDataSource dataSource{.failuresLeft = 1};
  auto storage = dataSource.MakeStorage();

  const auto record = storage.Create([]()
  {
    return std::make_pair(uint32_t{1}, uint32_t{42});
  });

  // The first store fails.
  storage.Tick();
  Check(dataSource.storeAttempts == 1, "the first store is attempted");
  Check(dataSource.storedValue == 0, "the first store fails");

  // The retry waits for the backoff.
  storage.Tick();
  Check(dataSource.storeAttempts == 1, "the retry waits for the backoff");

  // The retry after the backoff stores the latest value.
  record.Mutable([](uint32_t& value)
  {
    value = 43;
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));
  storage.Tick();
  Check(dataSource.storeAttempts == 2, "the store is retried after the backoff");
  Check(dataSource.storedValue == 43, "the retry stores the latest value");

  // Nothing is stored again once the store succeeded.
  storage.Tick();
  Check(dataSource.storeAttempts == 2, "a stored datum is not stored again");
}

void TestTerminateRetriesFailedStores()
{
  FailingDataSource dataSource{.failuresLeft = 2};
  auto storage = dataSource.MakeStorage();

  [[maybe_unused]] const auto record = storage.Create([]()
  {
    return std::make_pair(uint32_t{1}, uint32_t{42});
  });

  // The termination ignores the backoff and retries the failed store.
  storage.Terminate();
  Check(dataSource.storeAttempts == 3, "the termination retries the failed store");
  Check(dataSource.storedValue == 42, "the termination stores the datum");
}

void TestTerminateGivesUp()
{
  FailingDataSource dataSource{.failuresLeft = 100};
  auto storage = dataSource.MakeStorage();

  [[maybe_unused]] const auto record = storage.Create([]()
  {
    return std::make_pair(uint32_t{1}, uint32_t{42});
  });

  // The termination gives up after a bounded number of attempts.
  storage.Terminate();
  Check(dataSource.storeAttempts == 3, "the termination gives up after its attempts");
}

} // anon namespace

int main()
{
  TestFailedStoreIsRetried();
  TestTerminateRetriesFailedStores();
  TestTerminateGivesUp();
  return 0;
}
