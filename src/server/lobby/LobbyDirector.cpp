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

#include "server/lobby/LobbyDirector.hpp"

#include "server/system/RoomSystem.hpp"
#include "server/ServerInstance.hpp"

#include <libserver/data/helper/ProtocolHelper.hpp>

#include <boost/container_hash/hash.hpp>
#include <spdlog/spdlog.h>
#include <zlib.h>

#include <random>

namespace
{

std::random_device rd;

} // namespace

namespace server
{

LobbyDirector::LobbyDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
  , _loginHandler(*this, _commandServer)
{
  _commandServer.RegisterCommandHandler<protocol::LobbyCommandLogin>(
    [this](ClientId clientId, const auto& command)
    {
      // Alicia 1.0
      assert(command.constant0 == 50
        && command.constant1 == 281
        && "Game version mismatch");

      spdlog::info("Handling user login for '{}'", command.loginId);

      _loginHandler.HandleUserLogin(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandCreateNickname>(
    [this](ClientId clientId, const auto& command)
    {
      _loginHandler.HandleUserCreateCharacter(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandEnterChannel>(
    [this](ClientId clientId, const auto& command)
    {
      HandleEnterChannel(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandRoomList>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRoomList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandMakeRoom>(
    [this](ClientId clientId, const auto& command)
    {
      HandleMakeRoom(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandEnterRoom>(
    [this](ClientId clientId, const auto& command)
    {
      HandleEnterRoom(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCLHeartbeat>(
    [this](ClientId clientId, const auto& command)
    {
      HandleHeartbeat(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandShowInventory>(
    [this](ClientId clientId, const auto& command)
    {
      HandleShowInventory(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandAchievementCompleteList>(
    [this](ClientId clientId, const auto& command)
    {
      HandleAchievementCompleteList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandRequestLeagueInfo>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRequestLeagueInfo(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandRequestQuestList>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRequestQuestList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandRequestDailyQuestList>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRequestDailyQuestList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandRequestSpecialEventList>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRequestSpecialEventList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandRequestPersonalInfo>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRequestPersonalInfo(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandSetIntroduction>(
    [this](ClientId clientId, const auto& command)
    {
      HandleSetIntroduction(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandEnterRanch>(
    [this](ClientId clientId, const auto& command)
    {
      HandleEnterRanch(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandGetMessengerInfo>(
    [this](ClientId clientId, const auto& command)
    {
      HandleGetMessengerInfo(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCLGoodsShopList>(
    [this](ClientId clientId, const auto& command)
    {
      HandleGoodsShopList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCLInquiryTreecash>(
    [this](ClientId clientId, const auto& command)
    {
      HandleInquiryTreecash(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandClientNotify>(
    [this](ClientId clientId, const auto& command)
    {
      if (command.val0 != 1)
        spdlog::error("Client error notification: state[{}], value[{}]", command.val0, command.val1);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandEnterRandomRanch>(
    [this](ClientId clientId, const auto& command)
    {
      // this is just for prototype, it can suck
      auto& clientContext = GetClientContext(clientId);
      const auto requestingCharacterUid = clientContext.characterUid;

      data::Uid rancherUid = data::InvalidUid;

      // If the user has a visit preference apply it.
      if (clientContext.rancherVisitPreference != data::InvalidUid)
      {
        rancherUid = clientContext.rancherVisitPreference;
        clientContext.rancherVisitPreference = data::InvalidUid;
      }

      // If the rancher's uid is invalid randomize it.
      if (rancherUid == data::InvalidUid)
      {
        std::vector<data::Uid> availableRanches;

        auto& characters = GetServerInstance().GetDataDirector().GetCharacterCache();
        const auto& characterKeys = characters.GetKeys();

        for (const auto& randomRancherUid : characterKeys)
        {
          const auto character = characters.Get(randomRancherUid);
          character->Immutable([&availableRanches, requestingCharacterUid](const data::Character& character)
          {
            // Only consider ranches that are unlocked and that
            // do not belong to the character that requested the random ranch.
            if (character.isRanchLocked() || character.uid() == requestingCharacterUid)
              return;

            availableRanches.emplace_back(character.uid());
          });
        }

        // There must be at least the ranch the requesting character is the owner of.
        if (availableRanches.empty())
        {
          availableRanches.emplace_back(clientContext.characterUid);
        }

        // Pick a random character from the available list to join the ranch of.
        std::uniform_int_distribution<size_t> uidDistribution(0, availableRanches.size() - 1);
        rancherUid = availableRanches[uidDistribution(rd)];
      }

    QueueEnterRanchOK(clientId, rancherUid);
  });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandUpdateSystemContent>(
    [this](ClientId clientId, const auto& command)
    {
      HandleUpdateSystemContent(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandChangeRanchOption>(
    [this](ClientId clientId, const auto& command)
    {
      HandleChangeRanchOption(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCLUpdateUserSettings>(
    [this](ClientId clientId, const auto& command)
    {
      HandleUpdateUserSettings(clientId, command);
    });

    _commandServer.RegisterCommandHandler<protocol::AcCmdCLRequestMountInfo>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRequestMountInfo(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdLCInviteGuildJoinCancel>(
    [this](ClientId clientId, const auto& command)
    {
      HandleDeclineInviteToGuild(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdLCInviteGuildJoinOK>(
    [this](ClientId clientId, const auto& command)
    {
      HandleAcceptInviteToGuild(clientId, command);
    }); 
}

void LobbyDirector::Initialize()
{
  spdlog::debug(
    "Lobby is advertising ranch server on {}:{}",
    GetConfig().advertisement.ranch.address.to_string(),
    GetConfig().advertisement.ranch.port);
  spdlog::debug(
    "Lobby is advertising race server on {}:{}",
    GetConfig().advertisement.race.address.to_string(),
    GetConfig().advertisement.race.port);
  spdlog::debug(
    "Lobby is advertising messenger server on {}:{}",
    GetConfig().advertisement.messenger.address.to_string(),
    GetConfig().advertisement.messenger.port);

  spdlog::debug(
    "Lobby server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _commandServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void LobbyDirector::Terminate()
{
  _commandServer.EndHost();
}

void LobbyDirector::Tick()
{
  _loginHandler.Tick();
  _scheduler.Tick();
}

void LobbyDirector::HandleClientConnected(ClientId clientId)
{
  spdlog::info("Client {} connected to the lobby", clientId);
  _clients.try_emplace(clientId);
}

void LobbyDirector::HandleClientDisconnected(ClientId clientId)
{
  spdlog::info("Client {} disconnected from the lobby", clientId);
  _clients.erase(clientId);
}

ServerInstance& LobbyDirector::GetServerInstance()
{
  return _serverInstance;
}

Config::Lobby& LobbyDirector::GetConfig()
{
  return GetServerInstance().GetSettings().lobby;
}

void LobbyDirector::RequestCharacterCreator(data::Uid characterUid)
{
  _forcedCharacterCreator.emplace(characterUid);
}

void LobbyDirector::Disconnect(data::Uid characterUid)
{
  protocol::AcCmdLCOpKick kick{};
  for (const auto& [clientId, clientContext] : _clients)
  {
    if (not clientContext.isAuthenticated)
      continue;

    if (clientContext.characterUid == characterUid)
    {
      _commandServer.DisconnectClient(clientId);
      return;
    }
  }
}

void LobbyDirector::Mute(data::Uid characterUid, data::Clock::time_point expiration)
{
  protocol::AcCmdLCOpMute mute{
    .duration = util::DurationToAliciaTime(std::chrono::seconds(10))};

  for (const auto& [clientId, clientContext] : _clients)
  {
    if (not clientContext.isAuthenticated)
      continue;

    if (clientContext.characterUid == characterUid)
    {
      _commandServer.QueueCommand<decltype(mute)>(
        clientId,
        [mute]()
        {
          return mute;
        });
      return;
    }
  }
}

void LobbyDirector::Notice(data::Uid characterUid, const std::string& message)
{
  protocol::AcCmdLCNotice notice{
    .notice = message};

  for (const auto& [clientId, clientContext] : _clients)
  {
    if (not clientContext.isAuthenticated)
      continue;

    if (clientContext.characterUid == characterUid)
    {
      _commandServer.QueueCommand<decltype(notice)>(
        clientId,
        [notice]()
        {
          return notice;
        });
      return;
    }
  }
}

void LobbyDirector::InviteToGuild(std::string characterName, data::Uid guildUid, data::Uid inviterCharacterUid)
{
  // Inviter character name
  std::string inviterCharacterName;
  GetServerInstance().GetDataDirector().GetCharacter(inviterCharacterUid).Immutable(
    [&inviterCharacterName](const data::Character& character)
  {
    inviterCharacterName = character.name();
  });

  // For all clients
  for (const auto& [clientId, clientContext] : _clients)
  {
    // Skip unauthorized clients.
    if (not clientContext.isAuthenticated)
      continue;
    
    // Ensure character record exists (do not retrieve)
    const auto& characterRecord = GetServerInstance().GetDataDirector().GetCharacterCache().Get(
      clientContext.characterUid, false);
    if (not characterRecord.has_value())
    {
      continue;
    }

    bool found = false;
    characterRecord.value().Immutable([&found, characterName, guildUid](const data::Character& character)
    {
      // If character record with matching invite name
      if (character.name() == characterName)
      {
        found = true;
      }
    });

    if (not found)
      continue;

    std::string guildName, guildDescription;
    GetServerInstance().GetDataDirector().GetGuild(guildUid).Immutable(
      [&guildName, &guildDescription](const data::Guild& guild)
      {
        guildName = guild.name();
        guildDescription = guild.description();
      });

    protocol::AcCmdLCInviteGuildJoin command{
      .characterUid = clientContext.characterUid,
      .inviterCharacterUid = inviterCharacterUid, // clientContext.characterUid?
      .inviterCharacterName = inviterCharacterName,
      .unk3 = guildDescription,
      .guild = {
        .uid = guildUid,
        .val1 = 1,
        .val2 = 2,
        .name = guildName,
        .guildRole = protocol::GuildRole::Member,
        .val5 = 5,
        .val6 = 6
      }
    };

    _commandServer.QueueCommand<decltype(command)>(
      clientId,
      [command]()
      {
        return command;
      });

    // Add character UID to pending invites for the guild
    _pendingGuildInvites.emplace(guildUid, clientContext.characterUid);
    
    // Found client and sent invite command, no need to process the rest of the clients
    break;
  }
}

std::vector<std::string> LobbyDirector::GetOnlineUsers()
{
  std::vector<std::string> userName;

  for (const auto& [clientId, clientContext] : _clients)
  {
    if (not clientContext.isAuthenticated)
      continue;
    userName.emplace_back(clientContext.userName);
  }

  return userName;
}

std::vector<data::Uid> LobbyDirector::GetOnlineCharacters()
{
  std::vector<data::Uid> characters;

  for (const auto& [clientId, clientContext] : _clients)
  {
    if (not clientContext.isAuthenticated)
      continue;
    characters.emplace_back(clientContext.characterUid);
  }

  return characters;
}

void LobbyDirector::UpdateVisitPreference(data::Uid characterUid, data::Uid visitingCharacterUid)
{
  const auto clientContextIter = std::ranges::find_if(
    _clients,
    [characterUid](const auto pair)
    {
      const auto& [clientId, ctx] = pair;
      return ctx.characterUid == characterUid;
    });

  if (clientContextIter == _clients.cend())
    return;

  clientContextIter->second.rancherVisitPreference = visitingCharacterUid;
}

void LobbyDirector::HandleEnterChannel(
  ClientId clientId,
  const protocol::LobbyCommandEnterChannel& command)
{
  protocol::LobbyCommandEnterChannelOK response{
    .unk0 = command.channel,
    .unk1 = 557,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRoomList(
  ClientId clientId,
  const protocol::LobbyCommandRoomList& command)
{
  constexpr uint32_t RoomsPerPage = 9;

  protocol::LobbyCommandRoomListOK response{
    .page = command.page,
    .gameMode = command.gameMode,
    .teamMode = command.teamMode};

  // todo: update every x tick
  const auto roomSnapshots = _serverInstance.GetRoomSystem().GetRoomsSnapshot();
  const auto roomChunks = std::views::chunk(
    roomSnapshots,
    RoomsPerPage);

  if (not roomChunks.empty())
  {
    // Clamp the page index to the la
    const auto pageIndex = std::max(
      std::min(
        roomChunks.size() - 1,
        static_cast<size_t>(command.page)),
      size_t{0});

    for (const auto& room : roomChunks[pageIndex])
    {
      const protocol::GameMode roomGameMode = static_cast<
        protocol::GameMode>(room.details.gameMode);
      const protocol::TeamMode roomTeamMode = static_cast<
        protocol::TeamMode>(room.details.teamMode);

      if (roomGameMode != command.gameMode
        || roomTeamMode != command.teamMode)
      {
        continue;
      }

      auto& roomResponse = response.rooms.emplace_back();

      roomResponse.hasStarted = room.isPlaying;

      roomResponse.uid = room.uid;
      if (not room.details.password.empty())
      {
        roomResponse.isLocked = true;
      }

      roomResponse.playerCount = room.playerCount;
      roomResponse.maxPlayerCount = room.details.maxPlayerCount;
      // todo: skill bracket
      roomResponse.skillBracket = protocol::LobbyCommandRoomListOK::Room::SkillBracket::Experienced;
      roomResponse.name = room.details.name;
      roomResponse.map = room.details.courseId;
    }
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleMakeRoom(
  ClientId clientId,
  const protocol::LobbyCommandMakeRoom& command)
{
  const auto clientContext = GetClientContext(clientId);
  uint32_t createdRoomUid{0};

  _serverInstance.GetRoomSystem().CreateRoom(
    [&createdRoomUid, &command, characterUid = clientContext.characterUid](
      Room& room)
    {
      const bool isTraining = command.playerCount == 1;

      // Only allow an empty room name in training/tutorial rooms.
      // todo: better way to detect this?
      if (command.name.empty() && not isTraining)
        return;

      room.GetRoomDetails().name = command.name;
      room.GetRoomDetails().password = command.password;
      room.GetRoomDetails().missionId = command.missionId;
      // todo: validate mission id

      room.GetRoomDetails().maxPlayerCount = std::max(
        std::min(command.playerCount,uint8_t{8}),
        uint8_t{0});

      switch (command.gameMode)
      {
        case protocol::GameMode::Speed:
          room.GetRoomDetails().gameMode = Room::GameMode::Speed;
          break;
        case protocol::GameMode::Magic:
          room.GetRoomDetails().gameMode = Room::GameMode::Magic;
          break;
        case protocol::GameMode::Tutorial:
          room.GetRoomDetails().gameMode = Room::GameMode::Tutorial;
          break;
        default:
          spdlog::error("Unknown game mode '{}'", static_cast<uint32_t>(command.gameMode));
      }

      switch (command.teamMode)
      {
        case protocol::TeamMode::FFA:
          room.GetRoomDetails().teamMode = Room::TeamMode::FFA;
          break;
        case protocol::TeamMode::Team:
          room.GetRoomDetails().teamMode = Room::TeamMode::Team;
          break;
        case protocol::TeamMode::Single:
          room.GetRoomDetails().teamMode = Room::TeamMode::Single;
          break;
        default:
          spdlog::error("Unknown team mode '{}'", static_cast<uint32_t>(command.gameMode));
      }

      room.GetRoomDetails().member11 = command.unk3;
      room.GetRoomDetails().skillBracket = command.unk4;
      // default to all courses
      room.GetRoomDetails().courseId = 10002;

      // Queue the master as a player.
      room.QueuePlayer(characterUid);
      createdRoomUid = room.GetUid();
    });

  if (createdRoomUid == 0)
  {
    protocol::LobbyCommandMakeRoomCancel response{};
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });

    return;
  }

  size_t identityHash = std::hash<uint32_t>()(clientContext.characterUid);
  boost::hash_combine(identityHash, createdRoomUid);

  const auto roomOtp = _serverInstance.GetOtpSystem().GrantCode(
    identityHash);

  protocol::LobbyCommandMakeRoomOK response{
    .roomUid = createdRoomUid,
    .oneTimePassword = roomOtp,
    .raceServerAddress = GetConfig().advertisement.race.address.to_uint(),
    .raceServerPort = GetConfig().advertisement.race.port};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleEnterRoom(
  ClientId clientId,
  const protocol::LobbyCommandEnterRoom& command)
{
  const auto clientContext = GetClientContext(clientId);

  // Whether the room is valid.
  bool isRoomValid = true;
  // Whether the user is authorized to enter.
  bool isAuthorized = false;
  // Whether the room is full.
  bool isRoomFull = false;

  try
  {
    _serverInstance.GetRoomSystem().GetRoom(
      command.roomUid,
      [&isAuthorized, &isRoomFull, &command, characterUid = clientContext.characterUid](
        Room& room)
      {
        const auto& roomPassword = room.GetRoomDetails().password;
        if (not roomPassword.empty())
          isAuthorized = roomPassword == command.password;
        else
          isAuthorized = true;

        isRoomFull = room.IsRoomFull();
        if (isRoomFull)
          return;

        room.QueuePlayer(characterUid);
      });
  }
  catch (const std::exception&)
  {
    // The client requested to join a room which no longer exists.
    // We do care in this case.
    isRoomValid = false;
  }

  if (not isRoomValid)
  {
    protocol::LobbyCommandEnterRoomCancel response{
      .status = protocol::LobbyCommandEnterRoomCancel::Status::CR_INVALID_ROOM};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  if (not isAuthorized)
  {
    protocol::LobbyCommandEnterRoomCancel response{
      .status = protocol::LobbyCommandEnterRoomCancel::Status::CR_BAD_PASSWORD};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  if (isRoomFull)
  {
    protocol::LobbyCommandEnterRoomCancel response{
      .status = protocol::LobbyCommandEnterRoomCancel::Status::CR_CROWDED_ROOM};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  size_t identityHash = std::hash<uint32_t>()(clientContext.characterUid);
  boost::hash_combine(identityHash, command.roomUid);

  const auto roomOtp = _serverInstance.GetOtpSystem().GrantCode(
    identityHash);

  protocol::LobbyCommandEnterRoomOK response{
    .roomUid = command.roomUid,
    .oneTimePassword = roomOtp,
    .raceServerAddress = GetConfig().advertisement.race.address.to_uint(),
    .raceServerPort = GetConfig().advertisement.race.port};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Schedule removal of the player from the room queue.
  _scheduler.Queue(
    [this, roomUid = command.roomUid, characterUid = clientContext.characterUid]()
    {
      try
      {
        _serverInstance.GetRoomSystem().GetRoom(
          roomUid,
          [this, characterUid](Room& room)
          {
            const bool dequeued = room.DequeuePlayer(characterUid);

            if (not dequeued)
              return;

            // If the player was actually dequeued it means
            // they have never connected to the room.
            _serverInstance.GetDataDirector().GetCharacter(characterUid).Immutable(
              [](const data::Character& character)
              {
                  spdlog::warn("Player '{}' did not connect to the room before the timeout", character.name());
              });
          });
      }
      catch (const std::exception&)
      {
        // We really don't care.
      }
    },
    Scheduler::Clock::now() + std::chrono::seconds(7));
}

void LobbyDirector::HandleHeartbeat(
  ClientId clientId,
  const protocol::AcCmdCLHeartbeat& command)
{
  // TODO: Statistics for the heartbeat.
}

void LobbyDirector::HandleShowInventory(
  ClientId clientId,
  const protocol::LobbyCommandShowInventory& command)
{
  QueueShowInventory(clientId);
}

void LobbyDirector::QueueShowInventory(ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  if (not characterRecord)
    throw std::runtime_error("Character record unavailable");

  protocol::LobbyCommandShowInventoryOK response{
    .items = {},
    .horses = {}};

  characterRecord.Immutable(
    [this, &response](const data::Character& character)
    {
      const auto itemRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(
        character.inventory());
      protocol::BuildProtocolItems(response.items, *itemRecords);

      const auto horseRecords = GetServerInstance().GetDataDirector().GetHorseCache().Get(
        character.horses());
      protocol::BuildProtocolHorses(response.horses, *horseRecords);
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleAchievementCompleteList(
  ClientId clientId,
  const protocol::LobbyCommandAchievementCompleteList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::LobbyCommandAchievementCompleteListOK response{};

  characterRecord.Immutable(
    [&response](const data::Character& character)
    {
      response.unk0 = character.uid();
    });

  // These are the level-up achievements from the `Achievement` table with the event id 75.
  response.achievements.emplace_back().tid = 20008;
  response.achievements.emplace_back().tid = 20009;
  response.achievements.emplace_back().tid = 20010;
  response.achievements.emplace_back().tid = 20011;
  response.achievements.emplace_back().tid = 20012;

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestLeagueInfo(
  ClientId clientId,
  const protocol::LobbyCommandRequestLeagueInfo& command)
{
  protocol::LobbyCommandRequestLeagueInfoOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestQuestList(
  ClientId clientId,
  const protocol::LobbyCommandRequestQuestList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::LobbyCommandRequestQuestListOK response{};

  characterRecord.Immutable(
    [&response](const data::Character& character)
    {
      response.unk0 = character.uid();
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestDailyQuestList(
  ClientId clientId,
  const protocol::LobbyCommandRequestDailyQuestList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::LobbyCommandRequestDailyQuestListOK response{};

  characterRecord.Immutable(
    [&response](const data::Character& character)
    {
      response.val0 = character.uid();
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestSpecialEventList(
  ClientId clientId,
  const protocol::LobbyCommandRequestSpecialEventList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::LobbyCommandRequestSpecialEventListOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::BuildPersonalInfoBasicResponse(
  const data::Character& character,
  protocol::LobbyCommandPersonalInfo& response)
{
  const auto& guildRecord = GetServerInstance().GetDataDirector().GetGuild(character.guildUid());
  if (guildRecord.IsAvailable())
  {
    guildRecord.Immutable([&response](const data::Guild& guild)
    {
      response.basic.guildName = guild.name();
    });
  }
  
  response.basic.introduction = character.introduction();
  response.basic.level = character.level();
  // TODO: implement other stats
}

void LobbyDirector::HandleRequestPersonalInfo(
  ClientId clientId,
  const protocol::LobbyCommandRequestPersonalInfo& command)
{
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    command.characterUid);

  protocol::LobbyCommandPersonalInfo response{
    .characterUid = command.characterUid,
    .type = command.type,
  };

  characterRecord.Immutable([this, &response](const data::Character& character)
  {
    switch (response.type)
    {
      case protocol::LobbyCommandRequestPersonalInfo::Type::Basic:
      {
        BuildPersonalInfoBasicResponse(character, response);
        break;
      }
      case protocol::LobbyCommandRequestPersonalInfo::Type::Courses:
      {
        // TODO: implement
        break;
      }
      case protocol::LobbyCommandRequestPersonalInfo::Type::Eight:
      {
        // TODO: (what on earth uses "Eight")
        break;
      }
    }
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleSetIntroduction(ClientId clientId, const protocol::LobbyCommandSetIntroduction& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  characterRecord.Mutable(
    [&command](data::Character& character)
    {
      character.introduction() = command.introduction;
    });

  GetServerInstance().GetRanchDirector().BroadcastSetIntroductionNotify(
    clientContext.characterUid, command.introduction);
}

void LobbyDirector::HandleEnterRanch(
  ClientId clientId,
  const protocol::LobbyCommandEnterRanch& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto rancherRecord = GetServerInstance().GetDataDirector().GetCharacter(
    command.rancherUid);

  bool isRanchLocked = true;
  if (rancherRecord)
  {
    rancherRecord.Immutable([&isRanchLocked](const data::Character& rancher)
    {
      isRanchLocked = rancher.isRanchLocked();
    });
  }

  const bool isEnteringOwnRanch = command.rancherUid == clientContext.characterUid;

  if (isRanchLocked && not isEnteringOwnRanch)
  {
    protocol::LobbyCommandEnterRanchCancel response{};

    _commandServer.QueueCommand<decltype(response)>(
      clientId, [response]()
      {
        return response;
      });
  }

  QueueEnterRanchOK(clientId, command.rancherUid);
}

void LobbyDirector::QueueEnterRanchOK(
  ClientId clientId,
  data::Uid rancherUid)
{
  const auto& clientContext = GetClientContext(clientId);
  protocol::LobbyCommandEnterRanchOK response{
    .rancherUid = rancherUid,
    .otp = GetServerInstance().GetOtpSystem().GrantCode(clientContext.characterUid),
    .ranchAddress = GetConfig().advertisement.ranch.address.to_uint(),
    .ranchPort = GetConfig().advertisement.ranch.port};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGetMessengerInfo(
  ClientId clientId,
  const protocol::LobbyCommandGetMessengerInfo& command)
{
  protocol::LobbyCommandGetMessengerInfoOK response{
    .code = 0xDEAD,
    .ip = static_cast<uint32_t>(htonl(GetConfig().advertisement.messenger.address.to_uint())),
    .port = GetConfig().advertisement.messenger.port,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGoodsShopList(
  ClientId clientId,
  const protocol::AcCmdCLGoodsShopList& command)
{
  protocol::AcCmdCLGoodsShopListOK response{
    .data = command.data};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  const std::string xml =
    "<ShopList>\n"
    "  <GoodsList>\n"
    "    <GoodsSQ>0</GoodsSQ>\n"
    "    <SetType>0</SetType>\n"
    "    <MoneyType>0</MoneyType>\n"
    "    <GoodsType>0</GoodsType>\n"
    "    <RecommendType>1</RecommendType>\n"
    "    <RecommendNO>1</RecommendNO>\n"
    "    <GiftType>0</GiftType>\n"
    "    <SalesRank>1</SalesRank>\n"
    "    <BonusGameMoney>0</BonusGameMoney>\n"
    "    <GoodsNM><![CDATA[Goods name]]></GoodsNM>\n"
    "    <GoodsDesc><![CDATA[Goods desc]]></GoodsDesc>\n"
    "    <ItemCapacityDesc><![CDATA[Capacity desc]]></ItemCapacityDesc>\n"
    "    <SellST>0</SellST>\n"
    "    <ItemUID>30013</ItemUID>\n"
    "    <ItemElem>\n"
    "      <Item>\n"
    "        <PriceID>1</PriceID>\n"
    "        <PriceRange>1</PriceRange>\n"
    "        <GoodsPrice>1</GoodsPrice>\n"
    "      </Item>\n"
    "    </ItemElem>\n"
    "  </GoodsList>\n"
    "</ShopList>\n";

  std::vector<std::byte> compressedXml;
  compressedXml.resize(xml.size());

  uLongf compressedSize = compressedXml.size();
  compress(
    reinterpret_cast<Bytef*>(compressedXml.data()),
    &compressedSize,
    reinterpret_cast<const Bytef*>(xml.c_str()),
    xml.length());

  compressedXml.resize(compressedSize);

  protocol::AcCmdLCGoodsShopListData data{
    .member3 = 1,
    .data = compressedXml};

  _commandServer.QueueCommand<decltype(data)>(
    clientId,
    [data]()
    {
      return data;
    });
}

void LobbyDirector::HandleInquiryTreecash(
  ClientId clientId,
  const protocol::AcCmdCLInquiryTreecash& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::LobbyCommandInquiryTreecashOK response{};

  characterRecord.Immutable(
    [&response](const data::Character& character)
    {
      response.cash = character.cash();
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGuildPartyList(
  ClientId clientId,
  const protocol::LobbyCommandGuildPartyList& command)
{
  const protocol::LobbyCommandGuildPartyListOK response{};
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleUpdateSystemContent(
  ClientId clientId,
  const protocol::LobbyCommandUpdateSystemContent& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  bool hasPermission = false;
  characterRecord.Immutable([&hasPermission](const data::Character& character)
  {
    hasPermission = character.role() != data::Character::Role::User;
  });

  if (not hasPermission)
    return;

  _systemContent.values[command.key] = command.value;

  protocol::LobbyCommandUpdateSystemContentNotify notify{
    .systemContent = _systemContent};

  for (const auto& connectedClientId : _clients | std::views::keys)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      connectedClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void LobbyDirector::HandleChangeRanchOption(
  ClientId clientId,
  const protocol::LobbyCommandChangeRanchOption& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
  protocol::LobbyCommandChangeRanchOptionOK response{
    .unk0 = command.unk0,
    .unk1 = command.unk1,
    .unk2 = command.unk2};
  characterRecord.Mutable([](data::Character& character)
    {
      character.isRanchLocked() = !character.isRanchLocked();
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

LobbyDirector::ClientContext& LobbyDirector::GetClientContext(
  const ClientId clientId,
  const bool requireAuthentication)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Lobby client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Lobby client is not authenticated");

  return clientContext;
}

void LobbyDirector::HandleUpdateUserSettings(
  ClientId clientId,
  const protocol::AcCmdCLUpdateUserSettings& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
   clientContext.characterUid);

  auto settingsUid = data::InvalidUid;
  characterRecord.Immutable([&settingsUid](const data::Character& character)
  {
    settingsUid = character.settingsUid();
  });

  const bool wasCreated = settingsUid == data::InvalidUid;
  const auto settingsRecord = settingsUid != data::InvalidUid
    ? _serverInstance.GetDataDirector().GetSettings(settingsUid)
    : _serverInstance.GetDataDirector().CreateSettings();

  settingsRecord.Mutable([&settingsUid, &command](data::Settings& settings)
  {
    // Copy the keyboard bindings if present in the command.
    if (command.settings.typeBitset.test(protocol::Settings::Keyboard))
    {
      if (not settings.keyboardBindings())
        settings.keyboardBindings().emplace();

      for (const auto& protocolBinding : command.settings.keyboardOptions.bindings)
      {
        auto& bindingRecord = settings.keyboardBindings()->emplace_back();

        bindingRecord.type = protocolBinding.type;
        bindingRecord.primaryKey = protocolBinding.primaryKey;
        bindingRecord.secondaryKey = protocolBinding.secondaryKey;
      }
    }

    // Copy the gamepad bindings if present in the command.
    if (command.settings.typeBitset.test(protocol::Settings::Gamepad))
    {
      if (not settings.gamepadBindings())
        settings.gamepadBindings().emplace();

      auto protocolBindings = command.settings.gamepadOptions.bindings;

      // The last binding is invalid, sends type 2 and overwrites real settings
      if (!protocolBindings.empty())
       protocolBindings.pop_back();

      for (const auto& protocolBinding : protocolBindings)
      {
        auto& bindingRecord = settings.gamepadBindings()->emplace_back();

        bindingRecord.type = protocolBinding.type;
        bindingRecord.primaryKey = protocolBinding.primaryButton;
        bindingRecord.secondaryKey = protocolBinding.secondaryButton;
      }
    }

    // Copy the macros if present in the command.
    if (command.settings.typeBitset.test(protocol::Settings::Macros))
    {
      settings.macros() = command.settings.macroOptions.macros;
    }

    settingsUid = settings.uid();
  });

  if (wasCreated)
  {
    characterRecord.Mutable([&settingsUid](data::Character& character)
    {
      character.settingsUid() = settingsUid;
    });
  }

  // We explicitly do not update the `age` and `hideAge` members,
  // as the client uses dedicated `AcCmdCRChangeAge` and `AcCmdCRHideAge` commands instead.

  protocol::AcCmdCLUpdateUserSettingsOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestMountInfo(
  ClientId clientId,
  const protocol::AcCmdCLRequestMountInfo& command)
{
  protocol::AcCmdCLRequestMountInfoOK response{
    .characterUid = command.characterUid,
  };

  const auto& characterUid = GetClientContext(clientId).characterUid;
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(characterUid);

  std::vector<protocol::AcCmdCLRequestMountInfoOK::MountInfo> mountInfos;
  std::vector<data::Uid> mountUids;
  characterRecord.Immutable([&mountUids](const data::Character& character)
  {
    mountUids = character.horses();
    if (character.mountUid() != data::InvalidUid)
      mountUids.emplace_back(character.mountUid());
  });

  for (const auto mountUid : mountUids)
  {
    protocol::AcCmdCLRequestMountInfoOK::MountInfo mountInfo;
    mountInfo.horseUid = mountUid;
    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorse(mountUid);

    horseRecord.Immutable([&mountInfo](const data::Horse& horse)
      {
        mountInfo.boostsInARow = horse.mountInfo.boostsInARow();
        mountInfo.winsSpeedSingle = horse.mountInfo.winsSpeedSingle();
        mountInfo.winsSpeedTeam = horse.mountInfo.winsSpeedTeam();
        mountInfo.winsMagicSingle = horse.mountInfo.winsMagicSingle();
        mountInfo.winsMagicTeam = horse.mountInfo.winsMagicTeam();
        mountInfo.totalDistance = horse.mountInfo.totalDistance();
        mountInfo.topSpeed = horse.mountInfo.topSpeed();
        mountInfo.longestGlideDistance = horse.mountInfo.longestGlideDistance();
        mountInfo.participated = horse.mountInfo.participated();
        mountInfo.cumulativePrize = horse.mountInfo.cumulativePrize();
        mountInfo.biggestPrize = horse.mountInfo.biggestPrize();
      });

    mountInfos.emplace_back(std::move(mountInfo));
  }

  response.mountInfos = std::move(mountInfos);

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleDeclineInviteToGuild(
  ClientId clientId,
  const protocol::AcCmdLCInviteGuildJoinCancel& command)
{
  // TODO: command data check
  GetServerInstance().GetRanchDirector().SendGuildInviteDeclined(
    command.characterUid,
    command.inviterCharacterUid,
    command.inviterCharacterName,
    command.guild.uid
  );
}

void LobbyDirector::HandleAcceptInviteToGuild(
  ClientId clientId,
  const protocol::AcCmdLCInviteGuildJoinOK& command)
{
  // TODO: command data check

  const auto& clientContext = GetClientContext(clientId);

  // Pending invites for guild
  auto& pendingGuildInvites = _pendingGuildInvites[command.guild.uid];

  // Check if character has guild invite
  const auto& guildInvite = std::find(
    pendingGuildInvites.begin(),
    pendingGuildInvites.end(),
    clientContext.characterUid);

  if (guildInvite != pendingGuildInvites.end())
  {
    // Guild invite exists, erase and process
    pendingGuildInvites.erase(guildInvite);
  }
  else
  {
    // Character tried to join guild but has no pending (online) invite
    spdlog::warn("Character {} tried to join a guild {} but does not have a valid invite",
      clientContext.characterUid, command.guild.uid);
    return;
  }

  std::string inviteeCharacterName;
  GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&inviteeCharacterName, guildUid = command.guild.uid](data::Character& character)
  {
    inviteeCharacterName = character.name();
    character.guildUid() = guildUid;
  });

  bool guildAddSuccess = false;
  GetServerInstance().GetDataDirector().GetGuild(command.guild.uid).Mutable(
    [&guildAddSuccess, inviteeCharacterUid = command.characterUid](data::Guild& guild)
    {
      // Check if invitee who accepted is in the guild
      if (std::ranges::contains(guild.members(), inviteeCharacterUid) ||
          std::ranges::contains(guild.officers(), inviteeCharacterUid) ||
          guild.owner() == inviteeCharacterUid)
      {
        spdlog::warn("Character {} tried to join guild {} that they are already a part of",
          inviteeCharacterUid, guild.uid());
        return;
      }

      guild.members().emplace_back(inviteeCharacterUid);
      guildAddSuccess = true;
    });

  if (not guildAddSuccess)
  {
    // TODO: return some error to the accepting client?
    return;
  }

  GetServerInstance().GetRanchDirector().SendGuildInviteAccepted(
    command.guild.uid,
    command.characterUid,
    inviteeCharacterName
  );
}

} // namespace server
