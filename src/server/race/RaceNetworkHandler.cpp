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

#include "server/race/MagicSystem.hpp"
#include "server/race/RaceNetworkHandler.hpp"
#include "server/system/PotentialSystem.hpp"

#include "server/ServerInstance.hpp"

#include <libserver/data/helper/ProtocolHelper.hpp>
#include <libserver/util/Util.hpp>

#include <boost/container_hash/hash.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include <bitset>
#include <ranges>

namespace server
{

RaceNetworkHandler::RaceNetworkHandler(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
{
  _commandServer.RegisterCommandHandler<protocol::AcCmdCREnterRoom>(
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRoom(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeRoomOptions>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeRoomOptions(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeTeam>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeTeam(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRLeaveRoom>(
    [this](ClientId clientId, const auto&)
    {
      HandleLeaveRoom(clientId);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStartRace>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStartRace(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceTimer>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceTimer(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRLoadingComplete>(
    [this](ClientId clientId, const auto& message)
    {
      HandleLoadingComplete(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRReadyRace>(
    [this](ClientId clientId, const auto& message)
    {
      HandleReadyRace(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceFinal>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUserRaceFinal(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRaceResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRP2PResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleP2PRaceResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceP2PResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleP2PUserRaceResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRAwardStart>(
    [this](ClientId clientId, const auto& message)
    {
      HandleAwardStart(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRAwardEnd>(
    [this](ClientId clientId, const auto& message)
    {
      HandleAwardEnd(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStarPointGet>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStarPointGet(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRequestSpur>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestSpur(clientId, message);
      HandleTeamGauge(clientId);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRHurdleClearResult>(
    [this](ClientId clientId, const auto& message)
    {
      HandleHurdleClearResult(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStartingRate>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStartingRate(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceUpdatePos>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRaceUserPos(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChat>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChat(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRelayCommand>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRelayCommand(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRelay>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRelay(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceActivateInteractiveEvent>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUserRaceActivateInteractiveEvent(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceActivateEvent>(
    [this](ClientId clientId, const auto& message)
     {
       HandleUserRaceActivateEvent(clientId, message);
     });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceDeactivateEvent>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUserRaceDeactivateEvent(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRequestMagicItem>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestMagicItem(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRUseMagicItem>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUseMagicItem(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdUserRaceItemGet>(
    [this](ClientId clientId, const auto& message)
    {
      HandleUserRaceItemGet(clientId, message);
    });

  // Magic Targeting Commands for Bolt System
  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStartMagicTarget>(
    [this](ClientId clientId, const auto& message)
    {
      HandleStartMagicTarget(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeMagicTarget>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeMagicTarget(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRActivateSkillEffect>(
    [this](ClientId clientId, const auto& message)
    {
      HandleActivateSkillEffect(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeSkillCardPresetID>(
    [this](ClientId clientId, const auto& message)
    {
      HandleChangeSkillCardPresetId(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCROpCmd>(
    [this](ClientId clientId, const auto& message)
    {
      HandleOpCmd(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRInviteUser>(
    [this](ClientId clientId, const auto& message)
    {
      HandleInviteUser(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRequestUser>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestUser(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRKick>(
    [this](ClientId clientId, const auto& message)
    {
      HandleKickUser(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRTriggerizeAct>(
    [this](ClientId clientId, const auto& message)
    {
      HandleTriggerizeAct(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRGameCreateClientItem>(
    [this](ClientId clientId, const auto& message)
    {
      HandleGameCreateClientItem(clientId, message);
    });
}

void RaceNetworkHandler::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _commandServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void RaceNetworkHandler::Terminate()
{
  _commandServer.EndHost();
}

void RaceNetworkHandler::Tick()
{
  try
  {
    _scheduler.Tick();
  }
  catch (const std::exception& x)
  {
    spdlog::error("Exception ticking a race scheduler: {}", x.what());
  }

  std::scoped_lock lock(_raceInstancesMutex);
  for (auto& raceInstance : _raceInstances | std::views::values)
  {
    try
    {
      raceInstance.Tick();
    }
    catch (const std::exception& x)
    {
      spdlog::error("Exception ticking a race scheduler: {}", x.what());
    }
  }
}

void RaceNetworkHandler::NotifySummonCharacter(
    data::Uid characterUid,
    bool force,
    std::string characterName,
    uint32_t roomUid,
    uint32_t ranchUid) noexcept
{
  protocol::AcCmdRCRequestUser notify{};
  notify.force = force;
  notify.characterName = characterName;
  notify.roomUid = roomUid;
  notify.ranchUid = ranchUid;

  try
  {
     const auto targetClientId = GetClientIdByCharacterUid(characterUid);
     _commandServer.QueueCommand<protocol::AcCmdRCRequestUser>(targetClientId, [notify](){ return notify; });
  }
  catch(const std::exception&)
  {
    // Dont care if the client is not found, we just won't send the notification
  }
}

void RaceNetworkHandler::NotifyRoomNameChanged(
  const uint32_t roomUid) noexcept
{
  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [this](const Room& room)
    {
      const protocol::AcCmdCRChangeRoomOptionsNotify notify{
        .optionsBitfield = protocol::RoomOptionType::Name,
        .name = room.GetRoomSnapshot().details.name};

      for (const auto& player : room.GetPlayers() | std::views::values)
      {
        try
        {
          _commandServer.QueueCommand<protocol::AcCmdCRChangeRoomOptionsNotify>(
            player.GetClientId(),
            [notify]()
            {
              return notify;
            });
        }
        catch (const std::exception&)
        {
          // the player disconnected
        }
      }
    });
}

void RaceNetworkHandler::SendDailyQuestNotificationToCharacter(
  uint32_t characterUid,
  uint16_t questId,
  const protocol::ObjectiveProgress& objectiveProgress,
  uint32_t carrotsReward,
  protocol::QuestRewardType rewardType,
  uint32_t unk2,
  uint32_t mountExp)
{
  const protocol::AcCmdRCUpdateDailyQuestNotify updateNotify{
    .characterUid = characterUid,
    .questId = questId,
    .objectiveProgress = objectiveProgress,
    .carrotsReward = carrotsReward,
    .rewardType = rewardType,
    .unk2 = unk2,
    .mountExp = mountExp};

  try
  {
    const ClientId clientId = GetClientIdByCharacterUid(characterUid);
    _commandServer.QueueCommand<protocol::AcCmdRCUpdateDailyQuestNotify>(
      clientId,
      [updateNotify]()
      {
        return updateNotify;
      });
  }
  catch (const std::exception&)
  {
    // Ignore
  }
}

void RaceNetworkHandler::HandleClientConnected(ClientId clientId)
{
  _clients.try_emplace(clientId);

  spdlog::debug(
    "Client {} connected to the race server from {}",
    clientId,
    _commandServer.GetClientAddress(clientId).to_string());
}

void RaceNetworkHandler::HandleClientDisconnected(ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId, false);
  if (clientContext.isAuthenticated)
  {
    std::unique_lock lock(_raceInstancesMutex);
    const auto raceIter = _raceInstances.find(clientContext.roomUid);
    if (raceIter != _raceInstances.cend())
    {
      lock.unlock();
      HandleLeaveRoom(clientId);
    }
  }

  // If client had a P2dId, erase it from client map and release it from the pool
  if (_p2dIds.contains(clientId))
  {
    // Erase client P2dId and release it
    const race::P2dId p2dId = _p2dIds.at(clientId);
    _p2dIds.erase(clientId);
    _p2dIdPool.Release(p2dId);
  }

  spdlog::info("Client {} disconnected from the race server", clientId);
  _clients.erase(clientId);
}

void RaceNetworkHandler::DisconnectCharacter(data::Uid characterUid)
{
  try
  {
    _commandServer.DisconnectClient(GetClientIdByCharacterUid(characterUid));
  }
  catch (const std::exception&)
  {
    // We really don't care.
  }
}

size_t RaceNetworkHandler::GetRoomCount()
{
  std::scoped_lock lock(_raceInstancesMutex);
  return _raceInstances.size();
}

Config::Race& RaceNetworkHandler::GetConfig()
{
  return GetServerInstance().GetSettings().race;
}

ServerInstance& RaceNetworkHandler::GetServerInstance()
{
  return _serverInstance;
}

CommandServer& RaceNetworkHandler::GetCommandServer()
{
  return _commandServer;
}

uint16_t RaceNetworkHandler::GetOrCreateP2dId(ClientId clientId)
{
  const auto existingP2dIdIter = _p2dIds.find(clientId);
  if (existingP2dIdIter != _p2dIds.end())
    return existingP2dIdIter->second;

  const std::optional<race::P2dId> p2dId = _p2dIdPool.Acquire();
  if (not p2dId.has_value())
    throw std::runtime_error("P2dId pool has been exhausted.");

  _p2dIds.emplace(clientId, p2dId.value());
  return p2dId.value();
}

RaceNetworkHandler::ClientContext& RaceNetworkHandler::GetClientContext(ClientId clientId, bool requireAuthorized)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Race client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthorized && not clientContext.isAuthenticated)
    throw std::runtime_error("Race client is not authenticated");

  return clientContext;
}

std::optional<ClientId> RaceNetworkHandler::FindClientIdByCharacterUid(data::Uid characterUid)
{
  for (auto& [clientId, clientContext] : _clients)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientId;
  }

  return std::nullopt;
}

ClientId RaceNetworkHandler::GetClientIdByCharacterUid(data::Uid characterUid)
{
  const auto clientId = FindClientIdByCharacterUid(characterUid);
  if (not clientId)
    throw std::runtime_error("Character not associated with any client");

  return *clientId;
}

RaceNetworkHandler::ClientContext& RaceNetworkHandler::GetClientContextByCharacterUid(
  const data::Uid characterUid)
{
  for (auto& clientContext : _clients | std::views::values)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientContext;
  }

  throw std::runtime_error("Character not associated with any client");
}

RaceInstance& RaceNetworkHandler::GetRaceInstance(
  const ClientContext& clientContext,
  const bool checkRacer)
{
  // Check if the client has an invalid room UID
  if (clientContext.roomUid == data::InvalidUid)
    throw std::runtime_error(
      std::format("Tried to get race instance for character '{}' but room uid is invalid",
        clientContext.characterUid));

  // Sanity check if a race instance by that room UID exists
  if (not _raceInstances.contains(clientContext.roomUid))
    throw std::runtime_error(
      std::format("Tried to get race instance for character '{}' but room '{}' does not exist",
        clientContext.characterUid,
        clientContext.roomUid));

  auto& raceInstance = _raceInstances.at(clientContext.roomUid);

  // If not racing cqommand then we are done here
  // HurdleClearResult, HandleSpur etc.
  if (not checkRacer)
    return raceInstance;

  // Check if the character is a racer
  // Protects against characters waiting in the waiting room but emitting racing commands
  if (not raceInstance.GetTracker().IsRacer(clientContext.characterUid))
    throw std::runtime_error(
      std::format("Tried to get race instance '{}' but character '{}' is not a racer",
        clientContext.roomUid,
        clientContext.characterUid));

  return raceInstance;
}

void RaceNetworkHandler::HandleEnterRoom(
  ClientId clientId,
  const protocol::AcCmdCREnterRoom& command)
{
  auto& clientContext = _clients[clientId];

  size_t identityHash = std::hash<uint32_t>()(command.characterUid);
  boost::hash_combine(identityHash, command.roomUid);

  clientContext.isAuthenticated = _serverInstance.GetOtpSystem().AuthorizeCode(
    identityHash,
    command.oneTimePassword);

  const bool doesRoomExist = _serverInstance.GetRoomSystem().RoomExists(
    command.roomUid);

  // Determine the racer count and whether the room is full.
  bool isOvercrowded = false;
  if (clientContext.isAuthenticated)
  {
    _serverInstance.GetRoomSystem().GetRoom(
      command.roomUid,
      [&isOvercrowded, clientId, characterUid = command.characterUid](Room& room)
      {
        // If the player is not able to be added, the room is full.
        isOvercrowded = not room.AddPlayer(clientId, characterUid);
      });
  }

  // Cancel the enter room if the client is not authenticated,
  // the room does not exist or the room is full.
  if (not clientContext.isAuthenticated
    || not doesRoomExist
    || isOvercrowded)
  {
    const protocol::AcCmdCREnterRoomCancel response{};
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  // The client is authorized so we can trust the identifiers
  // that were provided.
  clientContext.characterUid = command.characterUid;
  clientContext.roomUid = command.roomUid;
  clientContext.userName = _serverInstance.GetLobbyDirector().GetUserByCharacterUid(
    clientContext.characterUid).userName;

  std::scoped_lock lock(_raceInstancesMutex);
  // Try to emplace the room instance.
  const auto& [raceInstanceIter, inserted] = _raceInstances.try_emplace(
    command.roomUid,
    *this,
    command.roomUid);

  auto& raceInstance = raceInstanceIter->second;

  // If the room instance was just created, set it up.
  if (inserted)
  {
    raceInstance.GetRoom([masterUid = command.characterUid](Room& room)
    {
      auto& roomDetails = room.GetRoomDetails();
      roomDetails.masterUid = masterUid;
    });
  }

  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [inserted, clientContext](const data::Character& character)
    {
      if (inserted)
        spdlog::info("Player {} ({}) has created [Room {}]",
          clientContext.userName,
          character.name(),
          clientContext.roomUid);
      else
        spdlog::info("Player {} ({}) has joined [Room {}]",
          clientContext.userName,
          character.name(),
          clientContext.roomUid);
    });

  // Todo: Roll the code for the connecting client.
  // Todo: The response contains the code, somewhere.
  _commandServer.SetCode(clientId, {});

  protocol::AcCmdCREnterRoomOK response{
    .isRoomWaiting = raceInstance.GetStage() == RaceInstance::Stage::Waiting,
    .uid = command.roomUid};

  // If race instance exists and race is not waiting then
  // set the elapsed time since loading started
  if (not inserted and raceInstance.GetStage() != RaceInstance::Stage::Waiting)
  {
    response.elapsedTime = static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - raceInstance.GetLoadingStartTimePoint()).count());
  }

  try
  {
    _serverInstance.GetRoomSystem().GetRoom(
      command.roomUid,
      [&response](Room& room)
      {
        const auto& roomDetails = room.GetRoomDetails();
        response.roomDescription = {
          .name = roomDetails.name,
          .maxPlayerCount = static_cast<uint8_t>(roomDetails.maxPlayerCount),
          .password = roomDetails.password,
          .gameModeMaps = static_cast<uint8_t>(roomDetails.gameMode),
          .gameMode = static_cast<protocol::GameMode>(roomDetails.gameMode),
          .mapBlockId = roomDetails.courseId,
          .teamMode = static_cast<protocol::TeamMode>(roomDetails.teamMode),
          .missionId = roomDetails.missionId,
          .unk6 = roomDetails.npcDifficulty,
          .skillBracket = roomDetails.skillBracket};
      });
  }
  catch (const std::exception&)
  {
    throw std::runtime_error("Client tried entering a deleted room");
  }

  protocol::Racer joiningRacer;

  // Collect the room players.
  std::vector<data::Uid> characterUids;
  data::Uid roomMasterUid{data::InvalidUid};
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&characterUids, &roomMasterUid](Room& room)
    {
      roomMasterUid = room.GetRoomDetails().masterUid;
      for (const auto& characterUid : room.GetPlayers() | std::views::keys)
      {
        characterUids.emplace_back(characterUid);
      }
    });

  // Build the room players.
  for (const auto& characterUid : characterUids)
  {
    auto& protocolRacer = response.racers.emplace_back();
    protocolRacer.isMaster = characterUid == roomMasterUid;

    bool isPlayerReady = false;
    Room::Player::Team team;

    _serverInstance.GetRoomSystem().GetRoom(
      clientContext.roomUid,
      [&isPlayerReady, &team, characterUid](Room& room)
      {
        const auto& player = room.GetPlayer(characterUid);
        isPlayerReady = player.IsReady();
        team = player.GetTeam();
      });

    // Fill data from the character record.
    const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
      characterUid);
    characterRecord.Immutable(
      [this, isPlayerReady, team, &protocolRacer](
        const data::Character& character)
      {
        const auto& settingsRecord = GetServerInstance().GetDataDirector().GetSettings(character.settingsUid());
        if (settingsRecord.IsAvailable())
        {
          settingsRecord.Immutable(
            [&protocolRacer, modelId = character.parts.modelId()](const data::Settings& settings)
            {
              if (not settings.hideAge())
              {
                // TODO: Add age here (find if it is even possible)
                // todo: model constants
                protocolRacer.gender =
                  modelId == 10 ? protocol::Gender::Boy :
                  modelId == 20 ? protocol::Gender::Girl :
                  throw std::runtime_error("Character gender not recognised by model ID");
              }
            });
        }
        else
        {
          spdlog::warn("Settings record for character {} was not found, skipping role/gender assignment...",
            character.uid());
        }

        protocolRacer.level = character.level();
        protocolRacer.uid = character.uid();
        protocolRacer.name = character.name();
        protocolRacer.role = static_cast<protocol::Racer::Role>(character.role());
        protocolRacer.isHidden = false;
        protocolRacer.isNPC = false;
        protocolRacer.isReady = isPlayerReady;

        switch (team)
        {
          case Room::Player::Team::Red:
            protocolRacer.teamColor = protocol::TeamColor::Red;
            break;
          case Room::Player::Team::Blue:
            protocolRacer.teamColor = protocol::TeamColor::Blue;
            break;
          default:
            protocolRacer.teamColor = protocol::TeamColor::None;
            break;
        }

        protocolRacer.avatar = protocol::Avatar{};

        protocol::BuildProtocolCharacter(
          protocolRacer.avatar->character, character);

        // Build the character equipment.
        protocol::BuildProtocolItems(
          protocolRacer.avatar->equipment,
          *_serverInstance.GetDataDirector().GetItemCache().Get(
            character.characterEquipment()));

        const auto mountRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(
          character.mountUid());
        mountRecord->Immutable(
          [&protocolRacer](const data::Horse& mount)
          {
            protocol::BuildProtocolHorse(protocolRacer.avatar->mount, mount);
          });

        if (character.guildUid() != data::InvalidUid)
        {
          GetServerInstance().GetDataDirector().GetGuild(character.guildUid()).Immutable(
            [&protocolRacer, characterUid = character.uid()](const data::Guild& guild)
            {
              protocol::BuildProtocolGuild(protocolRacer.guild, guild);

              if (guild.owner() == characterUid)
              {
                protocolRacer.guild.guildRole = protocol::GuildRole::Owner;
              }
              else if (std::ranges::contains(guild.officers(), characterUid))
              {
                protocolRacer.guild.guildRole = protocol::GuildRole::Officer;
              }
              else
              {
                protocolRacer.guild.guildRole = protocol::GuildRole::Member;
              }
            });
        }

        if (character.petUid() != data::InvalidUid)
        {
          const auto& petRecord = GetServerInstance().GetDataDirector().GetPet(character.petUid());
          if (petRecord.IsAvailable())
          {
            petRecord.Immutable(
              [&protocolRacer](const data::Pet& pet)
              {
                protocol::BuildProtocolPet(protocolRacer.pet, pet);
              });
          }
          else
          {
            spdlog::warn("Character {} tried to load pet {} but it is not available.",
              character.uid(),
              character.petUid());
          }
        }
      });

    if (characterUid == clientContext.characterUid)
    {
      joiningRacer = protocolRacer;
    }
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  const protocol::AcCmdCREnterRoomNotify notify{
    .racer = joiningRacer,
    .averageTimeRecord = clientContext.characterUid};

  // Player should be added to the room at this point,
  // broadcast to room except joining player
  this->BroadcastExceptCharacterUid(
    raceInstance,
    notify,
    clientContext.characterUid);
}

void RaceNetworkHandler::HandleChangeRoomOptions(
  const ClientId clientId,
  const protocol::AcCmdCRChangeRoomOptions& command)
{
  // todo: validate command fields
  const auto& clientContext = GetClientContext(clientId);

  if (command.optionsBitfield == protocol::RoomOptionType::None)
    // If no options have been changed then do not broadcast notify
    // This prevents a bug with the race elapsed time from occurring
    return;

  const std::bitset<6> options(
    static_cast<uint16_t>(command.optionsBitfield));

  if (options.test(0))
  {
    _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
      [&command, clientContext](const data::Character& character)
      {
        spdlog::info("Room {}'s name changed by '{}' ('{}') to '{}'",
          clientContext.roomUid,
          clientContext.userName,
          character.name(),
          command.name);
      });
  }

  // Change the room options.
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&options, &command](Room& room)
    {
      auto& roomDetails = room.GetRoomDetails();

      if (options.test(0))
      {
        roomDetails.name = command.name;
      }
      if (options.test(1))
      {
        roomDetails.maxPlayerCount = command.playerCount;
      }
      if (options.test(2))
      {
        roomDetails.password = command.password;
      }
      if (options.test(3))
      {
        switch (command.gameMode)
        {
          case protocol::GameMode::Speed:
            roomDetails.gameMode = Room::GameMode::Speed;
            break;
          case protocol::GameMode::Magic:
            roomDetails.gameMode = Room::GameMode::Magic;
            break;
          case protocol::GameMode::Tutorial:
            roomDetails.gameMode = Room::GameMode::Tutorial;
            break;
          default:
            spdlog::error("Unknown game mode '{}'", static_cast<uint32_t>(command.gameMode));
        }
      }
      if (options.test(4))
      {
        roomDetails.courseId = command.mapBlockId;
      }
      if (options.test(5))
      {
        roomDetails.npcDifficulty = command.npcDifficulty;
      }
    });

  const protocol::AcCmdCRChangeRoomOptionsNotify notify{
    .optionsBitfield = command.optionsBitfield,
    .name = command.name,
    .playerCount = command.playerCount,
    .password = command.password,
    .gameMode = command.gameMode,
    .mapBlockId = command.mapBlockId,
    .npcDifficulty = command.npcDifficulty};

  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [this, &notify](const Room& room)
    {
      for (const auto& player : room.GetPlayers() | std::views::values)
      {
        try
        {
          _commandServer.QueueCommand<protocol::AcCmdCRChangeRoomOptionsNotify>(
            player.GetClientId(),
            [notify]()
            {
              return notify;
            });
        }
        catch (const std::exception&)
        {
          // the player disconnected
        }
      }
    });
}

void RaceNetworkHandler::HandleChangeTeam(
  const ClientId clientId,
  const protocol::AcCmdCRChangeTeam& command)
{
  const auto& clientContext = GetClientContext(clientId);

  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&command](Room& room)
    {
      auto& player = room.GetPlayer(command.characterOid);
      switch (command.teamColor)
      {
        case protocol::TeamColor::Red:
          player.SetTeam(Room::Player::Team::Red);
          break;
        case protocol::TeamColor::Blue:
          player.SetTeam(Room::Player::Team::Blue);
          break;
        default: {}
      }
    });

  std::scoped_lock lock(_raceInstancesMutex);
  const auto& raceInstance = GetRaceInstance(clientContext, false);

  if (raceInstance.GetStage() != RaceInstance::Stage::Waiting)
  {
    // A racer tried to change teams when not in the waiting room
    // No response needed, client does not change until it receives an OK
    return;
  }

  const protocol::AcCmdCRChangeTeamOK response{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Notify all other clients in the room
  const protocol::AcCmdCRChangeTeamNotify notify{
    .characterOid = command.characterOid,
    .teamColor = command.teamColor};
  this->BroadcastExceptCharacterUid(
    raceInstance,
    notify,
    clientContext.characterUid);
}

void RaceNetworkHandler::HandleLeaveRoom(ClientId clientId)
{
  protocol::AcCmdCRLeaveRoomOK response{};

  auto& clientContext = GetClientContext(clientId);
  if (clientContext.roomUid == 0)
    return;

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext, false);

  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [clientContext](const data::Character& character)
    {
      spdlog::info("Player {} ({}) has left [Room {}]",
        clientContext.userName,
        character.name(),
        clientContext.roomUid);
    });

  if (raceInstance.GetTracker().IsRacer(clientContext.characterUid))
  {
    auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);
    racer.state = tracker::RaceTracker::Racer::State::Disconnected;

    // Notify all the other racers that the client has disconnected
    const protocol::AcCmdUserRaceDeleteNotify deleteNotify{
      .racerOid = racer.oid};
    this->BroadcastExceptCharacterUid(
      raceInstance,
      deleteNotify,
      clientContext.characterUid);
  }

  data::Uid roomMasterUid{data::InvalidUid};
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&roomMasterUid, characterUid = clientContext.characterUid](Room& room)
    {
      roomMasterUid = room.GetRoomDetails().masterUid;
      room.RemovePlayer(characterUid);
    });

  // Check if the leaving player was the leader
  const bool wasMaster = roomMasterUid == clientContext.characterUid;

  {
    // Notify other clients in the room about the character leaving.
    const protocol::AcCmdCRLeaveRoomNotify notify{
      .characterId = clientContext.characterUid,
      .unk0 = 1};
    // No need to prevent self broadcast, player should be
    // removed from the room
    this->Broadcast(raceInstance, notify);
  }

  if (wasMaster)
  {
    std::vector<data::Uid> candidates;

    // If the race room is waiting pick from room users,
    // otherwise we have to pick a player from the race.
    // This prevents the new leader from being able to start
    // next race and cause confusion.
    if (raceInstance.GetStage() == RaceInstance::Stage::Waiting)
    {
      _serverInstance.GetRoomSystem().GetRoom(
        clientContext.roomUid,
        [&candidates](const Room& room)
        {
          std::ranges::copy(
            room.GetPlayers() | std::views::keys,
            std::back_inserter(candidates));
        });
    }
    else
    {
      // Get active racers (that are still connected)
      auto& tracker = raceInstance.GetTracker();
      std::ranges::copy_if(
        tracker.GetRacers() | std::views::keys,
        std::back_inserter(candidates),
        [&tracker](const data::Uid characterUid)
        {
          const auto& racer = tracker.GetRacer(characterUid);
          return racer.state != tracker::RaceTracker::Racer::State::Disconnected;
        });
    }

    // Pick a candidate
    // For now, we pick the first racer
    // todo: sort by performance
    if (not candidates.empty())
    {
      const data::Uid candidateUid = candidates.front();
      const auto& newMasterClientContext = GetClientContextByCharacterUid(candidateUid);

      std::string newMasterCharacterName;
      _serverInstance.GetDataDirector().GetCharacter(newMasterClientContext.characterUid).Immutable(
        [&newMasterCharacterName](const data::Character& character)
        {
          newMasterCharacterName = character.name();
        });

      spdlog::info("Player {} ({}) became the master of [Room {}] after the previous master left",
        newMasterClientContext.userName,
        newMasterCharacterName,
        clientContext.roomUid);

      // Update the room details to make the new master official
      _serverInstance.GetRoomSystem().GetRoom(
        clientContext.roomUid,
        [masterUid = newMasterClientContext.characterUid](Room& room)
        {
          room.GetRoomDetails().masterUid = masterUid;
        });

      // Notify other clients in the room about the new master.
      const protocol::AcCmdCRChangeMasterNotify notify{
        .masterUid = newMasterClientContext.characterUid};
      this->Broadcast(raceInstance, notify);
    }
  }

  {
    // Delete room if empty
    bool roomEmpty{false};
    raceInstance.GetRoom(
      [this, &roomEmpty](const Room& room)
      {
        if (room.GetPlayerCount() != 0)
          // Room is not empty
          return;

        roomEmpty = true;
      });

    if (roomEmpty)
    {
      _serverInstance.GetRoomSystem().DeleteRoom(clientContext.roomUid);
      _raceInstances.erase(clientContext.roomUid);
    }
  }

  clientContext.roomUid = data::InvalidUid;

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceNetworkHandler::HandleReadyRace(
  const ClientId clientId,
  const protocol::AcCmdCRReadyRace&)
{
  const auto& clientContext = GetClientContext(clientId);

  bool isPlayerReady = false;
  bool isGuildMatchAllReady = false;

  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&isPlayerReady, &isGuildMatchAllReady, characterUid = clientContext.characterUid](Room& room)
    {
      isPlayerReady = room.GetPlayer(characterUid).ToggleReady();

      if (room.GetRoomDetails().teamMode == Room::TeamMode::Guild)
      {
        const auto& players = room.GetPlayers();
        const auto maxPlayerCount = room.GetRoomDetails().maxPlayerCount;
        if (players.size() == maxPlayerCount)
        {
          isGuildMatchAllReady = std::ranges::all_of(
            players | std::views::values,
            [](const Room::Player& player)
            {
              return player.IsReady();
            });
        }
      }
    });

  const protocol::AcCmdCRReadyRaceNotify notify{
    .characterUid = clientContext.characterUid,
    .isReady = isPlayerReady};

  {
    std::scoped_lock lock(_raceInstancesMutex);
    const auto& raceInstance = GetRaceInstance(clientContext, false);
    this->Broadcast(raceInstance, notify);
  }

  // Auto start if guild match + all players are ready
  if (isGuildMatchAllReady)
    HandleStartRace(clientId, protocol::AcCmdCRStartRace{});
}

void RaceNetworkHandler::HandleStartRace(
  const ClientId clientId,
  [[maybe_unused]] const protocol::AcCmdCRStartRace& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext, false);

  // Check if all race requirements are met to start the race
  data::Uid roomMasterUid{data::InvalidUid};
  Room::PreventStartReason preventStartReason{};
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&preventStartReason, &roomMasterUid, invokerCharacterUid = clientContext.characterUid](Room& room)
    {
      roomMasterUid = room.GetRoomDetails().masterUid;
      const bool isGuildMatch = room.GetRoomDetails().teamMode == Room::TeamMode::Guild;
      if (not isGuildMatch and invokerCharacterUid != roomMasterUid)
        throw std::runtime_error("Client tried to start the race even though they're not the master");

      preventStartReason = room.CanRoomStart();
    });

  // Check if there is a reason why race cannot start
  switch (preventStartReason)
  {
    case Room::PreventStartReason::None:
      // No reason to prevent race start, continue
      break;
    case Room::PreventStartReason::NotAllPlayersReady:
      SendStartRaceCancel(clientId, protocol::AcCmdCRStartRaceCancel::Reason::NotReady);
      return;
    case Room::PreventStartReason::TeamImbalance:
      SendStartRaceCancel(clientId, protocol::AcCmdCRStartRaceCancel::Reason::NotTeamBalance);
      return;
    default:
      throw std::runtime_error("Prevent start reason not implemented");
  }

  const auto roomUid = clientContext.roomUid;

  RaceInstance::Parameters parameters;
  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [&parameters](Room& room)
    {
      auto& details = room.GetRoomDetails();

      parameters.gameMode = static_cast<protocol::GameMode>(details.gameMode);
      parameters.teamMode = static_cast<protocol::TeamMode>(details.teamMode);
      parameters.missionId = details.missionId;
      parameters.mapBlockId = details.courseId;
    });

  parameters.masterUid = roomMasterUid;

  // Clear the tracker before the race.
  raceInstance.GetTracker().Clear();

  if (not raceInstance.Start(parameters))
  {
    SendStartRaceCancel(clientId, protocol::AcCmdCRStartRaceCancel::Reason::Generic);
    return;
  }

  constexpr uint32_t GameCountdownKey = 17;
  constexpr uint32_t DefaultCountdownMs = 5310;

  const auto countdown = GetServerInstance()
    .GetSystemContentRegistry()
    .GetValue(GameCountdownKey);

  protocol::AcCmdRCRoomCountdown roomCountdown{
    .countdown = countdown.has_value()
      ? countdown.value()
      : DefaultCountdownMs,
    .mapBlockId = static_cast<uint16_t>(raceInstance.GetMapBlockId())};

  // Start with bonus course set to none by default
  raceInstance.SetBonusCourseType(protocol::BonusCourseType::None);

  // Randomly assign a bonus course if the room has 8 players
  {
    constexpr size_t RequiredPlayerCount = 8;
    bool hasRequiredPlayerCount = false;
    _serverInstance.GetRoomSystem().GetRoom(
      roomUid,
      [&hasRequiredPlayerCount](const Room& room)
      {
        hasRequiredPlayerCount = room.GetPlayerCount() == RequiredPlayerCount;
      });

    if (hasRequiredPlayerCount)
    {
      auto& gen = server::util::GetRandomEngine();
      std::uniform_int_distribution<uint32_t> chanceDist(1, 100);

      constexpr uint32_t BonusCourseChance = 25;
      const bool isBonusCourse = chanceDist(gen) <= BonusCourseChance;
      if (isBonusCourse)
      {
        std::uniform_int_distribution<uint32_t> typeDist(1, 3);
        const auto selectedType = static_cast<protocol::BonusCourseType>(typeDist(gen));
        roomCountdown.bonusCourseType = selectedType;
        raceInstance.SetBonusCourseType(selectedType);
      }
    }
  }

  // Broadcast room countdown.
  this->Broadcast(raceInstance, roomCountdown);

  // Add the racers.
  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [&raceInstance](Room& room)
    {
      // todo: observers
      for (const auto& [characterUid, roomPlayer] : room.GetPlayers())
      {
        auto& racer = raceInstance.GetTracker().AddRacer(characterUid);
        racer.state = tracker::RaceTracker::Racer::State::Loading;
        switch (roomPlayer.GetTeam())
        {
          case Room::Player::Team::Solo:
            racer.team = tracker::RaceTracker::Racer::Team::Solo;
            break;
          case Room::Player::Team::Red:
            racer.team = tracker::RaceTracker::Racer::Team::Red;
            break;
          case Room::Player::Team::Blue:
            racer.team = tracker::RaceTracker::Racer::Team::Blue;
            break;
        }
      }
    });

  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [](Room& room)
    {
      room.SetRoomPlaying(true);
    });

  // Queue race start after room countdown.
  _scheduler.Queue(
    [this, roomUid]()
    {
      std::scoped_lock raceInstanceLock(_raceInstancesMutex);

      const auto raceInstanceIter = _raceInstances.find(roomUid);;
      if (raceInstanceIter == _raceInstances.cend())
        return;

      auto& raceInstance = raceInstanceIter->second;
      const auto& parameters = raceInstance.GetParameters();

      const auto& lobbyConfig = GetServerInstance().GetLobbyDirector().GetConfig();
      protocol::AcCmdCRStartRaceNotify notify{
        .raceGameMode = parameters.gameMode,
        .raceTeamMode = parameters.teamMode,
        .raceMapBlockId = static_cast<uint16_t>(raceInstance.GetMapBlockId()),
        .p2pRelayAddress = lobbyConfig.advertisement.udpRaceRelay.address.to_uint(),
        .p2pRelayPort = lobbyConfig.advertisement.udpRaceRelay.port,
        .raceMissionId = parameters.missionId,};

      // Build the racers.
      for (const auto& [characterUid, racer] : raceInstance.GetTracker().GetRacers())
      {
        if (racer.state == tracker::RaceTracker::Racer::State::Disconnected)
          continue;

        std::string characterName;
        GetServerInstance().GetDataDirector().GetCharacter(characterUid).Immutable(
          [&characterName](const data::Character& character)
          {
            characterName = character.name();
          });

        auto& protocolRacer = notify.racers.emplace_back(
          protocol::AcCmdCRStartRaceNotify::Player{
            .oid = racer.oid,
            .name = characterName});

        // Assign the racer P2dId
        const ClientId racerClientId = GetClientIdByCharacterUid(characterUid);
        protocolRacer.p2dId = GetOrCreateP2dId(racerClientId);

        switch (racer.team)
        {
          case tracker::RaceTracker::Racer::Team::Solo:
            protocolRacer.teamColor = protocol::TeamColor::None;
            break;
          case tracker::RaceTracker::Racer::Team::Red:
            protocolRacer.teamColor = protocol::TeamColor::Red;
            break;
          case tracker::RaceTracker::Racer::Team::Blue:
            protocolRacer.teamColor = protocol::TeamColor::Blue;
            break;
        }
      }

      const bool isEligibleForSkills = (notify.raceGameMode == protocol::GameMode::Speed
        || notify.raceGameMode == protocol::GameMode::Magic)
        && notify.raceTeamMode == protocol::TeamMode::FFA;

      // Send to all clients participating in the race.
      raceInstance.GetRoom(
        [this, &raceInstance, &notify, isEligibleForSkills](const Room& room)
        {
          for (const auto& [characterUid, player] : room.GetPlayers())
          {
            if (not raceInstance.GetTracker().IsRacer(characterUid))
              continue;

            auto& racer = raceInstance.GetTracker().GetRacer(characterUid);
            notify.hostOid = racer.oid;

            // Skills only apply for speed single or magic single
            if (isEligibleForSkills)
            {
              // Notify racer of confirmed selection of skills
              GetServerInstance().GetDataDirector().GetCharacter(characterUid).Immutable(
                [&notify](const data::Character& character)
                {
                  // Get skill set by gamemode
                  const auto& skillSets =
                    notify.raceGameMode == protocol::GameMode::Speed ? character.skills.speed() :
                    notify.raceGameMode == protocol::GameMode::Magic ? character.skills.magic() :
                      throw std::runtime_error("Unknown game mode");

                  // Get racer's active skill set ID and set it in notify
                  notify.racerActiveSkillSet.setId = static_cast<uint8_t>(skillSets.activeSetId);

                  const auto& skillSet =
                    skillSets.activeSetId == 0 ? skillSets.set1 :
                    skillSets.activeSetId == 1 ? skillSets.set2 :
                    throw std::runtime_error("Invalid skill set ID");

                  // Slot 1, slot 2, bonus (calculated after)
                  notify.racerActiveSkillSet.skills[0] = skillSet.slot1;
                  notify.racerActiveSkillSet.skills[1] = skillSet.slot2;
                });

              // Bonus skills are unique for each racer in the racer
              // TODO: put these in a skill registry table
              std::vector<uint32_t> speedOnlyBonusSkills = {59, 32, 31};
              std::vector<uint32_t> magicOnlyBonusSkills = {34, 35, 36, 57, 58};
              std::vector<uint32_t> bonusSkillIds = {43, 29, 30}; // Speed + magic

              // Append to list depending on gamemode
              if (notify.raceGameMode == protocol::GameMode::Speed)
              {
                bonusSkillIds.insert(
                  bonusSkillIds.end(),
                  speedOnlyBonusSkills.begin(),
                  speedOnlyBonusSkills.end());
              }
              else if (notify.raceGameMode == protocol::GameMode::Magic)
              {
                bonusSkillIds.insert(
                  bonusSkillIds.end(),
                  magicOnlyBonusSkills.begin(),
                  magicOnlyBonusSkills.end());
              }

              std::uniform_int_distribution<uint32_t> bonusSkillDist(
                0,
                static_cast<uint32_t>(bonusSkillIds.size()) - 1);

              const auto bonusSkillIdx = bonusSkillDist(server::util::GetRandomEngine());
              notify.racerActiveSkillSet.skills[2] = bonusSkillIds[bonusSkillIdx];
            }

            _commandServer.QueueCommand<decltype(notify)>(
              player.GetClientId(),
              [notify]()
              {
                return notify;
              });
          }
        });
    },
    Scheduler::Clock::now() + std::chrono::milliseconds(roomCountdown.countdown));
}

void RaceNetworkHandler::SendStartRaceCancel(
  ClientId clientId,
  protocol::AcCmdCRStartRaceCancel::Reason reason)
{
  _commandServer.QueueCommand<protocol::AcCmdCRStartRaceCancel>(
    clientId,
    [reason]()
    {
      return protocol::AcCmdCRStartRaceCancel{
        .reason = reason};
    });
}

void RaceNetworkHandler::HandleRaceTimer(
  ClientId clientId,
  const protocol::AcCmdUserRaceTimer& command)
{
  protocol::AcCmdUserRaceTimerOK response{
    .clientRaceClock = command.clientClock,
    .serverRaceClock = util::TimePointToRaceTimePoint(
      std::chrono::steady_clock::now()),};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceNetworkHandler::HandleLoadingComplete(
  ClientId clientId,
  const protocol::AcCmdCRLoadingComplete&)
{
  auto& clientContext = GetClientContext(clientId);
  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  // Switch the racer to the racing state.
  racer.state = tracker::RaceTracker::Racer::State::Racing;

  // Snapshot mount stats so per-tick magic calculations don't hit the data store on every pos-update.
  GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [this, &racer](const data::Character& character)
    {
      GetServerInstance().GetDataDirector().GetHorse(character.mountUid()).Immutable(
        [&racer](const data::Horse& horse)
        {
          racer.mountStats = {
            .agility = horse.stats.agility(),
            .ambition = horse.stats.ambition(),
            .rush = horse.stats.rush(),
            .endurance = horse.stats.endurance(),
            .courage = horse.stats.courage(),
          };

          racer.potential = {
            .type = horse.potential.type(),
            .value = horse.potential.value(),
          };
        });

      auto& itemRegistry = GetServerInstance().GetItemRegistry();
      const auto equipmentRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(
        character.characterEquipment());

      std::vector<uint32_t> equippedMountTids;
      if (equipmentRecords)
      {
        for (const auto& equipmentRecord : *equipmentRecords)
        {
          data::Tid itemTid{data::InvalidTid};
          equipmentRecord.Immutable([&itemTid](const data::Item& item)
          {
            itemTid = item.tid();
          });

          const auto itemTemplate = itemRegistry.GetItem(itemTid);
          if (not itemTemplate.has_value()
            || not itemTemplate->mountPartInfo.has_value())
          {
            continue;
          }

          equippedMountTids.emplace_back(itemTid);

          if (itemTemplate->mountAbility.has_value())
          {
            const auto& ability = itemTemplate->mountAbility.value();
            racer.mountStats.agility += ability.agility;
            racer.mountStats.ambition += ability.ambition;
            racer.mountStats.rush += ability.rush;
            racer.mountStats.endurance += ability.endurance;
            racer.mountStats.courage += ability.courage;
          }
        }
      }

      // A racer can only benefit from a single set bonus at a time, so the first
      // fully-equipped set wins.
      racer.activeSetEffect = registry::SetEquipEffect::None;
      const auto activeSets = itemRegistry.GetActiveSets(equippedMountTids);
      if (not activeSets.empty())
        racer.activeSetEffect = activeSets.front()->equipEffect;
    });

  // Notify all clients in the room that this player's loading is complete
  const protocol::AcCmdCRLoadingCompleteNotify notify{
    .oid = racer.oid};
  this->Broadcast(raceInstance, notify);

  // Egg spawning mechanism

  // Character eligibility check
  const auto& isCharacterEligible = [this](data::Uid characterUid) -> bool
  {
    // Get character level to check min level
    uint32_t characterLevel{};
    GetServerInstance().GetDataDirector().GetCharacter(characterUid).Immutable(
      [&characterLevel](const data::Character& character)
      {
        characterLevel = character.level();
      });

    // Get configured minimum level required for egg spawning
    constexpr uint32_t MinCharLevelForEggSpawningKey = 61u;
    constexpr uint32_t DefaultMinCharLevelForEggSpawning = 12u;
    const auto& minCharacterLevelOpt = GetServerInstance().GetSystemContentRegistry().GetValue(
      MinCharLevelForEggSpawningKey);

    // Simple existence check in the system content registry, fallback to default
    const uint32_t minCharacterLevel = minCharacterLevelOpt.has_value() ?
      minCharacterLevelOpt.value() :
      DefaultMinCharLevelForEggSpawning;

    // If character level is above minimum level then character is eligible
    return characterLevel > minCharacterLevel;
  };

  // Randomness check
  const auto& shouldEggSpawn = []() -> bool
  {
    // TODO: verify if egg spawning probability is truly 50%
    return std::uniform_int_distribution<uint32_t>(0, 1)(server::util::GetRandomEngine()) != 0;
  };

  // Check gamemode eligibility
  // All teammodes including single (training, level 1 eggs only) can spawn eggs
  const bool isGameModeEligible =
    parameters.gameMode == protocol::GameMode::Speed or
    parameters.gameMode == protocol::GameMode::Magic;

  // If gamemode and character is eligible, and egg should spawn (chance)
  // then spawn egg
  const bool isEggSpawnEligible =
    isGameModeEligible and
    isCharacterEligible(clientContext.characterUid) and
    shouldEggSpawn();

  if (not isEggSpawnEligible)
    // Egg spawn not eligible, we are done here
    return;

  const protocol::AcCmdRCGameCreateClientItem spawnClientItem{
    .racerOid = racer.oid,
    .unk1 = 0};

  _commandServer.QueueCommand<decltype(spawnClientItem)>(
    clientId,
    [spawnClientItem]()
    {
      return spawnClientItem;
    });
}

void RaceNetworkHandler::HandleUserRaceFinal(
  ClientId clientId,
  const protocol::AcCmdUserRaceFinal& command)
{
  // todo: this should be verified as part of the anti cheat -
  //       we should track the race track progress and make sure it's linear
  //       and was done within reasonable timespan.

  const bool didNotFinish = command.raceTrackProgress >= 0.f;

  // debug
  {
    const std::chrono::hh_mm_ss raceTime{
      command.courseTime};
    spdlog::debug("[{}] AcCmdUserRaceFinal: {} {} {}",
      clientId,
      command.oid,
      didNotFinish
        ? "DNF"
        : std::format("{}:{}.{}",
            raceTime.minutes().count(),
            raceTime.seconds().count(),
            raceTime.subseconds().count()),
      command.raceTrackProgress);
  }

  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);

  // todo: sanity check for course time
  // todo: address npc racers and update their states
  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  racer.state = tracker::RaceTracker::Racer::State::Finishing;
  racer.courseTime = didNotFinish
    ? tracker::InvalidCourseTime
    : static_cast<uint32_t>(command.courseTime.count());

  const protocol::AcCmdUserRaceFinalNotify notify{
    .oid = racer.oid,
    .courseTime = racer.courseTime};

  this->Broadcast(raceInstance, notify);
}

void RaceNetworkHandler::HandleRaceResult(
  const ClientId clientId,
  [[maybe_unused]] const protocol::AcCmdCRRaceResult& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // todo:
  //  - record replays,
  //  - mount emblem unlocked
  //  - implement mount fatigue
  protocol::AcCmdCRRaceResultOK response{};
  protocol::AcCmdRCUpdateMountInfoNotify potentialNotify{
    .characterUid = clientContext.characterUid,
    .action = protocol::AcCmdRCUpdateMountInfoNotify::Action::ProgressHorsePotential};
  bool potentialProgressed = false;

  characterRecord.Immutable(
    [this, &response, &potentialNotify, &potentialProgressed,
      gainedClassProgress = command.gainedClassProgress](const data::Character& character)
    {
      response.currentCarrots = character.carrots();

      GetServerInstance().GetDataDirector().GetHorse(character.mountUid()).Mutable(
        [this, &response, &potentialNotify, &potentialProgressed, gainedClassProgress](data::Horse& horse)
        {
          response.horseFatigue = static_cast<uint16_t>(
            horse.fatigue());

          GetServerInstance().GetHorseRegistry().ApplyClassProgress(
            horse, gainedClassProgress);

          potentialProgressed =
            GetServerInstance().GetHorseRegistry().ApplyPotentialGrowth(horse) > 0;
          if (potentialProgressed)
            protocol::BuildProtocolHorse(potentialNotify.horse, horse);
        });
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  if (potentialProgressed)
  {
    _commandServer.QueueCommand<decltype(potentialNotify)>(
      clientId,
      [potentialNotify]()
      {
        return potentialNotify;
      });
  }
}

void RaceNetworkHandler::HandleP2PRaceResult(
  ClientId,
  const protocol::AcCmdCRP2PResult&)
{
  // const auto& clientContext = GetClientContext(clientId);

  // std::scoped_lock lock(_raceInstancesMutex);
  // auto& raceInstance = GetRaceInstance(clientContext);

  // protocol::AcCmdGameRaceP2PResult result{};
  // for (const auto & [uid, racer] : raceInstance.GetTracker().GetRacers())
  // {
  //   auto& protocolRacer = result.member1.emplace_back();
  //   protocolRacer.oid = racer.oid;
  // }

  // _commandServer.QueueCommand<decltype(result)>(clientId, [result](){return result;});
}

void RaceNetworkHandler::HandleP2PUserRaceResult(
  ClientId,
  const protocol::AcCmdUserRaceP2PResult&)
{
}

void RaceNetworkHandler::HandleAwardStart(
  ClientId clientId,
  const protocol::AcCmdCRAwardStart& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);

  protocol::AcCmdRCAwardNotify notify{
    .member1 = command.member1};

  // Send to clients not participating in races.
  raceInstance.GetRoom(
    [this, &notify, &raceInstance](const Room& room)
    {
      for (const auto& [characterUid, player] : room.GetPlayers())
      {
        // Whether the client is a participating racer that did not disconnect.
        bool isParticipatingRacer = false;
        if (raceInstance.GetTracker().IsRacer(characterUid))
        {
          auto& racer = raceInstance.GetTracker().GetRacer(
            characterUid);
          // todo: handle player reconnect instead of ignoring them here
          isParticipatingRacer = racer.state != tracker::RaceTracker::Racer::State::Disconnected;
        }

        if (isParticipatingRacer)
          continue;

        _commandServer.QueueCommand<decltype(notify)>(
          player.GetClientId(),
          [notify]()
          {
            return notify;
          });
      }
    });
}

void RaceNetworkHandler::HandleAwardEnd(
  ClientId,
  const protocol::AcCmdCRAwardEnd&)
{
  // todo: this always crashes everyone

  // const auto& clientContext = GetClientContext(clientId);
  // auto& raceInstance = GetRaceInstance(clientContext);
  //
  // protocol::AcCmdCRAwardEndNotify notify{};
  //
  // // Send to clients not participating in races.
  // for (const auto raceClientId : raceInstance.clients)
  // {
  //   const auto& roomClientContext = _clients[raceClientId];
  //
  //   // Whether the client is a participating racer that did not disconnect.
  //   bool isParticipatingRacer = false;
  //   if (raceInstance.GetTracker().IsRacer(roomClientContext.characterUid))
  //   {
  //     auto& racer = raceInstance.GetTracker().GetRacer(
  //       roomClientContext.characterUid);
  //     isParticipatingRacer = racer.state != tracker::RaceTracker::Racer::State::Disconnected;
  //   }
  //
  //   if (isParticipatingRacer)
  //     continue;
  //
  //   _commandServer.QueueCommand<decltype(notify)>(
  //     raceClientId,
  //     [notify]()
  //     {
  //       return notify;
  //     });
  // }
}

void RaceNetworkHandler::HandleStarPointGet(
  ClientId clientId,
  const protocol::AcCmdCRStarPointGet& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(parameters.gameMode));

  uint32_t gainedStarPoints = command.gainedStarPoints;
  if (racer.effects[race::SkillEffect::BufGauge] || racer.effects[race::SkillEffect::BufGaugeCritical]) {
    // TODO: Something sensible, idk what the bonus does
    gainedStarPoints *= 2;
  }

  racer.starPointValue = std::min(
    racer.starPointValue + gainedStarPoints,
    gameModeTemplate.starPointsMax);

  // Star point get (boost get) is only called in speed, should never give magic item
  protocol::AcCmdCRStarPointGetOK response{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceNetworkHandler::HandleRequestSpur(
  ClientId clientId,
  const protocol::AcCmdCRRequestSpur& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(parameters.gameMode));

  if (racer.starPointValue < gameModeTemplate.spurConsumeStarPoints)
    throw std::runtime_error("Client is dead ass cheating (or is really desynced)");

  racer.starPointValue -= gameModeTemplate.spurConsumeStarPoints;

  protocol::AcCmdCRRequestSpurOK response{
    .characterOid = command.characterOid,
    .activeBoosters = command.activeBoosters,
    .startPointValue = racer.starPointValue,
    .comboBreak = command.comboBreak};

  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  _commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]()
    {
      return starPointResponse;
    });
}

void RaceNetworkHandler::HandleHurdleClearResult(
  ClientId clientId,
  const protocol::AcCmdCRHurdleClearResult& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  protocol::AcCmdCRHurdleClearResultOK response{
    .characterOid = command.characterOid,
    .hurdleClearType = command.hurdleClearType,
    .jumpCombo = 0,
    .unk3 = 0
  };

  // Give magic item is calculated later
  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false
  };

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(parameters.gameMode));

  switch (command.hurdleClearType)
  {
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect:
    {
      // Perfect jump over the hurdle.
      racer.jumpComboValue = std::min(
        static_cast<uint32_t>(99),
        racer.jumpComboValue + 1);

      if (parameters.gameMode == protocol::GameMode::Speed)
      {
        // Only send jump combo if it is a speed race
        response.jumpCombo = racer.jumpComboValue;
      }

      // Calculate max applicable combo
      const auto& applicableComboCount = std::min(
        gameModeTemplate.perfectJumpMaxBonusCombo,
        racer.jumpComboValue);
      // Calculate max combo count * perfect jump boost unit points
      const auto& gainedStarPointsFromCombo = applicableComboCount * gameModeTemplate.perfectJumpUnitStarPoints;
      // Add boost points to character boost tracker
      racer.starPointValue = std::min(
        racer.starPointValue + gameModeTemplate.perfectJumpStarPoints + gainedStarPointsFromCombo,
        gameModeTemplate.starPointsMax);

      // Update boost gauge
      starPointResponse.starPointValue = racer.starPointValue;
      break;
    }
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Good:
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::DoubleJumpOrGlide:
    {
      // Not a perfect jump over the hurdle, reset the jump combo.
      racer.jumpComboValue = 0;
      response.jumpCombo = racer.jumpComboValue;

      uint32_t gainedStarPoints = gameModeTemplate.goodJumpStarPoints;
      if (racer.effects[race::SkillEffect::BufGauge] || racer.effects[race::SkillEffect::BufGaugeCritical]) {
        // TODO: Something sensible, idk what the bonus does
        gainedStarPoints *= 2;
      }

      // Increment boost gauge by a good jump
      racer.starPointValue = std::min(
        racer.starPointValue + gainedStarPoints,
        gameModeTemplate.starPointsMax);

      // Update boost gauge
      starPointResponse.starPointValue = racer.starPointValue;
      break;
    }
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Collision:
    {
      // A collision with hurdle, reset the jump combo.
      racer.jumpComboValue = 0;
      response.jumpCombo = racer.jumpComboValue;
      break;
    }
    default:
    {
      spdlog::warn("Unhandled hurdle clear type {}",
        static_cast<uint8_t>(command.hurdleClearType));
      return;
    }
  }

  // Needs to be assigned after hurdle clear result calculations
  // Triggers magic item request when set to true (if gamemode is magic and magic gauge is max)
  starPointResponse.giveMagicItem =
    parameters.gameMode == protocol::GameMode::Magic &&
    racer.starPointValue >= gameModeTemplate.starPointsMax &&
    not racer.magicItem.has_value() &&
    command.hurdleClearType == protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect;

  // Update the star point value if the jump was not a collision.
  if (command.hurdleClearType != protocol::AcCmdCRHurdleClearResult::HurdleClearType::Collision)
  {
    _commandServer.QueueCommand<decltype(starPointResponse)>(
      clientId,
      [clientId, starPointResponse]()
      {
        return starPointResponse;
      });
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [clientId, response]()
    {
      return response;
    });
}

void RaceNetworkHandler::HandleStartingRate(
  ClientId clientId,
  const protocol::AcCmdCRStartingRate& command)
{
  // TODO: check for sensible values
  if (command.unk1 < 1 && command.boostGained < 1)
  {
    // Velocity and boost gained is not valid
    // TODO: throw?
    return;
  }

  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(parameters.gameMode));

  // TODO: validate boost gained against a table and determine good/perfect start
  racer.starPointValue = std::min(
    racer.starPointValue + command.boostGained,
    gameModeTemplate.starPointsMax);

  // Only send this on good/perfect starts
  protocol::AcCmdCRStarPointGetOK response{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false // TODO: this would never give a magic item on race start, right?
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [clientId, response]()
    {
      return response;
    });
}

void RaceNetworkHandler::HandleRaceUserPos(
  const ClientId clientId,
  const protocol::AcCmdUserRaceUpdatePos& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.oid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  // TODO: player position anticheat

  racer.worldPosition = command.position;
  racer.raceProgress = command.progress;
}

void RaceNetworkHandler::HandleChat(
  const ClientId clientId,
  const protocol::AcCmdCRChat& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Perform moderation before proceeding with chat processing
  const auto verdict = _serverInstance.GetChatSystem().ProcessChatMessage(
    clientContext.characterUid, command.message);

  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  std::string characterName;
  characterRecord.Immutable([&characterName](const data::Character& character)
  {
    characterName = character.name();
  });

  const auto& userName = clientContext.userName;

  std::vector<protocol::AcCmdCRChatNotify> response;
  const bool isCommand = verdict.commandVerdict.has_value();

  if (isCommand)
  {
    for (const auto& line : verdict.commandVerdict->result)
    {
      response.emplace_back(protocol::AcCmdCRChatNotify{
        .message = line,
        .author = "",
        .isSystem = true});
    }
  }
  else
  {
    if (verdict.isMuted)
    {
      if (verdict.isPrevented)
      {
        spdlog::info("[Room {}] (prevented) {} ({}): {}",
          clientContext.roomUid,
          characterName,
          userName,
          command.message);
      }
      else
      {
        spdlog::info("[Room {}] (muted) {} ({}): {}",
          clientContext.roomUid,
          characterName,
          userName,
          command.message);
      }
      protocol::AcCmdCRChatNotify notify{
        .message  = verdict.message,
        .author   = verdict.isPrevented ? "AutoMod" : "System",
        .isSystem = true};
      _commandServer.QueueCommand<decltype(notify)>(clientId, [notify](){ return notify; });
      return;
    }

    spdlog::info("[Room {}] {} ({}): {}",
      clientContext.roomUid,
      characterName,
      userName,
      command.message);

    response.emplace_back(protocol::AcCmdCRChatNotify{
      .message = verdict.message,
      .author = characterName,
      .isSystem = false,});
  }

  if (isCommand)
  {
    for (const auto& notify : response)
    {
      _commandServer.QueueCommand<protocol::AcCmdCRChatNotify>(
        clientId,
        [notify]{ return notify; });
    }
  }
  else
  {
    std::scoped_lock lock(_raceInstancesMutex);
    // Don't check racer since chat can be sent either
    // in the waiting room or during a race.
    const auto& raceInstance = GetRaceInstance(clientContext, false);
    for (const auto& notify : response)
    {
      this->Broadcast(raceInstance, notify);
    }
  }
}

void RaceNetworkHandler::HandleRelayCommand(
  ClientId clientId,
  const protocol::AcCmdCRRelayCommand& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Create relay notify message
  protocol::AcCmdCRRelayCommandNotify notify{
    .member1 = command.member1,
    .member2 = command.member2};

  std::scoped_lock lock(_raceInstancesMutex);
  // Get the room instance for this client
  const auto& raceInstance = GetRaceInstance(clientContext);

  // Relay the command to all other clients in the room
  this->BroadcastExceptCharacterUid(
    raceInstance,
    notify,
    clientContext.characterUid);
}

void RaceNetworkHandler::HandleRelay(
  ClientId clientId,
  const protocol::AcCmdCRRelay& command)
{
  const auto& clientContext = GetClientContext(clientId);

  // Create relay notify message
  protocol::AcCmdCRRelayNotify notify{
    .fromOid = command.fromOid,
    .toOid = command.toOid,
    .payloadType = command.payloadType,
    .data = std::move(command.data),};

  switch (command.payloadType)
  {
    case protocol::relay::RelayCommandId::Snapshot:
    {
      // Do anything related to `command.snapshot`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::SyncProgress:
    {
      // Do anything related to `command.syncProgress`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::SetTargetStateEnabled:
    case protocol::relay::RelayCommandId::SetTargetStateDisabled:
    {
      // Do anything related to `command.setTargetState`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::NetSetState:
    {
      // Do anything related to `command.netSetState`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::NetSetLayerAnimation:
    {
      // Do anything related to `command.netSetLayerAnimation`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::SyncGoalIn:
    {
      // Do anything related to `command.syncGoalIn`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::SpurLevel:
    {
      // Do anything related to `command.spurLevel`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::SlidingMotion:
    {
      // Do anything related to `command.slidingMotion`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::BroadcastCharacterUid:
    {
      // Do anything related to `command.broadcastCharacterUid`, if needed
      break;
    }
    case protocol::relay::RelayCommandId::ResetPosOther:
    {
      // Do anything related to `command.resetPosOther`, if needed
      break;
    }
    default:
    {
      const std::string header = command.toOid == 0 ?
        std::format("{:#x}->Broadcast", command.fromOid) :
        std::format("{:#x}->{:#x}",
          command.fromOid,
          command.toOid);

      spdlog::warn("Relay payload from client '{}', with oids {}, sent an unrecognised relay payload type '{:#04x}': {:02X}",
        clientId,
        header,
        static_cast<uint16_t>(command.payloadType),
        spdlog::to_hex(command.data));
      break;
    }
  }

  std::scoped_lock lock(_raceInstancesMutex);
  // Get the room instance for this client
  const auto& raceInstance = GetRaceInstance(clientContext);

  // Relay the command to all other clients in the room

  // TODO: potential improvement - instead of blindly broadcasting to room,
  // forward packet to recepient if `toOid` is non-zero.
  this->BroadcastExceptCharacterUid(raceInstance, notify, clientContext.characterUid);
}

void RaceNetworkHandler::HandleUserRaceActivateInteractiveEvent(
  ClientId clientId,
  const protocol::AcCmdUserRaceActivateInteractiveEvent& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);

  // Get the sender's OID from the room tracker
  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  protocol::AcCmdUserRaceActivateInteractiveEvent notify{
    .member1 = command.member1,
    .characterOid = racer.oid, // sender oid
    .member3 = command.member3
  };

  // Broadcast to all clients in the room
  this->Broadcast(raceInstance, notify);
}

void RaceNetworkHandler::HandleUserRaceActivateEvent(
  ClientId clientId,
  const protocol::AcCmdUserRaceActivateEvent& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // Check if event is throttled, or add event if it is a new one
  if (raceInstance.GetTracker().IsEventThrottled(command.eventId))
  {
    // Event throttled
    return;
  }

  // Schedule a deactivate event notify
  _scheduler.Queue([this, clientId, eventId = command.eventId]()
  {
    protocol::AcCmdUserRaceDeactivateEvent deactivateCommand{
      .eventId = eventId};
    this->HandleUserRaceDeactivateEvent(clientId, deactivateCommand);
  }, std::chrono::steady_clock::now() + tracker::EventThrottleDuration);

  // Broadcast to all active racers in the race
  const protocol::AcCmdUserRaceActivateEventNotify notify{
    .eventId = command.eventId,
    .characterOid = racer.oid};
  this->Broadcast(raceInstance, notify);
}

void RaceNetworkHandler::HandleUserRaceDeactivateEvent(
  ClientId clientId,
  const protocol::AcCmdUserRaceDeactivateEvent& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // Check if event is throttled, or add event if it is a new one
  if (raceInstance.GetTracker().IsEventThrottled(command.eventId))
  {
    // Event throttled
    return;
  }

  // Broadcast to all active racers in the race
  const protocol::AcCmdUserRaceDeactivateEventNotify notify{
    .eventId = command.eventId,
    .characterOid = racer.oid};
  this->Broadcast(raceInstance, notify);
}

void RaceNetworkHandler::HandleRequestMagicItem(
  const ClientId clientId,
  const protocol::AcCmdCRRequestMagicItem& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();
  auto& tracker = raceInstance.GetTracker();
  auto& racer = tracker.GetRacer(clientContext.characterUid);

  // TODO: Revise this on NPC races
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Client tried to perform action on behalf of different racer");
    return;
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(parameters.gameMode));

  // Only assign + respond if the gauge is full and the racer is empty-handed.
  // Anything else (stale request, duplicate after assignment, request while holding an item) drops.
  if (racer.magicItem.has_value()
    || racer.starPointValue < gameModeTemplate.starPointsMax)
  {
    return;
  }

  racer.starPointValue = 0;

  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue,
    .giveMagicItem = false};

  _commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]
    {
      return starPointResponse;
    });

  GrantMagicItem(raceInstance, clientId, clientContext.characterUid, racer);
}

void RaceNetworkHandler::GrantMagicItem(
  RaceInstance& raceInstance,
  const ClientId clientId,
  const data::Uid characterUid,
  tracker::RaceTracker::Racer& racer)
{
  const auto& magicItemSlotInfo = race::MagicSystem::RandomMagicItem(
    _serverInstance.GetMagicRegistry(),
    raceInstance.GetTracker(),
    characterUid);
  racer.magicItem.emplace(magicItemSlotInfo.type);
  ++racer.magicItemGeneration;

  const protocol::AcCmdCRRequestMagicItemOK response{
    .characterOid = racer.oid,
    .magicItemId = racer.magicItem.value(),
    .member3 = 0};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });

  // Notify other racers that racer is holding the magic item
  const protocol::AcCmdCRRequestMagicItemNotify notify{
    .magicItemId = response.magicItemId,
    .characterOid = response.characterOid};
  this->BroadcastExceptCharacterUid(raceInstance, notify, characterUid);
}

void RaceNetworkHandler::GrantOnePlusOneMagicItem(
  RaceInstance& raceInstance,
  const tracker::Oid attackerOid)
{
  auto& racers = raceInstance.GetTracker().GetRacers();
  const auto attackerIter = race::MagicSystem::FindRacerByOid(racers, attackerOid);
  if (attackerIter == racers.end())
    return;

  auto& attackerRacer = attackerIter->second;

  if (attackerRacer.magicItem.has_value())
    return;

  if (not PotentialSystem::GrantsExtraMagicItem(
    GetServerInstance().GetHorseRegistry(), attackerRacer.potential))
  {
    return;
  }

  const auto attackerClientId = FindClientIdByCharacterUid(attackerIter->first);
  if (not attackerClientId)
    return;

  GrantMagicItem(raceInstance, *attackerClientId, attackerIter->first, attackerRacer);
}

void RaceNetworkHandler::AcknowledgeEmptyMagicUse(
  const ClientId clientId,
  const tracker::Oid characterOid)
{
  const protocol::AcCmdCRUseMagicItemOK response{
    .characterOid = characterOid};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });
}

const registry::Magic::SlotInfo& RaceNetworkHandler::ConsumeCriticalAura(
  RaceInstance& raceInstance,
  tracker::RaceTracker::Racer& racer,
  const registry::Magic::SlotInfo& magicSlotInfo)
{
  constexpr std::array auraEffectIds{
    race::SkillEffect::BufPower,
    race::SkillEffect::BufPowerCritical};

  const bool hasAura = std::ranges::any_of(
    auraEffectIds,
    [&racer](const uint32_t effectId)
    {
      return racer.effects[effectId];
    });

  // A spell with no critical variant leaves the aura in place for the next cast.
  if (not hasAura || magicSlotInfo.criticalType == 0)
    return magicSlotInfo;

  for (const uint32_t effectId : auraEffectIds)
  {
    if (racer.effects[effectId])
      RemoveEffect(raceInstance, racer, effectId);
  }

  return GetServerInstance().GetMagicRegistry().GetSlotInfo(magicSlotInfo.criticalType);
}

std::vector<tracker::Oid> RaceNetworkHandler::ResolveMagicTargets(
  RaceInstance& raceInstance,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const std::vector<tracker::Oid>& targetList)
{
  auto resolvedTargetList = targetList;
  if (magicSlotInfo.type == race::MagicType::DarkFire)
  {
    resolvedTargetList.resize(1);
    return resolvedTargetList;
  }

  if (magicSlotInfo.basicType == race::MagicType::Summon && not resolvedTargetList.empty())
  {
    auto& racers = raceInstance.GetTracker().GetRacers();
    const auto targetIter = race::MagicSystem::FindRacerByOid(racers, resolvedTargetList.front());

    if (targetIter == racers.end()
      || targetIter->second.pendingMagicTarget.has_value()
      || race::MagicSystem::IsDowned(targetIter->second))
      resolvedTargetList.clear();
  }

  return resolvedTargetList;
}

void RaceNetworkHandler::QueueIceWallExpiry(
  RaceInstance& raceInstance,
  const uint32_t magicType,
  const uint16_t firstObstacleInstanceId,
  const uint16_t obstacleInstanceCount)
{
  constexpr auto IceWallLifetime = std::chrono::seconds(4);

  _scheduler.Queue(
    [this, magicType, firstObstacleInstanceId, obstacleInstanceCount,
      roomUid = raceInstance.GetRoomUid()]
    {
      std::scoped_lock lock(_raceInstancesMutex);
      const auto raceInstanceIter = _raceInstances.find(roomUid);
      if (raceInstanceIter == _raceInstances.cend())
        return;

      this->Broadcast(
        raceInstanceIter->second,
        protocol::AcCmdRCMagicExpire{
          .magicType = magicType,
          .firstObstacleInstanceId = static_cast<uint16_t>(firstObstacleInstanceId),
          .obstacleInstanceCount = 3,
          .breakdown = 0});
    },
    Scheduler::Clock::now() + IceWallLifetime);
}

void RaceNetworkHandler::ApplyImmediateMagicEffects(
  RaceInstance& raceInstance,
  const tracker::RaceTracker::Racer& racer,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const uint16_t effectInstanceId,
  const uint16_t obstacleInstanceCount)
{
  // Shield, Booster and HotRodding buff the caster themselves.
  if (race::MagicSystem::IsSelfCast(magicSlotInfo.type))
  {
    this->ScheduleSkillEffect(raceInstance, racer.oid, racer.oid, magicSlotInfo, effectInstanceId);
    return;
  }

  // An ice wall has no per-racer effect, only obstacles that expire on their own.
  if (race::MagicSystem::IsIceWall(magicSlotInfo.type))
  {
    this->QueueIceWallExpiry(
      raceInstance,
      magicSlotInfo.type,
      effectInstanceId,
      obstacleInstanceCount);
    return;
  }

  // BufPower, BufGauge and BufSpeed buff the caster and, in team modes, their team.
  if (race::MagicSystem::IsTeamBuff(magicSlotInfo.type))
  {
    for (auto& otherRacer : raceInstance.GetTracker().GetRacers() | std::views::values)
    {
      const bool isSelf = racer.oid == otherRacer.oid;
      const bool isTeamMate = racer.team != tracker::RaceTracker::Racer::Team::Solo
        && racer.team == otherRacer.team;

      if (isSelf || isTeamMate)
      {
        this->ScheduleSkillEffect(
          raceInstance, racer.oid, otherRacer.oid, magicSlotInfo, effectInstanceId);
      }
    }
  }
}

void RaceNetworkHandler::HandleUseMagicItem(
  const ClientId clientId,
  const protocol::AcCmdCRUseMagicItem& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Client tried to perform action on behalf of different racer");
    return;
  }

  // Nothing to cast — acknowledge anyway so the client drops its held item indicator.
  if (not racer.magicItem.has_value() || command.magicItemId == 0)
  {
    racer.magicItem.reset();
    AcknowledgeEmptyMagicUse(clientId, command.characterOid);
    return;
  }

  const auto& magicSlotInfo = ConsumeCriticalAura(
    raceInstance,
    racer,
    GetServerInstance().GetMagicRegistry().GetSlotInfo(command.magicItemId));

  const auto targetList = ResolveMagicTargets(raceInstance, magicSlotInfo, command.targetList);

  // An ice wall claims one effect instance id per obstacle it places, everything else claims one.
  const auto obstacleInstanceCount = static_cast<uint16_t>(command.targetList.size());
  const uint16_t effectInstanceId = raceInstance.GetTracker().GetNextEffectInstanceIdAndIncrementBy(
    race::MagicSystem::IsIceWall(magicSlotInfo.type) ? obstacleInstanceCount : 1u);

  const protocol::AcCmdCRUseMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = magicSlotInfo.type,
    .iceWallProperties = command.iceWallProperties,
    .targetList = targetList,
    .effectInstanceId = effectInstanceId,
    .unk4 = magicSlotInfo.castingTime};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });

  // Notify the other racers that this racer used their magic item.
  const protocol::AcCmdCRUseMagicItemNotify usageNotify{
    .characterOid = command.characterOid,
    .magicItemId = magicSlotInfo.type,
    .iceWallProperties = command.iceWallProperties,
    .targetList = targetList,
    .effectInstanceId = effectInstanceId,
    .unk4 = magicSlotInfo.castingTime};

  this->BroadcastExceptCharacterUid(raceInstance, usageNotify, clientContext.characterUid);

  ApplyImmediateMagicEffects(
    raceInstance,
    racer,
    magicSlotInfo,
    effectInstanceId,
    obstacleInstanceCount);

  racer.magicItem.reset();
}

void RaceNetworkHandler::HandleUserRaceItemGet(
  const ClientId clientId,
  const protocol::AcCmdUserRaceItemGet& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);

  auto& raceInstance = GetRaceInstance(clientContext);
  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // Check event items first (eggs, etc.)
  const auto eventItemOid = raceInstance.GetTracker().FindEventItem(
    clientContext.characterUid,
    command.itemDeckId);

  if (eventItemOid != tracker::InvalidEntityOid)
  {
    auto& eventItem = raceInstance.GetTracker().GetEventItem(clientContext.characterUid, eventItemOid);
    const auto eggInfo = _serverInstance.GetPetRegistry().GetEggInfoByDeckId(eventItem.itemType);
    auto itemUid = data::InvalidUid;
    const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
      clientContext.characterUid);

    characterRecord.Mutable([this, &eggInfo, &itemUid](data::Character& character)
    {
      itemUid = _serverInstance.GetItemSystem().AddItem(character, eggInfo.tid, 1);
    });

    // Notify racers that invoker got the egg
    const protocol::AcCmdRCObtainEgg obtainEgg{
      .characterUid = clientContext.characterUid,
      .ItemUid = itemUid,
      .ItemTid = eggInfo.tid};
    this->Broadcast(raceInstance, obtainEgg);

    const protocol::AcCmdGameRaceItemGet itemGet{
      .characterOid = command.characterOid,
      .itemId = command.itemDeckId,
      .itemType = eventItem.itemType};
    this->Broadcast(raceInstance, itemGet);

    raceInstance.GetTracker().RemoveEventItem(clientContext.characterUid, command.itemDeckId);
    racer.trackedDecks.erase(command.itemDeckId);

    return;
  }

  auto& items = raceInstance.GetTracker().GetItemDecks();
  const auto deckIter = items.find(command.itemDeckId);
  if (deckIter == items.end())
  {
    spdlog::warn("Client {} picked up untracked item deck {}", clientId, command.itemDeckId);
    return;
  }

  auto& deck = deckIter->second;

  const auto now = std::chrono::steady_clock::now();

  const auto deckCooldownIter = racer.deckCooldown.find(
    command.itemDeckId);

  if (deckCooldownIter != racer.deckCooldown.end()
    && deckCooldownIter->second > now)
  {
    // Picker's client predictively hid the item on collision; untrack it
    // so processItemSpawn re-broadcasts the spawn for them next tick.
    racer.trackedDecks.erase(command.itemDeckId);
    return;
  }

  // On pickup, clear the spawner cooldown for all other racers
  // so it remains available for them immediately
  for (auto& [otherUid, otherRacer] : raceInstance.GetTracker().GetRacers())
  {
    if (otherUid != clientContext.characterUid)
    {
      otherRacer.deckCooldown.erase(command.itemDeckId);
    }
  }

  // Set the pickup cooldown exclusively for the collecting racer
  racer.deckCooldown[command.itemDeckId] = now + deck.respawnTime;

  Room::GameMode gameMode;
  registry::Course::GameModeInfo gameModeInfo;
  _serverInstance.GetRoomSystem().GetRoom(clientContext.roomUid, [this, &gameMode, &gameModeInfo](const Room& room)
  {
    gameMode = room.GetRoomSnapshot().details.gameMode;
    gameModeInfo = this->GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(static_cast<uint8_t>(gameMode));
  });

  switch(gameMode)
  {
    // TODO: Deduplicate from StarPointGet
    case Room::GameMode::Speed:
      {
        switch (deck.currentItem)
        {
          case 101: // Gold horseshoe. Get star points until the next boost
            racer.starPointValue = std::min(((racer.starPointValue/40000)+1) * 40000, gameModeInfo.starPointsMax);
            break;
          case 102: // Silver horseshoe. Get 10k star points
            racer.starPointValue = std::min(racer.starPointValue+10000, gameModeInfo.starPointsMax);
            break;
          default:
            spdlog::warn("Player {} picked up unknown item type {}",
              clientId, deck.currentItem);
            break;
        }

        // Only send this on good/perfect starts
        protocol::AcCmdCRStarPointGetOK starPointResponse{
          .characterOid = command.characterOid,
          .starPointValue = racer.starPointValue,
          .giveMagicItem = false
        };

        _commandServer.QueueCommand<decltype(starPointResponse)>(
          clientId,
          [clientId, starPointResponse]()
          {
            return starPointResponse;
          });
      }
      break;

    // TODO: Deduplicate from RequestMagicItem
    case Room::GameMode::Magic:
    {
      uint32_t magicItem{};
      if (not racer.magicItem.has_value())
      {
        // Racer is empty handed

        // Get the item type of the picked up item (408, 409 etc)
        const uint32_t magicItemType = deck.currentItem;

        // Get the magic slot index to indicate to the racer that they
        // have the item (water shield, ice wall etc).
        magicItem = _serverInstance.GetCourseRegistry()
          .GetDeckItemInfo(magicItemType).magicSlot;

        // Get the magic item's slot info and check if it gives positional magic
        const auto& slotInfo = _serverInstance.GetMagicRegistry().GetSlotInfo(magicItem);
        if (slotInfo.givePositionalMagic != 0)
        {
          const auto& magicItemSlotInfo = race::MagicSystem::RandomMagicItem(
            _serverInstance.GetMagicRegistry(),
            raceInstance.GetTracker(),
            clientContext.characterUid);
          magicItem = magicItemSlotInfo.type;
        }

        // Response with OK to the client that they have a new item in hand
        protocol::AcCmdCRRequestMagicItemOK magicItemOk{
          .characterOid = command.characterOid,
          .magicItemId = racer.magicItem.emplace(magicItem),
          .member3 = 0};
        ++racer.magicItemGeneration;

        _commandServer.QueueCommand<decltype(magicItemOk)>(
          clientId,
          [clientId, magicItemOk]()
          {
            return magicItemOk;
          });

        racer.starPointValue = 0;

        protocol::AcCmdCRStarPointGetOK starPointResponse{
          .characterOid = command.characterOid,
          .starPointValue = 0,
          .giveMagicItem = false};

        _commandServer.QueueCommand<decltype(starPointResponse)>(
          clientId,
          [starPointResponse]()
          {
            return starPointResponse;
          });
      }
      else
      {
        // Racer is already holding the item, do not replace it
        magicItem = racer.magicItem.value();
      }

      // The item was picked up, generate a new item.
      raceInstance.PickRandomItemFromDeck(deck);

      // Notify racers in the race room that the invoking racer is now
      // holding a new magic item
      const protocol::AcCmdCRRequestMagicItemNotify notify{
        .magicItemId = magicItem,
        .characterOid = command.characterOid,};

      // Prevent self broadcast,
      // this prevents the double pickup UI bug for the invoker)
      this->BroadcastExceptCharacterUid(
        raceInstance,
        notify,
        clientContext.characterUid);

      break;
    }
  }

  // Notify all clients in the room that this item has been picked up
  const protocol::AcCmdGameRaceItemGet get{
    .characterOid = command.characterOid,
    .itemId = command.itemDeckId,
    .itemType = deck.currentItem,
  };
  this->Broadcast(raceInstance, get);

  // Erase the item from item instances of each client.
  for (auto& raceRacer : raceInstance.GetTracker().GetRacers() | std::views::values)
  {
    raceRacer.trackedDecks.erase(deck.oid);
  }
}

// Magic Targeting System Implementation for Bolt
void RaceNetworkHandler::HandleStartMagicTarget(
  const ClientId clientId,
  const protocol::AcCmdCRStartMagicTarget& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.casterOid != racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleStartMagicTarget");
    return;
  }

  auto& racers = raceInstance.GetTracker().GetRacers();
  const auto targetIter = std::ranges::find_if(
    racers,
    [&command](const auto& entry)
    {
      return entry.second.oid == command.targetOid;
    });

  if (targetIter == racers.end())
  {
    spdlog::warn("Target OID {} not found in HandleStartMagicTarget", command.targetOid);
    return;
  }

  auto& targetRacer = targetIter->second;

  if (targetRacer.pendingMagicTarget.has_value()
    || race::MagicSystem::IsDowned(targetRacer))
  {
    const protocol::AcCmdRCRemoveMagicTarget removeMagicTarget{
      .effectInstanceId = command.effectInstanceId,
      .casterOid = command.casterOid,
      .targetOid = command.targetOid,
      .targetOid2 = command.targetOid2};
    this->Broadcast(raceInstance, removeMagicTarget);
    return;
  }

  targetRacer.dragonReceivedAt = std::chrono::steady_clock::now();
  targetRacer.pendingMagicTarget = {command.casterOid, command.effectInstanceId};
}

void RaceNetworkHandler::HandleChangeMagicTarget(
  const ClientId clientId,
  const protocol::AcCmdCRChangeMagicTarget& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  if (command.targetOid!= racer.oid)
  {
    spdlog::warn("Character OID mismatch in HandleChangeMagicTarget");
    return;
  }

  if (!racer.pendingMagicTarget.has_value())
  {
    spdlog::warn("Caster does not have dragon in HandleChangeMagicTarget");

    // The client thinks it is carrying a dragon that the server does not know about.
    // Cancel so it drops the phantom, instead of leaving it stuck holding one forever.
    const protocol::AcCmdCRChangeMagicTargetCancel response{
      .effectInstanceId = command.effectInstanceId,
      .casterOid = command.casterOid,
      .targetOid = command.targetOid,
      .targetOid2 = command.targetOid2};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]
      {
        return response;
      });
    return;
  }

  // Find the target racer based on targetOid2
  auto& racers = raceInstance.GetTracker().GetRacers();
  const auto targetIter = std::ranges::find_if(
    racers,
    [&command](const auto& entry)
    {
      return entry.second.oid == command.targetOid2;
    });

  if (targetIter == racers.end())
  {
    spdlog::warn("Target OID {} not found in HandleStartMagicTarget", command.targetOid);
    return;
  }

  auto& targetRacer = targetIter->second;

  // Enforce cooldown: dragon cannot be passed until 5s after it was received
  constexpr auto DragonPassCooldown = std::chrono::milliseconds(500);
  if (std::chrono::steady_clock::now() - racer.dragonReceivedAt < DragonPassCooldown)
  {
    protocol::AcCmdCRChangeMagicTargetCancel response{
      .effectInstanceId = command.effectInstanceId,
      .casterOid = command.casterOid,
      .targetOid = command.targetOid,
      .targetOid2 = command.targetOid2
    };

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]() { return response; });
    return;
  }

  if (targetRacer.pendingMagicTarget.has_value()
    || race::MagicSystem::IsDowned(targetRacer))
  {
    // Send Cancel response
    protocol::AcCmdCRChangeMagicTargetCancel response{
      .effectInstanceId = command.effectInstanceId,
      .casterOid = command.casterOid,
      .targetOid = command.targetOid,
      .targetOid2 = command.targetOid2
    };

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]() { return response; });

    return;
  }

  targetRacer.dragonReceivedAt = std::chrono::steady_clock::now();
  targetRacer.pendingMagicTarget = {command.casterOid, command.effectInstanceId};
  racer.pendingMagicTarget.reset();

  // Send OK response
  protocol::AcCmdCRChangeMagicTargetOK response{
    .effectInstanceId = command.effectInstanceId,
    .casterOid = command.casterOid,
    .targetOid = command.targetOid,
    .targetOid2 = command.targetOid2
  };
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]() { return response; });

  // Send targeting notification to the target
  const protocol::AcCmdCRChangeMagicTargetNotify targetNotify{
    .effectInstanceId = command.effectInstanceId,
    .casterOid = command.casterOid,
    .targetOid = command.targetOid,
    .targetOid2 = command.targetOid2
  };
  this->Broadcast(raceInstance, targetNotify);
}

void RaceNetworkHandler::StripHeldMagicItem(
  RaceInstance& raceInstance,
  tracker::RaceTracker::Racer& targetRacer)
{
  targetRacer.magicItem.reset();

  const protocol::AcCmdCRUseItemSlotNotify notify{
    .magicItemId = 0,
    .unk1 = 0,
    .characterOid = targetRacer.oid};
  this->Broadcast(raceInstance, notify);
}

void RaceNetworkHandler::QueueHeldMagicItemStrip(
  RaceInstance& raceInstance,
  const data::Uid targetCharacterUid,
  const uint32_t generation)
{
  constexpr auto MagicItemStripDelay = std::chrono::milliseconds(500);

  _scheduler.Queue(
    [this, roomUid = raceInstance.GetRoomUid(), targetCharacterUid, generation]
    {
      std::scoped_lock raceInstanceLock(_raceInstancesMutex);
      const auto raceInstanceIter = _raceInstances.find(roomUid);
      if (raceInstanceIter == _raceInstances.cend())
        return;
      auto& raceInstance = raceInstanceIter->second;
      if (not raceInstance.GetTracker().IsRacer(targetCharacterUid))
        return;

      auto& targetRacer = raceInstance.GetTracker().GetRacer(targetCharacterUid);
      if (not targetRacer.magicItem.has_value())
        return;

      if (targetRacer.magicItemGeneration != generation)
        return;

      StripHeldMagicItem(raceInstance, targetRacer);
    },
    Scheduler::Clock::now() + MagicItemStripDelay);
}

void RaceNetworkHandler::HandleActivateSkillEffect(
  const ClientId clientId,
  const protocol::AcCmdCRActivateSkillEffect& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);

  auto& targetRacer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // A racer reports the magic that landed on themselves. Everything below assumes the
  // sender is the target, so reject any attempt to report a hit on somebody else.
  // TODO: Revise this in NPC races
  if (command.targetOid != targetRacer.oid)
  {
    spdlog::warn("Client tried to perform action on behalf of different racer");
    return;
  }

  const auto& magicRegistry = GetServerInstance().GetMagicRegistry();
  const auto* magicSlotInfo = &magicRegistry.GetSlotInfoByEffectId(command.effectId);

  const bool isDarkFireBurning = targetRacer.effects[race::SkillEffect::DarkFire]
    || targetRacer.effects[race::SkillEffect::DarkFireCritical];

  if (isDarkFireBurning && magicSlotInfo->criticalByDarkFire)
    magicSlotInfo = &magicRegistry.GetSlotInfo(magicSlotInfo->criticalType);

  if (race::MagicSystem::IsIceWall(magicSlotInfo->type))
  {
    this->Broadcast(
      raceInstance,
      protocol::AcCmdRCMagicExpire{
        .magicType = magicSlotInfo->type,
        .firstObstacleInstanceId = command.effectInstanceId,
        .obstacleInstanceCount = 1,
        .breakdown = 1});
  }

  const EffectVerdict verdict = this->ScheduleSkillEffect(
    raceInstance,
    command.attackerOid,
    command.targetOid,
    *magicSlotInfo,
    command.effectInstanceId);

  if (verdict == EffectVerdict::Applied)
  {
    if (magicSlotInfo->attackRank > 1 && targetRacer.pendingMagicTarget)
    {
      const protocol::AcCmdRCRemoveMagicTarget removeMagicTarget{
        .effectInstanceId = targetRacer.pendingMagicTarget->effectInstanceId,
        .casterOid = targetRacer.pendingMagicTarget->casterOid,
        .targetOid = command.targetOid,
        .targetOid2 = command.targetOid};
      this->Broadcast(raceInstance, removeMagicTarget);
      targetRacer.pendingMagicTarget.reset();
    }

    // TODO:: Add a Conditional for the SystemContent that can enable/disable this behavior
    if (magicSlotInfo->removeMagic == 1 && targetRacer.magicItem.has_value()
      && not PotentialSystem::PreventsMagicItemLoss(
        GetServerInstance().GetHorseRegistry(), targetRacer.potential))
    {
      QueueHeldMagicItemStrip(
        raceInstance, clientContext.characterUid, targetRacer.magicItemGeneration);
    }

    if (magicSlotInfo->basicType == race::MagicType::FireBall)
      GrantOnePlusOneMagicItem(raceInstance, command.attackerOid);
  }
  if (magicSlotInfo->basicType == race::MagicType::Summon)
    targetRacer.pendingMagicTarget.reset();
}

void RaceNetworkHandler::HandleOpCmd(
  ClientId clientId,
  const protocol::AcCmdCROpCmd& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::vector<std::string> feedback;

  const auto result = GetServerInstance().GetChatSystem().ProcessChatMessage(
    clientContext.characterUid, "//" + command.command);

  if (not result.commandVerdict)
  {
    return;
  }

  for (const auto& response : result.commandVerdict->result)
  {
    _commandServer.QueueCommand<protocol::RanchCommandOpCmdOK>(
      clientId,
      [response = std::move(response)]()
      {
        return protocol::RanchCommandOpCmdOK{
          .feedback = response,
          .observerState = protocol::RanchCommandOpCmdOK::Observer::Disabled};
      });
  }
}

void RaceNetworkHandler::HandleChangeSkillCardPresetId(
  const ClientId clientId,
  const protocol::AcCmdCRChangeSkillCardPresetID& command)
{
  if (command.setId < 0 || command.setId > 2)
  {
    // TODO: throw? return?
    // Calling client requested to change skill preset to something out of range
    // 0 < setId < 3
    return;
  }

  if (command.gamemode != protocol::GameMode::Speed && command.gamemode != protocol::GameMode::Magic)
  {
    // TODO: throw? return?
    // Gamemode can either be speed (1) or magic (2)
    return;
  }

  const auto& clientContext = GetClientContext(clientId);
  GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&command](data::Character& character)
    {
      // Get skill sets by gamemode
      auto& skillSets =
        command.gamemode == protocol::GameMode::Speed ? character.skills.speed() :
        command.gamemode == protocol::GameMode::Magic ? character.skills.magic() :
        throw std::runtime_error("Invalid gamemode");
      // Set character's active skill set in the record
      skillSets.activeSetId = command.setId;
    }
  );

  // No response command
}

void RaceNetworkHandler::RemoveEffect(
  RaceInstance& raceInstance,
  tracker::RaceTracker::Racer& racer,
  uint32_t effectId)
{
  if (not race::MagicSystem::IsValidSkillEffectId(effectId))
  {
    spdlog::error("RemoveEffect: effectId {} is not usable", effectId);
    return;
  }
  racer.effects[effectId] = false;
  ++racer.effectGenerations[effectId];

  const protocol::AcCmdRCRemoveSkillEffect removeSkillEffect{
    .characterOid = racer.oid,
    .effectId = effectId,
    .targetOid = racer.oid,
    .unk1 = 0};
  this->Broadcast(raceInstance, removeSkillEffect);
}

void RaceNetworkHandler::StripEffectsOnAttack(
  RaceInstance& raceInstance,
  tracker::RaceTracker::Racer& targetRacer,
  const registry::Magic::SlotInfo& magicSlotInfo)
{
  for (const auto& slot : GetServerInstance().GetMagicRegistry().GetSlotInfoMap() | std::views::values)
  {
    // Slots that carry no real effect (e.g. positional magic) have an unusable id.
    if (not race::MagicSystem::IsValidSkillEffectId(slot.skillEffectId))
      continue;

    if (race::MagicSystem::IsStrippedByAttack(magicSlotInfo, slot)
      && targetRacer.effects[slot.skillEffectId])
    {
      RemoveEffect(raceInstance, targetRacer, slot.skillEffectId);
    }
  }
}

void RaceNetworkHandler::DrainGauge(
  const data::Uid targetCharacterUid,
  tracker::RaceTracker::Racer& targetRacer)
{
  targetRacer.starPointValue = 0;

  // The gauge is only ever reported to its own owner, who may have already left.
  const auto targetClientId = FindClientIdByCharacterUid(targetCharacterUid);

  if (not targetClientId)
    return;

  const protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = targetRacer.oid,
    .starPointValue = targetRacer.starPointValue,
    .giveMagicItem = false};

  _commandServer.QueueCommand<decltype(starPointResponse)>(
    *targetClientId,
    [starPointResponse]
    {
      return starPointResponse;
    });
}

void RaceNetworkHandler::QueueEffectExpiry(
  RaceInstance& raceInstance,
  const data::Uid targetCharacterUid,
  const tracker::Oid targetOid,
  const uint32_t effectId,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const uint32_t generation,
  const uint32_t effectDurationMs)
{
  _scheduler.Queue(
    [this, roomUid = raceInstance.GetRoomUid(), targetCharacterUid, targetOid, effectId, generation,
      attackRank = magicSlotInfo.attackRank]
    {
      std::scoped_lock raceInstanceLock(_raceInstancesMutex);
      const auto raceInstanceIter = _raceInstances.find(roomUid);
      if (raceInstanceIter == _raceInstances.cend())
        return;

      auto& raceInstance = raceInstanceIter->second;

      if (not raceInstance.GetTracker().IsRacer(targetCharacterUid))
        return;

      auto& racer = raceInstance.GetTracker().GetRacer(targetCharacterUid);

      // If the generation has changed, this effect was extended
      if (racer.effectGenerations[effectId] != generation)
        return;

      racer.effects[effectId] = false;
      // Only clear attackRank if a higher-rank attack hasn't replaced this one
      if (attackRank > 0 && racer.attackRank == attackRank)
        racer.attackRank = 0;

      const protocol::AcCmdRCRemoveSkillEffect removeSkillEffect{
        .characterOid = targetOid,
        .effectId = effectId,
        .targetOid = targetOid,
        .unk1 = 0};
      this->Broadcast(raceInstance, removeSkillEffect);
    },
    Scheduler::Clock::now() + std::chrono::milliseconds(effectDurationMs));
}

RaceNetworkHandler::EffectVerdict RaceNetworkHandler::ScheduleSkillEffect(
  RaceInstance& raceInstance,
  tracker::Oid attackerOid, tracker::Oid targetOid,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const uint16_t effectInstanceId)
{
  auto& racers = raceInstance.GetTracker().GetRacers();

  const auto targetRacerIter = race::MagicSystem::FindRacerByOid(racers, targetOid);
  if (targetRacerIter == racers.end())
    return EffectVerdict::Failed;

  // Guard against a misconfigured skillEffectId crashing the server, or every client in
  // the room — the add/remove skill effect commands are broadcast to all of them.
  if (not race::MagicSystem::IsValidSkillEffectId(magicSlotInfo.skillEffectId))
  {
    spdlog::error(
      "ScheduleSkillEffect: magic type {} has unusable skillEffectId {} (max {}, 4 unused)",
      magicSlotInfo.type,
      magicSlotInfo.skillEffectId,
      tracker::RaceTracker::Racer::EffectCount - 1);
    return EffectVerdict::Failed;
  }

  const data::Uid targetCharacterUid = targetRacerIter->first;
  auto& targetRacer = targetRacerIter->second;

  const auto& magicRegistry = GetServerInstance().GetMagicRegistry();

  const auto resolution = race::MagicSystem::ResolveEffect(
    magicRegistry, magicSlotInfo, targetRacer);

  const auto attackerRacerIter = race::MagicSystem::FindRacerByOid(racers, attackerOid);
  const uint32_t effectDurationMs = race::MagicSystem::ComputeEffectDurationMs(
    magicRegistry,
    GetServerInstance().GetHorseRegistry(),
    magicSlotInfo,
    attackerRacerIter != racers.end() ? &attackerRacerIter->second : nullptr,
    targetRacer);
  // TODO: Verify if characterOid and targetOid should be the same once we have NPCs
  this->Broadcast(
    raceInstance,
    protocol::AcCmdRCAddSkillEffect{
      .characterOid = targetOid,
      .effectId = resolution.effectId,
      .targetOid = targetOid,
      .attackerOid = attackerOid,
      .unk2 = effectInstanceId,
      .unk3 = resolution.isDuplicated ? 1u : 0u,
      .shieldEffect = protocol::AcCmdRCAddSkillEffect::ShieldEffect{
        .unk0 = resolution.shieldBlocks ? 2u : 0u,
        .unk1 = 0},
      .boostEffectMs = effectDurationMs});

  if (resolution.shieldBlocks)
    return EffectVerdict::Shielded;

  if (resolution.isDuplicated)
    return EffectVerdict::Duplicated;

  targetRacer.effects[resolution.effectId] = true;
  const uint32_t generation = ++targetRacer.effectGenerations[resolution.effectId];
  if (magicSlotInfo.attackRank > 0)
    targetRacer.attackRank = magicSlotInfo.attackRank;

  const bool isAttack = magicSlotInfo.attackValue > 0;
  if (isAttack)
  {
    StripEffectsOnAttack(raceInstance, targetRacer, magicSlotInfo);

    if (race::MagicSystem::DrainsGaugeOnHit(magicSlotInfo))
      DrainGauge(targetCharacterUid, targetRacer);
  }

  QueueEffectExpiry(
    raceInstance,
    targetCharacterUid,
    targetOid,
    resolution.effectId,
    magicSlotInfo,
    generation,
    effectDurationMs);

  return EffectVerdict::Applied;
}

void RaceNetworkHandler::HandleInviteUser(
  ClientId clientId,
  const protocol::AcCmdCRInviteUser& command)
{
  const auto& clientContext = GetClientContext(clientId);

  protocol::AcCmdCRInviteUserCancel cancel{};
  cancel.recipientCharacterUid = command.recipientCharacterUid;
  cancel.recipientCharacterName = command.recipientCharacterName;

  // Check if character by that uid is online
  const auto clientOpt = GetServerInstance().GetMessengerDirector().GetClientByCharacterUid(
    command.recipientCharacterUid);
  if (not clientOpt.has_value())
  {
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Check if there's a name mismatch
  // TODO: this could benefit from caching the character name within the messenger client context
  bool isNameMatch{false};
  GetServerInstance().GetDataDirector().GetCharacter(command.recipientCharacterUid).Immutable(
    [&isNameMatch, recipientCharacterName = command.recipientCharacterName](const data::Character& character)
    {
      isNameMatch = character.name() == recipientCharacterName;
    });

  if (not isNameMatch)
  {
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  // Race director invites are generally more relaxed, you can invite characters that are in
  // either a ranch or race waiting room

  // Sanity check if character can be invited (is away, online or in waiting room)
  const auto& recipientStatus = clientOpt.value().clientContext.presence.status;
  bool canInvite = recipientStatus == protocol::Status::Away or
    recipientStatus == protocol::Status::Online or
    recipientStatus == protocol::Status::WaitingRoom;

  if (not canInvite)
  {
    // Cannot invite character
    spdlog::warn("Character '{}', which is in a race waiting room, tried to invite character '{}' who is not in an invitable state",
      clientContext.characterUid,
      command.recipientCharacterUid);
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  protocol::AcCmdCRInviteUserOK response{};
  response.recipientCharacterUid = command.recipientCharacterUid;
  response.recipientCharacterName = command.recipientCharacterName;

  _commandServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void RaceNetworkHandler::HandleRequestUser(
  const ClientId clientId,
  const protocol::AcCmdCRRequestUser& command)
{
  const auto& clientContext = GetClientContext(clientId);

  const auto& invokerCharacterUid = clientContext.characterUid;

  const auto invokerRecord = _serverInstance.GetDataDirector().GetCharacter(invokerCharacterUid);
  if (not invokerRecord)
    return;

  bool isAdmin = false;
  std::string invokerCharacterName{};
  invokerRecord.Immutable([&isAdmin, &invokerCharacterName](const data::Character& character)
    {

      isAdmin = character.role() != data::Character::Role::User;
      invokerCharacterName = character.name();
    });
  const auto& userName = clientContext.userName;

  if (not isAdmin)
  {
    spdlog::warn("User '{}'('{}'), which is not an admin, tried to summon character '{}'",
      userName,
      invokerCharacterName,
      command.characterName);
    return;
  }

  protocol::AcCmdCRRequestUserCancel cancel{};
  cancel.force= command.force;
  cancel.characterName = command.characterName;
  cancel.roomUid = command.roomUid;
  cancel.ranchUid = command.ranchUid;

  const data::Uid characterUid = GetServerInstance()
    .GetDataDirector()
    .GetDataSource()
    .RetrieveCharacterUidByName(command.characterName);

  if (characterUid == data::InvalidUid)
  {
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  try
  {
    const auto clientOpt = GetServerInstance()
      .GetLobbyDirector().GetUserByCharacterUid(characterUid);
  }
  catch (const std::exception&)
  {
    _commandServer.QueueCommand<decltype(cancel)>(clientId, [cancel](){ return cancel; });
    return;
  }

  GetServerInstance().GetRaceDirector().NotifySummonCharacter(
    characterUid,
    command.force,
    command.characterName,
    command.roomUid,
    command.ranchUid);

  GetServerInstance().GetRanchDirector().SummonCharacter(
    characterUid,
    command.force,
    command.characterName,
    command.roomUid,
    command.ranchUid);

  protocol::AcCmdCRRequestUserOK response{
    {
      .force= command.force,
      .characterName = command.characterName,
      .roomUid = command.roomUid,
      .ranchUid = command.ranchUid,}};


  _commandServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void RaceNetworkHandler::HandleKickUser(
  ClientId clientId,
  const protocol::AcCmdCRKick& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::unique_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext, false);

  std::string kickerCharacterName;
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Immutable(
    [&kickerCharacterName](const data::Character& character)
    {
      kickerCharacterName = character.name();
    });

  std::string targetCharacterName;
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Immutable(
    [&targetCharacterName](const data::Character& character)
    {
      targetCharacterName = character.name();
    });

  const auto& kickerUserName = clientContext.userName;
  const auto targetUserName = GetClientContextByCharacterUid(command.characterUid).userName;

  // Only the room master may kick players.
  data::Uid roomMasterUid{data::InvalidUid};
  raceInstance.GetRoom([&roomMasterUid](Room& room)
  {
    roomMasterUid = room.GetRoomDetails().masterUid;
  });

  if (clientContext.characterUid != roomMasterUid)
  {
    spdlog::warn(
      "Player {} ({}) tried to kick Player {} ({}) but is not the room master.",
      kickerUserName,
      kickerCharacterName,
      targetUserName,
      targetCharacterName);
    return;
  }

  // Prevent self-kick.
  if (command.characterUid == clientContext.characterUid)
  {
    spdlog::warn(
      "Player {} ({}) tried to kick themselves.",
      kickerUserName,
      kickerCharacterName);
    return;
  }

  // Verify the target character is actually in this room.
  bool isTargetInRoom{false};
  raceInstance.GetRoom(
    [&isTargetInRoom, targetCharacterUid = command.characterUid](const Room& room)
    {
      isTargetInRoom = room.GetPlayers().contains(targetCharacterUid);
    });

  if (!isTargetInRoom)
  {
    spdlog::warn(
      "Player {} ({}) tried to kick Player {} ({}) who is not in the room.",
      kickerUserName,
      kickerCharacterName,
      targetUserName,
      targetCharacterName);
    return;
  }

  // GameMasters (role 2) cannot be kicked.
  bool targetIsGameMaster = false;
  _serverInstance.GetDataDirector().GetCharacter(command.characterUid).Immutable(
    [&targetIsGameMaster](const data::Character& character)
    {
      targetIsGameMaster = character.role() == data::Character::Role::GameMaster;
    });

  if (targetIsGameMaster)
  {
    spdlog::info(
      "Player {} ({}) tried to kick Player {} ({}) who is a GameMaster.",
      kickerUserName,
      kickerCharacterName,
      targetUserName,
      targetCharacterName);
    return;
  }

  // Retrieve the clientId of the targeted player (IMPORTANT)
  ClientId targetClientId{};
  try
  {
    targetClientId = GetClientIdByCharacterUid(command.characterUid);
  }
  catch (const std::exception& ex)
  {
    spdlog::warn(
      "Player {} ({}) tried to kick Player {} ({}) but no active client was found: {}",
      kickerUserName,
      kickerCharacterName,
      targetUserName,
      targetCharacterName,
      ex.what());
    return;
  }

  spdlog::info(
    "Player {} ({}) kicked Player {} ({}) from [Room {}].",
    kickerUserName,
    kickerCharacterName,
    targetUserName,
    targetCharacterName,
    clientContext.roomUid);

  // Broadcast the kick notification to all clients in the room.
  const protocol::AcCmdCRKickNotify notify{
    .characterUid = command.characterUid};
  this->Broadcast(raceInstance, notify);

  lock.unlock();
  HandleLeaveRoom(targetClientId);
}

//! Handles team gauge-related logic, including speed and theoretically guild battles.
//! Primary logic reference: `TeamSpurGaugeInfo` in libconfig
void RaceNetworkHandler::HandleTeamGauge(const ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  // If race teammode is not team then we are done here.
  // This is necessary to ensure no team-related logic is handled when spur logic is handled.
  // Sanity check for speed gamemode
  bool isTeamMode = parameters.teamMode == protocol::TeamMode::Team;
  bool isSpeedGameMode = parameters.gameMode == protocol::GameMode::Speed;
  if (not isTeamMode or not isSpeedGameMode)
    return;

  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  auto& blueTeam = raceInstance.GetTracker().blueTeam;
  auto& redTeam = raceInstance.GetTracker().redTeam;
  auto& team =
    racer.team == tracker::RaceTracker::Racer::Team::Red ? redTeam :
    racer.team == tracker::RaceTracker::Racer::Team::Blue ? blueTeam :
    throw std::runtime_error(
      std::format(
        "Racer character uid {} is on unrecognised team {}",
        clientContext.characterUid,
        static_cast<uint32_t>(racer.team)));

  // If the invoker's team gauge is locked (beaten by opposing team's spur), reject gauge fill.
  if (team.gaugeLocked)
    return;

  // Track team boost count for gauge fill rate calculation.
  team.boostCount += 1;

  //! Boost fill rates, scaled with team count, iterated with boost count.
  //! Reference: `TeamSpurGaugeInfo` in libconfig
  // TODO: put this in the config somewhere
  const std::vector<float> baseFillRates{
    1.25f,
    2.50f,
    3.00f,
    3.75f,
    5.50f,
    6.50f};

  // Get team size from the racer tracker (immutable for the race duration).
  // Use the max of the two team sizes to handle potentially unbalanced teams.
  uint32_t redTeamCount = 0;
  uint32_t blueTeamCount = 0;
  for (const auto& _ : raceInstance.GetTracker().GetRacers() | std::views::values)
  {
    if (_.team == tracker::RaceTracker::Racer::Team::Red)
      ++redTeamCount;
    else if (_.team == tracker::RaceTracker::Racer::Team::Blue)
      ++blueTeamCount;
  }
  const auto teamSize = std::max(redTeamCount, blueTeamCount);

  // boostCount was incremented above; 1st boost should correspond to index 0 (FillRate_1).
  const auto fillRateIndex = std::min(
    team.boostCount - 1,
    static_cast<uint32_t>(baseFillRates.size() - 1));
  protocol::AcCmdRCTeamSpurGauge spur{
    .team = racer.team,
    .markerSpeed = baseFillRates[fillRateIndex] * teamSize, // Base fill rate * team size
    .unk5 = 0 // TODO: identify use
  };

  //! Base points per boost per team member in 1/10th scale
  //! (Point_1 in TeamSpurGaugeInfo is 3.5 per team member = 35 internal points)
  constexpr uint32_t BaseBoostPointsPerMember = 35;
  const uint32_t pointsPerBoost = BaseBoostPointsPerMember * teamSize;

  //! Max team spur points based on team size (1v1, 2v2, 3v3, 4v4)
  //! from libconfig `TeamSpurGaugeInfo` (`TeamSpurMax` * 10)
  static constexpr std::array<uint32_t, 4> MaxTeamSpurPoints{250, 400, 650, 1000};
  const uint32_t teamSizeIndex = std::clamp(
    teamSize,
    1u,
    static_cast<uint32_t>(MaxTeamSpurPoints.size())) - 1;
  const uint32_t maxPoints = MaxTeamSpurPoints[teamSizeIndex];

  auto& blueTeamPoints = blueTeam.points;
  auto& redTeamPoints = redTeam.points;
  auto& teamPoints =
    racer.team == tracker::RaceTracker::Racer::Team::Red ? redTeamPoints :
    racer.team == tracker::RaceTracker::Racer::Team::Blue ? blueTeamPoints :
    throw std::runtime_error(
      std::format(
        "Racer character uid {} is on unrecognised team {}",
        clientContext.characterUid,
        static_cast<uint32_t>(racer.team)));

  spur.currentPoints = teamPoints / 10.0f;
  teamPoints = std::min(
    maxPoints,
    teamPoints + pointsPerBoost);
  spur.newPoints = teamPoints / 10.0f;

  // If any of the teams got max points to spur, reset points and broadcast team spur
  bool isTeamRed = racer.team == tracker::RaceTracker::Racer::Team::Red;
  bool isTeamBlue = racer.team == tracker::RaceTracker::Racer::Team::Blue;

  // Can invoker's team spur
  bool isTeamSpur = false;
  // Check if either red or blue team points have hit max
  if (redTeamPoints >= maxPoints or blueTeamPoints >= maxPoints)
  {
    // If any (red or blue) team can spur.
    // Team check is added for additional validation.
    isTeamSpur = (isTeamRed and redTeamPoints >= maxPoints) or
      (isTeamBlue and blueTeamPoints >= maxPoints);

    // Reset points
    redTeamPoints = 0;
    blueTeamPoints = 0;
  }

  // If any of the teams can spur, schedule a spur/reset event.
  if (isTeamSpur)
  {
    // Reset team boost counters
    redTeam.boostCount = 0;
    blueTeam.boostCount = 0;

    // Lock the spurring team's gauge so it cannot fill during the spur.
    auto& spurringTeamInfo =
      racer.team == tracker::RaceTracker::Racer::Team::Red ? redTeam :
      racer.team == tracker::RaceTracker::Racer::Team::Blue ? blueTeam :
      throw std::runtime_error(
        std::format(
          "Unrecognised racer team '{}'",
          static_cast<uint32_t>(racer.team)));
    spurringTeamInfo.gaugeLocked = true;

    // TODO: put this into the config somewhere
    // When to begin the spur/reset event.
    // Reference: `TeamSpurGaugeInfo`/`ReduceWaitTime` in libconfig
    constexpr auto SpurStartDelay = std::chrono::milliseconds(1500);

    _scheduler.Queue(
      [this, roomUid = raceInstance.GetRoomUid(), &racer, &spurringTeamInfo, maxPoints, teamSize]()
      {
        std::scoped_lock lock(_raceInstancesMutex);
        const auto raceInstanceIter = _raceInstances.find(roomUid);;
        if (raceInstanceIter == _raceInstances.cend())
          return;

        const auto& raceInstance = raceInstanceIter->second;

        const float BaseLoseTeamSpurConsumeRate = -10.0f;
        const float BaseWinTeamSpurConsumeRate = -2.5f;

        // Reset boost gauge for the team that lost it.
        protocol::AcCmdRCTeamSpurGauge beatenSpur{
          .team =
            // This red/blue swap is intentional, if team A wins, team B is punished and reset.
            racer.team == tracker::RaceTracker::Racer::Team::Red ? tracker::RaceTracker::Racer::Team::Blue :
            racer.team == tracker::RaceTracker::Racer::Team::Blue ? tracker::RaceTracker::Racer::Team::Red :
            throw std::runtime_error(
              std::format(
                "Unrecognised racer team '{}'",
                static_cast<uint32_t>(racer.team))),
          .currentPoints = 0.0f,
          .newPoints = 0.0f,
          .markerSpeed = BaseLoseTeamSpurConsumeRate * teamSize, // Scales with `LoseTeamSpurConsumeRate`
          .unk5 = 3 // Reset gauge and markers.
        };

        // Trigger spur for the team that has won it.
        protocol::AcCmdRCTeamSpurGauge successfulSpur{
          .team = racer.team,
          .currentPoints = maxPoints / 10.0f,
          .newPoints = 0.0f,
          .markerSpeed = BaseWinTeamSpurConsumeRate * teamSize, // Scales with `WinTeamSpurConsumeRate`
          .unk5 = 0
        };

        // Spur duration = (maxPoints / 10.0f) / (abs(consumeRate) * teamSize)
        // For example: 25.0f / (2.5f * 1) = 10s for a team of 1.
        const float spurDurationSeconds =
          (maxPoints / 10.0f) / (std::abs(BaseWinTeamSpurConsumeRate) * teamSize);

        // Schedule unlock of the spurring team's gauge after the spur completes.
        _scheduler.Queue(
          [&spurringTeamInfo]()
          {
            spurringTeamInfo.gaugeLocked = false;
          },
          Scheduler::Clock::now() + std::chrono::milliseconds(
            static_cast<int64_t>(spurDurationSeconds * 1000)));

        // Broadcast losing team's gauge status
        this->Broadcast(raceInstance, beatenSpur);
        // Broadcast winning team's gauge status
        this->Broadcast(raceInstance, successfulSpur);
      },
      Scheduler::Clock::now() + SpurStartDelay);
  }

  // Broadcast invoker's team gauge status
  this->Broadcast(raceInstance, spur);
}

void RaceNetworkHandler::HandleTriggerizeAct(
  ClientId clientId,
  const protocol::AcCmdCRTriggerizeAct& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  const bool isSpeedGameMode = parameters.gameMode == protocol::GameMode::Speed;

  const auto& mapBlockInfo = _serverInstance.GetCourseRegistry()
    .GetMapBlockInfo(
      raceInstance.GetMapBlockId());

  const bool isAdvMap = mapBlockInfo.trainingFee > 0;

  // The racer is neither in a speed mode or adv map
  if (not isSpeedGameMode or not isAdvMap)
  {
    spdlog::warn("Character '{}' tried to trigger an interactive object but is not in a speed adv map race.",
      clientContext.characterUid);
    return;
  }

  // TODO: check if the object ID is within range
  // TODO: check if the event ID is valid

  const protocol::AcCmdCRTriggerizeAct response{
    .unk0 = 1, // Setting this to either 1 or 2 satisfies the conditional in the handler
    .unk1 = command.unk1,
    .unk2 = command.unk2};
  this->BroadcastExceptCharacterUid(raceInstance, response, clientContext.characterUid);
}

void RaceNetworkHandler::HandleGameCreateClientItem(
  ClientId clientId,
  const protocol::AcCmdCRGameCreateClientItem& command)
{
  spdlog::debug(
    "AcCmdCRGameCreateClientItem: {} {} [{}, {}, {}] [{}, {}, {}, {}]",
    command.someonesOid,
    command.unk1,
    command.position.x, command.position.y, command.position.z,
    command.unk3[0], command.unk3[1], command.unk3[2], command.unk3[3]);

  if (command.unk1 != 0)
    // Only egg spawning (unk1 == 0) is implemented
    throw new std::runtime_error("AcCmdCRGameCreateClientItem::unk1 != 0, other case not implemented");

  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = GetRaceInstance(clientContext);

  // Get region for this map.
  const auto& mapBlockInfo = _serverInstance.GetCourseRegistry().GetMapBlockInfo(
    raceInstance.GetMapBlockId());
  const auto regionEggs = _serverInstance.GetPetRegistry().GetEggsByRegion(mapBlockInfo.region);
  if (regionEggs.empty())
    return;

  // Weighted random selection using ObtainRatio (owned eggs still included in weight pool).
  std::vector<uint32_t> weights;
  weights.reserve(regionEggs.size());
  for (const auto& egg : regionEggs)
    weights.push_back(egg.obtainRatio);

  std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
  const auto& selectedEgg = regionEggs[dist(server::util::GetRandomEngine())];

  // Check if the player already owns this egg.
  bool alreadyOwned = false;
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);
  characterRecord.Immutable([&](const data::Character& character)
  {
    alreadyOwned = _serverInstance.GetItemSystem().HasItem(character, selectedEgg.tid);
  });

  // If player already owns the egg, do nothing.
  if (alreadyOwned)
    return;

  // Add to per-racer event item tracker regardless of ownership.
  auto& item = raceInstance.GetTracker().AddEventItem(clientContext.characterUid);
  item.position = command.position;
  item.itemType = selectedEgg.deckItemId;
}

} // namespace server
