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

#ifndef DEFERRED_HPP
#define DEFERRED_HPP

#include <functional>

namespace server
{

//! Performs deferred call on destruction.
struct Deferred final
{
  //! Function.
  using Fnc = std::function<void()>;

  //! Construct deferred call that invokes
  //! the provided function on destruction of this object.
  explicit Deferred(Fnc func) noexcept
    : _func(std::move(func)) {}

  //! Deleted copy constructor.
  Deferred(const Deferred&) noexcept = delete;
  //! Deleted move constructor.
  Deferred(Deferred&&) noexcept = delete;

  //! Destructor.
  ~Deferred() { _func(); }

private:
  //! A function to invoke on destruction.
  Fnc _func;
};

} // namespace server

#endif // DEFERRED_HPP
