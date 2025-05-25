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

#include <libserver/util/Stream.hpp>

#include <boost/asio/streambuf.hpp>

#include <cassert>

namespace
{

struct Datum
{
  uint32_t val0 = 0;
  uint32_t val1 = 0;

  static void Write(
    const Datum& value,
    alicia::SinkStream& stream)
  {
    stream.Write(value.val0)
      .Write(value.val1);
  }

  static void Read(
    Datum& value,
    alicia::SourceStream& stream)
  {
    stream.Read(value.val0)
      .Read(value.val1);
  }
};

//! Perform test of magic encoding/decoding.
void TestBuffers()
{
  boost::asio::streambuf buf;
  auto mutableBuffer = buf.prepare(16);
  alicia::SinkStream sink(std::span(static_cast<std::byte*>(mutableBuffer.data()), mutableBuffer.size()));

  sink.Write(0xCAFE);
  sink.Write(0xBABE);

  Datum structToWrite{
    .val0 = 0xBAAD,
    .val1 = 0xF00D};
  sink.Write(structToWrite);

  assert(sink.GetCursor() == 4 * sizeof(uint32_t));
  buf.commit(sink.GetCursor());

  auto constBuffer = buf.data();
  alicia::SourceStream source(std::span(static_cast<const std::byte*>(constBuffer.data()), constBuffer.size()));

  uint32_t cafe{};
  uint32_t babe{};
  Datum structToRead{};

  source.Read(cafe)
    .Read(babe)
    .Read(structToRead);

  assert(cafe == 0xCAFE && babe == 0xBABE);
  assert(structToRead.val0 == structToWrite.val0);
  assert(structToRead.val1 == structToWrite.val1);
  assert(source.GetCursor() == 4 * sizeof(uint32_t));

  while (true)
  {
    auto mutableBuffer2 = buf.prepare(16);
    alicia::SinkStream sink2(std::span(static_cast<std::byte*>(mutableBuffer2.data()), mutableBuffer2.size()));

    sink2.Write(0x5757'5757);
    buf.commit(16);
    buf.consume(16);
    printf("abc");
  }
}

} // namespace

int main()
{
  TestBuffers();
}
