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

#ifndef ALICIA_SERVER_RACEDIRECTOR_HPP
#define ALICIA_SERVER_RACEDIRECTOR_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/network/command/proto/CommonStructureDefinitions.hpp>
#include <libserver/util/Scheduler.hpp>

namespace server
{

class ServerInstance;
class RaceNetworkHandler;

class RaceDirector
{
public:
  explicit RaceDirector(ServerInstance& serverInstance);
  ~RaceDirector();

  RaceDirector(const RaceDirector&) = delete;
  RaceDirector& operator=(const RaceDirector&) = delete;

  RaceDirector(RaceDirector&&) = delete;
  RaceDirector& operator=(RaceDirector&&) = delete;

  void Initialize();
  void Terminate();
  void Tick();

  void DisconnectCharacter(data::Uid characterUid);
  void NotifySummonCharacter(
    data::Uid characterUid,
    bool force,
    const std::string& characterName,
    uint32_t roomUid,
    uint32_t ranchUid) noexcept;
  void NotifyRoomNameChanged(uint32_t roomUid) noexcept;
  void SendDailyQuestNotificationToCharacter(
    uint32_t characterUid,
    uint16_t questId,
    const protocol::ObjectiveProgress& objectiveProgress,
    uint32_t carrotsReward,
    protocol::QuestRewardType rewardType,
    uint32_t unk2,
    uint32_t mountExp);

  [[nodiscard]] RaceNetworkHandler& GetNetworkHandler();

private:
  //! A server instance reference.
  ServerInstance& _serverInstance;
  //! A network handler.
  RaceNetworkHandler* _networkHandler;
  //! A scheduler.
  Scheduler _scheduler;
};

}

#endif // ALICIA_SERVER_RACEDIRECTOR_HPP
