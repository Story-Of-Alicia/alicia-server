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

#ifndef GAMEEVENT_HPP
#define GAMEEVENT_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/event/Event.hpp>
#include <libserver/registry/QuestRegistry.hpp>
#include <libserver/registry/RegistryDefinitions.hpp>

namespace server
{

struct GameEvent
{
  enum class Origin
  {
    Ranch,
    Race,
  };

  registry::UserAchvEvent userAchvEvent{registry::UserAchvEvent::None};
  registry::Function function{registry::Function::Unknown};

  Origin origin{Origin::Ranch};
  //! Character the event happened to/for.
  data::Uid characterUid{data::InvalidUid};
  //! Game mode the event occurred in, if applicable.
  registry::GameModeFlag gameMode{registry::GameModeFlag::None};
  //! Optional scalar payload (map ID, distance. etc...).
  uint32_t value{};
};

//! The event bus game systems fire domain events into and subscribe to.
using GameEventBus = Event<const GameEvent&>;

} // namespace server

#endif // GAMEEVENT_HPP
