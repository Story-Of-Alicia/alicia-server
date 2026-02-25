/**
 * server Server - dedicated server software
 * Copyright (C) 2024 Story Of server
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
    server::SinkStream& stream)
  {
    stream.Write(value.val0)
      .Write(value.val1);
  }

  static void Read(
    Datum& value,
    server::SourceStream& stream)
  {
    stream.Read(value.val0)
      .Read(value.val1);
  }
};

//! Perform test of magic encoding/decoding.
void TestStreams()
{
  boost::asio::streambuf buf;
  auto mutableBuffer = buf.prepare(17);
  server::SinkStream sink(std::span(static_cast<std::byte*>(mutableBuffer.data()), mutableBuffer.size()));

  const bool value = true;
  sink.Write(value);
  sink.Write(0xCAFE);
  sink.Write(0xBABE);

  Datum structToWrite{
    .val0 = 0xBAAD,
    .val1 = 0xF00D};
  sink.Write(structToWrite);

  assert(sink.GetCursor() == 4 * sizeof(uint32_t) + 1);
  buf.commit(sink.GetCursor());

  auto constBuffer = buf.data();
  server::SourceStream source(std::span(static_cast<const std::byte*>(constBuffer.data()), constBuffer.size()));

  bool status{false};
  uint32_t cafe{};
  uint32_t babe{};
  Datum structToRead{};

  source.Read(status)
    .Read(cafe)
    .Read(babe)
    .Read(structToRead);

  assert(status);
  assert(cafe == 0xCAFE && babe == 0xBABE);
  assert(structToRead.val0 == structToWrite.val0);
  assert(structToRead.val1 == structToWrite.val1);
  assert(source.GetCursor() == 4 * sizeof(uint32_t) + 1);
}

} // namespace

int main()
{
  TestStreams();
}
