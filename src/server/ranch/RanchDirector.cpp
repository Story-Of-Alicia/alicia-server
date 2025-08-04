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

#include "server/ranch/RanchDirector.hpp"
#include "server/ServerInstance.hpp"

#include "libserver/data/helper/ProtocolHelper.hpp"
#include "libserver/registry/HorseRegistry.hpp"
#include "libserver/util/Util.hpp"

#include <ranges>

#include <spdlog/spdlog.h>

namespace server
{

namespace
{

constexpr size_t MaxRanchHorseCount = 10;
constexpr size_t MaxRanchCharacterCount = 20;
constexpr size_t MaxRanchHousingCount = 13;

} // namespace anon

RanchDirector::RanchDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
  , _commandServer(*this)
{
  _commandServer.RegisterCommandHandler<protocol::AcCmdCREnterRanch>(
    [this](ClientId clientId, const auto& message)
    {
      HandleEnterRanch(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRLeaveRanch>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRanchLeave(clientId);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRanchChat>(
    [this](ClientId clientId, const auto& command)
    {
      HandleChat(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRanchSnapshot>(
    [this](ClientId clientId, const auto& message)
    {
      HandleSnapshot(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRanchCmdAction>(
    [this](ClientId clientId, const auto& message)
    {
      HandleCmdAction(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRanchStuff>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRanchStuff(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUpdateBusyState>(
    [this](ClientId clientId, auto& command)
    {
      HandleUpdateBusyState(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandSearchStallion>(
    [this](ClientId clientId, auto& command)
    {
      HandleSearchStallion(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandEnterBreedingMarket>(
    [this](ClientId clientId, auto& command)
    {
      HandleEnterBreedingMarket(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandTryBreeding>(
    [this](ClientId clientId, auto& command)
    {
      HandleTryBreeding(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandBreedingWishlist>(
    [this](ClientId clientId, auto& command)
    {
      HandleBreedingWishlist(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUpdateMountNickname>(
    [this](ClientId clientId, auto& command)
    {
      HandleUpdateMountNickname(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestStorage>(
    [this](ClientId clientId, auto& command)
    {
      HandleRequestStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandGetItemFromStorage>(
    [this](ClientId clientId, auto& command)
    {
      HandleGetItemFromStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandWearEquipment>(
    [this](ClientId clientId, auto& command)
    {
      HandleWearEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRemoveEquipment>(
    [this](ClientId clientId, auto& command)
    {
      HandleRemoveEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUseItem>(
    [this](ClientId clientId, auto& command)
    {
      HandleUseItem(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandCreateGuild>(
    [this](ClientId clientId, auto& command)
    {
      HandleCreateGuild(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestGuildInfo>(
    [this](ClientId clientId, auto& command)
    {
      HandleRequestGuildInfo(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRUpdatePet>(
    [this](ClientId clientId, auto& command)
    {
      HandleUpdatePet(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUserPetInfos>(
    [this](ClientId clientId, auto& command)
    {
      HandleUserPetInfos(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestNpcDressList>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestNpcDressList(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandHousingBuild>(
    [this](ClientId clientId, auto& command)
    {
      HandleHousingBuild(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandHousingRepair>(
    [this](ClientId clientId, auto& command)
    {
      HandleHousingRepair(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandMissionEvent>(
    [this](ClientId clientId, auto& command)
    {
      protocol::RanchCommandMissionEvent event
      {
        .event = protocol::RanchCommandMissionEvent::Event::EVENT_CALL_NPC_RESULT,
        .callerOid = command.callerOid,
        .calledOid = 0x40'00'00'00,
      };

      _commandServer.QueueCommand<decltype(event)>(clientId, [event](){return event;});
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandKickRanch>(
    [this](ClientId clientId, auto& command)
    {
      protocol::RanchCommandKickRanchOK response{};
      _commandServer.QueueCommand<decltype(response)>(clientId, [response](){return response;});

      protocol::RanchCommandKickRanchNotify notify{
        .characterUid = command.characterUid};

      const auto& clientContext = GetAuthorizedClientContext(clientId);
      for (const ClientId& ranchClientId : _ranches[clientContext.visitingRancherUid].clients)
      {
        _commandServer.QueueCommand<decltype(notify)>(
          ranchClientId,
          [notify]()
          {
            return notify;
          });
      }
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandOpCmd>(
    [this](ClientId clientId, auto& command)
    {
      HandleOpCmd(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestLeagueTeamList>(
    [this](ClientId clientId, auto& command)
    {
      HandleRequestLeagueTeamList(clientId, command);
    });

}

void RanchDirector::Initialize()
{
  spdlog::debug(
    "Ranch server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _commandServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
}

void RanchDirector::Terminate()
{
  _commandServer.EndHost();
}

void RanchDirector::Tick()
{
}

void RanchDirector::HandleClientConnected(ClientId clientId)
{
  spdlog::info("Client {} connected to the ranch", clientId);
}

void RanchDirector::HandleClientDisconnected(ClientId clientId)
{
  spdlog::info("Client {} disconnected from the ranch", clientId);

  HandleRanchLeave(clientId);
  _clients.erase(clientId);
}

void RanchDirector::BroadcastSetIntroductionNotify(
  uint32_t characterUid,
  const std::string& introduction)
{
  const auto& clientContext = GetClientContextByCharacterUid(characterUid);

  protocol::RanchCommandSetIntroductionNotify notify{
    .characterUid = characterUid,
    .introduction = introduction};

  for (const ClientId& ranchClientId : _ranches[clientContext.visitingRancherUid].clients)
  {
    // Prevent broadcast to self.
    if (ranchClientId == clientContext.characterUid)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::BroadcastUpdateMountInfoNotify(
  const data::Uid characterUid,
  const data::Uid rancherUid,
  const data::Uid horseUid)
{
  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
    horseUid);

  protocol::RanchCommandUpdateMountInfoNotify notify{};
  horseRecord->Immutable([&notify](const data::Horse& horse)
  {
    protocol::BuildProtocolHorse(notify.horse, horse);
  });

  for (const ClientId& ranchClientId : _ranches[rancherUid].clients)
  {
    const auto& ranchClientContext = GetAuthorizedClientContext(ranchClientId);

    // Prevent broadcast to self.
    if (ranchClientContext.characterUid == characterUid)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

ServerInstance& RanchDirector::GetServerInstance()
{
  return _serverInstance;
}

Config::Ranch& RanchDirector::GetConfig()
{
  return GetServerInstance().GetSettings().ranch;
}

RanchDirector::ClientContext& RanchDirector::GetAuthorizedClientContext(ClientId clientId)
{
  const auto clientIter = _clients.find(clientId);
  if (clientIter == _clients.cend())
    throw std::runtime_error("Client context not available");

  auto& clientContext = clientIter->second;
  if (not clientContext.isAuthorized)
    throw std::runtime_error("Client unauthorized");

  return clientContext;
}

RanchDirector::ClientContext& RanchDirector::GetClientContextByCharacterUid(
  data::Uid characterUid)
{
  for (auto& clientContext : _clients | std::views::values)
  {
    if (clientContext.characterUid == characterUid)
      return clientContext;
  }

  throw std::runtime_error("Character not associated with any client");
}

void RanchDirector::HandleEnterRanch(
  ClientId clientId,
  const protocol::AcCmdCREnterRanch& command)
{
  const auto [clientIter, clientCreated] = _clients.try_emplace(clientId);
  auto& clientContext = clientIter->second;

  const auto rancherRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
    command.rancherUid);
  if (not rancherRecord)
    throw std::runtime_error(
      std::format("Rancher's character [{}] not available", command.rancherUid));

  clientContext.isAuthorized = GetServerInstance().GetOtpRegistry().AuthorizeCode(
    command.characterUid, command.otp);

  bool isRanchLocked = false;
  if (command.rancherUid != command.characterUid)
  {
    rancherRecord->Immutable(
      [&isRanchLocked](const data::Character& character)
      {
        isRanchLocked = character.isRanchLocked();
      });
  }

  const auto [ranchIter, ranchCreated] = _ranches.try_emplace(command.rancherUid);
  auto& ranchInstance = ranchIter->second;

  const bool isRanchFull = ranchInstance.clients.size() > MaxRanchCharacterCount;

  if (not clientContext.isAuthorized
    || isRanchLocked
    || isRanchFull)
  {
    protocol::RanchCommandEnterRanchCancel response{};
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  if (clientCreated)
    clientContext.characterUid = command.characterUid;
  clientContext.visitingRancherUid = command.rancherUid;

  protocol::AcCmdCREnterRanchOK response{
    .rancherUid = command.rancherUid,
    .league = {
      .type = League::Type::Platinum,
      .rankingPercentile = 50}};

  rancherRecord->Immutable(
    [this, &response, ranchInstance, ranchCreated](
      const data::Character& rancher) mutable
    {
      const auto& rancherName = rancher.name();
      const bool endsWithPlural = rancherName.ends_with("s") || rancherName.ends_with("S");
      const std::string possessiveSuffix = endsWithPlural ? "'" : "'s";

      response.rancherName = rancherName;
      response.ranchName = std::format("{}{} ranch", rancherName, possessiveSuffix);

      // If the ranch was just created add the horses to the world tracker.
      if (ranchCreated)
      {
        for (const auto& horseUid : rancher.horses())
        {
          ranchInstance.worldTracker.AddHorse(horseUid);
        }
      }

      // Fill the housing info.
      const auto housingRecords = GetServerInstance().GetDataDirector().GetHousing().Get(
        rancher.housing());
      if (housingRecords)
      {
        protocol::BuildProtocolHousing(response.housing, *housingRecords);
      }
      else
      {
        spdlog::warn("Housing records not available for rancher {} ({})", rancherName, rancher.uid());
      }

      if (rancher.isRanchLocked())
        response.bitset = protocol::AcCmdCREnterRanchOK::Bitset::IsLocked;
    });

  // Add the character to the ranch.
  ranchInstance.worldTracker.AddCharacter(
    command.characterUid);

  // The character that is currently entering the ranch.
  RanchCharacter characterEnteringRanch;

  // Add the ranch horses.
  for (auto [horseUid, horseOid] : ranchInstance.worldTracker.GetHorses())
  {
    auto& ranchHorse = response.horses.emplace_back();
    ranchHorse.horseOid = horseOid;

    auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(horseUid);
    if (not horseRecord)
      throw std::runtime_error(
        std::format("Ranch horse [{}] not available", horseUid));

    horseRecord->Immutable([&ranchHorse](const data::Horse& horse)
    {
      protocol::BuildProtocolHorse(ranchHorse.horse, horse);
    });
  }

  // Add the ranch characters.
  for (auto [characterUid, characterOid] : ranchInstance.worldTracker.GetCharacters())
  {
    auto& protocolCharacter = response.characters.emplace_back();
    protocolCharacter.oid = characterOid;

    auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(characterUid);
    if (not characterRecord)
      throw std::runtime_error(
        std::format("Ranch character [{}] not available", characterUid));

    characterRecord.Immutable([this, &protocolCharacter](const data::Character& character)
    {
      protocolCharacter.uid = character.uid();
      protocolCharacter.name = character.name();
      protocolCharacter.profileIcon = character.role() == data::Character::Role::GameMaster
        ? RanchCharacter::ProfileIcon::GameMaster
        : character.parts.modelId() == 10
          ? RanchCharacter::ProfileIcon::Boy
          : RanchCharacter::ProfileIcon::Girl;
      protocolCharacter.age = 19;
      protocolCharacter.hideGenderAndAge = 0;

      protocolCharacter.introduction = character.introduction();

      protocol::BuildProtocolCharacter(protocolCharacter.character, character);

      // Character's equipment.
      const auto equipment = GetServerInstance().GetDataDirector().GetItems().Get(
        character.characterEquipment());
      if (not equipment)
      {
        throw std::runtime_error(
          std::format(
            "Ranch character's [{} ({})] equipment is not available",
            character.name(),
            character.uid()));
      }

      protocol::BuildProtocolItems(protocolCharacter.characterEquipment, *equipment);

      // Character's mount.
      const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
        character.mountUid());
      if (not mountRecord)
      {
        throw std::runtime_error(
          std::format(
            "Ranch character's [{} ({})] mount [{}] is not available",
            character.name(),
            character.uid(),
            character.mountUid()));
      }

      mountRecord->Immutable([&protocolCharacter](const data::Horse& horse)
      {
        protocol::BuildProtocolHorse(protocolCharacter.mount, horse);
        protocolCharacter.rent = {
          .mountUid = horse.uid(),
          .val1 = 0x12};
      });

      // Character's guild
      if (character.guildUid() != data::InvalidUid)
      {
        const auto guildRecord =  GetServerInstance().GetDataDirector().GetGuild(
          character.guildUid());
        if (not guildRecord)
        {
          throw std::runtime_error(
            std::format(
              "Ranch character's [{} ({})] guild [{}] is not available",
              character.name(),
              character.uid(),
              character.guildUid()));
        }

        guildRecord.Immutable([&protocolCharacter](const data::Guild& guild)
        {
          protocol::BuildProtocolGuild(protocolCharacter.guild, guild);
        });
      }

      // Character's pet
      if (character.petUid() != data::InvalidUid)
      {
        const auto petRecord =  GetServerInstance().GetDataDirector().GetPet(
          character.petUid());
        if (not petRecord)
        {
          throw std::runtime_error(
            std::format(
              "Ranch character's [{} ({})] pet [{}] is not available",
              character.name(),
              character.uid(),
              character.petUid()));
        }

        petRecord.Immutable([&protocolCharacter](const data::Pet& pet)
        {
          protocol::BuildProtocolPet(protocolCharacter.pet, pet);
        });
      }
    });

    if (command.characterUid == characterUid)
    {
      characterEnteringRanch = protocolCharacter;
    }
  }

  // Todo: Roll the code for the connecting client.
  _commandServer.SetCode(clientId, {});
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Notify to all other players of the entering player.
  protocol::RanchCommandEnterRanchNotify ranchJoinNotification{
    .character = characterEnteringRanch};

  // Iterate over all the clients connected
  // to the ranch and broadcast join notification.
  for (ClientId ranchClient : ranchInstance.clients)
  {
    _commandServer.QueueCommand<decltype(ranchJoinNotification)>(
      ranchClient,
      [ranchJoinNotification](){
        return ranchJoinNotification;
      });
  }

  ranchInstance.clients.emplace(clientId);
}

void RanchDirector::HandleRanchLeave(ClientId clientId)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);

  const auto ranchIter = _ranches.find(clientContext.visitingRancherUid);
  if (ranchIter == _ranches.cend())
  {
    spdlog::warn(
      "Client {} tried to leave a ranch of {} which is not instanced",
      clientId,
      clientContext.visitingRancherUid);
    return;
  }

  auto& ranchInstance = ranchIter->second;

  protocol::AcCmdCRLeaveRanchNotify notify{
    .characterId = clientContext.characterUid};

  ranchInstance.worldTracker.RemoveCharacter(notify.characterId);
  ranchInstance.clients.erase(clientId);

  for (const ClientId& ranchClientId : ranchInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}


void RanchDirector::HandleChat(
  ClientId clientId,
  const protocol::AcCmdCRRanchChat& chat)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);

  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
  const auto rancherRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.visitingRancherUid);

  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];

  const std::string message = chat.message;
  std::string sendersName;
  bool isSenderOp = false;

  characterRecord.Immutable([&sendersName, &isSenderOp](const data::Character& character)
  {
    sendersName = character.name();
    isSenderOp = character.role() == data::Character::Role::GameMaster;
  });

  rancherRecord.Immutable([&sendersName, &message](const data::Character& rancher)
  {
    spdlog::debug("[{}'s ranch] {}: {}", rancher.name(), sendersName, message);
  });

  if (chat.message.starts_with("//"))
  {
    for (const auto& response : HandleCommand(clientId, chat.message))
    {
      protocol::AcCmdCRRanchChatNotify notify{
        .author = "system",
        .message = response,
        .isBlue = true,
      };
      SendChat(clientId, notify);
    }

    return;
  }

  protocol::AcCmdCRRanchChatNotify response{
    .author = sendersName,
    .message = message,
    .isBlue = isSenderOp};

  for (const auto ranchClientId : ranchInstance.clients)
  {
    SendChat(ranchClientId, response);
  }
}

std::vector<std::string> RanchDirector::HandleCommand(
  ClientId clientId,
  const std::string& message)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  const auto command = util::TokenizeString(
    message.substr(2), ' ');

  if (command.empty())
    return {"No command provided."};

  if (command[0] == "help")
  {
    return {
      "List of available commands:",

      "//online\n"
      "Lists online players",

      "//emblem <ID>\n"
      "Sets your character's emblem",

      "//give item <count> <tid>\n"
      "  - count - Count of items\n"
      "  - tid - TID of item",

      "//horse parts <skinId>\n"
      "              <faceId>\n"
      "              <maneId>\n"
      "              <tailId>\n"
      "  - skinId - ID of the coat\n"
      "  - faceId - ID of the face\n"
      "  - maneId - ID of the mane\n"
      "  - tailId - ID of the tail",

      "Note: to ignore any parameter,\n"
      "       simply specify 0 as the value.",

      "//horse appearance <scale>\n"
      "                   <legLength>\n"
      "                   <legVolume>\n"
      "                   <bodyLength>\n"
      "                   <bodyVolume>\n"
      "  - scale - Defaults to 5\n"
      "  - legLength - Defaults to 5\n"
      "  - legVolume - Defaults to 5\n"
      "  - bodyLength - Defaults to 5\n"
      "  - bodyVolume - Defaults to 5",

      "Note: to ignore any parameter,\n"
      "       simply specify 0 as the value.",


      "//horse randomize\n"
      "  Randomizes your horse appearance and parts."

      "//model <modelId>\n"
      "Sets your character's model",

      };
  }

  if (command[0] == "emblem")
  {
    if (command.size() < 2)
      return {"Invalid command argument. (//emblem <ID>)"};

    const uint32_t emblemId = std::atoi(command[1].c_str());
    characterRecord.Mutable([emblemId](data::Character& character)
    {
      character.appearance.emblemId = emblemId;
    });
    return {std::format("Set your emblem, restart your game.")};
  }

  if (command[0] == "model")
  {
    if (command.size() < 2)
      return {"Invalid command argument. (//model <ID>)"};

    const uint32_t modelId = std::atoi(command[1].c_str());
    characterRecord.Mutable([modelId](data::Character& character)
    {
      character.parts.modelId = modelId;
    });
    return {std::format("Set your model id, restart your game.")};
  }

  if (command[0] == "visit")
  {
    if (command.size() < 2)
      return {"Invalid command argument. (//visit <name>)"};

    const std::string characterName = command[1];

    auto visitingCharacterUid = data::InvalidUid;
    bool ranchLocked = true;

    const auto onlineCharacters = GetServerInstance().GetDataDirector().GetCharacters().GetKeys();
    for (const data::Uid onlineCharacterUid : onlineCharacters)
    {
      const auto onlineCharacterRecord = GetServerInstance().GetDataDirector().GetCharacters().Get(
        onlineCharacterUid, false);
      if (not onlineCharacterRecord)
        continue;

      onlineCharacterRecord->Immutable([&visitingCharacterUid, &characterName, &ranchLocked](const data::Character& character)
      {
        if (characterName == character.name())
        {
          if (not character.isRanchLocked())
          {
            visitingCharacterUid = character.uid();
            ranchLocked = false;
          }
        }
      });

      if (visitingCharacterUid != data::InvalidUid)
        break;
    }

    if (visitingCharacterUid != data::InvalidUid)
    {
      GetServerInstance().GetLobbyDirector().UpdateVisitPreference(
        clientContext.characterUid, visitingCharacterUid);

      return {std::format("Next time you enter the portal, you'll visit {}", characterName)};
    }

    if (ranchLocked)
    {
      return {std::format("This player's ranch is locked.", characterName)};
    }

    return {std::format("Nobody with name '{}' online. Use //online to view online player.", characterName)};
  }

  if (command[0] == "online")
  {
    std::vector<std::string> response;
    response.emplace_back() = std::format(
      "Players online ({}):",
      _clients.size());

    for (const auto& clientContext : _clients)
    {
      const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
        clientContext.second.characterUid);
      characterRecord.Immutable([&response, &clientContext, clientId](const data::Character& character)
      {
        response.emplace_back() = std::format(
          "{}{}", character.name(), clientContext.first == clientId ? " (you)" : "");
      });
    }
    return response;
  }

  if (command[0] == "horse")
  {
    if (command.size() < 2)
      return {"Invalid command sub-literal. (//horse <appearance/parts/randomize>)"};

    auto mountUid = data::InvalidUid;

    if (command[1] == "parts")
    {
      if (command.size() < 6)
        return {"Invalid command arguments. (//horse parts <skinId> <faceId> <maneId> <tailId>)"};
      data::Horse::Parts parts{
        .skinTid = static_cast<uint32_t>(std::atoi(command[2].c_str())),
        .faceTid = static_cast<uint32_t>(std::atoi(command[3].c_str())),
        .maneTid = static_cast<uint32_t>(std::atoi(command[4].c_str())),
        .tailTid = static_cast<uint32_t>(std::atoi(command[5].c_str()))};

      characterRecord.Immutable([this, &mountUid, &parts](const data::Character& character)
      {
        GetServerInstance().GetDataDirector().GetHorses().Get(character.mountUid())->Mutable([&parts](data::Horse& horse)
        {
          if (parts.faceTid() != 0)
            horse.parts.faceTid() = parts.faceTid();
          if (parts.maneTid() != 0)
            horse.parts.maneTid() = parts.maneTid();
          if (parts.skinTid() != 0)
            horse.parts.skinTid() = parts.skinTid();
          if (parts.tailTid() != 0)
            horse.parts.tailTid() = parts.tailTid();
        });

        mountUid = character.mountUid();
      });

      BroadcastUpdateMountInfoNotify(clientContext.characterUid, clientContext.visitingRancherUid, mountUid);
      return {"Parts set! Restart the client."};
    }

    if (command[1] == "appearance")
    {
      if (command.size() < 7)
        return {"Invalid command arguments. (//horse appearance <scale> <legLength> <legVolume> <bodyLength> <bodyVolume>)"};
      data::Horse::Appearance appearance{
        .scale = static_cast<uint32_t>(std::atoi(command[2].c_str())),
        .legLength = static_cast<uint32_t>(std::atoi(command[3].c_str())),
        .legVolume = static_cast<uint32_t>(std::atoi(command[4].c_str())),
        .bodyLength = static_cast<uint32_t>(std::atoi(command[5].c_str())),
        .bodyVolume = static_cast<uint32_t>(std::atoi(command[6].c_str()))};

      characterRecord.Immutable([this, &mountUid, &appearance](const data::Character& character)
      {
        GetServerInstance().GetDataDirector().GetHorses().Get(character.mountUid())->Mutable([&appearance](
          data::Horse& horse)
        {
          if (appearance.scale() != 0)
            horse.appearance.scale() = appearance.scale();
          if (appearance.legLength() != 0)
            horse.appearance.legLength() = appearance.legLength();
          if (appearance.legVolume() != 0)
            horse.appearance.legVolume() = appearance.legVolume();
          if (appearance.bodyLength() != 0)
            horse.appearance.bodyLength() = appearance.bodyLength();
          if (appearance.bodyVolume() != 0)
            horse.appearance.bodyVolume() = appearance.bodyVolume();
        });
        mountUid = character.mountUid();
      });

      BroadcastUpdateMountInfoNotify(clientContext.characterUid, clientContext.visitingRancherUid, mountUid);
      return {"Appearance set! Restart the client."};
    }

    if (command[1] == "randomize")
    {
      characterRecord.Immutable([this, &mountUid](const data::Character& character)
      {
        GetServerInstance().GetDataDirector().GetHorses().Get(character.mountUid())->Mutable([](data::Horse& horse)
        {
          HorseRegistry::Get().BuildRandomHorse(horse.parts, horse.appearance);
        });
        mountUid = character.mountUid();
      });
      return {"Appearance and parts randomized! Restart the client."};
    }
  }
  if (command[0] == "give")
  {
    if (command.size() < 2)
      return {"Invalid command sub-literal. (//give <item/horse>)"};

    if (command[1] == "item")
    {
      if (command.size() < 4)
        return {"Invalid command arguments. (//give item <count> <tid>)"};

      uint32_t itemCount = std::atoi(command[2].c_str());
      data::Uid createdItemTid = std::atoi(command[3].c_str());

      // Create the item.
      auto createdItemUid = data::InvalidUid;
      const auto createdItemRecord = GetServerInstance().GetDataDirector().CreateItem();
      createdItemRecord.Mutable([createdItemTid, itemCount, &createdItemUid](data::Item& item)
      {
        item.tid() = createdItemTid;
        item.count() = itemCount;

        createdItemUid = item.uid();
      });

      // Create the stored item.
      auto giftUid = data::InvalidUid;
      const auto storedItem = GetServerInstance().GetDataDirector().CreateStorageItem();
      storedItem.Mutable([this, &giftUid, createdItemUid, createdItemTid](data::StorageItem& storedItem)
      {
        storedItem.items().emplace_back(createdItemUid);
        storedItem.sender() = "System";
        storedItem.message() = std::format("Item '{}'", createdItemTid);

        giftUid = storedItem.uid();
      });

      // Add the stored item as a gift.

      characterRecord.Mutable([giftUid](data::Character& character)
      {
        character.gifts().emplace_back(giftUid);
      });

      return {"Item given. Check your gifts in inventory!"};
    }

    if (command[1] == "horse")
    {
      if (command.size() < 3)
      {
        return {"Not implemented"};
      }
    }
  }

  return {"Invalid command"};
}

void RanchDirector::SendChat(
  ClientId clientId,
  const protocol::AcCmdCRRanchChatNotify& chat)
{
  _commandServer.QueueCommand<protocol::AcCmdCRRanchChatNotify>(
    clientId,
    [chat]()
    {
      return chat;
    });
}

void RanchDirector::HandleSnapshot(
  ClientId clientId,
  const protocol::AcCmdCRRanchSnapshot& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];

  protocol::RanchCommandRanchSnapshotNotify notify{
    .ranchIndex = ranchInstance.worldTracker.GetCharacterOid(
      clientContext.characterUid),
    .type = command.type,
  };

  switch (command.type)
  {
    case protocol::AcCmdCRRanchSnapshot::Full:
    {
      notify.full = command.full;
      break;
    }
    case protocol::AcCmdCRRanchSnapshot::Partial:
    {
      notify.partial = command.partial;
      break;
    }
  }

  for (const auto& ranchClient : ranchInstance.clients)
  {
    // Do not broadcast to the client that sent the snapshot.
    if (ranchClient == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClient,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::HandleCmdAction(
  ClientId clientId,
  const protocol::AcCmdCRRanchCmdAction& command)
{
  protocol::RanchCommandRanchCmdActionNotify response{
    .unk0 = 2,
    .unk1 = 3,
    .unk2 = 1,};

  // TODO: Actual implementation of it
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRanchStuff(
  ClientId clientId,
  const protocol::RanchCommandRanchStuff& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  if (not characterRecord)
  {
    throw std::runtime_error(
      std::format("Character [{}] not available", clientContext.characterUid));
  }

  protocol::RanchCommandRanchStuffOK response{
    command.eventId,
    command.value};

  // Todo: needs validation
  characterRecord.Mutable([&command, &response](data::Character& character)
  {
    character.carrots() += command.value;
    response.totalMoney = character.carrots();
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]
    {
      return response;
    });
}

void RanchDirector::HandleUpdateBusyState(
  ClientId clientId,
  const protocol::RanchCommandUpdateBusyState& command)
{
  auto& clientContext = GetAuthorizedClientContext(clientId);
  auto& ranchInstance = _ranches[clientContext.visitingRancherUid];

  protocol::RanchCommandUpdateBusyStateNotify response {
    .characterUid = clientContext.characterUid,
    .busyState = command.busyState};

  clientContext.busyState = command.busyState;

  for (auto ranchClientId : ranchInstance.clients)
  {
    // Do not broadcast to self.
    if (ranchClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(response)>(
      ranchClientId,
      [response]()
      {
        return response;
      });
  }
}

void RanchDirector::HandleSearchStallion(
  ClientId clientId,
  const protocol::RanchCommandSearchStallion& command)
{
  protocol::RanchCommandSearchStallionOK response{
    .unk0 = 0,
    .unk1 = 0};

  const auto horseUids = GetServerInstance().GetDataDirector().GetHorses().GetKeys();
  for (const auto& horseUid : horseUids)
  {
    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
      horseUid);

    auto& stallion = response.stallions.emplace_back();
    horseRecord->Immutable([&stallion](const data::Horse& horse)
    {
      stallion.unk0 = "unk";
      stallion.uid = horse.uid();
      stallion.tid = horse.tid();

      stallion.chance = 0xFF;
      stallion.unk7 = 0xFF;
      stallion.unk8 = 0xFF;

      stallion.name = horse.name();
      stallion.grade = horse.grade();
      stallion.unk11 = 0xFF;

      protocol::BuildProtocolHorseStats(stallion.stats, horse.stats);
      protocol::BuildProtocolHorseParts(stallion.parts, horse.parts);
      protocol::BuildProtocolHorseAppearance(stallion.appearance, horse.appearance);
    });
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleEnterBreedingMarket(
  ClientId clientId,
  const protocol::RanchCommandEnterBreedingMarket& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::RanchCommandEnterBreedingMarketOK response;

  characterRecord.Immutable([this, &response](const data::Character& character)
  {
    const auto horseRecords = GetServerInstance().GetDataDirector().GetHorses().Get(
      character.horses());

    for (const auto& horseRecord : *horseRecords)
    {
      auto& protocolHorse = response.availableHorses.emplace_back();
      horseRecord.Immutable([&protocolHorse](const data::Horse& horse)
      {
        protocolHorse.uid = horse.uid();
        protocolHorse.tid = horse.tid();

        // todo figure out the rest
      });
    }
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleTryBreeding(
  ClientId clientId,
  const protocol::RanchCommandTryBreeding& command)
{
  protocol::RanchCommandTryBreedingOK response{
    .uid = command.unk0, // wild guess
    .tid = command.unk1, // lmao
    .val = 0,
    .count = 0,
    .unk0 = 0,
    .parts = {
      .skinId = 1,
      .maneId = 4,
      .tailId = 4,
      .faceId = 5},
    .appearance = {.scale = 4, .legLength = 4, .legVolume = 5, .bodyLength = 3, .bodyVolume = 4},
    .stats = {.agility = 9, .control = 9, .speed = 9, .strength = 9, .spirit = 9},
    .unk1 = 0,
    .unk2 = 0,
    .unk3 = 0,
    .unk4 = 0,
    .unk5 = 0,
    .unk6 = 0,
    .unk7 = 0,
    .unk8 = 0,
    .unk9 = 0,
    .unk10 = 0,
  };

  // TODO: Actually do something
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleBreedingWishlist(
  ClientId clientId,
  const protocol::RanchCommandBreedingWishlist& command)
{
  protocol::RanchCommandBreedingWishlistOK response{};

  // TODO: Actually do something
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleUpdateMountNickname(
  ClientId clientId,
  const protocol::RanchCommandUpdateMountNickname& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // Flag indicating whether the user is allowed to rename their mount which is when:
  // - the user has a horse rename item
  // - the mount's name is empty
  bool canRenameHorse = false;

  characterRecord.Mutable([this, &canRenameHorse, horseUid = command.horseUid](data::Character& character)
  {
    const bool ownsHorse =  character.mountUid() == horseUid
      || std::ranges::contains(character.horses(), horseUid);

    if (not ownsHorse)
      return;

    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorse(horseUid);
    if (not horseRecord)
      return;

    // If the horse does not have a name, allow them to rename it.
    horseRecord.Immutable([&canRenameHorse](const data::Horse& horse)
    {
      canRenameHorse = horse.name().empty();
    });

    if (canRenameHorse)
      return;

    constexpr data::Tid HorseRenameItem = 45003;
    const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
      character.items());

    // Find the horse rename item.
    auto horseRenameItemUid = data::InvalidUid;
    for (const auto& itemRecord : *itemRecords)
    {
      itemRecord.Immutable([&horseRenameItemUid](const data::Item& item)
      {
        if (item.tid() == HorseRenameItem)
        {
          horseRenameItemUid = item.uid();
        }
      });

      // Break early if the item was found.
      if (horseRenameItemUid != data::InvalidUid)
        break;
    }

    if (horseRenameItemUid == data::InvalidUid)
    {
      return;
    }

    // Find the item in the inventory.
    const auto itemInventoryIter = std::ranges::find(
      character.items(), horseRenameItemUid);

    // Remove the item from the inventory.
    character.items().erase(itemInventoryIter);
    canRenameHorse = true;
  });

  if (not canRenameHorse)
  {
    protocol::RanchCommandUpdateMountNicknameCancel response{};
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
    command.horseUid);

  horseRecord->Mutable([horseName = command.name](data::Horse& horse)
  {
    horse.name() = horseName;
  });

  protocol::RanchCommandUpdateMountNicknameOK response{
    .horseUid = command.horseUid,
    .nickname = command.name,
    .unk1 = command.unk1,
    .unk2 = 0};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  BroadcastUpdateMountInfoNotify(clientContext.characterUid, clientContext.visitingRancherUid, response.horseUid);
}

void RanchDirector::HandleRequestStorage(
  ClientId clientId,
  const protocol::RanchCommandRequestStorage& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::RanchCommandRequestStorageOK response{
    .category = command.category,
    .page = command.page};

  const bool showPurchases = command.category == protocol::RanchCommandRequestStorage::Category::Purchases;

  // Fill the stored items, either from the purchase category or the gift category.

  characterRecord.Immutable(
    [this, showPurchases, page = static_cast<size_t>(command.page), &response](
      const data::Character& character) mutable
    {
      const auto storedItemRecords = GetServerInstance().GetDataDirector().GetStorageItem().Get(
        showPurchases ? character.purchases() : character.gifts());
      if (not storedItemRecords || storedItemRecords->empty())
        return;

      const auto pagination = std::views::chunk(*storedItemRecords, 5);
      page = std::max(std::min(page - 1, pagination.size() - 1), size_t{0});

      response.pageCountAndNotification = pagination.size() << 2;

      protocol::BuildProtocolStoredItems(response.storedItems, pagination[page]);
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleGetItemFromStorage(
  ClientId clientId,
  const protocol::RanchCommandGetItemFromStorage& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  bool storedItemIsValid = true;

  // Try to remove the stored item from the character.
  characterRecord.Mutable(
    [this, &storedItemIsValid, storedItemUid = command.storedItemUid](
      data::Character& character)
    {
      // The stored item is either a gift or a purchase.
      // Try to remove from both gift and purchase vectors.

      const auto storedGiftIter = std::ranges::find(character.gifts(), storedItemUid);
      if (storedGiftIter != character.gifts().cend())
      {
        character.gifts().erase(storedGiftIter);
        return;
      }

      const auto storedPurchaseIter = std::ranges::find(character.purchases(), storedItemUid);
      if (storedGiftIter != character.purchases().cend())
      {
        character.purchases().erase(storedGiftIter);
        return;
      }

      storedItemIsValid = false;
    });

  // If the stored item is invalid cancel the takeout.
  if (not storedItemIsValid)
  {
    protocol::RanchCommandGetItemFromStorageCancel response{
      .storedItemUid = command.storedItemUid,
      .status = 0};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    return;
  }

  protocol::RanchCommandGetItemFromStorageOK response{
    .storedItemUid = command.storedItemUid,
    .member0 = 0};

  // Get the items assigned to the stored item and fill the protocol command.
  characterRecord.Mutable([this, &response](
    data::Character& character)
    {
      const auto storedItemRecord = GetServerInstance().GetDataDirector().GetStorageItem().Get(
        response.storedItemUid);

      // Collection of the items received from the stored item.
      std::vector<data::Uid> items;

      storedItemRecord->Immutable([this, &items, &response](const data::StorageItem& storedItem)
      {
        items = storedItem.items();
        const auto itemRecords = GetServerInstance().GetDataDirector().GetItems().Get(
          items);

        protocol::BuildProtocolItems(response.items, *itemRecords);
      });

      // Add the items to the character's inventory.
      character.items().insert(
        character.items().end(),
        items.begin(),
        items.end());
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestNpcDressList(
  ClientId clientId,
  const protocol::RanchCommandRequestNpcDressList& requestNpcDressList)
{
  protocol::RanchCommandRequestNpcDressListOK response{
    .unk0 = requestNpcDressList.unk0,
    .dressList = {} // TODO: Fetch dress list from somewhere
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleWearEquipment(
  ClientId clientId,
  const protocol::RanchCommandWearEquipment& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  bool equipSuccessful = false;
  characterRecord.Mutable([&command, &equipSuccessful](data::Character& character)
  {
    const bool hasEquippedItem = std::ranges::contains(
      character.items(), command.itemUid);
    const bool hasMountedHorse = std::ranges::contains(
      character.horses(), command.itemUid);

    // Make sure the equip UID is either a valid item or a horse.
    equipSuccessful = hasEquippedItem || hasMountedHorse;

    if (hasMountedHorse)
    {
      // Add the mount back to the horse list.
      character.horses().emplace_back(character.mountUid());
      character.mountUid() = command.itemUid;

      // Remove the new mount from the horse list.
      character.horses().erase(std::ranges::find(character.horses(), command.itemUid));
    }
    else if (hasEquippedItem)
    {
      const bool isItemAlreadyEquipped = std::ranges::contains(
        character.characterEquipment(), command.itemUid);
      if (not isItemAlreadyEquipped)
      {
        character.characterEquipment().emplace_back(command.itemUid);
      }
    }
  });

  if (equipSuccessful)
  {
    protocol::RanchCommandWearEquipmentOK response{
      .itemUid = command.itemUid,
      .member = command.member};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    BroadcastEquipmentUpdate(clientId);

    return;
  }

  protocol::RanchCommandWearEquipmentCancel response{
    .itemUid = command.itemUid,
    .member = command.member};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRemoveEquipment(
  ClientId clientId,
  const protocol::RanchCommandRemoveEquipment& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  characterRecord.Mutable([&command](data::Character& character)
  {
    const bool ownsItem = std::ranges::contains(
      character.items(), command.itemUid);

    // You can't really unequip a horse. You can only switch to a different one.
    // At least in Alicia 1.0.

    if (ownsItem)
    {
      const auto range = std::ranges::remove(
        character.characterEquipment(), command.itemUid);
      character.characterEquipment().erase(range.begin(), range.end());
    }
  });

  // We really don't need to cancel the unequip.
  // Always respond with OK.
  protocol::RanchCommandRemoveEquipmentOK response{
    .uid = command.itemUid};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  BroadcastEquipmentUpdate(clientId);
}

void RanchDirector::HandleCreateGuild(
  ClientId clientId,
  const protocol::RanchCommandCreateGuild& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  bool canCreateGuild = false;
  characterRecord.Mutable([&command, &canCreateGuild](data::Character& character)
  {
    // todo money check
    // todo name check
    canCreateGuild = true;
  });

  if (not canCreateGuild)
  {
    protocol::RanchCommandCreateGuildCancel response{
      .status = 0,
      .member2 = 0};

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
  }

  protocol::RanchCommandCreateGuildOK response{
    .uid = 0,
    .member2 = 0};

  const auto guildRecord = GetServerInstance().GetDataDirector().CreateGuild();
  guildRecord.Mutable([&response, &command](data::Guild& guild)
  {
    guild.name = command.name;

    response.uid = guild.uid();
  });

  characterRecord.Mutable([&response](data::Character& character)
  {
    character.guildUid = response.uid;
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestGuildInfo(
  ClientId clientId,
  const protocol::RanchCommandRequestGuildInfo& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  auto guildUid = data::InvalidUid;
  characterRecord.Immutable([&guildUid](const data::Character& character)
    {
      guildUid = character.guildUid();
    });

  protocol::RanchCommandRequestGuildInfoOK response{};

  if (guildUid != data::InvalidUid)
  {
    const auto guildRecord = GetServerInstance().GetDataDirector().GetGuild(guildUid);
    if (not guildRecord)
      throw std::runtime_error("Guild unavailable");

    guildRecord.Immutable([&response](const data::Guild& guild)
      {
        response.guildInfo = {
          .uid = guild.uid(),
          .name = guild.name()};
      });
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleUpdatePet(
  ClientId clientId,
  const protocol::AcCmdCRUpdatePet& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  auto petUid = data::InvalidUid;

  characterRecord.Mutable(
    [this, &command, &petUid](data::Character& character)
    {
      // The pets of the character.
      const auto storedPetRecords = GetServerInstance().GetDataDirector().GetPets().Get(
        character.pets());

      if (not storedPetRecords || storedPetRecords->empty())
      {
        // No pets found for the character.
        spdlog::warn("No pets found for character {}", character.uid());
        // TODO: When Handle Pet Birth exists, this should have a return
      }

      // Find the pet record based on the item used.
      for (const auto& petRecord : *storedPetRecords)
      {
        petRecord.Immutable(
          [&command, &petUid](const data::Pet& pet)
          {
            if (pet.itemUid() == command.petInfo.itemUid)
            {
              petUid = pet.uid();
            }
          });
      }

      // If pet record does not exist, create it.
      // For prototype purposes only.
      if (petUid == data::InvalidUid
        && command.petInfo.pet.petId != 0)
      {
        const auto petRecord = GetServerInstance().GetDataDirector().CreatePet();
        petRecord.Mutable(
          [&command, &petUid](data::Pet& pet)
          {
            pet.petId = command.petInfo.pet.petId;
            pet.name = command.petInfo.pet.name;
            pet.itemUid = command.petInfo.itemUid;

            petUid = pet.uid();
          });

        character.pets().emplace_back(petUid);
      }

      if (command.actionBitset == protocol::AcCmdCRUpdatePet::Action::Rename)
      {
        const auto petRecord = GetServerInstance().GetDataDirector().GetPet(petUid);
        petRecord.Mutable(
          [&command](data::Pet& pet)
          {
            pet.name() = command.petInfo.pet.name;
          });
      }
      else
      {
        character.petUid = petUid;
      }
    });

  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];

  protocol::AcCmdRCUpdatePet response;
  response.petInfo = command.petInfo;

  const auto petRecord = GetServerInstance().GetDataDirector().GetPet(petUid);
  petRecord.Immutable(
    [&response](const data::Pet& pet)
    {
      response.petInfo.pet.name = pet.name();
    });

  response.petInfo.characterUid = clientContext.characterUid;

  for (const ClientId ranchClientId : ranchInstance.clients)
  {
    _commandServer.QueueCommand<decltype(response)>(ranchClientId, [response]()
    {
      return response;
    });
  }
}

void RanchDirector::HandleUserPetInfos(
  ClientId clientId,
  const protocol::RanchCommandUserPetInfos& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
    
  protocol::RanchCommandUserPetInfosOK response{
    .member1 = 0,
    .member3 = 0
  };

  characterRecord.Mutable(
    [this, &command, &response](data::Character& character)
    {
      response.petCount = character.pets().size();
      auto storedPetRecords = GetServerInstance().GetDataDirector().GetPets().Get(
        character.pets());
      if (!storedPetRecords || storedPetRecords->empty())
        return;

      protocol::BuildProtocolPets(response.pets,
        storedPetRecords.value());
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response](){
      return response;
    });
}

void RanchDirector::HandleRequestPetBirth(
  ClientId clientId,
  const protocol::RanchCommandRequestPetBirth& command)
{
}

void RanchDirector::BroadcastEquipmentUpdate(ClientId clientId)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::RanchCommandUpdateEquipmentNotify notify{
    .characterUid = clientContext.characterUid};

  characterRecord.Immutable([this, &notify](const data::Character& character)
  {
    // Character equipment
    const auto characterEquipment = GetServerInstance().GetDataDirector().GetItems().Get(
      character.characterEquipment());
    protocol::BuildProtocolItems(notify.characterEquipment, *characterEquipment);

    // Mount equipment
    const auto mountEquipment = GetServerInstance().GetDataDirector().GetItems().Get(
      character.mountEquipment());
    protocol::BuildProtocolItems(notify.mountEquipment, *mountEquipment);

    // Mount
    const auto mountRecord = GetServerInstance().GetDataDirector().GetHorses().Get(
      character.mountUid());

    mountRecord->Immutable([&notify](const data::Horse& mount)
    {
      protocol::BuildProtocolHorse(notify.mount, mount);
    });
  });

  // Broadcast to all the ranch clients.
  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];
  for (ClientId ranchClientId : ranchInstance.clients)
  {
    // Prevent broadcasting to self.
    if (ranchClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::HandleUseItem(
  ClientId clientId,
  const protocol::RanchCommandUseItem& command)
{
  protocol::RanchCommandUseItemOK response{
    response.itemUid = command.itemUid,
    response.unk1 = command.always1,
    response.type = protocol::RanchCommandUseItemOK::ActionType::Empty};

  // feed, Action 1 through 3
  //   success - both bytes zero
  //   failure - Action empty
  // toys, always Action 1 to Action 3,
  //   play success indicated by the second byte
  // brushes, always empty response
  //   success - Action empty

  spdlog::info("Play - itemUid: {}, mem1: {}, mem2: {}, {} play",
    command.itemUid,
    command.always1,
    command.always1too,
    command.play == protocol::RanchCommandUseItem::Play::Bad
      ? "Bad"
      : command.play == protocol::RanchCommandUseItem::Play::Good
        ? "Good"
        : command.play == protocol::RanchCommandUseItem::Play::CriticalGood
          ? "Critical good"
          : "Perfect");

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleHousingBuild(
  ClientId clientId,
  const protocol::RanchCommandHousingBuild& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // todo catalogue housing uids and handle transaction

  protocol::RanchCommandHousingBuildOK response{
    .member1 = clientContext.characterUid,
    .housingTid = command.housingTid,
    .member3 = 1,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  auto housingUid = data::InvalidUid;

  const auto housingRecord = GetServerInstance().GetDataDirector().CreateHousing();
  housingRecord.Mutable([housingId = command.housingTid, &housingUid](data::Housing& housing)
    {
      housing.housingId = housingId;

      housingUid = housing.uid();
    });

  characterRecord.Mutable([&housingUid](data::Character& character)
    {
      character.housing().emplace_back(housingUid);
    });

  assert(clientContext.visitingRancherUid == clientContext.characterUid);

  protocol::RanchCommandHousingBuildNotify notify{
    .member1 = 1,
    .housingTid = command.housingTid,
  };

  // Broadcast to all the ranch clients.
  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];
  for (ClientId ranchClientId : ranchInstance.clients)
  {
    // Prevent broadcasting to self.
    if (ranchClientId == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClientId,
      [notify]()
      {
        return notify;
      });
  }
}

void RanchDirector::HandleHousingRepair(
  ClientId clientId,
  const protocol::RanchCommandHousingRepair& command)
{
  const auto& clientContext = GetAuthorizedClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
  // todo catalogue housing uids and handle transaction
  protocol::RanchCommandHousingRepairOK response{
    .housingUid = command.housingUid,
    .member2 = 1,
  };
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
};

void RanchDirector::HandleOpCmd(
  ClientId clientId,
  const protocol::RanchCommandOpCmd& command)
{
  std::vector<std::string> feedback;
  const auto commandTokens = util::TokenizeString(command.command, ' ');

  protocol::RanchCommandOpCmdOK response{
    .feedback = "test",
    .observerState = protocol::RanchCommandOpCmdOK::Observer::Enabled};

  _commandServer.QueueCommand<decltype(response)>(clientId, [response](){return response;});
}

void RanchDirector::HandleRequestLeagueTeamList(
  ClientId clientId,
  const protocol::RanchCommandRequestLeagueTeamList& command)
{
  protocol::RanchCommandRequestLeagueTeamListOK response{
    .season = 46,
    .league = 0,
    .group = 1,
    .points = 4,
    .rank = 10,
    .previousRank = 200,
    .breakPoints = 0,
    .unk7 = 0,
    .unk8 = 0,
    .lastWeekLeague = 1,
    .lastWeekGroup = 100,
    .lastWeekRank = 4,
    .lastWeekAvailable = 1,
    .unk13 = 1,
    .members = {
      protocol::RanchCommandRequestLeagueTeamListOK::Member{
        .uid = 1,
        .points = 4000,
        .name = "test"
      }}
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

} // namespace server