//
// Created by rgnter on 25/05/2025.
//

#include "libserver/registry/RoomRegistry.hpp"

#include <stdexcept>
#include <cassert>

namespace server
{

Room& RoomRegistry::CreateRoom()
{
  const auto [it, inserted] = _rooms.try_emplace(++_sequencedId);
  assert(inserted);

  it->second.uid = _sequencedId;

  return it->second;
}

Room& RoomRegistry::GetRoom(uint32_t uid)
{
  const auto it = _rooms.find(uid);
  if (it == _rooms.end())
    throw std::runtime_error("room does not exist");
  return it->second;
}

} // namespace server
