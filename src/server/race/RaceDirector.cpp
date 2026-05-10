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

#include "server/race/RaceDirector.hpp"

#include "server/ServerInstance.hpp"

#include <libserver/data/helper/ProtocolHelper.hpp>

#include <boost/container_hash/hash.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include <bitset>

namespace server
{

namespace
{

static const server::registry::Magic::SlotInfo RandomMagicItem(ServerInstance& serverInstance, tracker::RaceTracker::Racer& racer)
{
  const auto& itemPool = (racer.team == tracker::RaceTracker::Racer::Team::Solo
    ? serverInstance.GetMagicRegistry().GetSoloPool()
    : serverInstance.GetMagicRegistry().GetTeamPool());
  static std::random_device rd;

  // Build weights: Lightning (type 18) gets a reduced roll chance.
  // TODO: Replace with a proper per-spell weight system.
  std::vector<uint32_t> weights;
  weights.reserve(itemPool.size());
  for (const uint32_t type : itemPool)
    weights.push_back(type == 18 ? 1u : 3u);

  std::discrete_distribution<size_t> distribution(weights.begin(), weights.end());
  auto magicSlotInfo = serverInstance.GetMagicRegistry().GetSlotInfo(itemPool[distribution(rd)]);
  if ((rand() % 100) < 5)
  {
    magicSlotInfo = serverInstance.GetMagicRegistry().GetSlotInfo(magicSlotInfo.criticalType);
  }
  return magicSlotInfo;
}

} // anon namespace

RaceDirector::RaceDirector(ServerInstance& serverInstance)
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

void RaceDirector::Initialize()
{
  spdlog::debug(
    "Race server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  test = std::thread([this]()
  {
    std::unordered_set<asio::ip::udp::endpoint> _clients;

    asio::io_context ioCtx;
    asio::ip::udp::socket skt(
      ioCtx,
      asio::ip::udp::endpoint(
        asio::ip::address_v4::loopback(),
        10500));

    asio::streambuf readBuffer;
    asio::streambuf writeBuffer;

    while (run_test)
    {
      const auto request = readBuffer.prepare(1024);
      asio::ip::udp::endpoint sender;

      try
      {
        const size_t bytesRead = skt.receive_from(request, sender);

        struct RelayHeader
        {
          uint16_t member0{};
          uint16_t member1{};
          uint16_t member2{};
        };

        const auto response = writeBuffer.prepare(1024);

        RelayHeader* header = static_cast<RelayHeader*>(response.data());
        header->member2 = 1;

        for (const auto idx : std::views::iota(0ull, bytesRead))
        {
          static_cast<std::byte*>(response.data())[idx + sizeof(RelayHeader)] = static_cast<const std::byte*>(
            request.data())[idx];
        }

        writeBuffer.commit(bytesRead + sizeof(RelayHeader));
        readBuffer.consume(bytesRead + sizeof(RelayHeader));

        for (auto& client : _clients)
        {
          if (client == sender)
            continue;

          writeBuffer.consume(
            skt.send_to(writeBuffer.data(), client));
        }

        if (not _clients.contains(sender))
          _clients.insert(sender);

      } catch (const std::exception&) {
      }
    }
  });
  test.detach();

  _commandServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void RaceDirector::Terminate()
{
  run_test = false;
  _commandServer.EndHost();
}

void RaceDirector::Tick()
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
  for (auto& [raceUid, raceInstance] : _raceInstances)
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

void RaceDirector::NotifyRequestUser(
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

void RaceDirector::BroadcastChangeRoomOptions(
  const data::Uid& roomUid,
  const protocol::AcCmdCRChangeRoomOptionsNotify notify)
{
  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [this, &notify](const Room& room)
    {
      for (const auto& [characterUid, player] : room.GetPlayers())
      {
        _commandServer.QueueCommand<protocol::AcCmdCRChangeRoomOptionsNotify>(
          player.GetClientId(),
          [notify]()
          {
            return notify;
          });
      }
    });
}

void RaceDirector::HandleClientConnected(ClientId clientId)
{
  _clients.try_emplace(clientId);

  spdlog::debug(
    "Client {} connected to the race server from {}",
    clientId,
    _commandServer.GetClientAddress(clientId).to_string());
}

void RaceDirector::HandleClientDisconnected(ClientId clientId)
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

void RaceDirector::DisconnectCharacter(data::Uid characterUid)
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

size_t RaceDirector::GetRoomCount()
{
  std::scoped_lock lock(_raceInstancesMutex);
  return _raceInstances.size();
}

ServerInstance& RaceDirector::GetServerInstance()
{
  return _serverInstance;
}

CommandServer& RaceDirector::GetCommandServer()
{
  return _commandServer;
}

Config::Race& RaceDirector::GetConfig()
{
  return GetServerInstance().GetSettings().race;
}

uint16_t RaceDirector::GetOrCreateP2dId(ClientId clientId)
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

RaceDirector::ClientContext& RaceDirector::GetClientContext(ClientId clientId, bool requireAuthorized)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Race client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthorized && not clientContext.isAuthenticated)
    throw std::runtime_error("Race client is not authenticated");

  return clientContext;
}

ClientId RaceDirector::GetClientIdByCharacterUid(data::Uid characterUid)
{
  for (auto& [clientId, clientContext] : _clients)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientId;
  }

  throw std::runtime_error("Character not associated with any client");
}

RaceDirector::ClientContext& RaceDirector::GetClientContextByCharacterUid(
  data::Uid characterUid)
{
  for (auto& clientContext : _clients | std::views::values)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientContext;
  }

  throw std::runtime_error("Character not associated with any client");
}

RaceInstance& RaceDirector::GetRaceInstance(
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
  
  // If not racing command then we are done here
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

void RaceDirector::HandleEnterRoom(
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
  const auto& parameters = raceInstance.GetParameters();

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
    .isRoomWaiting = parameters.stage == RaceInstance::Parameters::Stage::Waiting,
    .uid = command.roomUid};

  // If race instance exists and race is not waiting then
  // set the elapsed time since loading started
  if (not inserted and parameters.stage != RaceInstance::Parameters::Stage::Waiting)
    response.elapsedTime = static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - parameters.loadingStartTimePoint).count());

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

        // Build the mount equipment.
        protocol::BuildProtocolItems(
          protocolRacer.avatar->equipment,
          *_serverInstance.GetDataDirector().GetItemCache().Get(
            character.expiredEquipment()));

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

void RaceDirector::HandleChangeRoomOptions(
  ClientId clientId,
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
      [this, &command, clientContext](const data::Character& character)
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

  protocol::AcCmdCRChangeRoomOptionsNotify notify{
    .optionsBitfield = command.optionsBitfield,
    .name = command.name,
    .playerCount = command.playerCount,
    .password = command.password,
    .gameMode = command.gameMode,
    .mapBlockId = command.mapBlockId,
    .npcDifficulty = command.npcDifficulty};

  BroadcastChangeRoomOptions(clientContext.roomUid, notify);
}

void RaceDirector::HandleChangeTeam(
  ClientId clientId,
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
  const auto& parameters = raceInstance.GetParameters();

  if (parameters.stage != RaceInstance::Parameters::Stage::Waiting)
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

void RaceDirector::HandleLeaveRoom(ClientId clientId)
{
  protocol::AcCmdCRLeaveRoomOK response{};

  auto& clientContext = GetClientContext(clientId);
  if (clientContext.roomUid == 0)
    return;

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext, false);
  const auto& parameters = raceInstance.GetParameters();

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
    // Try to find the next master.
    auto nextMasterUid{data::InvalidUid};

    // todo: improve this
    // If the room is waiting, pick from room users.
    if (parameters.stage == RaceInstance::Parameters::Stage::Waiting)
    {
      _serverInstance.GetRoomSystem().GetRoom(
        clientContext.roomUid,
        [&nextMasterUid](Room& room)
        {
          for (const auto characterUid : room.GetPlayers() | std::views::keys)
          {
            // todo: assign mastership to the best player
            nextMasterUid = characterUid;
            break;
          }
        });
    }
    else
    {
      for (const auto& characterUid : raceInstance.GetTracker().GetRacers() | std::views::keys)
      {
        nextMasterUid = characterUid;
        break;
      }
    }

    if (nextMasterUid != data::InvalidUid)
    {
      // Set new room master
      raceInstance.GetRoom([nextMasterUid](Room& room)
      {
        auto& details = room.GetRoomDetails();
        details.masterUid = nextMasterUid;
      });

      const auto& newMasterClientContext = GetClientContextByCharacterUid(nextMasterUid);

      std::string newMasterCharacterName;
      _serverInstance.GetDataDirector().GetCharacter(nextMasterUid).Immutable(
        [&newMasterCharacterName](const data::Character& character)
        {
          newMasterCharacterName = character.name();
        });

      spdlog::info("Player {} ({}) became the master of [Room {}] after the previous master left",
        newMasterClientContext.userName,
        newMasterCharacterName,
        clientContext.roomUid);

      // Notify other clients in the room about the new master.
      const protocol::AcCmdCRChangeMasterNotify notify{
        .masterUid = nextMasterUid};
      this->Broadcast(raceInstance, notify);
    }
  }

  {
    // Delete room if empty
    bool roomEmpty{false};
    raceInstance.GetRoom(
      [this, &roomEmpty, roomUid = clientContext.roomUid](const server::Room& room)
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

void RaceDirector::HandleReadyRace(
  ClientId clientId,
  const protocol::AcCmdCRReadyRace&)
{
  const auto& clientContext = GetClientContext(clientId);

  bool isPlayerReady = false;
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&isPlayerReady, characterUid = clientContext.characterUid](Room& room)
    {
      isPlayerReady = room.GetPlayer(characterUid).ToggleReady();
    });

  const protocol::AcCmdCRReadyRaceNotify notify{
    .characterUid = clientContext.characterUid,
    .isReady = isPlayerReady};

  std::scoped_lock lock(_raceInstancesMutex);
  const auto& raceInstance = GetRaceInstance(clientContext, false);
  this->Broadcast(raceInstance, notify);
}

void RaceDirector::PrepareItemSpawners(RaceInstance& raceInstance)
{
  const auto& parameters = raceInstance.GetParameters();
  try {
    const auto& gameModeInfo = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
      static_cast<uint32_t>(parameters.raceGameMode));
    const auto& mapBlockInfo = GetServerInstance().GetCourseRegistry().GetMapBlockInfo(
      parameters.raceMapBlockId);

    // Get the map position offset
    const auto& offset = mapBlockInfo.offset;

    // Spawn items based on map positions and game mode allowed deck IDs
    for (const uint32_t usedDeckItemId : gameModeInfo.usedDeckItemIds)
    {
      const auto& deckItemInfo = GetServerInstance().GetCourseRegistry().GetDeckItemInfo(usedDeckItemId);
      for (const auto& mapDeckItemInstance : mapBlockInfo.deckItems)
      {
        if (mapDeckItemInstance.deckId != usedDeckItemId)
          continue;

        auto& item = raceInstance.GetTracker().AddItem();
        item.itemTypes = deckItemInfo.itemTypes;
        
        // Randomly pick an initial type
        if (!item.itemTypes.empty())
        {
          static std::random_device rd;
          std::uniform_int_distribution<size_t> distribution(0, item.itemTypes.size() - 1);
          item.currentType = item.itemTypes[distribution(rd)];
        }

        item.position[0] = mapDeckItemInstance.position[0] + offset[0];
        item.position[1] = mapDeckItemInstance.position[1] + offset[1];
        item.position[2] = mapDeckItemInstance.position[2] + offset[2];
      }
    }

  }
  catch (const std::exception& e)
  {
    spdlog::warn("Failed to prepare item spawners for room {}: {}",
      raceInstance.GetRoomUid(),
      e.what());
  }
}

void RaceDirector::HandleStartRace(
  const ClientId clientId,
  [[maybe_unused]] const protocol::AcCmdCRStartRace& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext, false);
  auto& parameters = raceInstance.GetParameters();

  // Check if all race requirements are met to start the race
  data::Uid roomMasterUid{data::InvalidUid};
  Room::PreventStartReason preventStartReason{};
  _serverInstance.GetRoomSystem().GetRoom(
    clientContext.roomUid,
    [&preventStartReason, &roomMasterUid, invokerCharacterUid = clientContext.characterUid](Room& room)
    {
      roomMasterUid = room.GetRoomDetails().masterUid;
      if (invokerCharacterUid != roomMasterUid)
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
  uint16_t roomSelectedCourses;
  uint8_t roomGameMode;

  _serverInstance.GetRoomSystem().GetRoom(
    roomUid,
    [&roomSelectedCourses, &roomGameMode, &parameters](Room& room)
    {
      auto& details = room.GetRoomDetails();

      parameters.raceGameMode = static_cast<protocol::GameMode>(details.gameMode);
      parameters.raceTeamMode = static_cast<protocol::TeamMode>(details.teamMode);
      parameters.raceMissionId = details.missionId;

      roomGameMode = static_cast<uint8_t>(details.gameMode);
      roomSelectedCourses = details.courseId;
    });

  constexpr uint32_t AllMapsCourseId = 10000;
  constexpr uint32_t NewMapsCourseId = 10001;
  constexpr uint32_t HotMapsCourseId = 10002;

  if (roomSelectedCourses == AllMapsCourseId
    || roomSelectedCourses == NewMapsCourseId
    || roomSelectedCourses == HotMapsCourseId)
  {
    const auto& gameMode = _serverInstance.GetCourseRegistry().GetCourseGameModeInfo(
      roomGameMode);
    if (not gameMode.mapPool.empty())
    {
      uint32_t masterLevel{};
      // Use the room master's level to filter the maps
      _serverInstance.GetDataDirector().GetCharacter(roomMasterUid).Immutable(
        [&masterLevel](const data::Character& character)
        {
          masterLevel = character.level();
        });

      // Filter out the maps that are above the master's level.
      std::vector<uint16_t> filteredMaps;
      std::copy_if(
        gameMode.mapPool.cbegin(),
        gameMode.mapPool.cend(),
        std::back_inserter(filteredMaps),
        [this, masterLevel](uint32_t mapBlockId)
        {
          try
          {
            const auto& mapBlockInfo = _serverInstance.GetCourseRegistry().GetMapBlockInfo(
              mapBlockId);
            return mapBlockInfo.requiredLevel <= masterLevel;
          }
          catch (const std::exception& e)
          {
            spdlog::warn("Failed to get map block info for mapBlockId {}: {}", mapBlockId, e.what());
            return false;
          }
        });

      // Select a random map from the pool.
      static std::random_device rd;
      std::uniform_int_distribution distribution(0, static_cast<int>(filteredMaps.size() - 1));
      parameters.raceMapBlockId = filteredMaps[distribution(rd)];
    }
    else
    {
      parameters.raceMapBlockId = 1;
    }
  }
  else
  {
    parameters.raceMapBlockId = roomSelectedCourses;
  }

  constexpr uint32_t GameCountdownKey = 17;
  constexpr uint32_t DefaultCountdownMs = 5310;
  const auto& countdown = GetServerInstance().GetSystemContentRegistry().GetValue(GameCountdownKey);
  const protocol::AcCmdRCRoomCountdown roomCountdown{
    .countdown = countdown.has_value() ? countdown.value() : DefaultCountdownMs,
    .mapBlockId = parameters.raceMapBlockId};

  // Broadcast room countdown.
  this->Broadcast(raceInstance, roomCountdown);

  // Clear the tracker before the race.
  raceInstance.GetTracker().Clear();

  // Add the items.
  PrepareItemSpawners(raceInstance);

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

  parameters.stage = RaceInstance::Parameters::Stage::Loading;
  // Mark the start time of when race started loading
  parameters.loadingStartTimePoint = std::chrono::steady_clock::now();
  parameters.stageTimeoutTimePoint = parameters.loadingStartTimePoint + std::chrono::seconds(30);

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
        .raceGameMode = parameters.raceGameMode,
        .raceTeamMode = parameters.raceTeamMode,
        .raceMapBlockId = parameters.raceMapBlockId,
        .p2pRelayAddress = lobbyConfig.advertisement.udpRaceRelay.address.to_uint(),
        .p2pRelayPort = lobbyConfig.advertisement.udpRaceRelay.port,
        .raceMissionId = parameters.raceMissionId,};

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
        [this, &raceInstance, &notify, isEligibleForSkills](const server::Room& room)
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

              const auto bonusSkillIdx = bonusSkillDist(_randomDevice);
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

void RaceDirector::SendStartRaceCancel(
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

void RaceDirector::HandleRaceTimer(
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

void RaceDirector::HandleLoadingComplete(
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
    static std::random_device rd;
    // TODO: verify if egg spawning probability is truly 50%
    return rd() % 2 != 0;
  };

  // Check gamemode eligibility
  // All teammodes including single (training, level 1 eggs only) can spawn eggs
  const bool isGameModeEligible =
    parameters.raceGameMode == protocol::GameMode::Speed or
    parameters.raceGameMode == protocol::GameMode::Magic;

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

void RaceDirector::HandleUserRaceFinal(
  ClientId clientId,
  const protocol::AcCmdUserRaceFinal& command)
{
  bool isDnf = command.member3 > 0;
  std::chrono::hh_mm_ss raceTime{command.courseTime};
  spdlog::debug("[{}] AcCmdUserRaceFinal: {} {} {}",
    clientId,
    command.oid,
    isDnf ?
      "DNF" :
      std::format("{}:{}.{}",
        raceTime.minutes().count(),
        raceTime.seconds().count(),
        raceTime.subseconds().count()),
    command.member3);

  auto& clientContext = GetClientContext(clientId);
  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);

  // todo: sanity check for course time
  // todo: address npc racers and update their states
  auto& racer = raceInstance.GetTracker().GetRacer(
    clientContext.characterUid);

  racer.state = tracker::RaceTracker::Racer::State::Finishing;
  racer.courseTime = isDnf ? -1 :static_cast<int32_t>(command.courseTime.count());

  const protocol::AcCmdUserRaceFinalNotify notify{
    .oid = racer.oid,
    .courseTime = command.member3 < 0 ? 
      command.courseTime :
      std::chrono::milliseconds{-1}};
  this->Broadcast(raceInstance, notify);
}

void RaceDirector::HandleRaceResult(
  ClientId clientId,
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

  characterRecord.Immutable(
    [this, &response](const data::Character& character)
    {
      response.currentCarrots = character.carrots();

      GetServerInstance().GetDataDirector().GetHorse(character.mountUid()).Immutable(
        [&response](const data::Horse& horse)
        {
          response.horseFatigue = static_cast<uint16_t>(
            horse.fatigue());
        });
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RaceDirector::HandleP2PRaceResult(
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

void RaceDirector::HandleP2PUserRaceResult(
  ClientId,
  const protocol::AcCmdUserRaceP2PResult&)
{
}

void RaceDirector::HandleAwardStart(
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
    [this, &notify, &raceInstance](const server::Room& room)
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

void RaceDirector::HandleAwardEnd(
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

void RaceDirector::HandleStarPointGet(
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
    static_cast<uint8_t>(parameters.raceGameMode));

  uint32_t gainedStarPoints = command.gainedStarPoints;
  if (racer.effects[20] || racer.effects[21]) {
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

void RaceDirector::HandleRequestSpur(
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
    static_cast<uint8_t>(parameters.raceGameMode));

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

void RaceDirector::HandleHurdleClearResult(
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
      static_cast<uint8_t>(parameters.raceGameMode));

  switch (command.hurdleClearType)
  {
    case protocol::AcCmdCRHurdleClearResult::HurdleClearType::Perfect:
    {
      // Perfect jump over the hurdle.
      racer.jumpComboValue = std::min(
        static_cast<uint32_t>(99),
        racer.jumpComboValue + 1);

      if (parameters.raceGameMode == protocol::GameMode::Speed)
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
      if (racer.effects[20] || racer.effects[21]) {
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
    parameters.raceGameMode == protocol::GameMode::Magic &&
    racer.starPointValue >= gameModeTemplate.starPointsMax &&
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

void RaceDirector::HandleStartingRate(
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
    static_cast<uint8_t>(parameters.raceGameMode));

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

void RaceDirector::HandleRaceUserPos(
  ClientId clientId,
  const protocol::AcCmdUserRaceUpdatePos& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // TODO: Revise this in NPC races
  if (command.oid != racer.oid)
  {
    throw std::runtime_error(
      "Client tried to perform action on behalf of different racer");
  }

  const auto& gameModeTemplate = GetServerInstance().GetCourseRegistry().GetCourseGameModeInfo(
    static_cast<uint8_t>(parameters.raceGameMode));

  constexpr double ItemSpawnDistanceThreshold = 90.0;

  auto processItemSpawn = [&](tracker::Oid oid, uint32_t itemType, const std::array<float, 3>& position)
  {
    const auto distance = std::sqrt(
      std::pow(command.member2[0] - position[0], 2) +
      std::pow(command.member2[1] - position[1], 2) +
      std::pow(command.member2[2] - position[2], 2));

    const bool isInProximity = distance < ItemSpawnDistanceThreshold;
    const bool isAlreadyTracked = racer.trackedItems.contains(oid);

    if (isAlreadyTracked)
    {
      if (not isInProximity)
        racer.trackedItems.erase(oid);
      return;
    }

    if (not isInProximity)
      return;

    protocol::AcCmdRCCreateItem spawn{
      .itemId = oid,
      .itemType = itemType,
      .position = position,
      .spawnStyle = 0,  // ITEM_SPAWN_STYLE_NONE, fix the item in position
      .spawnerId = 0,
      .sizeLevel = 0};

    racer.trackedItems.insert(oid);
    _commandServer.QueueCommand<decltype(spawn)>(clientId, [spawn]() { return spawn; });
  };

  for (const auto& [itemOid, item] : raceInstance.GetTracker().GetItems())
  {
    if (std::chrono::steady_clock::now() < item.respawnTimePoint)
      continue;
    processItemSpawn(item.oid, item.currentType, item.position);
  }

  for (const auto& eventItem : racer.eventItems)
    processItemSpawn(eventItem.oid, eventItem.itemType, eventItem.position);

  // Only regenerate magic during active race (after countdown finishes)
  // Check if game mode is magic, race is active, countdown finished, and not holding an item
  const bool raceActuallyStarted = std::chrono::steady_clock::now() >= parameters.raceStartTimePoint;

  if (parameters.raceGameMode == protocol::GameMode::Magic
    && racer.state == tracker::RaceTracker::Racer::State::Racing
    && raceActuallyStarted
    && not racer.magicItem.has_value())
  {
    if (racer.starPointValue < gameModeTemplate.starPointsMax)
    {
      // TODO: add these to configuration somewhere
      // Eyeballed these values from watching videos
      constexpr uint32_t NoItemHeldBoostAmount = 2000;
      // TODO: does holding an item and with certain equipment give you magic? At a reduced rate?
      constexpr uint32_t ItemHeldWithEquipmentBoostAmount = 1000;
      uint32_t gainedStarPoints;
      if (racer.magicItem.has_value()) {
        gainedStarPoints = ItemHeldWithEquipmentBoostAmount;
      } else {
        gainedStarPoints = NoItemHeldBoostAmount;
      }
      if (racer.effects[20] || racer.effects[21]) {
        // TODO: Something sensible, idk what the bonus does
        gainedStarPoints *= 2;
      }
      racer.starPointValue = std::min(gameModeTemplate.starPointsMax, racer.starPointValue + gainedStarPoints);
    }

    // Conditional already checks if there is no magic item and gamemode is magic,
    // only check if racer has max magic gauge to give magic item
    protocol::AcCmdCRStarPointGetOK starPointResponse{
      .characterOid = command.oid,
      .starPointValue = racer.starPointValue,
      .giveMagicItem = racer.starPointValue >= gameModeTemplate.starPointsMax
    };

    _commandServer.QueueCommand<decltype(starPointResponse)>(
      clientId,
      [starPointResponse]
      {
        return starPointResponse;
      });
  }
}

void RaceDirector::HandleChat(ClientId clientId, const protocol::AcCmdCRChat& command)
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

void RaceDirector::HandleRelayCommand(
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

void RaceDirector::HandleRelay(
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

void RaceDirector::HandleUserRaceActivateInteractiveEvent(
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

void RaceDirector::HandleUserRaceActivateEvent(
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
  }, std::chrono::steady_clock::now() + tracker::RaceTracker::ThrottleDurationMs);

  // Broadcast to all active racers in the race
  const protocol::AcCmdUserRaceActivateEventNotify notify{
    .eventId = command.eventId,
    .characterOid = racer.oid};
  this->Broadcast(raceInstance, notify);
}

void RaceDirector::HandleUserRaceDeactivateEvent(
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

void RaceDirector::HandleRequestMagicItem(
  ClientId clientId,
  const protocol::AcCmdCRRequestMagicItem& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // TODO: Revise this on NPC races
  if (command.characterOid != racer.oid)
  {
    spdlog::warn("Client tried to perform action on behalf of different racer");
    return;
  }

  // Check if racer is already holding a magic item
  if (racer.magicItem.has_value())
  {
    // todo: this seems to happen a lot, figure it out
    return;
  }

  protocol::AcCmdCRStarPointGetOK starPointResponse{
    .characterOid = command.characterOid,
    .starPointValue = racer.starPointValue = 0,
    .giveMagicItem = false
  };

  _commandServer.QueueCommand<decltype(starPointResponse)>(
    clientId,
    [starPointResponse]
    {
      return starPointResponse;
    });

  protocol::AcCmdCRRequestMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = racer.magicItem.emplace(RandomMagicItem(_serverInstance, racer).type),
    .member3 = 0
  };

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
  this->BroadcastExceptCharacterUid(raceInstance, notify, clientContext.characterUid);
}

void RaceDirector::HandleUseMagicItem(
  ClientId clientId,
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
  const uint16_t effectInstanceId = raceInstance.GetTracker().GetNextEffectInstanceIdAndIncrementBy(1);

  auto targetList = command.targetList;

  auto magicSlotInfo = GetServerInstance().GetMagicRegistry().GetSlotInfo(command.magicItemId);

  if ((racer.effects[18] || racer.effects[19]) && (magicSlotInfo.criticalType != 0))
  {
    magicSlotInfo = GetServerInstance().GetMagicRegistry().GetSlotInfo(magicSlotInfo.criticalType);

    // Consume the crit chance buff immediately
    for (const uint32_t critEffectId : {18u, 19u})
    {
      if (racer.effects[critEffectId])
        RemoveEffect(raceInstance, racer, critEffectId);
    }
  }

  // Darkfire should only affect one target
  // Client sends all targets infront of them but we should only apply the effect to the targeted one (the arrow above their head)
  if (magicSlotInfo.type == 14)
    targetList.resize(1);

  // Dragon handling
  if (magicSlotInfo.basicType == 16)
  {
    if (!targetList.empty())
    {
      auto& racers = raceInstance.GetTracker().GetRacers();
      const auto targetOid = targetList[0];
      const auto targetIter = std::ranges::find_if(
        racers,
        [targetOid](const auto& entry)
        {
          return entry.second.oid == targetOid;
        });

      if (targetIter == racers.end())
      {
        targetList.clear();
      }
      else
      {
        auto& targetRacer = targetIter->second;

        // If target has already a dragon, miss
        if (targetRacer.pendingMagicTarget.has_value())
        {
          targetList.clear();
        }
      }
    }
  }
  protocol::AcCmdCRUseMagicItemOK response{
    .characterOid = command.characterOid,
    .magicItemId = magicSlotInfo.type,
    .iceWallProperties = command.iceWallProperties,
    .targetList = targetList,
    .effectInstanceId = effectInstanceId,
    .unk4 = magicSlotInfo.castingTime
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });

  // Notify other players that this player used their magic item
  protocol::AcCmdCRUseMagicItemNotify usageNotify{
    .characterOid = command.characterOid,
    .magicItemId = magicSlotInfo.type,
    .iceWallProperties = command.iceWallProperties,
    .targetList = targetList,
    .effectInstanceId = effectInstanceId,
    .unk4 = magicSlotInfo.castingTime};

  // Send usage notification to other players
  this->BroadcastExceptCharacterUid(raceInstance, usageNotify, clientContext.characterUid);

  // Send effect for items that have instant effects
  switch (magicSlotInfo.type)
  {
    // Shield, Booster, Phoenix
    case 4: 
    case 5:
    case 6: 
    case 7:
    case 8: 
    case 9:
      this->ScheduleSkillEffect(raceInstance, command.characterOid, racer.oid, magicSlotInfo, effectInstanceId);
      break;
    // IceWall
    case 10:
    case 11:
    {
      const uint16_t obstacleInstanceCount = static_cast<uint16_t>(command.targetList.size());
      if (obstacleInstanceCount > 1)
      {
        // If its a crit ice wall, add the 2 missing InstanceIds to the tracker so that they can be used for the breakdown and expiration of the effect
        raceInstance.GetTracker().GetNextEffectInstanceIdAndIncrementBy(obstacleInstanceCount - 1);
      }
      auto magicExpire = protocol::AcCmdRCMagicExpire{
        .magicType = magicSlotInfo.type,
        .firstObstacleInstanceId = effectInstanceId,
        .obstacleInstanceCount = obstacleInstanceCount,
        .breakdown = 0
      };
      _scheduler.Queue(
        [this, magicExpire, &raceInstance]()
        {
          this->Broadcast(raceInstance, magicExpire);
        },
        Scheduler::Clock::now() + std::chrono::seconds(4)); // TODO: Change to 4 seconds
      break;
    }
    // BufPower, BufGauge, BufSpeed
    case 20: 
    case 21:
    case 22: 
    case 23:
    case 24: 
    case 25:
    {
      for (auto& otherRacer : raceInstance.GetTracker().GetRacers() | std::views::values)
      {
        if (racer.oid == otherRacer.oid
        || (racer.team != tracker::RaceTracker::Racer::Team::Solo && racer.team == otherRacer.team))
        {
          this->ScheduleSkillEffect(raceInstance, command.characterOid, otherRacer.oid, magicSlotInfo, effectInstanceId);
        }
      }
      break;
    }
  }

  racer.magicItem.reset();
}

void RaceDirector::HandleUserRaceItemGet(
  ClientId clientId,
  const protocol::AcCmdUserRaceItemGet& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  auto& racer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  // Check event items first (eggs, etc.)
  const auto eventItemOid = raceInstance.GetTracker().FindEventItem(clientContext.characterUid, command.itemId);
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
      .itemId = command.itemId,
      .itemType = eventItem.itemType};
    this->Broadcast(raceInstance, itemGet);

    raceInstance.GetTracker().RemoveEventItem(clientContext.characterUid, command.itemId);
    racer.trackedItems.erase(command.itemId);
    return;
  }

  auto& items = raceInstance.GetTracker().GetItems();
  const auto itemIter = items.find(command.itemId);
  if (itemIter == items.end())
  {
    spdlog::warn("Client {} picked up untracked item {}", clientId, command.itemId);
    return;
  }
  auto& item = itemIter->second;

  constexpr auto ItemRespawnDuration = std::chrono::milliseconds(500);
  item.respawnTimePoint = std::chrono::steady_clock::now() + ItemRespawnDuration;

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
        switch (item.currentType)
        {
          case 101: // Gold horseshoe. Get star points until the next boost
            racer.starPointValue = std::min(((racer.starPointValue/40000)+1) * 40000, gameModeInfo.starPointsMax);
            break;
          case 102: // Silver horseshoe. Get 10k star points
            racer.starPointValue = std::min(racer.starPointValue+10000, gameModeInfo.starPointsMax);
            break;
          default:
            spdlog::warn("Player {} picked up unknown item type {}",
              clientId, item.currentType);
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
      // Magic items should respawn at a near-instant rate
      item.respawnTimePoint = std::chrono::steady_clock::now();

      uint32_t magicItem{};
      if (not racer.magicItem.has_value())
      {
        // Racer is empty handed

        // Get the item type of the picked up item (408, 409 etc)
        const uint32_t magicItemType = item.currentType;

        // Get the magic slot index to indicate to the racer that they
        // have the item (water shield, ice wall etc).
        magicItem = _serverInstance.GetCourseRegistry()
          .GetItemTypeInfo(magicItemType).magicSlot;

        // Response with OK to the client that they have a new item in hand
        protocol::AcCmdCRRequestMagicItemOK magicItemOk{
          .characterOid = command.characterOid,
          .magicItemId = racer.magicItem.emplace(magicItem),
          .member3 = 0};

        _commandServer.QueueCommand<decltype(magicItemOk)>(
          clientId,
          [clientId, magicItemOk]()
          {
            return magicItemOk;
          });
      }
      else
      {
        // Racer is already holding the item, do not replace it
        magicItem = racer.magicItem.value();
      }

      // Now that the magic item on the ground has been picked up,
      // randomly pick the new item type for this picked up item
      if (!item.itemTypes.empty())
      {
        static std::random_device rd;
        std::uniform_int_distribution<size_t> distribution(0, item.itemTypes.size() - 1);
        item.currentType = item.itemTypes[distribution(rd)];
      }
      else
      {
        // TODO: Item types is empty, use deck ID instead?
      }

      // Notify racers in the race room that the invoking racer is now
      // holding a new magic item
      const protocol::AcCmdCRRequestMagicItemNotify notify{
        .magicItemId = magicItem,
        .characterOid = command.characterOid,
      };

      // Prevent self broadcast,
      // this prevents the double pickup UI bug for the invoker)
      this->BroadcastExceptCharacterUid(
        raceInstance,
        notify,
        clientContext.characterUid);

      // TODO: reset magic gauge to 0?
      break;
    }
  }

  // Notify all clients in the room that this item has been picked up
  const protocol::AcCmdGameRaceItemGet get{
    .characterOid = command.characterOid,
    .itemId = command.itemId,
    .itemType = item.currentType,
  };
  this->Broadcast(raceInstance, get);

  // Erase the item from item instances of each client.
  for (auto& raceRacer : raceInstance.GetTracker().GetRacers() | std::views::values)
  {
    raceRacer.trackedItems.erase(item.oid);
  }
}

// Magic Targeting System Implementation for Bolt
void RaceDirector::HandleStartMagicTarget(
  ClientId clientId,
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
  targetRacer.dragonReceivedAt = std::chrono::steady_clock::now();
  targetRacer.pendingMagicTarget = {command.casterOid, command.effectInstanceId};
}

void RaceDirector::HandleChangeMagicTarget(
  ClientId clientId,
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

  // Send Cancel if the target already has dragon, otherwise send OK and update the target's dragon status
  if (targetRacer.pendingMagicTarget.has_value())
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

void RaceDirector::HandleActivateSkillEffect(
  ClientId clientId,
  const protocol::AcCmdCRActivateSkillEffect& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);

  auto& targetRacer = raceInstance.GetTracker().GetRacer(clientContext.characterUid);

  auto magicSlotInfo = GetServerInstance().GetMagicRegistry().GetSlotInfoByEffectId(command.effectId);
  // If the target has a DarkFire effect active and the magic crits by dark fire, use the critical type instead
  if ((targetRacer.effects[12] || targetRacer.effects[13]) && magicSlotInfo.criticalByDarkFire)
  {
    magicSlotInfo = GetServerInstance().GetMagicRegistry().GetSlotInfo(magicSlotInfo.criticalType);
  }
  // only send the magic expire for icewall. other magic cant do anything with it.
  if (magicSlotInfo.type == 10 || magicSlotInfo.type == 11)
  {
    const auto magicExpire = protocol::AcCmdRCMagicExpire{
      .magicType = magicSlotInfo.type,
      .firstObstacleInstanceId = command.effectInstanceId,
      .obstacleInstanceCount = 1,
      .breakdown = 1};
    this->Broadcast(raceInstance, magicExpire);
  }

  EffectVerdict verdict = this->ScheduleSkillEffect(raceInstance, command.attackerOid, command.targetOid, magicSlotInfo, command.effectInstanceId);

  if (verdict == EffectVerdict::Applied && magicSlotInfo.attackRank > 1 && targetRacer.pendingMagicTarget)
  {
    const protocol::AcCmdRCRemoveMagicTarget removeMagicTarget{
      .effectInstanceId = targetRacer.pendingMagicTarget->effectInstanceId,
      .casterOid = targetRacer.pendingMagicTarget->casterOid,
      .targetOid = command.targetOid,
      .targetOid2 = command.targetOid};
    this->Broadcast(raceInstance, removeMagicTarget);
  }

  // TODO:: Add a Conditional for the SystemContent that can enable/disable this behavior
  if (verdict == EffectVerdict::Applied && magicSlotInfo.removeMagic == 1 && targetRacer.magicItem.has_value())
  {
    protocol::AcCmdCRUseItemSlotOK response{
      .magicItemId = 0,
      .characterOid = command.targetOid};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });

    const protocol::AcCmdCRUseItemSlotNotify notify{
      .magicItemId = 0,
      .characterOid = command.targetOid,
      .unk = 1};
    this->Broadcast(raceInstance, notify);

    targetRacer.magicItem.reset();
  }

  if (magicSlotInfo.basicType == 16)
    targetRacer.pendingMagicTarget.reset();
}

void RaceDirector::HandleOpCmd(
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

  for (const auto response : result.commandVerdict->result)
  {
    _commandServer.QueueCommand<protocol::RanchCommandOpCmdOK>(
      clientId,
      [response = std::move(response)]()
      {
        return protocol::RanchCommandOpCmdOK{
          .feedback = response};
      });
  }
}

void RaceDirector::HandleChangeSkillCardPresetId(
  ClientId clientId,
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

void RaceDirector::RemoveEffect(
  RaceInstance& raceInstance,
  tracker::RaceTracker::Racer& racer,
  uint32_t effectId)
{
  if (effectId >= tracker::RaceTracker::Racer::EffectCount)
  {
    spdlog::error("RemoveEffect: effectId {} out of range", effectId);
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

RaceDirector::EffectVerdict RaceDirector::ScheduleSkillEffect(
  RaceInstance& raceInstance,
  tracker::Oid attackerOid, tracker::Oid targetOid,
  const registry::Magic::SlotInfo& magicSlotInfo,
  const uint16_t effectInstanceId)
{
  auto& racers = raceInstance.GetTracker().GetRacers();
  const auto targetRacerIter = std::ranges::find_if(
    racers, [targetOid](const auto& pair) { return pair.second.oid == targetOid; });

  // Target racer not found
  if (targetRacerIter == racers.cend())
    return EffectVerdict::Failed;

  // Guard against misconfigured skillEffectId crashing the server
  if (magicSlotInfo.skillEffectId >= tracker::RaceTracker::Racer::EffectCount)
  {
    spdlog::error(
      "ScheduleSkillEffect: skillEffectId {} out of range (max {})",
      magicSlotInfo.skillEffectId,
      tracker::RaceTracker::Racer::EffectCount - 1);
    return EffectVerdict::Failed;
  }

  const data::Uid targetCharacterUid = targetRacerIter->first;
  auto& targetRacer = targetRacerIter->second;

  const bool isAttack = magicSlotInfo.attackValue > 0;

  // Shield check: effectId 2 = WaterShield Normal (threshold 100), effectId 3 = WaterShield Critical (threshold 200)
  const uint32_t shieldThreshold =
    targetRacer.effects[3] ? 200u :
    targetRacer.effects[2] ? 100u : 0u;
  const bool shieldBlocks = isAttack && magicSlotInfo.attackValue < shieldThreshold;

  // Any removeHotRodding attack is considered part of the lightning family.
  // For the current registry, critical variants have criticalType == 0.
  const bool isLightning = isAttack && magicSlotInfo.removeHotRodding;
  const bool isCritLightning = isLightning && magicSlotInfo.criticalType == 0;

  // Normal hotrodding (effectId 6): blocked by non-lightning attacks, canceled by any lightning.
  // Crit hotrodding (effectId 7): blocks everything except crit lightning.
  const bool hotroddingBlocks =
    (targetRacer.effects[6] && isAttack && !isLightning) ||
    (targetRacer.effects[7] && isAttack && !isCritLightning);

  const uint32_t effectId = shieldBlocks
    ? (targetRacer.effects[3] ? 3u : 2u)
    : magicSlotInfo.skillEffectId;

  // For removeMagic attacks: blocked if an equal-or-higher-rank attack is already active.
  // For other attacks (attackValue > 0): blocked if the same effect is already active.
  // For pure buffs: blocked only if already active and not replaceable (replaceEffect == 0 means no extension).
  // Duplication is checked against the basic-type effect slot so crit variants share occupancy with their base,
  // except for replaceEffect spells which track their own slot independently.
  // Attacks with rank < 2 are blocked if a rank-2+ attack (fireball/lightning) is already active.
  const uint32_t checkEffectId = magicSlotInfo.replaceEffect
    ? magicSlotInfo.skillEffectId
    : GetServerInstance().GetMagicRegistry().GetSlotInfo(magicSlotInfo.basicType).skillEffectId;
  const bool isDuplicated = hotroddingBlocks
    || (isAttack && magicSlotInfo.attackRank < 2 && targetRacer.attackRank >= 2)
    || (magicSlotInfo.attackRank > 0
      ? targetRacer.attackRank >= magicSlotInfo.attackRank
      : targetRacer.effects[checkEffectId] && (isAttack || !magicSlotInfo.replaceEffect));

  // TODO: Verify if characterOid and targetOid should be the same once we have NPCs
  const protocol::AcCmdRCAddSkillEffect addSkillEffect{
    .characterOid = targetOid,
    .effectId = effectId,
    .targetOid = targetOid,
    .attackerOid = attackerOid,
    .unk2 = effectInstanceId,
    .unk3 = isDuplicated ? 1u : 0u,
    .shieldEffect = protocol::AcCmdRCAddSkillEffect::ShieldEffect{
      .unk0 = shieldBlocks ? 2u : 0u,
      .unk1 = 0,
    },
    .boostEffectMs = static_cast<uint32_t>(magicSlotInfo.effectDelay * 1000.0f),
  };

  // Broadcast
  this->Broadcast(raceInstance, addSkillEffect);

  if (shieldBlocks)
    return EffectVerdict::Shielded;

  if (isDuplicated)
    return EffectVerdict::Duplicated;

  targetRacer.effects[effectId] = true;
  const uint32_t generation = ++targetRacer.effectGenerations[effectId];
  if (magicSlotInfo.attackRank > 0)
    targetRacer.attackRank = magicSlotInfo.attackRank;

  // Cancel any active adjustMotionSpeed buffs only when a removeMagic attack lands.
  // HotRodding (effectIds 6 and 7), crit chance buffs (18 and 19), and BufGauge buffs (20 and 21) are excluded.
  if (isAttack && magicSlotInfo.removeMagic)
  {
    for (const auto& [type, slot] : GetServerInstance().GetMagicRegistry().GetSlotInfoMap())
    {
      if (slot.adjustMotionSpeed && slot.attackValue == 0
        && slot.skillEffectId != 6 && slot.skillEffectId != 7
        && slot.skillEffectId != 18 && slot.skillEffectId != 19
        && slot.skillEffectId != 20 && slot.skillEffectId != 21
        && targetRacer.effects[slot.skillEffectId])
      {
        RemoveEffect(raceInstance, targetRacer, slot.skillEffectId);
      }
    }
  }

  _scheduler.Queue(
    [this, roomUid = raceInstance.GetRoomUid(), targetOid, targetCharacterUid, effectId,
      attackRank = magicSlotInfo.attackRank, generation,
      clearMagicTarget = magicSlotInfo.attackRank > 1]()
    {
      std::scoped_lock raceInstanceLock(_raceInstancesMutex);
      const auto raceInstanceIter = _raceInstances.find(roomUid);
      if (raceInstanceIter == _raceInstances.cend())
        return;

      auto& raceInstance = raceInstanceIter->second;

      if (!raceInstance.GetTracker().IsRacer(targetCharacterUid))
        return;

      auto& racer = raceInstance.GetTracker().GetRacer(targetCharacterUid);

      // If the generation has changed, this effect was extended — skip the removal
      if (racer.effectGenerations[effectId] != generation)
        return;

      racer.effects[effectId] = false;
      // Only clear attackRank if a higher-rank attack hasn't replaced this one
      if (attackRank > 0 && racer.attackRank == attackRank)
        racer.attackRank = 0;
      if (clearMagicTarget)
        racer.pendingMagicTarget.reset();

      const protocol::AcCmdRCRemoveSkillEffect removeSkillEffect{
        .characterOid = targetOid,
        .effectId = effectId,
        .targetOid = targetOid,
        .unk1 = 0,
      };
      this->Broadcast(raceInstance, removeSkillEffect);
    },
    Scheduler::Clock::now() + std::chrono::milliseconds(static_cast<int64_t>(magicSlotInfo.effectDelay * 1000.0)));
  return EffectVerdict::Applied;
}

void RaceDirector::HandleInviteUser(
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

void RaceDirector::HandleRequestUser(
  ClientId clientId,
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

  GetServerInstance().GetRaceDirector().NotifyRequestUser(characterUid, command.force, command.characterName, command.roomUid, command.ranchUid);
  GetServerInstance().GetRanchDirector().NotifyRequestUser(characterUid, command.force, command.characterName, command.roomUid, command.ranchUid);

  protocol::AcCmdCRRequestUserOK response{};
  response.force= command.force;
  response.characterName = command.characterName;
  response.roomUid = command.roomUid;
  response.ranchUid = command.ranchUid;

  _commandServer.QueueCommand<decltype(response)>(clientId, [response](){ return response; });
}

void RaceDirector::HandleKickUser(
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
    [&isTargetInRoom, targetCharacterUid = command.characterUid](const server::Room& room)
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
void RaceDirector::HandleTeamGauge(const ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  // If race teammode is not team then we are done here.
  // This is necessary to ensure no team-related logic is handled when spur logic is handled.
  // Sanity check for speed gamemode
  bool isTeamMode = parameters.raceTeamMode == protocol::TeamMode::Team;
  bool isSpeedGameMode = parameters.raceGameMode == protocol::GameMode::Speed;
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

  const auto fillRateIndex = std::min(
    team.boostCount,
    static_cast<uint32_t>(baseFillRates.size() - 1));
  protocol::AcCmdRCTeamSpurGauge spur{
    .team = racer.team,
    .markerSpeed = baseFillRates[fillRateIndex] * teamSize, // Base fill rate * boost count * team size
    .unk5 = 0 // TODO: identify use
  };

  //! Base point for a successful boost.
  constexpr uint32_t BaseBoostPoints = 50;
  //! Base point difference per team member in a team.
  constexpr uint32_t BoostPointsDiffBase = 20;

  //! Scale points per boost, based on team size.
  //! Scale = team size - 1 for the formula.
  const auto scale = teamSize - 1;
  //! Final points per boost = base boost + additional boost points.
  const auto additionalBoostPoints = (BoostPointsDiffBase * scale) + (10 * scale);
  
  //! Base max points.
  constexpr uint32_t BaseMaxPoints = 250;
  //! Max points difference per team member.
  constexpr uint32_t MaxPointsDiffBase = 150;
  //! Final max points for team size.
  const uint32_t maxPoints = BaseMaxPoints + (MaxPointsDiffBase * scale);

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
    teamPoints + BaseBoostPoints + additionalBoostPoints);
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

void RaceDirector::HandleTriggerizeAct(
  ClientId clientId,
  const protocol::AcCmdCRTriggerizeAct& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::scoped_lock lock(_raceInstancesMutex);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  const bool isSpeedGameMode = parameters.raceGameMode == protocol::GameMode::Speed;
  const auto& mapBlockInfo = _serverInstance.GetCourseRegistry().GetMapBlockInfo(parameters.raceMapBlockId);
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

void RaceDirector::HandleGameCreateClientItem(
  ClientId clientId,
  const protocol::AcCmdCRGameCreateClientItem& command)
{
  spdlog::debug(
    "AcCmdCRGameCreateClientItem: {} {} [{}, {}, {}] [{}, {}, {}, {}]",
    command.someonesOid,
    command.unk1,
    command.position[0], command.position[1], command.position[2],
    command.unk3[0], command.unk3[1], command.unk3[2], command.unk3[3]);

  if (command.unk1 != 0)
    // Only egg spawning (unk1 == 0) is implemented
    throw new std::runtime_error("AcCmdCRGameCreateClientItem::unk1 != 0, other case not implemented");

  const auto& clientContext = GetClientContext(clientId);
  auto& raceInstance = GetRaceInstance(clientContext);
  const auto& parameters = raceInstance.GetParameters();

  // Get region for this map.
  const auto& mapBlockInfo = _serverInstance.GetCourseRegistry().GetMapBlockInfo(
    parameters.raceMapBlockId);
  const auto regionEggs = _serverInstance.GetPetRegistry().GetEggsByRegion(mapBlockInfo.region);
  if (regionEggs.empty())
    return;

  // Weighted random selection using ObtainRatio (owned eggs still included in weight pool).
  std::vector<uint32_t> weights;
  weights.reserve(regionEggs.size());
  for (const auto& egg : regionEggs)
    weights.push_back(egg.obtainRatio);

  static std::random_device rd;
  std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
  const auto& selectedEgg = regionEggs[dist(rd)];

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
