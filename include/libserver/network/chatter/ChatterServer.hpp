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

#ifndef CHATTER_SERVER_HPP
#define CHATTER_SERVER_HPP

#include "libserver/network/Server.hpp"

namespace alicia
{

class ChatterServer
{
public:
  ChatterServer();
  ~ChatterServer();

  void Host();

  Server _server;
  std::thread _serverThread;
};

} // namespace alicia

#endif // CHATTER_SERVER_HPP
