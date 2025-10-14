//
// Created by maros.prejsa on 14/10/2025.
//

#include "server/lobby/LobbyNetworkHandler.hpp"

#include "server/ServerInstance.hpp"

#include <libserver/data/helper/ProtocolHelper.hpp>

#include <boost/container_hash/hash.hpp>
#include <spdlog/spdlog.h>
#include <zlib.h>

#include <random>

namespace server
{

namespace
{

//! A random device for random number generation.
std::random_device rd;

} // anon namespace

LobbyNetworkHandler::LobbyNetworkHandler(
  ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
{
  _commandServer.RegisterCommandHandler<protocol::AcCmdCLLogin>(
  [this](const ClientId clientId, const auto& command)
  {
    HandleLogin(clientId, command);
  });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCLCreateNickname>(
  [this](const ClientId clientId, const auto& command)
  {
    HandleCreateNickname(clientId, command);
  });
}

void LobbyNetworkHandler::Initialize()
{
  const auto& lobbyConfig = _serverInstance.GetLobbyDirector().GetConfig();

  spdlog::debug(
    "Lobby is advertising ranch server on {}:{}",
    lobbyConfig.advertisement.ranch.address.to_string(),
    lobbyConfig.advertisement.ranch.port);
  spdlog::debug(
    "Lobby is advertising race server on {}:{}",
    lobbyConfig.advertisement.race.address.to_string(),
    lobbyConfig.advertisement.race.port);
  spdlog::debug(
    "Lobby is advertising messenger server on {}:{}",
    lobbyConfig.advertisement.messenger.address.to_string(),
    lobbyConfig.advertisement.messenger.port);

  spdlog::debug(
    "Lobby server listening on {}:{}",
    lobbyConfig.listen.address.to_string(),
    lobbyConfig.listen.port);

  _commandServer.BeginHost(lobbyConfig.listen.address, lobbyConfig.listen.port);
}

void LobbyNetworkHandler::Terminate()
{
  _commandServer.EndHost();
}

void LobbyNetworkHandler::SendUserLoginRejected(
  const std::string& userName,
  protocol::AcCmdCLLoginCancel::Reason reason)
{
  try
  {
    const auto clientId = GetClientIdByUserName(userName);
    SendLoginCancel(clientId, reason);
  }
  catch (const std::exception&)
  {
    // We really don't care if the user disconnected.
  }
}

void LobbyNetworkHandler::SendUserLoginAccepted(
  const std::string& userName)
{
  try
  {
    const auto clientId = GetClientIdByUserName(userName);
    SendLoginOK(clientId);
  }
  catch (const std::exception&)
  {
    // We really don't care if the user disconnected.
  }
}

void LobbyNetworkHandler::SendUserToCharacterCreator(
  const std::string& userName)
{
  try
  {
    const auto clientId = GetClientIdByUserName(userName);
    SendCreateNicknameNotify(clientId);
  }
  catch (const std::exception&)
  {
    // We really don't care if the user disconnected.
  }
}

void LobbyNetworkHandler::SendCharacterGuildInvitation(
  const data::Uid inviteeUid,
  const data::Uid guildUid,
  const data::Uid inviterUid)
{
  const ClientId inviteeClientId = GetClientIdByCharacterUid(inviteeUid);

  std::string inviterName;
  _serverInstance.GetDataDirector().GetCharacter(inviteeUid).Immutable(
    [&inviterName](const data::Character& character)
    {
      inviterName = character.name();
    });

  std::string guildName, guildDescription;
  _serverInstance.GetDataDirector().GetGuild(guildUid).Immutable(
    [&guildName, &guildDescription](const data::Guild& guild)
    {
      guildName = guild.name();
      guildDescription = guild.description();
    });

  protocol::AcCmdLCInviteGuildJoin command{
    .characterUid = inviteeUid,
    .inviterCharacterUid = inviterUid, // clientContext.characterUid?
    .inviterCharacterName = inviterName,
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
    inviteeClientId,
    [command]()
    {
      return command;
    });
}

void LobbyNetworkHandler::SetCharacterVisitPreference(
  const data::Uid characterUid,
  const data::Uid rancherUid)
{
  try
  {
    const auto clientId = GetClientIdByCharacterUid(characterUid);
    auto& clientContext = GetClientContext(clientId);
    clientContext.rancherVisitPreference = rancherUid;
  }
  catch (const std::exception&)
  {
    // We really don't care if the user disconnected.
  }
}

void LobbyNetworkHandler::DisconnectCharacter(
  const data::Uid characterUid)
{
  try
  {
    const auto clientId = GetClientIdByCharacterUid(characterUid);
    _commandServer.DisconnectClient(clientId);
  }
  catch (const std::exception&)
  {
    // We really don't care if the user disconnected.
  }
}

void LobbyNetworkHandler::MuteCharacter(
  const data::Uid characterUid,
  const data::Clock::time_point expiration)
{
  try
  {
    const auto clientId = GetClientIdByCharacterUid(characterUid);

    protocol::AcCmdLCOpMute mute{
      .duration = util::TimePointToAliciaTime(expiration)};
    _commandServer.QueueCommand<decltype(mute)>(
      clientId,
      [mute]()
      {
        return mute;
      });
  }
  catch (const std::exception&)
  {
    // We really don't care if the user disconnected.
  }
}

void LobbyNetworkHandler::NotifyCharacter(
  const data::Uid characterUid,
  const std::string& message)
{
  try
  {
    const auto clientId = GetClientIdByCharacterUid(characterUid);

    protocol::AcCmdLCNotice notice{
      .notice = message};
    _commandServer.QueueCommand<decltype(notice)>(
      clientId,
      [notice]()
      {
        return notice;
      });
  }
  catch (const std::exception&)
  {
    // We really don't care if the user disconnected.
  }
}

ClientId LobbyNetworkHandler::GetClientIdByUserName(
  const std::string& userName,
  const bool requiresAuthorization)
{
  for (const auto& [clientId, clientContext] : _clients)
  {
    if (clientContext.userName != userName)
      continue;

    if (clientContext.isAuthenticated || not requiresAuthorization)
      return clientId;
  }

  throw std::runtime_error(
    std::format(
      "Lobby client with the user name '{}' is not available or not authenticated",
      userName));
}

ClientId LobbyNetworkHandler::GetClientIdByCharacterUid(
  const data::Uid characterUid,
  const bool requiresAuthorization)
{
  for (const auto& [clientId, clientContext] : _clients)
  {
    if (clientContext.characterUid != characterUid)
      continue;

    if (clientContext.isAuthenticated || not requiresAuthorization)
      return clientId;
  }

  throw std::runtime_error(
    std::format(
      "Lobby client with the character uid '{}' is not available or not authenticated",
      characterUid));
}

LobbyNetworkHandler::ClientContext& LobbyNetworkHandler::GetClientContext(
  const ClientId clientId,
  bool requireAuthentication)
{
  auto clientContextIter = _clients.find(clientId);
  if (clientContextIter == _clients.end())
    throw std::runtime_error("Lobby client is not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Lobby client is not authenticated");

  return clientContext;
}

void LobbyNetworkHandler::HandleClientConnected(ClientId clientId)
{
  _clients.try_emplace(clientId);
}

void LobbyNetworkHandler::HandleClientDisconnected(ClientId clientId)
{
  _clients.erase(clientId);
}

void LobbyNetworkHandler::HandleLogin(
  const ClientId clientId,
  const protocol::AcCmdCLLogin& command)
{
  // Alicia 1.0
  assert(command.constant0 == 50
    && command.constant1 == 281
    && "Game version mismatch");

  // Validate the command fields.
  if (command.loginId.empty() || command.authKey.empty())
  {
    SendLoginCancel(clientId, protocol::AcCmdCLLoginCancel::Reason::InvalidLoginId);
    return;
  }

  for (const auto& clientContext : _clients | std::views::values)
  {
    if (clientContext.userName != command.loginId || not clientContext.isAuthenticated)
      continue;

    SendLoginCancel(clientId, protocol::AcCmdCLLoginCancel::Reason::Duplicated);
    return;
  }

  _serverInstance.GetLobbyDirector().QueueUserLogin(command.loginId, command.authKey);
}

void LobbyNetworkHandler::SendLoginOK(ClientId clientId)
{
  auto& clientContext = GetClientContext(clientId);

  clientContext.isAuthenticated = true;

  const auto userRecord = _serverInstance.GetDataDirector().GetUserCache().Get(
    clientContext.userName);
  if (not userRecord)
    throw std::runtime_error("User record unavailable");

  const auto& lobbyConfig = _serverInstance.GetLobbyDirector().GetConfig();

  // Get the character UID of the user.
  auto userCharacterUid{data::InvalidUid};
  userRecord->Immutable(
    [&userCharacterUid](
      const data::User& user)
    {
      userCharacterUid = user.characterUid();
    });

  clientContext.characterUid = userCharacterUid;

  // Get the character record and fill the protocol data.
  // Also get the UID of the horse mounted by the character.
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    userCharacterUid);
  if (not characterRecord)
    throw std::runtime_error("Character record unavailable");

  protocol::LobbyCommandLoginOK response{
    .lobbyTime = util::TimePointToFileTime(util::Clock::now()),
    // .member0 = 0xCA794,
    .motd = std::format(
      "Welcome to Story of Alicia. Players online: {}",
      _serverInstance.GetLobbyDirector().GetUsers().size()),
    .val1 = 0x0,
    .val3 = 0x0,

    .missions = {
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x18,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
          .id = 2,
          .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x1F,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x23,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x29,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x2A,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x2B,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x2C,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x2D,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x2E,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},
      protocol::LobbyCommandLoginOK::Mission{
        .id = 0x2F,
        .progress = {
          protocol::LobbyCommandLoginOK::Mission::Progress{
            .id = 2,
            .value = 1}}},},

    .ranchAddress = lobbyConfig.advertisement.ranch.address.to_uint(),
    .ranchPort = lobbyConfig.advertisement.ranch.port,
    .scramblingConstant = 0,

    .systemContent = _systemContent,

    // .managementSkills = {4, 0x2B, 4},
    // .skillRanks = {.values = {{1,1}}},
    // .val14 = 0xca1b87db,
    // .guild = {.val1 = 1},
    // .val16 = 4,
    // .val18 = 0x2a,
    // .val19 = 0x38d,
    //.val20 = 0x1c7
  };
  
  data::Uid characterMountUid{
    data::InvalidUid};

  characterRecord.Immutable(
    [this, justCreatedCharacter = clientContext.justCreatedCharacter, &response, &characterMountUid](const data::Character& character)
    {
      response.uid = character.uid();
      response.name = character.name();

      response.introduction = character.introduction();

      // todo: model constant
      response.gender = character.parts.modelId() == 10
        ? protocol::Gender::Boy
        : protocol::Gender::Girl;

      response.level = character.level();
      response.carrots = character.carrots();
      response.role = std::bit_cast<protocol::LobbyCommandLoginOK::Role>(
        character.role());

      if (not justCreatedCharacter)
        response.bitfield = protocol::LobbyCommandLoginOK::HasPlayedBefore;

      // Character equipment.
      const auto characterEquipmentItems = _serverInstance.GetDataDirector().GetItemCache().Get(
        character.characterEquipment());
      if (not characterEquipmentItems)
        throw std::runtime_error("Character equipment items unavailable");

      protocol::BuildProtocolItems(
        response.characterEquipment,
        *characterEquipmentItems);

      // Mount equipment.
      const auto mountEquipmentItems = _serverInstance.GetDataDirector().GetItemCache().Get(
        character.mountEquipment());
      if (not mountEquipmentItems)
        throw std::runtime_error("Character equipment items unavailable");

      protocol::BuildProtocolItems(
        response.mountEquipment,
        *mountEquipmentItems);

      protocol::BuildProtocolCharacter(
        response.character,
        character);

      if (character.guildUid() != data::InvalidUid)
      {
        const auto guildRecord = _serverInstance.GetDataDirector().GetGuild(
          character.guildUid());
        if (not guildRecord)
          throw std::runtime_error("Character's guild not available");

        std::vector<uint32_t> guildMembers;
        guildRecord.Immutable([&response, &guildMembers](const data::Guild& guild)
        {
          guildMembers = guild.members();
          protocol::BuildProtocolGuild(response.guild, guild);
          const bool isOwner = guild.owner() == response.uid;
          const bool isOfficer = std::ranges::contains(guild.officers(), response.uid);
          const bool isMember = std::ranges::contains(guild.members(), response.uid);

          if (isOwner)
            response.guild.guildRole = protocol::GuildRole::Owner;
          else if (isOfficer)
            response.guild.guildRole = protocol::GuildRole::Officer;
          else if (isMember)
            response.guild.guildRole = protocol::GuildRole::Member;
          else
            throw std::runtime_error("Character is in a guild but not a member");
        });

        // FIXME: a patch to preload characters in the guild to memory
        // so the guild members list can compile and display fully
        for (const auto& guildMember : guildMembers)
        {
          // Just get character and don't do anything with it
          _serverInstance.GetDataDirector().GetCharacterCache().Get(guildMember, true);
        }
      }

      if (character.petUid() != data::InvalidUid)
      {
        const auto petRecord =  _serverInstance.GetDataDirector().GetPet(
          character.petUid());
        if (not petRecord)
          throw std::runtime_error("Character's pet not available");

        petRecord.Immutable([&response](const data::Pet& pet)
        {
          protocol::BuildProtocolPet(response.pet, pet);
        });
      }

      if (character.settingsUid() != data::InvalidUid)
      {
        const auto settingsRecord = _serverInstance.GetDataDirector().GetSettingsCache().Get(
          character.settingsUid());
        if (not settingsRecord)
          throw std::runtime_error("Character's settings not available");

        settingsRecord->Immutable([&response](const data::Settings& settings)
        {
          // We set the age despite if the hide age is set,
          // just so the user is able to see the last value set by them.
          response.settings.age = settings.age();
          response.settings.hideAge = settings.hideAge();

          protocol::BuildProtocolSettings(response.settings, settings);
        });
      }

      characterMountUid = character.mountUid();
    });

  // Get the mounted horse record and fill the protocol data.
  const auto mountRecord = _serverInstance.GetDataDirector().GetHorseCache().Get(
    characterMountUid);
  if (not mountRecord)
    throw std::runtime_error("Horse mount record unavailable");

  mountRecord->Immutable(
    [&response](const data::Horse& horse)
    {
      //    response.val17 = {
      //      .mountUid = horse.uid(),
      //      .tid = 0x12,
      //      .val2 = 0x16e67e4};

      protocol::BuildProtocolHorse(response.horse, horse);
    });

  _commandServer.SetCode(clientId, {});

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  protocol::AcCmdLCSkillCardPresetList skillPresetListResponse{};
  characterRecord.Immutable([&skillPresetListResponse](const data::Character& character)
  {
    const auto& speed = character.skills.speed();
    skillPresetListResponse.speedActiveSetId = speed.activeSetId;
    const auto& magic = character.skills.magic();
    skillPresetListResponse.magicActiveSetId = magic.activeSetId;

    skillPresetListResponse.skillSets = {
      protocol::SkillSet{.setId = 0, .gamemode = protocol::GameMode::Speed, .skills = {speed.set1.slot1, speed.set1.slot2}},
      protocol::SkillSet{.setId = 1, .gamemode = protocol::GameMode::Speed, .skills = {speed.set2.slot1, speed.set2.slot2}},
      protocol::SkillSet{.setId = 0, .gamemode = protocol::GameMode::Magic, .skills = {magic.set1.slot1, magic.set1.slot2}},
      protocol::SkillSet{.setId = 1, .gamemode = protocol::GameMode::Magic, .skills = {magic.set2.slot1, magic.set2.slot2}}
    };
  });

  _commandServer.QueueCommand<decltype(skillPresetListResponse)>(
    clientId,
    [skillPresetListResponse]()
    {
      return skillPresetListResponse;
    });
}

void LobbyNetworkHandler::SendLoginCancel(
  const ClientId clientId,
  const protocol::AcCmdCLLoginCancel::Reason reason)
{
  _commandServer.QueueCommand<protocol::AcCmdCLLoginCancel>(
    clientId,
    [reason]()
    {
      return protocol::AcCmdCLLoginCancel{
      .reason = reason };
    });
}

void LobbyNetworkHandler::HandleRoomList(
  const ClientId clientId,
  const protocol::AcCmdCLRoomList& command)
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

void LobbyNetworkHandler::HandleHeartbeat(
  const ClientId clientId)
{
  // todo: implement heartbeat statistics
}

void LobbyNetworkHandler::HandleMakeRoom(
  ClientId clientId,
  const protocol::LobbyCommandMakeRoom& command)
{
  const auto& clientContext = GetClientContext(clientId);
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

  const auto lobbyConfig = _serverInstance.GetLobbyDirector().GetConfig();
  protocol::LobbyCommandMakeRoomOK response{
    .roomUid = createdRoomUid,
    .oneTimePassword = roomOtp,
    .raceServerAddress = lobbyConfig.advertisement.race.address.to_uint(),
    .raceServerPort = lobbyConfig.advertisement.race.port};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyNetworkHandler::HandleEnterRoom(
  const ClientId clientId,
  const protocol::AcCmdCLEnterRoom& command)
{
  const auto& clientContext = GetClientContext(clientId);

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
    protocol::AcCmdCLEnterRoomCancel response{
      .status = protocol::AcCmdCLEnterRoomCancel::Status::CR_INVALID_ROOM};

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
    protocol::AcCmdCLEnterRoomCancel response{
      .status = protocol::AcCmdCLEnterRoomCancel::Status::CR_BAD_PASSWORD};

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
    protocol::AcCmdCLEnterRoomCancel response{
      .status = protocol::AcCmdCLEnterRoomCancel::Status::CR_CROWDED_ROOM};

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

  const auto& lobbyConfig = _serverInstance.GetLobbyDirector().GetConfig();

  protocol::AcCmdCLEnterRoomOK response{
    .roomUid = command.roomUid,
    .oneTimePassword = roomOtp,
    .raceServerAddress = lobbyConfig.advertisement.race.address.to_uint(),
    .raceServerPort = lobbyConfig.advertisement.race.port};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Schedule removal of the player from the room queue.
  _serverInstance.GetLobbyDirector().GetScheduler().Queue(
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

void LobbyNetworkHandler::HandleLeaveRoom(
  const ClientId clientId,
  const protocol::AcCmdCLLeaveRoom& command)
{
  spdlog::error("Not implemented - client {} left a room", clientId);
}

void LobbyNetworkHandler::HandleEnterChannel(
  const ClientId clientId,
  const protocol::AcCmdCLEnterChannel& command)
{
  // todo: implement channels
}

void LobbyNetworkHandler::HandleLeaveChannel(
  const ClientId clientId,
  const protocol::AcCmdCLLeaveChannel& command)
{
  // todo: implement channels
}

void LobbyNetworkHandler::SendCreateNicknameNotify(ClientId clientId)
{
  protocol::LobbyCommandCreateNicknameNotify notify{};

  _commandServer.QueueCommand<decltype(notify)>(
    clientId,
    [notify]()
    {
      return notify;
    });
}

void LobbyNetworkHandler::HandleCreateNickname(
  const ClientId clientId,
  const protocol::AcCmdCLCreateNickname& command)
{
  auto& clientContext = GetClientContext(clientId);

  clientContext.justCreatedCharacter = true;

  const auto userRecord = _serverInstance.GetDataDirector().GetUserCache().Get(
    clientContext.userName);
  if (not userRecord)
    throw std::runtime_error("User record does not exist");

  auto userCharacterUid{data::InvalidUid};
  userRecord->Immutable([&userCharacterUid](const data::User& user)
  {
    userCharacterUid = user.characterUid();
  });

  std::optional<Record<data::Character>> userCharacter;

  if (userCharacterUid == data::InvalidUid)
  {
    // Create a new mount for the character.
    const auto mountRecord  = _serverInstance.GetDataDirector().CreateHorse();

    auto mountUid = data::InvalidUid;
    mountRecord.Mutable(
      [this, &mountUid](data::Horse& horse)
      {
        // The TID of the horse specifies which body mesh is used for that horse.
        // Can be found in the `MountPartInfo` table.
        horse.tid() = 20002;
        horse.dateOfBirth() = data::Clock::now();
        horse.mountCondition.stamina = 3500;
        horse.growthPoints() = 150;

        _serverInstance.GetHorseRegistry().BuildRandomHorse(
          horse.parts,
          horse.appearance);

        mountUid = horse.uid();
      });

    // Create the new character.
    userCharacter = _serverInstance.GetDataDirector().CreateCharacter();
    userCharacter->Mutable(
      [&userCharacterUid,
        &mountUid,
        &command](data::Character& character)
      {
        character.name = command.nickname;

        // todo: default level configured
        character.level = 60;
        // todo: default carrots configured
        character.carrots = 10'000;

        character.mountUid() = mountUid;

        userCharacterUid = character.uid();
      });

    // Assign the character to the user.
    userRecord->Mutable(
      [&userCharacterUid](data::User& user)
      {
        user.characterUid() = userCharacterUid;
      });
  }
  else
  {
    // Retrieve the existing character.
    userCharacter = _serverInstance.GetDataDirector().GetCharacter(
      userCharacterUid);
  }

  assert(userCharacter.has_value());

  // Update the character's parts and appearance.
  userCharacter->Mutable(
    [&command](data::Character& character)
    {
      character.parts = data::Character::Parts{
        .modelId = command.character.parts.charId,
        .mouthId = command.character.parts.mouthSerialId,
        .faceId = command.character.parts.faceSerialId};
      character.appearance = data::Character::Appearance{
        .voiceId = command.character.appearance.voiceId,
        .headSize = command.character.appearance.headSize,
        .height = command.character.appearance.height,
        .thighVolume = command.character.appearance.thighVolume,
        .legVolume = command.character.appearance.legVolume,
        .emblemId = command.character.appearance.emblemId,
      };
    });

  _serverInstance.GetLobbyDirector().QueueCharacterCreated(
    clientContext.userName);
}

void LobbyNetworkHandler::HandleShowInventory(
  const ClientId clientId,
  const protocol::AcCmdCLShowInventory& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  if (not characterRecord)
    throw std::runtime_error("Character record unavailable");

  protocol::LobbyCommandShowInventoryOK response{
    .items = {},
    .horses = {}};

  characterRecord.Immutable(
    [this, &response](const data::Character& character)
    {
      const auto itemRecords = _serverInstance.GetDataDirector().GetItemCache().Get(
        character.inventory());
      protocol::BuildProtocolItems(response.items, *itemRecords);

      const auto horseRecords = _serverInstance.GetDataDirector().GetHorseCache().Get(
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

void LobbyNetworkHandler::HandleUpdateUserSettings(
  const ClientId clientId,
  const protocol::AcCmdCLUpdateUserSettings& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
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

void LobbyNetworkHandler::HandleEnterRoomQuick(
  const ClientId clientId,
  const protocol::AcCmdCLEnterRoomQuick& command)
{
  // todo: implement quick room enter
  spdlog::error("Not implemented - enter room quick");
  // AcCmdCLEnterRoomQuickSuccess
}

void LobbyNetworkHandler::HandleGoodsShopList(
  const ClientId clientId,
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

void LobbyNetworkHandler::HandleAchievementCompleteList(
  const ClientId clientId,
  const protocol::AcCmdCLAchievementCompleteList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCLAchievementCompleteListOK response{};

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

void LobbyNetworkHandler::HandleRequestPersonalInfo(
  const ClientId clientId,
  const protocol::AcCmdCLRequestPersonalInfo& command)
{
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    command.characterUid);

  protocol::AcCmdLCPersonalInfo response{
    .characterUid = command.characterUid,
    .type = command.type,};

  characterRecord.Immutable([this, &response](const data::Character& character)
  {
    switch (response.type)
    {
      case protocol::AcCmdCLRequestPersonalInfo::Type::Basic:
      {
        const auto& guildRecord = _serverInstance.GetDataDirector().GetGuild(
          character.guildUid());
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
        break;
      }
      case protocol::AcCmdCLRequestPersonalInfo::Type::Courses:
      {
        // TODO: implement
        break;
      }
      case protocol::AcCmdCLRequestPersonalInfo::Type::Eight:
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

void LobbyNetworkHandler::HandleEnterRanch(
  const ClientId clientId,
  const protocol::AcCmdCLEnterRanch& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto rancherRecord = _serverInstance.GetDataDirector().GetCharacter(
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
    protocol::AcCmdCLEnterRanchCancel response{};

    _commandServer.QueueCommand<decltype(response)>(
      clientId, [response]()
      {
        return response;
      });
  }

  SendEnterRanchOK(clientId, command.rancherUid);
}

void LobbyNetworkHandler::HandleEnterRanchRandomly(
  const ClientId clientId,
  const protocol::AcCmdCLEnterRanchRandomly& command)
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

    auto& characters = _serverInstance.GetDataDirector().GetCharacterCache();
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

  SendEnterRanchOK(clientId, rancherUid);
}

void LobbyNetworkHandler::SendEnterRanchOK(
  const ClientId clientId,
  const data::Uid rancherUid)
{
  const auto& clientContext = GetClientContext(clientId);

  const auto& lobbyConfig = _serverInstance.GetLobbyDirector().GetConfig();

  protocol::AcCmdCLEnterRanchOK response{
    .rancherUid = rancherUid,
    .otp = _serverInstance.GetOtpSystem().GrantCode(clientContext.characterUid),
    .ranchAddress = lobbyConfig.advertisement.ranch.address.to_uint(),
    .ranchPort = lobbyConfig.advertisement.ranch.port};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyNetworkHandler::HandleFeatureCommand(
  const ClientId clientId,
  const protocol::AcCmdCLFeatureCommand& command)
{
  spdlog::warn("Feature command: {}", command.command);
}

void LobbyNetworkHandler::HandleRequestFestivalResult(
  const ClientId clientId,
  const protocol::AcCmdCLRequestFestivalResult& command)
{
  // todo: implement festival
}

void LobbyNetworkHandler::HandleSetIntroduction(
  const ClientId clientId,
  const protocol::AcCmdCLSetIntroduction& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  characterRecord.Mutable(
    [&command](data::Character& character)
    {
      character.introduction() = command.introduction;
    });

  _serverInstance.GetRanchDirector().BroadcastSetIntroductionNotify(
    clientContext.characterUid, command.introduction);
}

void LobbyNetworkHandler::HandleGetMessengerInfo(
  const ClientId clientId,
  const protocol::AcCmdCLGetMessengerInfo& command)
{
  const auto& lobbyConfig = _serverInstance.GetLobbyDirector().GetConfig();
  
  protocol::AcCmdCLGetMessengerInfoOK response{
    .code = 0xDEAD,
    .ip = static_cast<uint32_t>(htonl(lobbyConfig.advertisement.messenger.address.to_uint())),
    .port = lobbyConfig.advertisement.messenger.port,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyNetworkHandler::HandleCheckWaitingSeqno(
  const ClientId clientId,
  const protocol::AcCmdCLCheckWaitingSeqno& command)
{
  // todo: implement waiting
}

void LobbyNetworkHandler::HandleUpdateSystemContent(
  const ClientId clientId,
  const protocol::AcCmdCLUpdateSystemContent& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  bool hasPermission = false;
  characterRecord.Immutable([&hasPermission](const data::Character& character)
  {
    hasPermission = character.role() != data::Character::Role::User;
  });

  if (not hasPermission)
    return;

  _systemContent.values[command.key] = command.value;

  protocol::AcCmdLCUpdateSystemContent notify{
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

void LobbyNetworkHandler::HandleEnterRoomQuickStop(
  const ClientId clientId,
  const protocol::AcCmdCLEnterRoomQuickStop& command)
{
  // todo: implement quick enter
}

void LobbyNetworkHandler::HandleRequestFestivalPrize(
  const ClientId clientId,
  const protocol::AcCmdCLRequestFestivalPrize& command)
{
  // todo: implement festivals
}

void LobbyNetworkHandler::HandleQueryServerTime(
  const ClientId clientId)
{
  protocol::AcCmdCLQueryServerTimeOK response{
    .lobbyTime = util::TimePointToFileTime(std::chrono::system_clock::now())};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyNetworkHandler::HandleRequestMountInfo(
  const ClientId clientId,
  const protocol::AcCmdCLRequestMountInfo& command)
{

  const auto& characterUid = GetClientContext(clientId).characterUid;
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(characterUid);

  protocol::AcCmdCLRequestMountInfoOK response{
    .characterUid = command.characterUid,};

  std::vector<data::Uid> mountUids;
  characterRecord.Immutable([&mountUids](const data::Character& character)
  {
    mountUids = character.horses();
    if (character.mountUid() != data::InvalidUid)
      mountUids.emplace_back(character.mountUid());
  });

  for (const auto mountUid : mountUids)
  {
    auto& mountInfo = response.mountInfos.emplace_back();
    mountInfo.horseUid = mountUid;

    const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(mountUid);
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
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyNetworkHandler::HandleInquiryTreecash(
  const ClientId clientId,
  const protocol::AcCmdCLInquiryTreecash& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
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

void LobbyNetworkHandler::HandleDeclineInviteToGuild(
  const ClientId clientId,
  const protocol::AcCmdLCInviteGuildJoinCancel& command)
{
  // TODO: command data check
  _serverInstance.GetRanchDirector().SendGuildInviteDeclined(
    command.characterUid,
    command.inviterCharacterUid,
    command.inviterCharacterName,
    command.guild.uid
  );
}

void LobbyNetworkHandler::HandleAcceptInviteToGuild(
  const ClientId clientId,
  const protocol::AcCmdLCInviteGuildJoinOK& command)
{
  // TODO: command data check

  const auto& clientContext = GetClientContext(clientId);

  // Pending invites for guild
  auto& pendingGuildInvites = _serverInstance.GetLobbyDirector().GetGuilds()[command.guild.uid].invites;

  // Check if the guild has outstanding character invite.
  const auto& guildInvite = std::ranges::find(
    pendingGuildInvites,
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
  _serverInstance.GetDataDirector().GetCharacter(clientContext.characterUid).Mutable(
    [&inviteeCharacterName, guildUid = command.guild.uid](data::Character& character)
  {
    inviteeCharacterName = character.name();
    character.guildUid() = guildUid;
  });

  bool guildAddSuccess = false;
  _serverInstance.GetDataDirector().GetGuild(command.guild.uid).Mutable(
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

  _serverInstance.GetRanchDirector().SendGuildInviteAccepted(
    command.guild.uid,
    command.characterUid,
    inviteeCharacterName
  );
}

void LobbyNetworkHandler::HandleClientNotify(
  const ClientId clientId,
  const protocol::AcCmdClientNotify& command)
{
  // todo: reset roll code?
  if (command.val0 != 1)
    spdlog::error("Client error notification: state[{}], value[{}]", command.val0, command.val1);
}

void LobbyNetworkHandler::HandleChangeRanchOption(
  const ClientId clientId,
  const protocol::LobbyCommandChangeRanchOption& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
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

void LobbyNetworkHandler::HandleRequestDailyQuestList(
  const ClientId clientId,
  const protocol::AcCmdCLRequestDailyQuestList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCLRequestDailyQuestListOK response{};

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

void LobbyNetworkHandler::HandleRequestLeagueInfo(
  const ClientId clientId,
  const protocol::LobbyCommandRequestLeagueInfo& command)
{
  protocol::LobbyCommandRequestLeagueInfoOK response{};

  // todo: implement leagues

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyNetworkHandler::HandleRequestQuestList(
  const ClientId clientId,
  const protocol::LobbyCommandRequestQuestList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCLRequestQuestListOK response{};

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

void LobbyNetworkHandler::HandleRequestPSecialEventList(
  const ClientId clientId,
  const protocol::LobbyCommandRequestSpecialEventList& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // todo: figure this out

  protocol::LobbyCommandRequestSpecialEventListOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

} // namespace server
