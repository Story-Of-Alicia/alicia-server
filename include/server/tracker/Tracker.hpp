//
// Created by rgnter on 28/09/2025.
//

#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <cstdint>
#include <array>

namespace server::tracker
{

//! Object identifier;
using Oid = uint16_t;
//! Invalid object identifier
constexpr Oid InvalidEntityOid = 0;

struct Vector3
{
  float x{};
  float y{};
  float z{};

  Vector3() {}
  Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
  Vector3(const std::array<float, 3>& position) : x(position[0]), y(position[1]), z(position[2]) {}
};

} // namespace server::tracker

#endif //TRACKER_HPP
