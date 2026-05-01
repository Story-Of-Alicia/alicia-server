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

#include "server/race/P2dIdPool.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <thread>
#include <vector>

namespace
{

using server::race::P2dId;
using server::race::P2dIdPool;

void TestSequentialAcquire()
{
  // Start with an empty pool.
  P2dIdPool pool;

  // Acquire a few fresh IDs.
  const auto first = pool.Acquire();
  const auto second = pool.Acquire();
  const auto third = pool.Acquire();

  // Each acquire should succeed.
  assert(first.has_value());
  assert(second.has_value());
  assert(third.has_value());

  // Fresh IDs should be assigned in sequence.
  assert(first.value() == 0);
  assert(second.value() == 1);
  assert(third.value() == 2);
}

void TestReleasedIdsAreReusedBeforeNewIds()
{
  // Start with an empty pool.
  P2dIdPool pool;

  // Acquire several fresh IDs.
  const auto first = pool.Acquire();
  const auto second = pool.Acquire();
  const auto third = pool.Acquire();

  // Make sure the initial IDs were allocated.
  assert(first.has_value());
  assert(second.has_value());
  assert(third.has_value());

  // Release two IDs back into the pool.
  pool.Release(second.value());
  pool.Release(first.value());

  // Reacquire IDs after recycling.
  const auto reusedFirst = pool.Acquire();
  const auto reusedSecond = pool.Acquire();
  const auto next = pool.Acquire();

  // All reacquire operations should succeed.
  assert(reusedFirst.has_value());
  assert(reusedSecond.has_value());
  assert(next.has_value());

  // Released IDs should be reused before issuing a new ID.
  assert(reusedFirst.value() == first.value());
  assert(reusedSecond.value() == second.value());
  assert(next.value() == 3);
}

void TestExhaustionAndReuseAfterExhaustion()
{
  // Start with an empty pool.
  P2dIdPool pool;

  // Acquire every possible P2dId value.
  for (uint32_t expected = 0; expected <= std::numeric_limits<P2dId>::max(); ++expected)
  {
    const auto id = pool.Acquire();
    assert(id.has_value());
    assert(id.value() == expected);
  }

  // The pool should report exhaustion after all IDs are issued.
  assert(not pool.Acquire().has_value());

  // Release one ID after exhaustion.
  constexpr P2dId ReleasedId = 42;
  pool.Release(ReleasedId);

  // The released ID should be reusable, then exhaustion should return.
  const auto reused = pool.Acquire();
  assert(reused.has_value());
  assert(reused.value() == ReleasedId);
  assert(not pool.Acquire().has_value());
}

void TestConcurrentAcquire()
{
  constexpr size_t ThreadCount = 8;
  constexpr size_t IdsPerThread = 512;
  constexpr size_t TotalIds = ThreadCount * IdsPerThread;

  // Share one pool across several threads.
  P2dIdPool pool;
  std::array<std::vector<P2dId>, ThreadCount> acquiredIds;
  std::array<std::thread, ThreadCount> threads;

  // Have each thread acquire a fixed number of IDs.
  for (size_t threadIdx = 0; threadIdx < ThreadCount; ++threadIdx)
  {
    threads[threadIdx] = std::thread(
      [&pool, &acquiredIds, threadIdx]()
      {
        auto& threadIds = acquiredIds[threadIdx];
        threadIds.reserve(IdsPerThread);

        for (size_t idIdx = 0; idIdx < IdsPerThread; ++idIdx)
        {
          const auto id = pool.Acquire();
          assert(id.has_value());
          threadIds.push_back(id.value());
        }
      });
  }

  // Wait for all acquisition work to finish.
  for (auto& thread : threads)
  {
    thread.join();
  }

  // Track which IDs were returned across all threads.
  std::vector<bool> seenIds(TotalIds, false);
  size_t acquiredCount = 0;

  // Verify every returned ID is unique and in the expected range.
  for (const auto& threadIds : acquiredIds)
  {
    for (const P2dId id : threadIds)
    {
      assert(id < TotalIds);
      assert(not seenIds[id]);
      seenIds[id] = true;
      ++acquiredCount;
    }
  }

  // Every ID from the expected range should have been acquired once.
  assert(acquiredCount == TotalIds);
  assert(std::ranges::all_of(seenIds, [](bool seen) { return seen; }));
}

} // namespace

int main()
{
  // Run all P2dId pool checks.
  TestSequentialAcquire();
  TestReleasedIdsAreReusedBeforeNewIds();
  TestExhaustionAndReuseAfterExhaustion();
  TestConcurrentAcquire();
}
