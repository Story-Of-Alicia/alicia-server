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
#include "zlib.h"

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
    .address = GetConfig().advertisement.race.address.to_uint(),
    .port = GetConfig().advertisement.race.port};

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
    .address = GetConfig().advertisement.race.address.to_uint(),
    .port = GetConfig().advertisement.race.port};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
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
        character.items());
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
    .otp = GetServerInstance().GetOtpRegistry().GrantCode(clientContext.characterUid),
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
    throw std::runtime_error("Client context not available");

  auto& clientContext = clientContextIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Client is not authenticated");

  return clientContext;
}

} // namespace server
