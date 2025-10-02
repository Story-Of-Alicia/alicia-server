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

#include "server/lobby/LoginHandler.hpp"

#include "server/lobby/LobbyDirector.hpp"
#include "server/ServerInstance.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "libserver/registry/HorseRegistry.hpp"

#include "spdlog/spdlog.h"

namespace server
{

LoginHandler::LoginHandler(
  LobbyDirector& lobbyDirector,
  CommandServer& server)
  : _server(server)
  , _lobbyDirector(lobbyDirector)
{
}

void LoginHandler::Tick()
{
  while (not _clientLoginRequestQueue.empty())
  {
    const ClientId clientId = _clientLoginRequestQueue.front();
    auto& loginContext = _clientLogins[clientId];

    // Request the load of the user data if not requested yet.
    if (not loginContext.userLoadRequested)
    {
      _lobbyDirector.GetServerInstance().GetDataDirector().RequestLoadUserData(
        loginContext.userName);

      loginContext.userLoadRequested = true;
      continue;
    }

    // If the data are still being loaded do not proceed with login.
    if (_lobbyDirector.GetServerInstance().GetDataDirector().AreDataBeingLoaded(
      loginContext.userName))
    {
      continue;
    }

    _clientLoginRequestQueue.pop();

    if (not _lobbyDirector.GetServerInstance().GetDataDirector().AreUserDataLoaded(
      loginContext.userName))
    {
      spdlog::error("User data for '{}' not available", loginContext.userName);
      QueueUserLoginRejected(clientId, protocol::LobbyCommandLoginCancel::Reason::Generic);
      break;
    }

    const auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUser(
      loginContext.userName);
    assert(userRecord.IsAvailable());

    bool isAuthenticated = false;
    userRecord.Immutable(
      [&isAuthenticated, &loginContext](const data::User& user)
      {
        isAuthenticated = user.token() == loginContext.userToken;
      });

    // If the user is not authenticated reject the login.
    if (not isAuthenticated)
    {
      spdlog::debug("User '{}' failed in authentication", loginContext.userName);
      QueueUserLoginRejected(
        clientId,
        protocol::LobbyCommandLoginCancel::Reason::InvalidUser);
    }
    else
    {
      // Check for any infractions preventing the user from joining.
      const auto infractionVerdict = _lobbyDirector.GetServerInstance().GetInfractionSystem().CheckOutstandingPunishments(
        loginContext.userName);

      if (infractionVerdict.preventServerJoining)
      {
        QueueUserLoginRejected(
          clientId,
          protocol::LobbyCommandLoginCancel::Reason::DisconnectYourself);
      }
      else
      {
        // Queue the user response.
        _clientLoginResponseQueue.emplace(clientId);
      }
    }

    // Only one request per tick.
    break;
  }

  while (not _clientLoginResponseQueue.empty())
  {
    const ClientId clientId = _clientLoginResponseQueue.front();
    auto& clientContext = _lobbyDirector.GetClientContext(clientId, false);
    auto& loginContext = _clientLogins[clientId];

    // If the user character load was already requested wait for the load to complete.
    if (loginContext.userCharacterLoadRequested)
    {
      if (_lobbyDirector.GetServerInstance().GetDataDirector().AreDataBeingLoaded(
        loginContext.userName))
      {
        continue;
      }
    }

    const auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUser(
      loginContext.userName);
    assert(userRecord.IsAvailable());

    auto characterUid = data::InvalidUid;
    userRecord.Immutable(
      [&characterUid](const data::User& user)
      {
        characterUid = user.characterUid();
      });

    const bool hasCharacter = characterUid != data::InvalidUid;

    // If the user has a character request the load.
    if (hasCharacter)
    {
      // If the user character is not loaded do not proceed.
      if (not loginContext.userCharacterLoadRequested)
      {
        _lobbyDirector.GetServerInstance().GetDataDirector().RequestLoadCharacterData(
          loginContext.userName,
          characterUid);

        loginContext.userCharacterLoadRequested = true;
        continue;
      }
    }

    _clientLoginResponseQueue.pop();

    const bool forcedCharacterCreator = _lobbyDirector._forcedCharacterCreator.erase(
      characterUid) > 0;

    // If the user does not have a character or the character creator was enforced
    // send them to the character creator.
    if (not hasCharacter || forcedCharacterCreator)
    {
      loginContext.justCreatedCharacter = true;

      spdlog::debug("User '{}' sent to the character creator", loginContext.userName);
      QueueUserCreateNickname(clientId, loginContext.userName);
      return;
    }

    // If the character was not loaded reject the login.
    if (not _lobbyDirector.GetServerInstance().GetDataDirector().AreCharacterDataLoaded(
      loginContext.userName))
    {
      spdlog::error("User character data for '{}' not available", loginContext.userName);
      QueueUserLoginRejected(clientId, protocol::LobbyCommandLoginCancel::Reason::Generic);
      break;
    }

    spdlog::debug("User '{}' succeeded in authentication", loginContext.userName);
    QueueUserLoginAccepted(clientId, loginContext.userName);

    clientContext.userName = loginContext.userName;
    clientContext.characterUid = characterUid;
    clientContext.isAuthenticated = true;

    // Only one response per tick.
    break;
  }
}

void LoginHandler::HandleUserLogin(
  const ClientId clientId,
  const protocol::LobbyCommandLogin& login)
{
  // Validate the command fields.
  if (login.loginId.empty() || login.authKey.empty())
  {
    spdlog::debug(
      "LoginHandler::HandleUserLogin - Rejecting login for client {}."
      " User name or user token empty.",
      clientId);

    QueueUserLoginRejected(clientId, protocol::LobbyCommandLoginCancel::Reason::InvalidUser);
    return;
  }

  // The login request must be unique for the client.
  if (_clientLogins.contains(clientId))
  {
    spdlog::debug(
      "LoginHandler::HandleUserLogin - Rejecting login for client {} ({})."
      " User login already queued.",
      clientId,
      login.loginId);

    QueueUserLoginRejected(clientId, protocol::LobbyCommandLoginCancel::Reason::InvalidUser);
    return;
  }

  // Queue the login.
  const auto [iterator, inserted] =
    _clientLogins.try_emplace(
      clientId,
      LoginContext{
        .userName = login.loginId,
        .userToken = login.authKey});
  assert(inserted && "Duplicate client login request.");

  std::scoped_lock lock(_clientLoginRequestQueueMutex);
  _clientLoginRequestQueue.emplace(clientId);
}

void LoginHandler::HandleUserCreateCharacter(
  ClientId clientId,
  const protocol::LobbyCommandCreateNickname& command)
{
  const auto& loginContext = _clientLogins[clientId];

  const auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUserCache().Get(
    loginContext.userName);
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
    const auto mountRecord = _lobbyDirector.GetServerInstance().GetDataDirector().CreateHorse();

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

        _lobbyDirector._serverInstance.GetHorseRegistry().BuildRandomHorse(
          horse.parts,
          horse.appearance);

        mountUid = horse.uid();
      });

    // Create the new character.
    userCharacter = _lobbyDirector.GetServerInstance().GetDataDirector().CreateCharacter();
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
    userCharacter = _lobbyDirector.GetServerInstance().GetDataDirector().GetCharacter(
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

  // Queue the processing of the login response.
  _clientLoginResponseQueue.emplace(clientId);
}

void LoginHandler::HandleUserDisconnect(ClientId clientId)
{
  // todo handle disconnect
}

void LoginHandler::QueueUserLoginAccepted(
  const ClientId clientId,
  const std::string& userName)
{
  const auto loginContext = _clientLogins[clientId];
  const auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUserCache().Get(
    userName);
  if (not userRecord)
    throw std::runtime_error("User record unavailable");

  protocol::LobbyCommandLoginOK response{
    .lobbyTime = util::TimePointToFileTime(util::Clock::now()),
    // .member0 = 0xCA794,
    .motd = std::format(
      "Welcome to Story of Alicia. Players online: {}",
      _lobbyDirector._clients.size()),
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

    // .optionType = OptionType::Value,
    // .valueOptions = 0x64,

    .ranchAddress = _lobbyDirector.GetConfig().advertisement.ranch.address.to_uint(),
    .ranchPort = _lobbyDirector.GetConfig().advertisement.ranch.port,
    .scramblingConstant = 0,

    .systemContent = _lobbyDirector._systemContent,

    // .managementSkills = {4, 0x2B, 4},
    // .skillRanks = {.values = {{1,1}}},
    // .val14 = 0xca1b87db,
    // .guild = {.val1 = 1},
    // .val16 = 4,
    // .val18 = 0x2a,
    // .val19 = 0x38d,
    //.val20 = 0x1c7
  };

  // Get the character UID of the user.
  auto userCharacterUid{data::InvalidUid};
  userRecord->Immutable(
    [&userCharacterUid](
      const data::User& user)
    {
      userCharacterUid = user.characterUid();
    });

  // Get the character record and fill the protocol data.
  // Also get the UID of the horse mounted by the character.
  const auto characterRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetCharacter(
    userCharacterUid);
  if (not characterRecord)
    throw std::runtime_error("Character record unavailable");

  data::Uid characterMountUid{
    data::InvalidUid};

  characterRecord.Immutable(
    [this, justCreatedCharacter = loginContext.justCreatedCharacter, &response, &characterMountUid](const data::Character& character)
    {
      response.uid = character.uid();
      response.name = character.name();

      response.introduction = character.introduction();

      // todo: model constant
      response.gender = character.parts.modelId() == 10
        ? Gender::Boy
        : Gender::Girl;

      response.level = character.level();
      response.carrots = character.carrots();
      response.role = std::bit_cast<protocol::LobbyCommandLoginOK::Role>(
        character.role());
      response.age = character.age();
      response.hideGenderAndAge = character.hideGenderAndAge();

      if (not justCreatedCharacter)
        response.bitfield = protocol::LobbyCommandLoginOK::HasPlayedBefore;

      // Character equipment.
      const auto characterEquipmentItems = _lobbyDirector.GetServerInstance().GetDataDirector().GetItemCache().Get(
        character.characterEquipment());
      if (not characterEquipmentItems)
        throw std::runtime_error("Character equipment items unavailable");

      protocol::BuildProtocolItems(
        response.characterEquipment,
        *characterEquipmentItems);

      // Mount equipment.
      const auto mountEquipmentItems = _lobbyDirector.GetServerInstance().GetDataDirector().GetItemCache().Get(
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
        const auto guildRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetGuild(
          character.guildUid());
        if (not guildRecord)
          throw std::runtime_error("Character's guild not available");

        guildRecord.Immutable([&response](const data::Guild& guild)
        {
          protocol::BuildProtocolGuild(response.guild, guild);
        });
      }

      if (character.petUid() != data::InvalidUid)
      {
        const auto petRecord =  _lobbyDirector.GetServerInstance().GetDataDirector().GetPet(
          character.petUid());
        if (not petRecord)
          throw std::runtime_error("Character's pet not available");

        petRecord.Immutable([&response](const data::Pet& pet)
        {
          protocol::BuildProtocolPet(response.pet, pet);
        });
      }

      characterMountUid = character.mountUid();
    });

  // Get the mounted horse record and fill the protocol data.
  const auto mountRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetHorseCache().Get(characterMountUid);
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

  _server.SetCode(clientId, {});

  _server.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LoginHandler::QueueUserCreateNickname(ClientId clientId, const std::string& userName)
{
  _server.QueueCommand<protocol::LobbyCommandCreateNicknameNotify>(
    clientId,
    []()
    {
      return protocol::LobbyCommandCreateNicknameNotify{};
    });
}

void LoginHandler::QueueUserLoginRejected(ClientId clientId, protocol::LobbyCommandLoginCancel::Reason reason)
{
  _server.QueueCommand<protocol::LobbyCommandLoginCancel>(
    clientId,
    [reason]()
    {
      return protocol::LobbyCommandLoginCancel{
      .reason = reason };
    });
}

} // namespace server
