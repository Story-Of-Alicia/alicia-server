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

#include "server/ranch/RanchDirector.hpp"

#include "server/ranch/RanchNetworkHandler.hpp"
#include "server/ServerInstance.hpp"

namespace server
{

namespace
{

//! How often the foal maturity sweep runs while players are on their ranch.
constexpr auto FoalMaturityCheckInterval = std::chrono::seconds(60);

} // namespace

RanchDirector::RanchDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _networkHandler(new RanchNetworkHandler(_serverInstance))
  , _breedingMarket(_serverInstance)
{
}

RanchDirector::~RanchDirector()
{
  delete _networkHandler;
}

void RanchDirector::Initialize()
{
  _breedingMarket.Initialize();

  ScheduleFoalMatureCheck();

  _networkHandler->Initialize();
  _networkHandler->GetCharacterConnectEvent().Subscribe(
    [this](const data::Uid characterUid)
    {
      auto& characterInstance = _characterInstances[characterUid];

      const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
        characterUid);
      if (not characterRecord)
        return;

      characterRecord.Immutable([this, &characterInstance](
        const data::Character& character)
      {
        characterInstance.foals = _serverInstance.GetHorseSystem().CollectCharacterFoals(
          character);
        //characterInstance.items = _serverInstance.GetItemSystem().CollectPerishableItems();
      });

    });

  _networkHandler->GetCharacterDisconnectEvent().Subscribe(
    [this](const data::Uid characterUid)
    {
      _characterInstances.erase(characterUid);
    });
}

void RanchDirector::Terminate()
{
  _breedingMarket.Terminate();

  _networkHandler->Terminate();
}

void RanchDirector::Tick()
{
  _breedingMarket.Tick();

  _networkHandler->Tick();
}

void RanchDirector::DisconnectCharacter(
  const data::Uid characterUid)
{
  GetNetworkHandler().DisconnectCharacter(characterUid);
}

void RanchDirector::SendGuildInviteAccepted(
  const data::Uid guildUid,
  const data::Uid characterUid,
  const std::string& newMemberCharacterName)
{
  GetNetworkHandler().SendGuildInviteAccepted(
    guildUid,
    characterUid,
    newMemberCharacterName);
}

void RanchDirector::SendGuildInviteDeclined(
  const data::Uid characterUid,
  const data::Uid inviterCharacterUid,
  const std::string& inviterCharacterName,
  const data::Uid guildUid)
{
  GetNetworkHandler().SendGuildInviteDeclined(
    characterUid,
    inviterCharacterUid,
    inviterCharacterName,
    guildUid);
}

void RanchDirector::SendCharacterSummon(
  const data::Uid characterUid,
  const bool force,
  const std::string& characterName,
  const data::Uid roomUid,
  const data::Uid ranchUid)
{
  GetNetworkHandler().SendCharacterSummon(
    characterUid,
    force,
    characterName,
    roomUid,
    ranchUid);
}

void RanchDirector::BroadcastSetIntroductionNotify(
  const data::Uid characterUid,
  const std::string& introduction)
{
  GetNetworkHandler().BroadcastSetIntroductionNotify(
    characterUid,
    introduction);
}

RanchNetworkHandler& RanchDirector::GetNetworkHandler() const
{
  if (_networkHandler == nullptr)
    throw std::runtime_error("Ranch network handler not available");
  return *_networkHandler;
}

Scheduler& RanchDirector::GetScheduler() noexcept
{
  return _scheduler;
}

Config::Ranch& RanchDirector::GetConfig() noexcept
{
  return _serverInstance.GetSettings().ranch;
}

BreedingMarket& RanchDirector::GetBreedingMarket() noexcept
{
  return _breedingMarket;
}

void RanchDirector::ScheduleFoalMatureCheck() noexcept
{
  _scheduler.Queue(
    [this]()
    {
      RunFoalMaturityCheck();
      ScheduleFoalMatureCheck();
    },
    Scheduler::Clock::now() + FoalMaturityCheckInterval);
}

void RanchDirector::RunFoalMaturityCheck()
{
  const auto now = data::Clock::now();

  for (const data::Uid characterUid : _characterInstances)
  {

  }

  for (auto& [clientId, clientContext] : _clients)
  {
    if (not clientContext.isAuthenticated
      || clientContext.maturingFoals.empty())
    {
      continue;
    }

    for (auto foalIter = clientContext.maturingFoals.begin();
      foalIter != clientContext.maturingFoals.end();)
    {
      if (now < foalIter->second)
      {
        ++foalIter;
        continue;
      }

      const auto horseUid = foalIter->first;
      const auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(horseUid);

      bool isFoal = false;
      if (horseRecord)
      {
        horseRecord->Immutable([&isFoal](const data::Horse& horse)
          {
            isFoal = horse.type() == data::Horse::Type::Foal;
          });
      }

      if (isFoal)
      {
        horseRecord->Mutable([](data::Horse& horse)
          {
            horse.type() = data::Horse::Type::Adult;
          });

        AnnounceFoalGrewUp(
          clientId,
          clientContext.characterUid,
          horseUid);
      }

      foalIter = clientContext.maturingFoals.erase(foalIter);
    }
  }
}

} // namespace server
