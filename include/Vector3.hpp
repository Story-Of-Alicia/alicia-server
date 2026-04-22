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

#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <cmath>

struct Vector3
{
public:
  float X{};
  float Y{};
  float Z{};

  Vector3()
    : X(0.0f), Y(0.0f), Z(0.0f) {}

  Vector3(float x, float y, float z)
    : X(x), Y(y), Z(z) {}

  /**
   * @brief Computes the Euclidean length (magnitude) of this vector.
   *
   * Uses the standard 3D distance formula: sqrt(x² + y² + z²).
   *
   * @return Length of the vector from the origin (0,0,0).
   */
  float Magnitude() const noexcept { return std::sqrt((X * X) + (Y * Y) + (Z * Z)); }

  /**
   * @brief Computes the distance between two 3D vectors.
   *
   * Equivalent to the magnitude of the difference vector (v1 - v2).
   *
   * @param v1 First vector.
   * @param v2 Second vector.
   * @return Euclidean distance between v1 and v2.
   */
  static float Magnitude(const Vector3& v1, const Vector3& v2) noexcept { return (v1 - v2).Magnitude(); }

  Vector3 operator+(const Vector3& v) const noexcept { return { X + v.X, Y + v.Y, Z + v.Z }; }
  Vector3 operator-(const Vector3& v) const noexcept { return { X - v.X, Y - v.Y, Z - v.Z }; }
  Vector3 operator*(const Vector3& v) const noexcept { return { X * v.X, Y * v.Y, Z * v.Z }; }
  Vector3 operator/(const Vector3& v) const noexcept { return { X / v.X, Y / v.Y, Z / v.Z }; }

  Vector3& operator+=(const Vector3& other)
  {
    X += other.X;
    Y += other.Y;
    Z += other.Z;
    return *this;
  }

  Vector3& operator-=(const Vector3& other)
  {
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;
    return *this;
  }

  Vector3& operator*=(const Vector3& other)
  {
    X *= other.X;
    Y *= other.Y;
    Z *= other.Z;
    return *this;
  }

  Vector3& operator/=(const Vector3& other)
  {
    X /= other.X;
    Y /= other.Y;
    Z /= other.Z;
    return *this;
  }
};

#endif // VECTOR3_HPP
