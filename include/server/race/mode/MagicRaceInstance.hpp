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

#ifndef MAGIC_RACEINSTANCE_HPP
#define MAGIC_RACEINSTANCE_HPP

#include "server/race/RaceInstance.hpp"

namespace server
{

class MagicRaceInstance final : public RaceInstance
{
public:
  explicit MagicRaceInstance(
    ServerInstance& serverInstance,
    CommandServer& commandServer);
  ~MagicRaceInstance() override;

private:
  void TickRacing() override;
  void TickGauge();

  std::chrono::steady_clock::time_point _nextMagicGaugeTickTimePoint{};

  static inline const std::chrono::milliseconds MagicGaugeTickInterval{250}; // TODO: is this the correct interval?

  // TODO: add these to configuration somewhere
  // Eyeballed these values from watching videos
  static const uint32_t NoItemHeldBoostAmount{2000};
  // TODO: does holding an item and with certain equipment give you magic? At a reduced rate?
  static const uint32_t ItemHeldWithEquipmentBoostAmount{1000};
};

}

#endif // MAGIC_RACEINSTANCE_HPP
