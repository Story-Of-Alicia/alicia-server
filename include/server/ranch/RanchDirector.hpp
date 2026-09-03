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

#ifndef ALICIA_SERVER_RANCHDIRECTOR_HPP
#define ALICIA_SERVER_RANCHDIRECTOR_HPP

#include "server/Config.hpp"
#include "server/ranch/BreedingMarket.hpp"

#include <libserver/util/Scheduler.hpp>

namespace server
{

class ServerInstance;
class BreedingMarket;
class RanchNetworkHandler;

class RanchDirector final
{
public:
  explicit RanchDirector(ServerInstance& serverInstance);
  ~RanchDirector();

  RanchDirector(const RanchDirector&) = delete;
  RanchDirector& operator=(const RanchDirector&) = delete;

  RanchDirector(RanchDirector&&) = delete;
  RanchDirector& operator=(RanchDirector&&) = delete;

  void Initialize();
  void Terminate();
  void Tick();

  void DisconnectCharacter(data::Uid characterUid);

  void SendGuildInviteAccepted(
    data::Uid guildUid,
    data::Uid characterUid,
    const std::string& newMemberCharacterName);
  void SendGuildInviteDeclined(
    data::Uid characterUid,
    data::Uid inviterCharacterUid,
    const std::string& inviterCharacterName,
    data::Uid guildUid);
  void SendCharacterSummon(
    data::Uid characterUid,
    bool force,
    const std::string& characterName,
    data::Uid roomUid,
    data::Uid ranchUid);

  void BroadcastSetIntroductionNotify(
    data::Uid characterUid,
    const std::string& introduction);

  //! Get ranch network handler.
  //! @return Ranch network handler.
  [[nodiscard]] RanchNetworkHandler& GetNetworkHandler() const;

  //! Get ranch scheduler.
  //! @return Ranch scheduler.
  [[nodiscard]] Scheduler& GetScheduler() noexcept;

  //! Get ranch config.
  //! @return Ranch config.
  [[nodiscard]] Config::Ranch& GetConfig() noexcept;

  //! Get breeding market.
  //! @return Breeding market.
  [[nodiscard]] BreedingMarket& GetBreedingMarket() noexcept;

private:
  struct CharacterInstance
  {
    //! Collection of foals which are to mature.
    std::vector<data::Uid> foals;
    //! Collection of items which are to expire.
    std::vector<data::Uid> items;
  };

  //! Promotes matured foals for every character currently standing on their
  //! own ranch, announcing the grow-up to that ranch. Only the tracked
  //! maturing foals are inspected.
  void RunFoalMaturityCheck();

  //! Queues the next foal maturity check on the scheduler, re-scheduling
  //! itself so the sweep runs on a fixed interval.
  void ScheduleFoalMaturityCheck() noexcept;

  //! A server instance reference.
  ServerInstance& _serverInstance;
  //! A network handler.
  RanchNetworkHandler* _networkHandler = nullptr;
  //! A scheduler.
  Scheduler _scheduler;
  //! Breeding market.
  BreedingMarket _breedingMarket;



  std::unordered_map<data::Uid, CharacterInstance> _characterInstances;
};

} // namespace server

#endif // ALICIA_SERVER_RANCHDIRECTOR_HPP
