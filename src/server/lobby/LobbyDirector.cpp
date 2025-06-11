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

#include "libserver/registry/RoomRegistry.hpp"

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
  _server.RegisterCommandHandler<LobbyCommandLogin>(
    CommandId::LobbyLogin,
    [this](ClientId clientId, const auto& message)
    {
      assert(message.constant0 == 50
        && message.constant1 == 281
        && "Game version mismatch");

      _loginHandler.HandleUserLogin(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandCreateNickname>(
    CommandId::LobbyCreateNickname,
    [this](ClientId clientId, const auto& message)
    {
      _loginHandler.HandleUserCreateCharacter(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandEnterChannel>(
    CommandId::LobbyEnterChannel,
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterChannel(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandMakeRoom>(
    CommandId::LobbyMakeRoom,
    [this](ClientId clientId, const auto& message)
    {
      HandleMakeRoom(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandHeartbeat>(
    CommandId::LobbyHeartbeat,
    [this](ClientId clientId, const auto& message)
    {
      HandleHeartbeat(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandShowInventory>(
    CommandId::LobbyShowInventory,
    [this](ClientId clientId, const auto& message)
    {
      HandleShowInventory(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandAchievementCompleteList>(
    CommandId::LobbyAchievementCompleteList,
    [this](ClientId clientId, const auto& message)
    {
      HandleAchievementCompleteList(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandRequestLeagueInfo>(
    CommandId::LobbyRequestLeagueInfo,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestLeagueInfo(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandRequestQuestList>(
    CommandId::LobbyRequestQuestList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestQuestList(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandRequestDailyQuestList>(
    CommandId::LobbyRequestDailyQuestList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestDailyQuestList(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandRequestSpecialEventList>(
    CommandId::LobbyRequestSpecialEventList,
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestSpecialEventList(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandEnterRanch>(
    CommandId::LobbyEnterRanch,
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRanch(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandGetMessengerInfo>(
    CommandId::LobbyGetMessengerInfo,
    [this](ClientId clientId, const auto& message)
    {
      HandleGetMessengerInfo(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandGoodsShopList>(
    CommandId::LobbyGoodsShopList,
    [this](ClientId clientId, const auto& message)
    {
      HandleGoodsShopList(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandInquiryTreecash>(
    CommandId::LobbyInquiryTreecash,
    [this](ClientId clientId, const auto& message)
    {
      HandleInquiryTreecash(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandClientNotify>(
    CommandId::LobbyClientNotify,
    [this](ClientId clientId, const auto& message)
    {
      if (message.val0 != 1)
        spdlog::error("Client error notification: state[{}], value[{}]", message.val0, message.val1);
    });

  _server.RegisterCommandHandler<LobbyCommandEnterRandomRanch>(
    CommandId::LobbyEnterRandomRanch,
    [this](ClientId clientId, auto& command)
    {
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

  _server.BeginHost(_settings.address, _settings.port);
}

void LobbyDirector::Terminate()
{
  _server.EndHost();
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
  auto& roomRegistry = soa::RoomRegistry::Get();
  auto& room = roomRegistry.CreateRoom();

  room.name = makeRoom.name;
  room.description = makeRoom.description;
  room.missionId = makeRoom.missionId;
  room.unk0 = makeRoom.unk0;
  room.unk1 = makeRoom.unk1;
  room.unk2 = makeRoom.unk2;
  room.unk3 = makeRoom.unk3;
  room.bitset = makeRoom.bitset;
  room.unk4 = makeRoom.unk4;

  LobbyCommandMakeRoomOK response{
    .roomUid = room.uid,
    .otp = 0xBAAD,
    .ip = htonl(_settings.raceAdvAddress.to_uint()),
    .port = _settings.raceAdvPort};

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyMakeRoomOK,
    [response]()
    {
      return response;
    });
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
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = _dataDirector.GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandAchievementCompleteListOK response{};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.unk0 = character.uid();
  });

  response.achievements.emplace_back().tid = 20'008;
  response.achievements.emplace_back().tid = 20'004;

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
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = _dataDirector.GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandRequestQuestListOK response{};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.unk0 = character.uid();
  });

  response.quests.emplace_back(Quest{
    .tid = 11'039,
    .member1 = 1,
    .member2 = 0xA,
    .member4 = 3});
  response.quests.emplace_back(Quest{
    .tid = 12'012,
    .member1 = 1,
    .member2 = 3,
    .member4 = 3});
  response.quests.emplace_back(Quest{
    .tid = 13'010,
    .member1 = 1,
    .member2 = 0x14,
    .member4 = 3});

  _server.QueueCommand<decltype(response)>(
    clientId,
    CommandId::LobbyAchievementCompleteListOK,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleRequestDailyQuestList(
  ClientId clientId,
  const LobbyCommandRequestDailyQuestList& requestQuestList)
{
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = _dataDirector.GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandRequestDailyQuestListOK response{};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.val0 = character.uid();
  });

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
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = _dataDirector.GetCharacters().Get(
    clientContext.characterUid);

  LobbyCommandRequestSpecialEventListOK response{};

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
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = _dataDirector.GetCharacters().Get(
    clientContext.characterUid);

  if (not characterRecord)
  {
    throw std::runtime_error(
      std::format("Character [{}] not available", clientContext.characterUid));
  }

  LobbyCommandEnterRanchOK response{
    .code = 0x44332211,
    .ip = static_cast<uint32_t>(htonl(_settings.ranchAdvAddress.to_uint())),
    .port = _settings.ranchAdvPort};

  characterRecord->Immutable([&response](const soa::data::Character& character)
  {
    response.ranchUid = character.ranchUid();
  });

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

void LobbyCommandEnterRandomRanch::Write(
  const LobbyCommandEnterRandomRanch& command,
  SinkStream& stream)
{
  throw std::runtime_error("Not implemented");
}

void LobbyCommandEnterRandomRanch::Read(
  LobbyCommandEnterRandomRanch& command,
  SourceStream& stream)
{
  // Empty.
}

} // namespace alicia
