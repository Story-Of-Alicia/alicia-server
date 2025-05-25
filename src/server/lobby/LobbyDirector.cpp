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

#include <random>

#include <spdlog/spdlog.h>

namespace
{

std::random_device rd;

} // namespace

namespace alicia
{

LobbyDirector::LobbyDirector(soa::DataDirector& dataDirector, Settings::LobbySettings settings)
  : _settings(std::move(settings))
  , _server()
  , _dataDirector(dataDirector)
  , _loginHandler(*this, _server)
{
  // LobbyCommandLogin
  _server.RegisterCommandHandler<LobbyCommandLogin>(
    CommandId::LobbyLogin,
    [this](ClientId clientId, const auto& message)
    {
      assert(message.constant0 == 50 && message.constant1 == 281 && "Game version mismatch");

      _clientUsers[clientId] = message.loginId;
      _loginHandler.HandleUserLogin(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandCreateNicknameOK>(
    CommandId::LobbyCreateNicknameOK,
    [this](ClientId clientId, const auto& message)
    { HandleCreateNicknameOK(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandEnterChannel>(
    CommandId::LobbyEnterChannel,
    [this](ClientId clientId, const auto& message)
    { HandleEnterChannel(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandMakeRoom>(
    CommandId::LobbyMakeRoom,
    [this](ClientId clientId, const auto& message)
    { HandleMakeRoom(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandHeartbeat>(
    CommandId::LobbyHeartbeat,
    [this](ClientId clientId, const auto& message)
    { HandleHeartbeat(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandShowInventory>(
    CommandId::LobbyShowInventory,
    [this](ClientId clientId, const auto& message)
    { HandleShowInventory(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandAchievementCompleteList>(
    CommandId::LobbyAchievementCompleteList,
    [this](ClientId clientId, const auto& message)
    { HandleAchievementCompleteList(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandRequestLeagueInfo>(
    CommandId::LobbyRequestLeagueInfo,
    [this](ClientId clientId, const auto& message)
    { HandleRequestLeagueInfo(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandRequestQuestList>(
    CommandId::LobbyRequestQuestList,
    [this](ClientId clientId, const auto& message)
    { HandleRequestQuestList(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandRequestDailyQuestList>(
    CommandId::LobbyRequestDailyQuestList,
    [this](ClientId clientId, const auto& message)
    { HandleRequestDailyQuestList(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandRequestSpecialEventList>(
    CommandId::LobbyRequestSpecialEventList,
    [this](ClientId clientId, const auto& message)
    { HandleRequestSpecialEventList(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandEnterRanch>(
    CommandId::LobbyEnterRanch,
    [this](ClientId clientId, const auto& message)
    { HandleEnterRanch(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandGetMessengerInfo>(
    CommandId::LobbyGetMessengerInfo,
    [this](ClientId clientId, const auto& message)
    { HandleGetMessengerInfo(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandGoodsShopList>(
    CommandId::LobbyGoodsShopList,
    [this](ClientId clientId, const auto& message)
    { HandleGoodsShopList(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandInquiryTreecash>(
    CommandId::LobbyInquiryTreecash,
    [this](ClientId clientId, const auto& message)
    { HandleInquiryTreecash(clientId, message); });

  _server.RegisterCommandHandler<LobbyCommandClientNotify>(
    CommandId::LobbyClientNotify,
    [this](ClientId clientId, const auto& message)
    {
      if (message.val0 != 1)
        spdlog::error("Client error notification: state[{}], value[{}]", message.val0, message.val1);
    });
}

void LobbyDirector::Initialize()
{
  spdlog::debug(
    "Lobby is advertising ranch server on {}:{}",
    _settings.ranchAdvAddress.to_string(),
    _settings.ranchAdvPort);
  spdlog::debug(
    "Lobby is advertising messenger server on {}:{}",
    _settings.messengerAdvAddress.to_string(),
    _settings.messengerAdvPort);

  spdlog::debug(
    "Lobby server listening on {}:{}",
    _settings.address.to_string(),
    _settings.port);

  _server.Host(_settings.address, _settings.port);
}

void LobbyDirector::Terminate()
{
}

void LobbyDirector::Tick()
{
  _loginHandler.Tick();
}

soa::DataDirector& LobbyDirector::GetDataDirector()
{
  return _dataDirector;
}

Settings::LobbySettings& LobbyDirector::GetSettings()
{
  return _settings;
}

void LobbyDirector::HandleCreateNicknameOK(
  ClientId clientId,
  const LobbyCommandCreateNicknameOK& createNickname)
{
  const auto userName = _clientUsers[clientId];
  auto user = _dataDirector.GetUsers().Get(userName);

  if (not user)
  {
    return;
  }

  (*user)().characterUid = 1;
  auto character = _dataDirector.GetCharacters().Create(1);
  character->Mutable([&](auto& character)
                     {
    character.name = createNickname.nickname;
    character.parts = soa::data::Character::Parts{
      .modelId = createNickname.character.parts.charId,
      .mouthId = createNickname.character.parts.mouthSerialId,
      .faceId = createNickname.character.parts.faceSerialId};
    character.appearance = soa::data::Character::Appearance{
      .headSize = createNickname.character.appearance.headSize,
      .height = createNickname.character.appearance.height,
      .thighVolume = createNickname.character.appearance.thighVolume,
      .legVolume = createNickname.character.appearance.legVolume,}; });

  _dataDirector.GetCharacters().Save(1);
}

void LobbyDirector::HandleEnterChannel(
  ClientId clientId,
  const LobbyCommandEnterChannel& enterChannel)
{
  LobbyCommandEnterChannelOK response{
    .unk0 = enterChannel.channel // potentially
  };

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyEnterChannelOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleMakeRoom(
  ClientId clientId,
  const LobbyCommandMakeRoom& makeRoom)
{
  /*const auto characterUid = _clientCharacters[clientId];
  const auto character = _dataDirector.GetCharacter(characterUid);
  character->roomUid = 123; // TODO: Generate somehow

  const auto room = _dataDirector.GetRoom(character->roomUid.value());
  room->name = makeRoom.name;
  room->description = makeRoom.description;
  room->unk0 = makeRoom.unk0;
  room->unk1 = makeRoom.unk1;
  room->unk2 = makeRoom.unk2;
  room->missionId = makeRoom.missionId;
  room->unk4 = makeRoom.unk4;
  room->bitset = makeRoom.bitset;
  room->unk6 = makeRoom.unk6;

  LobbyCommandMakeRoomOK response{
    .characterUid = characterUid,
    .otp = 0x44332211,
    .ip = htonl(_settings.raceAdvAddress.to_uint()),
    .port = _settings.raceAdvPort,
    .unk2 = 0
  };

  _server.QueueCommand(
    clientId,
    CommandId::LobbyMakeRoomOK,
    [response](auto& sink)
    {
      LobbyCommandMakeRoomOK::Write(response, sink);
    });*/
}

void LobbyDirector::HandleHeartbeat(
  ClientId clientId,
  const LobbyCommandHeartbeat& heartbeat)
{
  // TODO: Statistics for the heartbeat.
}

void LobbyDirector::HandleShowInventory(
  ClientId clientId,
  const LobbyCommandShowInventory& showInventory)
{
  LobbyCommandShowInventoryOK response{
    .items = {},
    .horses = {}};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyShowInventoryOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleAchievementCompleteList(
  ClientId clientId,
  const LobbyCommandAchievementCompleteList& achievementCompleteList)
{
  const auto& userName = _clientUsers[clientId];
  auto user = *_dataDirector.GetUsers().Get(userName);

  const LobbyCommandAchievementCompleteListOK response{
    .unk0 = user().characterUid()};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyAchievementCompleteListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestLeagueInfo(
  ClientId clientId,
  const LobbyCommandRequestLeagueInfo& requestLeagueInfo)
{
  LobbyCommandRequestLeagueInfoOK response{};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyRequestLeagueInfoOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestQuestList(
  ClientId clientId,
  const LobbyCommandRequestQuestList& requestQuestList)
{
  const auto& userName = _clientUsers[clientId];
  auto user = *_dataDirector.GetUsers().Get(userName);

  LobbyCommandRequestQuestListOK response{
    .unk0 = user().characterUid()};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyRequestQuestListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestDailyQuestList(
  ClientId clientId,
  const LobbyCommandRequestDailyQuestList& requestQuestList)
{
  const auto& userName = _clientUsers[clientId];
  auto user = *_dataDirector.GetUsers().Get(userName);

  LobbyCommandRequestDailyQuestListOK response{
    .val0 = user().characterUid()};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyRequestDailyQuestListOK,
    [response]()
    {
      return response;
    });
}
void LobbyDirector::HandleRequestSpecialEventList(
  ClientId clientId,
  const LobbyCommandRequestSpecialEventList& requestQuestList)
{
  const auto& userName = _clientUsers[clientId];
  auto user = *_dataDirector.GetUsers().Get(userName);

  LobbyCommandRequestSpecialEventListOK response{
    .unk0 = user().characterUid()};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyAchievementCompleteListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleEnterRanch(
  ClientId clientId,
  const LobbyCommandEnterRanch& requestEnterRanch)
{
  LobbyCommandEnterRanchOK response{
    .ranchUid = 1,
    .code = 0x44332211,
    .ip = static_cast<uint32_t>(htonl(_settings.ranchAdvAddress.to_uint())),
    .port = _settings.ranchAdvPort};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyEnterRanchOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGetMessengerInfo(
  ClientId clientId,
  const LobbyCommandGetMessengerInfo& requestMessengerInfo)
{
  LobbyCommandGetMessengerInfoOK response{
    .code = 0xDEAD,
    .ip = static_cast<uint32_t>(htonl(_settings.messengerAdvAddress.to_uint())),
    .port = _settings.messengerAdvPort,
  };

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyGetMessengerInfoOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGoodsShopList(
  ClientId clientId,
  const LobbyCommandGoodsShopList& message)
{
  LobbyCommandGoodsShopListOK response{
    .data = message.data};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyGoodsShopListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleInquiryTreecash(
  ClientId clientId,
  const LobbyCommandInquiryTreecash& message)
{
  LobbyCommandInquiryTreecashOK response{
    .cash = 1'000};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyInquiryTreecashOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleGuildPartyList(
  ClientId clientId,
  const LobbyCommandGuildPartyList& message)
{
}

//
// void LobbyDirector::HandleGuildPartyList(
//   ClientId clientId,
//   const LobbyCommandGuildPartyList& message)
// {
//   const LobbyCommandGuildPartyListOK response{};
//
//   _server.QueueCommand(
//     clientId,
//     CommandId::LobbyGuildPartyListOK,
//     [response](SinkStream& sink)
//     {
//       LobbyCommandGuildPartyListOK::Write(response, sink);
//     });
// }

} // namespace alicia
