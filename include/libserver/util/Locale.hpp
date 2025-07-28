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

#ifndef LOCALE_HPP
#define LOCALE_HPP

#include <string>

namespace server
{

namespace locale
{

//! Converts EUC-KR encoded string into a UTF-8 encoded string.
//! @param input Input string in the EUC-KR encoding.
//! @returns Output string encoded in UTF8 encoding.
//! @throws std::runtime_error If the conversion fails for any reason.
[[nodiscard]] std::string ToUtf8(const std::string& input);

//! Converts UTF-8 encoded string into a EUC-KR encoded string.
//! @param input Input string in the UTF8 encoding.
//! @returns Output string encoded in EUC-KR encoding.
//! @throws std::runtime_error If the conversion fails for any reason.
[[nodiscard]] std::string FromUtf8(const std::string& input);

} // namespace locale

} // namespace server

#endif //LOCALE_HPP
