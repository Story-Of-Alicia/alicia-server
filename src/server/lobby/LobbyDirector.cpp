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

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "libserver/registry/RoomRegistry.hpp"
#include "server/ServerInstance.hpp"

#include <random>

#include <spdlog/spdlog.h>

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
      assert(command.constant0 == 50 && command.constant1 == 281 && "Game version mismatch");

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

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandHeartbeat>(
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

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandGoodsShopList>(
    [this](ClientId clientId, const auto& command)
    {
      HandleGoodsShopList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandInquiryTreecash>(
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
      auto& clientContext = _clientContext[clientId];

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
        auto randomCharacterUid = GetServerInstance().GetDataDirector().GetCharacters().GetKeys();
        std::uniform_int_distribution<data::Uid> uidDistribution(
          0, randomCharacterUid.size() - 1);

        rancherUid = randomCharacterUid[uidDistribution(rd)];
      }

      QueueEnterRanchOK(clientId, rancherUid);
    });

  _commandServer.RegisterCommandHandler<protocol::LobbyCommandUpdateSystemContent>(
    [this](ClientId clientId, const auto& command)
    {
      HandleUpdateSystemContent(clientId, command);
    });
}

void LobbyDirector::Initialize()
{
  spdlog::debug(
    "Lobby is advertising ranch server on {}:{}",
    GetSettings().ranchAdvAddress.to_string(),
    GetSettings().ranchAdvPort);
  spdlog::debug(
    "Lobby is advertising messenger server on {}:{}",
    GetSettings().messengerAdvAddress.to_string(),
    GetSettings().messengerAdvPort);

  spdlog::debug(
    "Lobby server listening on {}:{}",
    GetSettings().address.to_string(),
    GetSettings().port);

  _commandServer.BeginHost(GetSettings().address, GetSettings().port);
}

void LobbyDirector::Terminate()
{
  _commandServer.EndHost();
}

void LobbyDirector::Tick()
{
  _loginHandler.Tick();
}

void LobbyDirector::HandleClientConnected(ClientId clientId)
{
  spdlog::info("Client {} connected to the lobby", clientId);
}

void LobbyDirector::HandleClientDisconnected(ClientId clientId)
{
  spdlog::info("Client {} disconnected from the lobby", clientId);
  _clientContext.erase(clientId);
}

ServerInstance& LobbyDirector::GetServerInstance()
{
  return _serverInstance;
}

Settings::LobbySettings& LobbyDirector::GetSettings()
{
  return GetServerInstance().GetSettings()._lobbySettings;
}

void LobbyDirector::UpdateVisitPreference(data::Uid characterUid, data::Uid visitingCharacterUid)
{
  const auto clientContextIter = std::ranges::find_if(_clientContext, [characterUid](const auto pair)
  {
    const auto& [clientId, ctx] = pair;
    return ctx.characterUid == characterUid;
  });

  if (clientContextIter == _clientContext.cend())
    return;

  clientContextIter->second.rancherVisitPreference = visitingCharacterUid;
}


void LobbyDirector::HandleEnterChannel(
  ClientId clientId,
  const protocol::LobbyCommandEnterChannel& command)
{
  protocol::LobbyCommandEnterChannelOK response{
    .unk0 = command.channel // potentially
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
  protocol::LobbyCommandRoomListOK response;
  response.page = command.page;
  response.unk1 = 1;
  response.unk2 = 1;

  for (const auto& room : RoomRegistry::Get().GetRooms() | std::views::values)
  {
    auto& roomResponse = response.rooms.emplace_back();
    roomResponse.id = room.uid;
    roomResponse.playerCount = 1;
    roomResponse.maxPlayers = 8;
    roomResponse.level = 2;
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
  auto& roomRegistry = RoomRegistry::Get();
  auto& room = roomRegistry.CreateRoom();

  room.name = command.name;
  room.description = command.password;
  room.missionId = command.missionId;
  room.playerCount = command.playerCount;
  room.gameMode = command.gameMode;
  room.teamMode = command.teamMode;
  room.unk3 = command.unk3;
  room.bitset = static_cast<uint16_t>(command.bitset);
  room.unk4 = command.unk4;

  protocol::LobbyCommandMakeRoomOK response{
    .roomUid = room.uid,
    .otp = 0xBAAD,
    .address = GetSettings().raceAdvAddress.to_uint(),
    .port = GetSettings().raceAdvPort};

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
  auto& roomRegistry = RoomRegistry::Get();

  protocol::LobbyCommandEnterRoomOK response{
    .roomUid = command.roomUid,
    .otp = 0xBAAD,
    .address = GetSettings().raceAdvAddress.to_uint(),
    .port = GetSettings().raceAdvPort,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleHeartbeat(
  ClientId clientId,
  const protocol::LobbyCommandHeartbeat& command)
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
  const auto& clientContext = _clientContext[clientId];
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
      const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
        character.items());
      protocol::BuildProtocolItems(response.items, *itemRecords);

      const auto horseRecords = GetServerInstance().GetDataDirector().GetHorses().Get(
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
  const auto& clientContext = _clientContext[clientId];
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
  const auto& clientContext = _clientContext[clientId];
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
  const auto& clientContext = _clientContext[clientId];
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
  const auto& clientContext = _clientContext[clientId];
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

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleSetIntroduction(ClientId clientId, const protocol::LobbyCommandSetIntroduction& command)
{
  const auto& clientContext = _clientContext[clientId];
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
  const auto& clientContext = _clientContext[clientId];
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    command.characterUid);

  if (not characterRecord)
  {
    protocol::LobbyCommandEnterRanchCancel response{};

    _commandServer.QueueCommand<decltype(response)>(
      clientId, [response]()
      {
        return response;
      });
  }

  QueueEnterRanchOK(clientId, clientContext.characterUid);
}

void LobbyDirector::QueueEnterRanchOK(
  ClientId clientId,
  data::Uid rancherUid)
{
  protocol::LobbyCommandEnterRanchOK response{
    .rancherUid = rancherUid,
    .otp = 0x44332211,
    .ip = static_cast<uint32_t>(htonl(
      GetSettings().ranchAdvAddress.to_uint())),
    .port = GetSettings().ranchAdvPort};

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
    .ip = static_cast<uint32_t>(htonl(GetSettings().messengerAdvAddress.to_uint())),
    .port = GetSettings().messengerAdvPort,
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
  const protocol::LobbyCommandGoodsShopList& command)
{
  protocol::LobbyCommandGoodsShopListOK response{
    .data = command.data};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void LobbyDirector::HandleInquiryTreecash(
  ClientId clientId,
  const protocol::LobbyCommandInquiryTreecash& command)
{
  const auto& clientContext = _clientContext[clientId];
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
  const auto& clientContext = _clientContext[clientId];
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

  for (const auto& connectedClientId : _clientContext | std::views::keys)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      connectedClientId,
      [notify]()
      {
        return notify;
      });
  }
}

} // namespace server
