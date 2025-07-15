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

#include "libserver/network/command/CommandProtocol.hpp"

#include <cassert>

namespace
{

//! Perform test of magic encoding/decoding.
void TestMagic()
{
  const server::protocol::MessageMagic magic{
    .id = 7,
    .length = 29};

  // Test encoding of the magic.
  const auto encoded_magic = server::protocol::encode_message_magic(magic);
  assert(encoded_magic == 0x8D06CD01);

  // Test decoding of the magic.
  const auto decoded_magic = server::protocol::decode_message_magic(encoded_magic);
  assert(decoded_magic.id == magic.id);
  assert(decoded_magic.length == magic.length);
}

} // namespace

int main()
{
  TestMagic();
}
