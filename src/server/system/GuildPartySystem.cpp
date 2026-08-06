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

#include "server/system/GuildPartySystem.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace server
{

GuildParty::GuildParty(uint32_t uid)
  : _uid(uid)
{
}

bool GuildParty::AddMember(data::Uid characterUid)
{
  if (_members.size() >= Details::MaxMemberCount)
    return false;

  const auto iter = std::find_if(
    _members.cbegin(),
    _members.cend(),
    [characterUid](const Member& m)
    {
      return m.characterUid == characterUid;
    });

  if (iter != _members.cend())
    return false;

  _members.emplace_back(Member{
    .characterUid = characterUid});

  if (_members.size() == 1)
  {
    _details.leaderUid = characterUid;
    _details.ranchUid = characterUid;
  }

  return true;
}

void GuildParty::RemoveMember(data::Uid characterUid)
{
  const auto iter = std::find_if(
    _members.cbegin(),
    _members.cend(),
    [characterUid](const Member& m)
    {
      return m.characterUid == characterUid;
    });

  if (iter == _members.cend())
    return;

  _members.erase(iter);
  if (characterUid == _details.leaderUid)
    MigrateLeader();
}

void GuildParty::MigrateLeader()
{
  if (_members.empty())
  {
    _details.leaderUid = data::InvalidUid;
    _details.ranchUid = data::InvalidUid;
    return;
  }

  // Next in queue of joined players becomes leader
  const auto& newLeader = _members.front();
  _details.leaderUid = newLeader.characterUid;
  _details.ranchUid = newLeader.characterUid;
}

bool GuildParty::IsFull() const
{
  return _members.size() >= Details::MaxMemberCount;
}

uint32_t GuildParty::GetUid() const
{
  return _uid;
}

size_t GuildParty::GetMemberCount() const
{
  return _members.size();
}

const GuildParty::Details& GuildParty::GetDetails() const
{
  return _details;
}

GuildParty::Details& GuildParty::GetDetails()
{
  return _details;
}

GuildParty::Snapshot GuildParty::GetSnapshot() const
{
  return Snapshot{
    .uid = _uid,
    .details = _details,
    .memberCount = _members.size()};
}

const std::vector<GuildParty::Member>& GuildParty::GetMembers() const
{
  return _members;
}

void GuildPartySystem::CreateParty(const std::function<void(GuildParty&)>& consumer)
{
  std::unique_lock partiesLock(_partiesLock);

  const auto partyUid = ++_sequencedId;
  const auto [iter, inserted] = _parties.try_emplace(
    partyUid,
    std::move(GuildParty(partyUid)));
  assert(inserted);

  auto& [party, partyMutex] = iter->second;
  partiesLock.unlock();

  std::scoped_lock lock(partyMutex);
  consumer(party);
}

void GuildPartySystem::GetParty(uint32_t uid, const std::function<void(GuildParty&)>& consumer)
{
  std::unique_lock partiesLock(_partiesLock);

  const auto iter = _parties.find(uid);
  if (iter == _parties.cend())
    throw std::runtime_error("Guild party does not exist");

  auto& [party, partyMutex] = iter->second;
  partiesLock.unlock();

  std::scoped_lock lock(partyMutex);
  consumer(party);
}

bool GuildPartySystem::PartyExists(uint32_t uid)
{
  std::scoped_lock lock(_partiesLock);
  return _parties.contains(uid);
}

std::vector<GuildParty::Snapshot> GuildPartySystem::GetPartiesSnapshot()
{
  std::scoped_lock partiesLock(_partiesLock);

  std::vector<GuildParty::Snapshot> parties;
  parties.reserve(_parties.size());
  for (auto& entry : _parties)
  {
    std::scoped_lock partyLock(entry.second.mutex);
    parties.emplace_back(entry.second.party.GetSnapshot());
  }

  return parties;
}

void GuildPartySystem::DeleteParty(uint32_t uid)
{
  std::scoped_lock lock(_partiesLock);

  const auto iter = _parties.find(uid);
  if (iter == _parties.cend())
    throw std::runtime_error("Guild party does not exist");

  _parties.erase(iter);
}

} // namespace server
