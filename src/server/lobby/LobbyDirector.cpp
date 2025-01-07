//
// Created by rgnter on 25/11/2024.
//

#include "server/lobby/LobbyDirector.hpp"
#include "server/DataDirector.hpp"

#include <random>

#include <spdlog/spdlog.h>

namespace
{

std::random_device rd;

} // anon namespace

namespace alicia
{

LobbyDirector::LobbyDirector(
  DataDirector& dataDirector,
  Settings::LobbySettings settings)
  : _settings(std::move(settings))
  , _dataDirector(dataDirector)
  , _loginHandler(dataDirector)
  , _server("Lobby")
{
  _server.RegisterCommandHandler<LobbyCommandLogin>(
    CommandId::LobbyLogin,
    [this](ClientId clientId, const auto& message)
    {
      HandleUserLogin(clientId, message);
    });

  _server.RegisterCommandHandler<LobbyCommandCreateNicknameOK>(
    CommandId::LobbyCreateNicknameOK,
    [this](ClientId clientId, const auto& message)
    {
      HandleCreateNicknameOK(clientId, message);
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

  spdlog::debug("Advertising ranch server on {}:{}",
    _settings.ranchAdvAddress.to_string(), _settings.ranchAdvPort);
  spdlog::debug("Advertising messenger server on {}:{}",
    _settings.messengerAdvAddress.to_string(), _settings.messengerAdvPort);

  _server.Host(_settings.address, _settings.port);
}

void LobbyDirector::HandleUserLogin(ClientId clientId, const LobbyCommandLogin& login)
{
  assert(login.constant0 == 50);
  assert(login.constant1 == 281);

  if (_queuedClientLogins.contains(clientId))
  {
    spdlog::warn(
      "Login from Client Id {} already queued, but received another attempt. "
      "Certainly not a vanilla behaviour!",
      clientId);
    return;
  }

  // Authenticate the user.
  if (!_loginHandler.Authenticate(login.loginId, login.authKey))
  {
    // The user has failed authentication.
    // Cancel the login.
    _server.QueueCommand(
      clientId,
      CommandId::LobbyLoginCancel,
      [](SinkStream& buffer)
      {
        const LobbyCommandLoginCancel command{
          .reason = LoginCancelReason::InvalidLoginId};
        LobbyCommandLoginCancel::Write(command, buffer);
      });

    spdlog::info("User '{}' failed to authenticate", login.loginId);

    return;
  }

  spdlog::info("User '{}' ({}) authenticated", login.loginId, login.memberNo);

  // Get the character
  const auto user = _dataDirector.GetUser(
    login.loginId);
  const auto character = _dataDirector.GetCharacter(
    user->characterUid);

  _clientCharacters[clientId] = user->characterUid;

  // Set XOR scrambler code
  uint32_t scramblingConstant = rd(); // TODO: Use something more secure
  XorCode code;
  *((uint32_t*) code.data()) = scramblingConstant;
  _server.SetCode(clientId, code);

  // If the character has no appearance set.
  // Send a mostly empty LobbyCommandLoginOK alongside a LobbyCommandCreateNicknameNotify
  if (!character->looks.has_value())
  {
    const WinFileTime time = UnixTimeToFileTime(
      std::chrono::system_clock::now());

    const LobbyCommandLoginOK command{
      .lobbyTime =
        {.dwLowDateTime = static_cast<uint32_t>(time.dwLowDateTime),
        .dwHighDateTime = static_cast<uint32_t>(time.dwHighDateTime)},

      .address = _settings.ranchAdvAddress.to_uint(),
      .port = _settings.ranchAdvPort,

      .scramblingConstant = scramblingConstant};
    _server.QueueCommand(clientId, CommandId::LobbyLoginOK, [command](SinkStream& sink)
    {
      LobbyCommandLoginOK::Write(command, sink);
    });

    const LobbyCommandCreateNicknameNotify createNicknameNotify {};
    _server.QueueCommand(clientId, CommandId::LobbyCreateNicknameNotify, [createNicknameNotify](SinkStream& sink)
    {
      LobbyCommandCreateNicknameNotify::Write(createNicknameNotify, sink);
    });
  }
  else
  {
    // Get the mount data of the user.
    const auto mount = _dataDirector.GetMount(
      character->mountUid);

    const WinFileTime time = UnixTimeToFileTime(
      std::chrono::system_clock::now());

    const LobbyCommandLoginOK command{
      .lobbyTime =
        {.dwLowDateTime = static_cast<uint32_t>(time.dwLowDateTime),
        .dwHighDateTime = static_cast<uint32_t>(time.dwHighDateTime)},
      .val0 = 0xCA794,

      .selfUid = user->characterUid,
      .nickName = character->nickName,
      .motd = "Welcome to SoA!",
      .profileGender = character->gender,
      .status = character->status,

      .characterEquipment = character->characterEquipment,
      .horseEquipment = character->horseEquipment,

      .level = character->level,
      .carrots = character->carrots,
      .val1 = 0x6130,
      .val2 = 0xFF,
      .val3 = 0xFF,

      .optionType = OptionType::Value,
      .valueOptions = 0x64,

      .ageGroup = AgeGroup::Adult,
      .val4 = 0,

      .val5 =
        {{0x18, {{2, 1}}},
        {0x1F, {{2, 1}}},
        {0x23, {{2, 1}}},
        {0x29, {{2, 1}}},
        {0x2A, {{2, 1}}},
        {0x2B, {{2, 1}}},
        {0x2E, {{2, 1}}}},

      .val6 = "val6",

      .address = _settings.ranchAdvAddress.to_uint(),
      .port = _settings.ranchAdvPort,

      .scramblingConstant = scramblingConstant,

      .character = character->looks.value(),
      .horse =
        {.uid = character->mountUid,
        .tid = mount->tid,
        .name = mount->name,
        .parts = {.skinId = 0x2, .maneId = 0x3, .tailId = 0x3, .faceId = 0x3},
          .appearance =
            {.scale = 0x4,
              .legLength = 0x4,
              .legVolume = 0x5,
              .bodyLength = 0x3,
              .bodyVolume = 0x4},
          .stats =
            {
              .agility = 9,
              .spirit = 9,
              .speed = 9,
              .strength = 9,
              .ambition = 0x13
            },
          .rating = 0,
          .clazz = 0x15,
          .val0 = 1,
          .grade = 5,
          .growthPoints = 2,
          .vals0 =
            {
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
              .val10 = 0x00,
            },
          .vals1 =
            {
              .val0 = 0x00,
              .val1 = 0x00,
              .val2 = 0xb8a167e4,
              .val3 = 0x02,
              .val4 = 0x00,
              .classProgression = 0x32e7d,
              .val5 = 0x00,
              .val6 = 0x00,
              .val7 = 0x00,
              .val8 = 0x00,
              .val9 = 0x00,
              .val10 = 0x04,
              .val11 = 0x00,
              .val12 = 0x00,
              .val13 = 0x00,
              .val14 = 0x00,
              .val15 = 0x01
            },
          .mastery =
            {
              .magic = 0x1fe,
              .jumping = 0x421,
              .sliding = 0x5f8,
              .gliding = 0xcfa4,
            },
          .val16 = 0xb8a167e4,
          .val17 = 0},
        .val7 =
          {.values =
            {{0x6, 0x0},
              {0xF, 0x4},
              {0x1B, 0x2},
              {0x1E, 0x0},
              {0x1F, 0x0},
              {0x25, 0x7530},
              {0x35, 0x4},
              {0x42, 0x2},
              {0x43, 0x4},
              {0x45, 0x0}}},
        .val8 = 0xE06,
        .val11 = {4, 0x2B, 4},
        .val14 = 0xca1b87db,
        .val15 = {.val1 = 1},
        .val16 = 4,
        .val17 = {.mountUid = character->mountUid, .val1 = 0x12, .val2 = 0x16e67e4},
        .val18 = 0x3a,
        .val19 = 0x38e,
        .val20 = 0x1c6};
    
    _server.QueueCommand(clientId, CommandId::LobbyLoginOK, [command](SinkStream& sink)
    {
      LobbyCommandLoginOK::Write(command, sink);
    });
  }
}

void LobbyDirector::HandleCreateNicknameOK(
  ClientId clientId,
  const LobbyCommandCreateNicknameOK& createNicknameOK)
{
  const auto characterUid = _clientCharacters[clientId];
  const auto character = _dataDirector.GetCharacter(characterUid);
  character->nickName = createNicknameOK.nickname;
  character->looks = std::optional(createNicknameOK.character);
  character->gender = createNicknameOK.character.parts.charId == 10 ? Gender::Boy : Gender::Girl;

  LobbyCommandShowInventoryOK response{};

  // Not sure why but this seems to be the correct reply to send after creating a nickname.
  // After all, it's the packet that goes right after LoginOK.
  _server.QueueCommand(
    clientId,
    CommandId::LobbyShowInventoryOK,
    [response](auto& sink)
    {
      LobbyCommandShowInventoryOK::Write(response, sink);
    });
}

void LobbyDirector::HandleEnterChannel(
  ClientId clientId,
  const LobbyCommandEnterChannel& enterChannel)
{
  const LobbyCommandEnterChannelOK response{};

  _server.QueueCommand(
    clientId,
    CommandId::LobbyEnterChannelOK,
    [response](auto& sink)
    {
      LobbyCommandEnterChannelOK::Write(response, sink);
    });
}

void LobbyDirector::HandleMakeRoom(
  ClientId clientId,
  const LobbyCommandMakeRoom& makeRoom)
{
  const auto characterUid = _clientCharacters[clientId];
  const LobbyCommandMakeRoomOK response{
    .unk0 = characterUid,
    .unk1 = 0x44332211,
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
    });
}

void LobbyDirector::HandleHeartbeat(
  ClientId clientId,
  const LobbyCommandHeartbeat& heartbeat)
{
  // Todo
  // auto userItr = _clientUsers.find(clientId);
  // if (userItr == _clientUsers.cend())
  // {
  //   return;
  // }
  //
  // auto& [userId, user] = *_users.find(userItr->second);
  // user.lastHeartbeat = std::chrono::system_clock::now();
}

void LobbyDirector::HandleShowInventory(
  ClientId clientId,
  const LobbyCommandShowInventory& showInventory)
{
  _server.QueueCommand(
    clientId,
    CommandId::LobbyShowInventoryOK,
    [&](auto& sink)
    {
      LobbyCommandShowInventoryOK response{};
      LobbyCommandShowInventoryOK::Write(response, sink);
    });
}

void LobbyDirector::HandleAchievementCompleteList(
  ClientId clientId,
  const LobbyCommandAchievementCompleteList& achievementCompleteList)
{
  _server.QueueCommand(
    clientId,
    CommandId::LobbyAchievementCompleteListOK,
    [&](auto& sink)
    {
      LobbyCommandAchievementCompleteListOK response{};
      LobbyCommandAchievementCompleteListOK::Write(response, sink);
    });
}

void LobbyDirector::HandleRequestLeagueInfo(
  ClientId clientId,
  const LobbyCommandRequestLeagueInfo& requestLeagueInfo)
{
  _server.QueueCommand(
    clientId,
    CommandId::LobbyRequestLeagueInfoOK,
    [&](auto& sink)
    {
      LobbyCommandRequestLeagueInfoOK response{};
      LobbyCommandRequestLeagueInfoOK::Write(response, sink);
    });
}

void LobbyDirector::HandleRequestQuestList(
  ClientId clientId,
  const LobbyCommandRequestQuestList& requestQuestList)
{
  _server.QueueCommand(
    clientId,
    CommandId::LobbyRequestQuestListOK,
    [&](auto& sink)
    {
      LobbyCommandRequestQuestListOK response{};
      LobbyCommandRequestQuestListOK::Write(response, sink);
    });
}

void LobbyDirector::HandleRequestSpecialEventList(
  ClientId clientId,
  const LobbyCommandRequestSpecialEventList& requestSpecialEventList)
{
  const LobbyCommandRequestSpecialEventListOK response{
    .unk0 = requestSpecialEventList.unk0
  };

  _server.QueueCommand(
    clientId,
    CommandId::LobbyRequestSpecialEventListOK,
    [response](auto& sink)
    {
      LobbyCommandRequestSpecialEventListOK::Write(response, sink);
    });
}

void LobbyDirector::HandleEnterRanch(
  ClientId clientId,
  const LobbyCommandEnterRanch& enterRanch)
{
  const auto [_, characterUid] = *_clientCharacters.find(clientId);

  _server.QueueCommand(
    clientId,
    CommandId::LobbyEnterRanchOK,
    [characterUid, this](auto& sink)
    {
      auto character = _dataDirector.GetCharacter(characterUid);

      LobbyCommandEnterRanchOK response{
        .ranchUid = character->ranchUid,
        .code = 0x44332211, // TODO: Generate and store in the ranch server instance
        .ip = htonl(_settings.ranchAdvAddress.to_uint()),
        .port = _settings.ranchAdvPort,
      };
      LobbyCommandEnterRanchOK::Write(response, sink);
    });
}

void LobbyDirector::HandleGetMessengerInfo(
  ClientId clientId,
  const LobbyCommandGetMessengerInfo& getMessengerInfo)
{
  _server.QueueCommand(
    clientId,
    CommandId::LobbyGetMessengerInfoOK,
    [&](auto& sink)
    {
      LobbyCommandGetMessengerInfoOK response{
        .code = 0xDEAD, // TODO: Generate and store in the messenger server instance
        .ip = htonl(_settings.messengerAdvAddress.to_uint()),
        .port = _settings.messengerAdvPort,
      };
      LobbyCommandGetMessengerInfoOK::Write(response, sink);
    });
}

void LobbyDirector::HandleGoodsShopList(
  ClientId clientId,
  const LobbyCommandGoodsShopList& message)
{
  const LobbyCommandGoodsShopListOK response{.data = message.data};

  _server.QueueCommand(
    clientId,
    CommandId::LobbyGoodsShopListOK,
    [response](SinkStream& sink)
    {
      LobbyCommandGoodsShopListOK::Write(response, sink);
    });
}

void LobbyDirector::HandleInquiryTreecash(
  ClientId clientId,
  const LobbyCommandInquiryTreecash& message)
{
  const LobbyCommandInquiryTreecashOK response{.cash = 1000};

  _server.QueueCommand(
    clientId,
    CommandId::LobbyInquiryTreecashOK,
    [=](SinkStream& sink)
    {
      LobbyCommandInquiryTreecashOK::Write(response, sink);
    });
}

} // namespace alicia