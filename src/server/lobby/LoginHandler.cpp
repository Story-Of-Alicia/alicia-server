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

#include "libserver/Constants.hpp"
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
      QueueUserLoginRejected(clientId);
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
      QueueUserLoginRejected(clientId, true);
    }
    else
    {
      // Queue the user response.
      _clientLoginResponseQueue.emplace(clientId);
    }

    // Only one request per tick.
    break;
  }

  while (not _clientLoginResponseQueue.empty())
  {
    const ClientId clientId = _clientLoginResponseQueue.front();
    auto& clientContext = _lobbyDirector._clientContext[clientId];
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

    // If the user does not have a character send them to the character creator.
    if (not hasCharacter)
    {
      spdlog::debug("User '{}' sent to character creator", loginContext.userName);
      QueueUserCreateNickname(clientId, loginContext.userName);
      return;
    }

    // If the character was not loaded reject the login.
    if (not _lobbyDirector.GetServerInstance().GetDataDirector().AreCharacterDataLoaded(
      loginContext.userName))
    {
      spdlog::error("User character data for '{}' not available", loginContext.userName);
      QueueUserLoginRejected(clientId);
      break;
    }

    spdlog::debug("User '{}' succeeded in authentication", loginContext.userName);
    QueueUserLoginAccepted(clientId, loginContext.userName);

    clientContext.characterUid = characterUid;

    // Only one response per tick.
    break;
  }
}

void LoginHandler::HandleUserLogin(
  const ClientId clientId,
  const protocol::LobbyCommandLogin& login)
{
  // Validate the command fields.
  if (login.loginId.empty())
  {
    if (login.authKey.empty() && not constants::IsDevelopmentMode)
    {
      spdlog::debug(
        "LoginHandler::HandleUserLogin - Rejecting login for client {}."
        " User name or user token empty.",
        clientId);

      QueueUserLoginRejected(clientId, true);
      return;
    }
  }

  // The login request must be unique for the client.
  if (_clientLogins.contains(clientId))
  {
    spdlog::debug(
      "LoginHandler::HandleUserLogin - Rejecting login for client {} ({})."
      " User login already queued.",
      clientId,
      login.loginId);

    QueueUserLoginRejected(clientId, true);
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

  _clientLoginRequestQueue.emplace(clientId);
}

void LoginHandler::HandleUserCreateCharacter(
  ClientId clientId,
  const protocol::LobbyCommandCreateNickname& command)
{
  const auto& loginContext = _clientLogins[clientId];

  const auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUsers().Get(
    loginContext.userName);
  if (not userRecord)
    throw std::runtime_error("User record does not exist");

  std::vector<data::Uid> horses;

  // Create the character's horses.
  for (uint32_t i = 0; i < 3; ++i)
  {
    // Create a new horse for the character.
    const auto horseRecord = _lobbyDirector.GetServerInstance().GetDataDirector().CreateHorse();

    horseRecord.Mutable(
      [&horses](data::Horse& horse)
      {
        // The TID of the horse specifies which body mesh is used for that horse.
        // Can be found in the `MountPartInfo` table.
        horse.tid() = 20002;

        HorseRegistry::Get().BuildRandomHorse(
          horse.parts,
          horse.appearance);

        horses.emplace_back(horse.uid());
      });
  }

  // Pick the first horse to be a mount.
  const data::Uid characterMountUid = horses.front();

  // Create the character.
  const auto characterRecord = _lobbyDirector.GetServerInstance().GetDataDirector().CreateCharacter();
  auto userCharacterUid{data::InvalidUid};

  characterRecord.Mutable(
    [&userCharacterUid,
     &horses,
     &characterMountUid,
     &command](data::Character& character)
    {
      userCharacterUid = character.uid();

      character.level = 60;
      character.carrots = 5000;

      character.name = command.nickname;
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

      character.horses = horses;
      character.mountUid() = characterMountUid;
    });

  // Assign the character to the user.
  userRecord->Mutable(
    [&userCharacterUid](data::User& user)
    {
      user.characterUid() = userCharacterUid;
    });

  // Queue the processing of the login response.
  _clientLoginResponseQueue.emplace(clientId);
}

void LoginHandler::QueueUserLoginAccepted(
  const ClientId clientId,
  const std::string& userName)
{
  const auto userRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetUsers().Get(
    userName);
  if (not userRecord)
    throw std::runtime_error("User record unavailable");

  const auto lobbyServerTime = util::UnixTimeToFileTime(
    std::chrono::system_clock::now());

  protocol::LobbyCommandLoginOK response{
    .lobbyTime =
      {.dwLowDateTime = static_cast<uint32_t>(lobbyServerTime.dwLowDateTime),
       .dwHighDateTime = static_cast<uint32_t>(lobbyServerTime.dwHighDateTime)},
    .member0 = 0xCA794,
    .motd = std::format(
      "Welcome to Story of Alicia. Players online: {}",
      _lobbyDirector._clientContext.size()),
    .val1 = 0x0,
    .val3 = 0x0,
    .optionType = OptionType::Value,
    .valueOptions = 0x64,

    .val5 = {
      {0x18, {{2, 1}}},
      {0x1F, {{2, 1}}},
      {0x23, {{2, 1}}},
      {0x29, {{2, 1}}},
      {0x2A, {{2, 1}}},
      {0x2B, {{2, 1}}},
      {0x2E, {{2, 1}}}},

    .address = _lobbyDirector.GetSettings().ranchAdvAddress.to_uint(),
    .port = _lobbyDirector.GetSettings().ranchAdvPort,
    .scramblingConstant = 0,

    .systemContent = _lobbyDirector._systemContent,

    .managmentSkills = {4, 0x2B, 4},
    .skillRanks = {.values = {{1,1}}},
    .val14 = 0xca1b87db,
    .guild = {.val1 = 1},
    .val16 = 4,
    .val18 = 0x2a,
    .val19 = 0x38d,
    .val20 = 0x1c7};

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
    [this, &response, &characterMountUid](const data::Character& character)
    {
      response.uid = character.uid();
      response.name = character.name();

      response.introduction = character.introduction();
      response.gender = character.parts.modelId() == 10 ? Gender::Boy : Gender::Girl;

      response.level = character.level();
      response.carrots = character.carrots();
      response.role = std::bit_cast<protocol::LobbyCommandLoginOK::Role>(
        character.role());
      response.age = 18;
      response.hideAge = false;

      response.bitfield = 2;

      // Character equipment.
      const auto characterEquipmentItems = _lobbyDirector.GetServerInstance().GetDataDirector().GetItems().Get(
        character.characterEquipment());
      if (not characterEquipmentItems)
        throw std::runtime_error("Character equipment items unavailable");

      protocol::BuildProtocolItems(
        response.characterEquipment,
        *characterEquipmentItems);

      // Mount equipment.
      const auto mountEquipmentItems = _lobbyDirector.GetServerInstance().GetDataDirector().GetItems().Get(
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
          throw std::runtime_error("Character guild not available");

        guildRecord.Immutable([&response](const data::Guild& guild)
        {
          protocol::BuildProtocolGuild(response.guild, guild);
        });
      }

      characterMountUid = character.mountUid();
    });

  // Get the mounted horse record and fill the protocol data.
  const auto mountRecord = _lobbyDirector.GetServerInstance().GetDataDirector().GetHorses().Get(characterMountUid);
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

void LoginHandler::QueueUserLoginRejected(ClientId clientId, bool invalidUser)
{
  _server.QueueCommand<protocol::LobbyCommandLoginCancel>(
    clientId,
    [invalidUser]()
    {
      return protocol::LobbyCommandLoginCancel{
      .reason = invalidUser ? protocol::LobbyCommandLoginCancel::Reason::InvalidUser : protocol::LobbyCommandLoginCancel::Reason::Generic };
    });
}

} // namespace server
