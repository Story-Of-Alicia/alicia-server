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
#include "spdlog/spdlog.h"

namespace alicia
{

LoginHandler::LoginHandler(
  LobbyDirector& lobbyDirector,
  CommandServer& server)
  : _server(server)
  , _lobbyDirector(lobbyDirector)
  , _dataDirector(_lobbyDirector.GetDataDirector())
{
}

void LoginHandler::Tick()
{
  // Process the login queue.
  while (not _clientLoginRequestQueue.empty())
  {
    const ClientId clientId = _clientLoginRequestQueue.front();
    const auto& loginContext = _clientLogins[clientId];

    // Load the user.
    auto user = _dataDirector.GetUsers().Get(
      loginContext.userName);
    if (not user)
    {
      continue;
    }

    _clientLoginRequestQueue.pop();

    bool isAuthenticated = false;
    user->Immutable([&isAuthenticated, &loginContext](auto& user){
      isAuthenticated = user.token() == loginContext.userToken;
    });

    // If the user succeeds in authentication queue user for further processing.
    if (isAuthenticated)
    {
      // Queue the processing of the response.
      _clientLoginResponseQueue.emplace(clientId);
    }
    else
    {
      QueueUserLoginRejected(clientId);
    }

    // Only one user login per tick.
    break;
  }

  while (not _clientLoginResponseQueue.empty())
  {
    const ClientId clientId = _clientLoginResponseQueue.front();
    const auto& loginContext = _clientLogins[clientId];
    bool hasCharacter = true;

    {
      // Load the character.

      auto user = *_dataDirector.GetUsers().Get(loginContext.userName);

      if (user().characterUid() != soa::data::InvalidUid)
      {
        auto character = _dataDirector.GetCharacters().Get(
          user().characterUid());
        if (not character)
        {
          continue;
        }

        // Load the character's equipment, horses and ranch.

        const auto characterEquipment = _dataDirector.GetItems().Get(
          (*character)().characterEquipment());
        const auto horseEquipment = _dataDirector.GetItems().Get(
          (*character)().horseEquipment());

        const auto horses = _dataDirector.GetHorses().Get((*character)().horses());
        const auto ranch = _dataDirector.GetRanches().Get((*character)().ranchUid());

        if (not characterEquipment
          || not horseEquipment
          || not horses
          || not ranch)
        {
          continue;
        }
      }
      else
      {
        hasCharacter = false;
      }
    }

    _clientLoginResponseQueue.pop();

    if (not hasCharacter)
      QueueUserCreateNickname(clientId, loginContext.userName);
    else
      QueueUserLoginAccepted(clientId, loginContext.userName);
  }
}

void LoginHandler::HandleUserLogin(
  const ClientId clientId,
  const LobbyCommandLogin& login)
{
  // Validate the command fields.
  if (login.loginId.empty())
  {
    spdlog::debug(
      "LoginHandler::HandleUserLogin - Rejecting login for client {}."
      " User name or user token empty.",
      clientId);

    QueueUserLoginRejected(clientId);
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

    QueueUserLoginRejected(clientId);
    return;
  }

  // Queue the login.
  const auto [iterator, inserted] =
    _clientLogins.try_emplace(clientId, LoginContext{.userName = login.loginId, .userToken = login.authKey});
  assert(inserted && "Duplicate client login request.");

  _clientLoginRequestQueue.emplace(clientId);
}

void LoginHandler::QueueUserLoginAccepted(const ClientId clientId, const std::string& userName)
{
  _server.QueueCommand<LobbyCommandLoginOK>(
    clientId,
    CommandId::LobbyLoginOK,
    [userName, clientId, this]()
    {
      auto user = *_dataDirector.GetUsers().Get(userName);
      auto character = _dataDirector.GetCharacters().Get(
        user().characterUid());

      Character gameCharacter {};
      Horse gameHorse{};

      if (character)
      {
        character->Immutable([&gameCharacter](const soa::data::Character& character)
        {
          gameCharacter.parts.charId = static_cast<uint8_t>(
            character.parts.modelId());
          gameCharacter.parts.mouthSerialId = static_cast<uint8_t>(
            character.parts.mouthId());
          gameCharacter.parts.faceSerialId = static_cast<uint8_t>(
            character.parts.faceId());

          gameCharacter.appearance.headSize = static_cast<uint8_t>(
            character.appearance.headSize());
          gameCharacter.appearance.height = static_cast<uint8_t>(
            character.appearance.height());
          gameCharacter.appearance.thighVolume = static_cast<uint8_t>(
            character.appearance.thighVolume());
          gameCharacter.appearance.legVolume = static_cast<uint8_t>(
            character.appearance.legVolume());
        });

        gameHorse = {
          .uid = 2,
          .tid = 0x4e21,
          .name = "default",
          .parts = {.skinId = 0x2, .maneId = 0x3, .tailId = 0x3, .faceId = 0x3},
          .appearance = {
            .scale = 0x4,
            .legLength = 0x4,
            .legVolume = 0x5,
            .bodyLength = 0x3,
            .bodyVolume = 0x4},
          .stats = {
            .agility = 9,
            .spirit = 9,
            .speed = 9,
            .strength = 9,
            .ambition = 0x13},
          .rating = 0,
          .clazz = 0x15,
          .val0 = 1,
          .grade = 5,
          .growthPoints = 2,
          .vals0 = {
           .stamina = 0x7d0,
           .attractiveness = 0x3c,
           .hunger = 0x21c,
           .val0 = 0x00,
           .val1 = 0x03E8,
           .val2 = 0x00,
           .val3 = 0x00,
           .val4 = 0x00,
           .val5 = 0x03E8,
           .val6 = 0x1E,
           .val7 = 0x0A,
           .val8 = 0x0A,
           .val9 = 0x0A,
           .val10 = 0x00,},
          .vals1 = {
            .val0 = 0x00,
            .val1 = 0x00,
            .dateOfBirth = 0xb8a167e4,
            .val3 = 0x02,
            .val4 = 0x00,
            .classProgression = 0x32e7d,
            .val5 = 0x00,
            .potentialLevel = 0x40,
            .hasPotential = 0x1,
            .potentialValue = 0x64,
            .val9 = 0x00,
            .luck = 0x05,
            .hasLuck = 0x00,
            .val12 = 0x00,
            .fatigue = 0x00,
            .val14 = 0x00,
            .emblem = 0xA},
           .mastery = {
             .magic = 0x1fe,
             .jumping = 0x421,
             .sliding = 0x5f8,
             .gliding = 0xcfa4,},
           .val16 = 0xb8a167e4,
           .val17 = 0};
      }

      //
      // auto characterEquipment = *_dataDirector.GetItems().Get(character().characterEquipment());
      // auto horseEquipment = *_dataDirector.GetItems().Get(character().horseEquipment());
      //
      // auto horses = *_dataDirector.GetHorses().Get(character().horses());
      //
      // auto ranch = *_dataDirector.GetRanches().Get(character().ranchUid());

      const auto time = UnixTimeToFileTime(std::chrono::system_clock::now());

      _server.SetCode(clientId, {});

      // Transform the server data to alicia protocol data.
      return LobbyCommandLoginOK{
        .lobbyTime =
          {.dwLowDateTime = static_cast<uint32_t>(time.dwLowDateTime),
           .dwHighDateTime = static_cast<uint32_t>(time.dwHighDateTime)},
        .val0 = 0xCA794,

        .selfUid = character.has_value() ? user().characterUid() : 0,
        .nickName = character.has_value() ? character->operator()().name() : "",
        .motd = "Welcome to SoA!",
        .profileGender = Gender::Unspecified,
        .status = "",

        .characterEquipment = {},
        .horseEquipment = {},

        .level = 0,
        .carrots = 0,
        .val1 = 0x0,
        .val2 = 0x0,
        .val3 = 0x0,

        .optionType = OptionType::Value,
        .valueOptions = 0x64,

        .ageGroup = AgeGroup::Adult,
        .hideAge = 0,

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

        .character = gameCharacter,
        .horse = gameHorse,

        .val7 = {
          .values = {
            {0x6, 0x0},
              {0xF, 0x4},
              {0x1B, 0x2},
              {0x1E, 0x0},
              {0x1F, 0x0},
              {0x25, 0x7530},
              {0x35, 0x4},
              {0x42, 0x2},
              {0x43, 0x4},
              {0x45, 0x0}}},

        .val8 = 0b0000'0000'0000'0000'0000'0000'0000'0010,
        .val11 = {4, 0x2B, 4},
        .val14 = 0xca1b87db,
        .val15 = {.val1 = 1},
        .val16 = 4,
        .val17 = {.mountUid = 2, .val1 = 0x12, .val2 = 0x16e67e4},
        .val18 = 0x3a,
        .val19 = 0x38e,
        .val20 = 0x1c6
        };
    });
}
void LoginHandler::QueueUserCreateNickname(ClientId clientId, const std::string& userName)
{
  _server.QueueCommand<LobbyCommandCreateNicknameNotify>(
    clientId,
    CommandId::LobbyCreateNicknameNotify,
    []()
    {
      return LobbyCommandCreateNicknameNotify{};
    });
}

void LoginHandler::QueueUserLoginRejected(ClientId clientId)
{
  _server.QueueCommand<LobbyCommandLoginCancel>(
    clientId,
    CommandId::LobbyLoginCancel,
    []()
    {
      return LobbyCommandLoginCancel{
        .reason = LoginCancelReason::InvalidUser};
    });
}

} // namespace alicia
