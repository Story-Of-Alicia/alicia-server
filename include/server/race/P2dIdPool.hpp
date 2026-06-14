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

#ifndef P2DIDPOOL_HPP
#define P2DIDPOOL_HPP

#include <cstdint>
#include <mutex>
#include <vector>
#include <optional>
#include <limits>

namespace server::race
{

using P2dId = uint16_t;

class P2dIdPool
{
public:
  // Get a P2DID ticket
  std::optional<P2dId> Acquire()
  {
    std::scoped_lock lock(_mutex);

    // 1. Check if we have any recycled IDs first
    if (not _free_list.empty())
    {
      const P2dId id = _free_list.back();
      _free_list.pop_back();
      return id;
    }

    // 2. Check if we have hit the numerical limit of the pool
    if (_next_new_id > std::numeric_limits<P2dId>::max())
      return std::nullopt;

    // 3. Take the next one from the sequence
    return static_cast<P2dId>(_next_new_id++);
  }

  // Return a P2DID ticket for others to use
  void Release(P2dId id)
  {
    std::scoped_lock lock(_mutex);
    _free_list.push_back(id);
  }

private:
  //! A mutex to control concurrency for this pool's operations.
  std::mutex _mutex;
  //! An incremental tracker for tracking the next new P2dId.
  //! This is a uint32_t to support checking whether the incremented
  //! P2dId has overflown uint16_t max.
  uint32_t _next_new_id = 0;
  //! A list of freed P2dIds to reuse.
  std::vector<P2dId> _free_list;
};
    
} // namespace server::race

#endif // P2DIDPOOL_HPP
