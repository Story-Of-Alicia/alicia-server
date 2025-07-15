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

#include <libserver/util/Locale.hpp>

#include <cassert>
#include <locale>

namespace
{

void TestLocale()
{
  const std::string eucSource = "\xb1\xb8";
  const std::string utfSource = "\xea\xb5\xac";

  const std::string utfOutput = server::locale::ToUtf8(eucSource);
  assert(utfOutput == utfSource);
  const std::string eucOutput = server::locale::FromUtf8(utfSource);
  assert(eucOutput == eucSource);
}

} // namespace

int main()
{
  TestLocale();
}
