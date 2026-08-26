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

#ifndef RANCHMANAGEMENTSYSTEM_HPP
#define RANCHMANAGEMENTSYSTEM_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <cstdint>
#include <optional>

namespace server
{

class ServerInstance;

//! System responsible for the recurring bonus a ranch pays its owner. Every
//! twentieth race finished pays out the ranch experience and carrots of the
//! housing standing on the ranch, increased by any completed housing set.
class RanchManagementSystem
{
public:
  //! How many races are needed for the ranch bonus to pay out.
  static constexpr uint32_t RacesPerPayout = 20;

  //! Temporary multiplier applied to what a single payout awards.
  //! Set back to 1 to pay the plain housing values.
  static constexpr uint32_t PayoutMultiplier = 10;

  //! What a single payout awarded.
  struct Payout
  {
    //! Ranch experience awarded, already added to the character.
    uint32_t ranchExperience{};
    //! Carrots awarded, already added to the character.
    uint32_t carrots{};
  };

  explicit RanchManagementSystem(ServerInstance& serverInstance);

  RanchManagementSystem(const RanchManagementSystem&) = delete;
  RanchManagementSystem& operator=(RanchManagementSystem&) = delete;
  RanchManagementSystem(RanchManagementSystem&&) = delete;
  RanchManagementSystem& operator=(RanchManagementSystem&&) = delete;

  //! Counts a finished race towards the character's ranch bonus
  //! Payout is awarded on every twentieth race
  //! @param characterUid Character that finished a race.
  //! @returns What the payout awarded, or nullopt when the race did not count
  //!          or did not complete a set of twenty.
  [[nodiscard]] std::optional<Payout> RecordRaceCompletion(data::Uid characterUid);

  //! Totals what one payout is currently worth for a character, without
  //! awarding anything. Expired housing is ignored.
  //! @param character Character to total the bonus for.
  //! @returns The ranch experience and carrots the character's housing pays,
  //!          both zero when nothing standing pays a recurring bonus.
  [[nodiscard]] Payout CalculatePayout(const data::Character& character) const;

private:
  ServerInstance& _serverInstance;
};

} // namespace server

#endif // RANCHMANAGEMENTSYSTEM_HPP
