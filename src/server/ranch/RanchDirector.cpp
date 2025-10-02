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
#include "libserver/registry/PetRegistry.hpp"
#include "libserver/util/Util.hpp"

#include <ranges>

#include <spdlog/spdlog.h>

#include <zlib.h>

namespace server
{

namespace
{

constexpr size_t MaxRanchHorseCount = 10;
constexpr size_t MaxRanchCharacterCount = 20;
constexpr size_t MaxRanchHousingCount = 13;

constexpr int16_t DoubleIncubatorId = 52;
constexpr int16_t SingleIncubatorId = 51;

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

  _commandServer.RegisterCommandHandler<protocol::AcCmdCREnterBreedingMarket>(
    [this](ClientId clientId, auto& command)
    {
      HandleEnterBreedingMarket(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRSearchStallion>(
    [this](ClientId clientId, auto& command)
    {
      HandleSearchStallion(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRegisterStallion>(
    [this](ClientId clientId, auto& command)
    {
      HandleRegisterStallion(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRUnregisterStallion>(
    [this](ClientId clientId, auto& command)
    {
      HandleUnregisterStallion(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRUnregisterStallionEstimateInfo>(
    [this](ClientId clientId, auto& command)
    {
      HandleUnregisterStallionEstimateInfo(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRStatusPointApply>(
    [this](ClientId clientId, auto& command)
    {
      HandleStatusPointApply(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRTryBreeding>(
    [this](ClientId clientId, auto& command)
    {
      HandleTryBreeding(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRBreedingAbandon>(
    [this](ClientId clientId, auto& command)
    {
      HandleBreedingAbandon(clientId, command);
    });

  // AcCmdCLRequestFestivalResult

  _commandServer.RegisterCommandHandler<protocol::RanchCommandBreedingWishlist>(
    [this](ClientId clientId, auto& command)
    {
      HandleBreedingWishlist(clientId, command);
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

  _commandServer.RegisterCommandHandler<protocol::RanchCommandUpdateMountNickname>(
    [this](ClientId clientId, auto& command)
    {
      HandleUpdateMountNickname(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRequestStorage>(
    [this](ClientId clientId, auto& command)
    {
      HandleRequestStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRGetItemFromStorage>(
    [this](ClientId clientId, auto& command)
    {
      HandleGetItemFromStorage(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRWearEquipment>(
    [this](ClientId clientId, auto& command)
    {
      HandleWearEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRemoveEquipment>(
    [this](ClientId clientId, auto& command)
    {
      HandleRemoveEquipment(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRUseItem>(
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
  
  _commandServer.RegisterCommandHandler<protocol::AcCmdCRIncubateEgg>(
    [this](ClientId clientId, auto& command)
    {
      HandleIncubateEgg(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRBoostIncubateEgg>(
    [this](ClientId clientId, auto& command)
    {
      HandleBoostIncubateEgg(clientId, command);
    });
  
  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRequestPetBirth>(
    [this](ClientId clientId, auto& command)
    {
      HandleRequestPetBirth(clientId, command);
    });
  _commandServer.RegisterCommandHandler<protocol::AcCmdCRBoostIncubateInfoList>(
    [this](ClientId clientId, auto& command)
    {
      HandleBoostIncubateInfoList(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::RanchCommandRequestNpcDressList>(
    [this](ClientId clientId, const auto& message)
    {
      HandleRequestNpcDressList(clientId, message);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRHousingBuild>(
    [this](ClientId clientId, auto& command)
    {
      HandleHousingBuild(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRHousingRepair>(
    [this](ClientId clientId, auto& command)
    {
      HandleHousingRepair(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdRCMissionEvent>(
    [this](ClientId clientId, auto& command)
    {
      protocol::AcCmdRCMissionEvent event
      {
        .event = protocol::AcCmdRCMissionEvent::Event::EVENT_CALL_NPC_RESULT,
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

      const auto& clientContext = GetClientContext(clientId);
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

  _commandServer.RegisterCommandHandler<protocol::RanchCommandMountFamilyTree>(
    [this](ClientId clientId, auto& command)
    {
      HandleMountFamilyTree(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRRecoverMount>(
    [this](ClientId clientId, const auto& command)
    {
      HandleRecoverMount(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRWithdrawGuildMember>(
    [this](ClientId clientId, const auto& command)
    {
      HandleLeaveGuild(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRCheckStorageItem>(
    [this](ClientId clientId, const auto& command)
    {
      HandleCheckStorageItem(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeAge>(
    [this](ClientId clientId, const auto& command)
    {
      HandleChangeAge(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRHideAge>(
    [this](ClientId clientId, const auto& command)
    {
      HandleHideAge(clientId, command);
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

std::vector<data::Uid> RanchDirector::GetOnlineCharacters()
{
  std::vector<data::Uid> onlineCharacterUids;

  for (const auto& clientContext : _clients | std::views::values)
  {
    if (not clientContext.isAuthenticated)
      continue;
    onlineCharacterUids.emplace_back(clientContext.characterUid);
  }

  return onlineCharacterUids;
}

void RanchDirector::HandleClientConnected(ClientId clientId)
{
  spdlog::info("Client {} connected to the ranch", clientId);
  _clients.try_emplace(clientId);
}

void RanchDirector::HandleClientDisconnected(ClientId clientId)
{
  spdlog::info("Client {} disconnected from the ranch", clientId);

  const auto& clientContext = GetClientContext(clientId, false);
  if (clientContext.isAuthenticated)
  {
    HandleRanchLeave(clientId);
  }

  _clients.erase(clientId);
}

void RanchDirector::Disconnect(data::Uid characterUid)
{
  for (auto& clientContext : _clients)
  {
    if (clientContext.second.characterUid == characterUid
      && clientContext.second.isAuthenticated)
    {
      _commandServer.DisconnectClient(clientContext.first);
      return;
    }
  }
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
  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(
    horseUid);

  protocol::AcCmdRCUpdateMountInfoNotify notify{};
  horseRecord->Immutable([&notify](const data::Horse& horse)
  {
    protocol::BuildProtocolHorse(notify.horse, horse);
  });

  for (const ClientId& ranchClientId : _ranches[rancherUid].clients)
  {
    const auto& ranchClientContext = GetClientContext(ranchClientId);

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

void RanchDirector::SendStorageNotification(
  data::Uid characterUid,
  protocol::AcCmdCRRequestStorage::Category category)
{
  ClientId clientId = -1;
  for (auto& clientContext : _clients)
  {
    if (clientContext.second.characterUid == characterUid && clientContext.second.isAuthenticated)
      clientId = clientContext.first;
  }

  if (clientId == -1)
  {
    spdlog::error("Tried to send storage notification to unknown client {} with character uid {}",
      clientId,
      characterUid);
    return;
  }

  // Setting pageCountAndNotification to 0b1 and category is enough
  protocol::AcCmdCRRequestStorageOK response{
    .category = category,
    .pageCountAndNotification = 0b1};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::BroadcastChangeAgeNotify(
  data::Uid characterUid,
  const data::Uid rancherUid,
  protocol::AcCmdCRChangeAge::Age age
)
{
  protocol::AcCmdRCChangeAgeNotify notify{
    .characterUid = characterUid,
    .age = age
  };

  for (const ClientId& ranchClientId : _ranches[rancherUid].clients)
  {
    const auto& ranchClientContext = GetClientContext(ranchClientId);

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

void RanchDirector::BroadcastHideAgeNotify(
  data::Uid characterUid,
  const data::Uid rancherUid,
  protocol::AcCmdCRHideAge::Option option
)
{
  protocol::AcCmdRCHideAgeNotify notify{
    .characterUid = characterUid,
    .option = option
  };

  for (const ClientId& ranchClientId : _ranches[rancherUid].clients)
  {
    const auto& ranchClientContext = GetClientContext(ranchClientId);

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

RanchDirector::ClientContext& RanchDirector::GetClientContext(
  const ClientId clientId,
  const bool requireAuthentication)
{
  const auto clientIter = _clients.find(clientId);
  if (clientIter == _clients.cend())
    throw std::runtime_error("Client context not available");

  auto& clientContext = clientIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Client is not authenticated");

  return clientContext;
}

RanchDirector::ClientContext& RanchDirector::GetClientContextByCharacterUid(
  data::Uid characterUid)
{
  for (auto& clientContext : _clients | std::views::values)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientContext;
  }

  throw std::runtime_error("Character not associated with any client");
}

void RanchDirector::HandleEnterRanch(
  ClientId clientId,
  const protocol::AcCmdCREnterRanch& command)
{
  auto& clientContext = GetClientContext(clientId, false);

  const auto rancherRecord = GetServerInstance().GetDataDirector().GetCharacterCache().Get(
    command.rancherUid);
  if (not rancherRecord)
    throw std::runtime_error(
      std::format("Rancher's character [{}] not available", command.rancherUid));

  clientContext.isAuthenticated = GetServerInstance().GetOtpSystem().AuthorizeCode(
    command.characterUid, command.otp);

  // Determine whether the ranch is locked.
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

  if (not clientContext.isAuthenticated
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

  clientContext.characterUid = command.characterUid;
  clientContext.visitingRancherUid = command.rancherUid;

  protocol::AcCmdCREnterRanchOK response{
    .rancherUid = command.rancherUid,
    .league = {
      .type = League::Type::Platinum,
      .rankingPercentile = 50}};

  rancherRecord->Immutable(
    [this, &response, &ranchInstance, ranchCreated](
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
          ranchInstance.tracker.AddHorse(horseUid);
        }
      }

      // Fill the housing info.
      const auto housingRecords = GetServerInstance().GetDataDirector().GetHousingCache().Get(
        rancher.housing());
      if (housingRecords)
      {
        for (const auto& housingRecord : *housingRecords)
        {
          housingRecord.Immutable([&response](const data::Housing& housing){

            // Certain types of housing have durability instead of expiration time.
            const bool hasDurability = (housing.housingId() == SingleIncubatorId || housing.housingId() == DoubleIncubatorId);
            if (hasDurability) 
            {
              response.incubatorUseCount = housing.durability();
              response.incubatorSlots = housing.housingId() == DoubleIncubatorId ? 2 : 1;
            }

            protocol::BuildProtocolHousing(response.housing.emplace_back(), housing, hasDurability);
          });
        }
      }
      else
      {
        spdlog::warn("Housing records not available for rancher {} ({})", rancherName, rancher.uid());
      }

      if (rancher.isRanchLocked())
        response.bitset = protocol::AcCmdCREnterRanchOK::Bitset::IsLocked;

      // Fill the incubator info.
      const auto eggRecords = GetServerInstance().GetDataDirector().GetEggCache().Get(
        rancher.eggs());
      if (eggRecords)
      {
        for (auto& eggRecord : *eggRecords)
        {
          eggRecord.Immutable(
            [this, &response](const data::Egg& egg)
            {
              // retrieve hatchDuration
              const registry::EggInfo eggTemplate = _serverInstance.GetPetRegistry().GetEggInfo(
                egg.itemTid());
              const auto hatchingDuration = eggTemplate.hatchDuration;
              protocol::BuildProtocolEgg(response.incubator[egg.incubatorSlot()], egg, hatchingDuration );
            });
        }
      }
    });

  // Add the character to the ranch.
  ranchInstance.tracker.AddCharacter(
    command.characterUid);

  // The character that is currently entering the ranch.
  RanchCharacter characterEnteringRanch;

  // Add the ranch horses.
  for (auto [horseUid, horseOid] : ranchInstance.tracker.GetHorses())
  {
    auto& ranchHorse = response.horses.emplace_back();
    ranchHorse.horseOid = horseOid;

    auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(horseUid);
    if (not horseRecord)
      throw std::runtime_error(
        std::format("Ranch horse [{}] not available", horseUid));

    horseRecord->Immutable([&ranchHorse](const data::Horse& horse)
    {
      protocol::BuildProtocolHorse(ranchHorse.horse, horse);
    });
  }

  // Add the ranch characters.
  for (auto [characterUid, characterOid] : ranchInstance.tracker.GetCharacters())
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
      protocolCharacter.role = character.role() == data::Character::Role::GameMaster
        ? RanchCharacter::Role::GameMaster
        : character.role() == data::Character::Role::Op
        ? RanchCharacter::Role::Op // Assumed, tried but no visual change
        : RanchCharacter::Role::User; 
      protocolCharacter.age = character.hideGenderAndAge() ? 0 : character.age();
      protocolCharacter.gender = character.parts.modelId() == 10
          ? RanchCharacter::Gender::Boy
          : RanchCharacter::Gender::Girl;

      protocolCharacter.introduction = character.introduction();

      protocol::BuildProtocolCharacter(protocolCharacter.character, character);

      // Character's equipment.
      const auto equipment = GetServerInstance().GetDataDirector().GetItemCache().Get(
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
      const auto mountRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(
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
  const auto& clientContext = GetClientContext(clientId);

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

  ranchInstance.tracker.RemoveCharacter(clientContext.characterUid);
  ranchInstance.clients.erase(clientId);

  protocol::AcCmdCRLeaveRanchOK response{};
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  protocol::AcCmdCRLeaveRanchNotify notify{
    .characterId = clientContext.characterUid};

  for (const ClientId& ranchClientId : ranchInstance.clients)
  {
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


void RanchDirector::HandleChat(
  ClientId clientId,
  const protocol::AcCmdCRRanchChat& chat)
{
  const auto& clientContext = GetClientContext(clientId);

  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
  const auto rancherRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.visitingRancherUid);

  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];

  std::string sendersName;
  characterRecord.Immutable([&sendersName](const data::Character& character)
  {
    sendersName = character.name();
  });

  std::string ranchersName;
  rancherRecord.Immutable([&ranchersName](const data::Character& rancher)
  {
    ranchersName = rancher.name();
  });

  const std::string message = chat.message;
  spdlog::debug("[{}'s ranch] {}: {}", ranchersName, sendersName, message);

  const auto verdict = _serverInstance.GetChatSystem().ProcessChatMessage(
    clientContext.characterUid,
    message);

  const auto sendAllMessages = [this](
    const ClientId clientId,
    const std::string& sender,
    const bool isSystem,
    const std::vector<std::string>& messages)
  {
    protocol::AcCmdCRRanchChatNotify notify{
      .author = not isSystem ? sender : "",
      .isSystem = isSystem};

    for (const auto& resultMessage : messages)
    {
      notify.message = resultMessage;
      _commandServer.QueueCommand<decltype(notify)>(
        clientId,
        [notify](){ return notify; });
    }
  };

  if (verdict.commandVerdict)
  {
    sendAllMessages(clientId, sendersName, true, verdict.commandVerdict->result);
    return;
  }

  for (const auto& ranchClientId : ranchInstance.clients)
  {
    sendAllMessages(ranchClientId, sendersName, false, {verdict.message});
  }
}

void RanchDirector::HandleSnapshot(
  ClientId clientId,
  const protocol::AcCmdCRRanchSnapshot& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];

  protocol::RanchCommandRanchSnapshotNotify notify{
    .ranchIndex = ranchInstance.tracker.GetCharacterOid(
      clientContext.characterUid),
    .type = command.type,
  };

  switch (command.type)
  {
    case protocol::AcCmdCRRanchSnapshot::Full:
    {
      if (command.full.ranchIndex != notify.ranchIndex)
        throw std::runtime_error("Client sent a snapshot for an entity it's not controlling");
      notify.full = command.full;
      break;
    }
    case protocol::AcCmdCRRanchSnapshot::Partial:
    {
      if (command.full.ranchIndex != notify.ranchIndex)
        throw std::runtime_error("Client sent a snapshot for an entity it's not controlling");
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

void RanchDirector::HandleEnterBreedingMarket(
  ClientId clientId,
  const protocol::AcCmdCREnterBreedingMarket& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::RanchCommandEnterBreedingMarketOK response;

  characterRecord.Immutable(
    [this, &response](const data::Character& character)
    {
      const auto horseRecords = GetServerInstance().GetDataDirector().GetHorseCache().Get(
        character.horses());

      for (const auto& horseRecord : *horseRecords)
      {
        auto& protocolHorse = response.stallions.emplace_back();

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

static std::vector<data::Uid> g_stallions;

void RanchDirector::HandleSearchStallion(
  ClientId clientId,
  const protocol::AcCmdCRSearchStallion& command)
{
  protocol::RanchCommandSearchStallionOK response{
    .unk0 = 0,
    .unk1 = 0};

  for (const data::Uid& stallionUid : g_stallions)
  {
    const auto stallionRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(
      stallionUid);

    auto& protocolStallion = response.stallions.emplace_back();
    stallionRecord->Immutable([&protocolStallion](const data::Horse& stallion)
    {
      protocolStallion.member1 = "unknown";
      protocolStallion.uid = stallion.uid();
      protocolStallion.tid = stallion.tid();

      protocolStallion.name = stallion.name();
      protocolStallion.grade = stallion.grade();

      protocolStallion.expiresAt = util::TimePointToAliciaTime(util::Clock::now() + std::chrono::hours(1));

      protocol::BuildProtocolHorseStats(protocolStallion.stats, stallion.stats);
      protocol::BuildProtocolHorseParts(protocolStallion.parts, stallion.parts);
      protocol::BuildProtocolHorseAppearance(protocolStallion.appearance, stallion.appearance);
    });
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRegisterStallion(
  ClientId clientId,
  const protocol::AcCmdCRRegisterStallion& command)
{
  g_stallions.emplace_back(command.horseUid);

  protocol::AcCmdCRRegisterStallionOK response{
    .horseUid = command.horseUid};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleUnregisterStallion(
  ClientId clientId,
  const protocol::AcCmdCRUnregisterStallion& command)
{
  g_stallions.erase(std::ranges::find(g_stallions, command.horseUid));

  protocol::AcCmdCRUnregisterStallionOK response{};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleUnregisterStallionEstimateInfo(
  ClientId clientId,
  const protocol::AcCmdCRUnregisterStallionEstimateInfo& command)
{
  protocol::AcCmdCRUnregisterStallionEstimateInfoOK response{
    .member1 = 0xFFFF'FFFF,
    .timesMated = 0,
    .matingCompensation = 0,
    .member4 = 0xFFFF'FFFF,
    .matingPrice = 0};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleTryBreeding(
  ClientId clientId,
  const protocol::AcCmdCRTryBreeding& command)
{
  protocol::RanchCommandTryBreedingOK response{
    .uid = command.mareUid,
    .tid = command.stallionUid,
    .val = 0,
    .count = 0,
    .unk0 = 0,
    .parts = {
      .skinId = 1,
      .maneId = 4,
      .tailId = 4,
      .faceId = 5},
    .appearance = {.scale = 4, .legLength = 4, .legVolume = 5, .bodyLength = 3, .bodyVolume = 4},
    .stats = {.agility = 9, .ambition = 9, .rush = 9, .endurance = 9, .courage = 9},
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

void RanchDirector::HandleBreedingAbandon(
  ClientId clientId,
  const protocol::AcCmdCRBreedingAbandon& command)
{
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
  const auto& clientContext = GetClientContext(clientId);
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
  auto& clientContext = GetClientContext(clientId);
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

void RanchDirector::HandleUpdateMountNickname(
  ClientId clientId,
  const protocol::RanchCommandUpdateMountNickname& command)
{
  const auto& clientContext = GetClientContext(clientId);
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
    const auto itemRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(
      character.inventory());

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
      character.inventory(), horseRenameItemUid);

    // Remove the item from the inventory.
    character.inventory().erase(itemInventoryIter);
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

  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(
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
  const protocol::AcCmdCRRequestStorage& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCRRequestStorageOK response{
    .category = command.category,
    .page = command.page};

  const bool showPurchases = command.category == protocol::AcCmdCRRequestStorage::Category::Purchases;

  // Fill the stored items, either from the purchase category or the gift category.

  characterRecord.Immutable(
    [this, showPurchases, page = static_cast<size_t>(command.page), &response](
      const data::Character& character) mutable
    {
      const auto storedItemRecords = GetServerInstance().GetDataDirector().GetStorageItemCache().Get(
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
  const protocol::AcCmdCRGetItemFromStorage& command)
{
  const auto& clientContext = GetClientContext(clientId);
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
      if (storedPurchaseIter != character.purchases().cend())
      {
        character.purchases().erase(storedPurchaseIter);
        return;
      }

      storedItemIsValid = false;
    });

  // If the stored item is invalid cancel the takeout.
  if (not storedItemIsValid)
  {
    protocol::AcCmdCRGetItemFromStorageCancel response{
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

  protocol::AcCmdCRGetItemFromStorageOK response{
    .storageItemUid = command.storedItemUid};

  // Get the items assigned to the storage item and fill the protocol command.
  characterRecord.Mutable([this, &response](
    data::Character& character)
    {
      const auto storedItemRecord = GetServerInstance().GetDataDirector().GetStorageItemCache().Get(
        response.storageItemUid);

      // Collection of the items received from the storage item.
      std::vector<data::Uid> items;

      storedItemRecord->Immutable([this, &items, &response](const data::StorageItem& storedItem)
      {
        items = storedItem.items();
        const auto itemRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(
          items);

        protocol::BuildProtocolItems(response.items, *itemRecords);
      });

      // Delete the storage item.
      GetServerInstance().GetDataDirector().GetStorageItemCache().Delete(
        response.storageItemUid);

      // Add the items to the character's inventory.
      character.inventory().insert(
        character.inventory().end(),
        items.begin(),
        items.end());

      // TODO: Update carrots as needed
      response.updatedCarrots = character.carrots();
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
    .dressList = {
    Item{
    .uid = 0xFFF,
    .tid = 10164,
    .count = 1}} // TODO: Fetch dress list from somewhere
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
  const protocol::AcCmdCRWearEquipment& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  bool isValidItem = false;
  bool isValidHorse = false;

  characterRecord.Immutable([&isValidItem, &isValidHorse, &command](
    const data::Character& character)
  {
    isValidItem = std::ranges::contains(
      character.inventory(), command.equipmentUid);
    isValidHorse = std::ranges::contains(
      character.horses(), command.equipmentUid);
  });

  if (isValidHorse)
  {
    const data::Uid equippedHorseUid = command.equipmentUid;
    characterRecord.Mutable([&equippedHorseUid](data::Character& character)
    {
      const bool isHorseAlreadyMounted = character.mountUid() == equippedHorseUid;
      if (isHorseAlreadyMounted)
        return;

      // Add the mount back to the horse list.
      character.horses().emplace_back(character.mountUid());
      character.mountUid() = equippedHorseUid;

      // Remove the new mount from the horse list.
      character.horses().erase(
        std::ranges::find(character.horses(), equippedHorseUid));
    });
  }
  else if (isValidItem)
  {
    const data::Uid equippedItemUid = command.equipmentUid;
    auto equippedItemTid = data::InvalidTid;

    const auto equippedItemRecord = _serverInstance.GetDataDirector().GetItem(
      equippedItemUid);
    equippedItemRecord.Immutable([&equippedItemTid](const data::Item& item)
    {
      equippedItemTid = item.tid();
    });

    // Determine whether the newly equipped item is valid and can be equipped.
    const auto equippedItemTemplate = _serverInstance.GetItemRegistry().GetItem(
      equippedItemTid);

    if (not equippedItemTemplate.has_value())
    {
      throw std::runtime_error("Tried equipping item which is not recognized by the server");
    }

    if (not equippedItemTemplate->characterPartInfo.has_value()
      && not equippedItemTemplate->mountPartInfo.has_value())
    {
      throw std::runtime_error("Tried equipping item which is not a valid character or mount equipment");
    }

    characterRecord.Mutable(
      [this, &equippedItemTemplate, &equippedItemUid](
      data::Character& character)
    {
      const bool isCharacterEquipment = equippedItemTemplate->characterPartInfo.has_value();
      const bool isMountEquipment = equippedItemTemplate->mountPartInfo.has_value();

      // Store the current character equipment UIDs
      std::vector<data::Uid> equipmentUids;
      if (isCharacterEquipment)
        equipmentUids = character.characterEquipment();
      else if (isMountEquipment)
        equipmentUids = character.mountEquipment();
      else
        assert(false && "invalid equipment type");

      // Determine which equipment is to be replaced by the newly equipped item.
      std::vector<data::Uid> equipmentToReplace;
      const auto equipmentRecords = _serverInstance.GetDataDirector().GetItemCache().Get(
        equipmentUids);
      for (const auto& equipmentRecord : *equipmentRecords)
      {
        auto equipmentUid{data::InvalidUid};
        auto equipmentTid{data::InvalidTid};
        equipmentRecord.Immutable([&equipmentUid, &equipmentTid](const data::Item& item)
        {
          equipmentUid = item.uid();
          equipmentTid = item.tid();
        });

        // Replace equipment which occupies the same slots as the newly equipped item.
        const auto equipmentTemplate = _serverInstance.GetItemRegistry().GetItem(
          equipmentTid);

        if (isCharacterEquipment)
        {
          if (static_cast<uint32_t>(equipmentTemplate->characterPartInfo->slot)
            & static_cast<uint32_t>(equippedItemTemplate->characterPartInfo->slot))
          {
            equipmentToReplace.emplace_back(equipmentUid);
          }
        }
        else if (isMountEquipment)
        {
          if (static_cast<uint32_t>(equipmentTemplate->mountPartInfo->slot)
            & static_cast<uint32_t>(equippedItemTemplate->mountPartInfo->slot))
          {
            equipmentToReplace.emplace_back(equipmentUid);
          }
        }
      }

      // Remove equipment replaced with the newly equipped item.
      const auto replacedEquipment = std::ranges::remove_if(
        equipmentUids,
        [&equipmentToReplace](const data::Uid uid)
        {
          return std::ranges::contains(equipmentToReplace, uid);
        });

      // Erase them from the equipment.
      equipmentUids.erase(replacedEquipment.begin(), replacedEquipment.end());
      // Add the newly equipped item.
      equipmentUids.emplace_back(equippedItemUid);

      if (isCharacterEquipment)
        character.characterEquipment = equipmentUids;
      else if (isMountEquipment)
        character.mountEquipment = equipmentUids;
      else
        assert(false && "invalid equipment type");

      // Remove the newly equipped item from the inventory.
      const auto equippedItemsToRemove = std::ranges::remove(
        character.inventory(), equippedItemUid);
      character.inventory().erase(equippedItemsToRemove.begin(), equippedItemsToRemove.end());

      // Add the replaced equipment back to the inventory.
      std::ranges::copy(equipmentToReplace, std::back_inserter(character.inventory()));
    });
  }

  // Make sure the equipment UID is either a valid item or a horse.
  const bool equipSuccessful = isValidItem || isValidHorse;
  if (equipSuccessful)
  {
    protocol::AcCmdCRWearEquipmentOK response{
      .itemUid = command.equipmentUid,
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

  protocol::AcCmdCRWearEquipmentCancel response{
    .itemUid = command.equipmentUid,
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
  const protocol::AcCmdCRRemoveEquipment& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  characterRecord.Mutable([&command](data::Character& character)
  {
    const auto characterEquipmentItemIter = std::ranges::find(
      character.characterEquipment(),
      command.itemUid);
    const auto mountEquipmentItemIter = std::ranges::find(
      character.mountEquipment(),
      command.itemUid);

    // You can't really unequip a horse. You can only switch to a different one.
    // At least in Alicia 1.0.

    if (characterEquipmentItemIter != character.characterEquipment().cend())
    {
      const auto range = std::ranges::remove(
        character.characterEquipment(), command.itemUid);
      character.characterEquipment().erase(range.begin(), range.end());
    }
    else if (mountEquipmentItemIter != character.mountEquipment().cend())
    {
      const auto range = std::ranges::remove(
        character.mountEquipment(), command.itemUid);
      character.mountEquipment().erase(range.begin(), range.end());
    }

    character.inventory().emplace_back(command.itemUid);
  });

  // We really don't need to cancel the unequip. Always respond with OK.
  protocol::AcCmdCRRemoveEquipmentOK response{
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
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  bool canCreateGuild = true;
  // todo: configurable
  constexpr int32_t GuildCost = 3000;
  characterRecord.Immutable([&command, &canCreateGuild, GuildCost](const data::Character& character)
  {
    // Check if character has sufficient carrots
    if (character.carrots() < GuildCost)
    {
      canCreateGuild = false;
    }
  });

  // todo: disabled guild name duplicate check (real guild system needs implementing)
  if (false)
  {
    const auto& guildKeys = GetServerInstance().GetDataDirector().GetGuildCache().GetKeys();
    
    // todo: This actually needs to retrieve all guilds from data source, 
    //       so that even offline guilds (guilds that have no members online) are checked.
    //       This is not yet implemented in the data source interface api.
    
    // Loop through each guild and check their names for deduplication
    for (const auto guildKey : guildKeys)
    {
      // Break early if character does not have enough carrots
      // or if new guild has duplicate name
      if (not canCreateGuild)
        break;

      const auto& guildRecord = GetServerInstance().GetDataDirector().GetGuildCache().Get(guildKey);
      guildRecord.value().Immutable([&canCreateGuild, command](const data::Guild& guild)
      {
        canCreateGuild = command.name != guild.name();
      });
    }
  }

  // If guild cannot be created, send cancel to client
  if (not canCreateGuild)
  {
    protocol::RanchCommandCreateGuildCancel response{
      .status = 0,
      .member2 = 0}; // TODO: Unidentified

    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    
    return;
  }

  protocol::RanchCommandCreateGuildOK response{
    .uid = 0};

  const auto guildRecord = GetServerInstance().GetDataDirector().CreateGuild();
  guildRecord.Mutable([&response, &command](data::Guild& guild)
  {
    guild.name = command.name;

    response.uid = guild.uid();
  });

  characterRecord.Mutable([&response, GuildCost](data::Character& character)
  {
    character.carrots() -= GuildCost;
    response.updatedCarrots = character.carrots();
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
  const auto& clientContext = GetClientContext(clientId);
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

void RanchDirector::HandleLeaveGuild(
  ClientId clientId,
  const protocol::AcCmdCRWithdrawGuildMember& command)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  const bool isUserValid = clientContext.characterUid == command.characterUid;
  if (not isUserValid)
  {
    protocol::AcCmdCRWithdrawGuildMemberCancel response{
      .status = 0
    };
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]()
      {
        return response;
      });
    
    return;
  }

  characterRecord.Mutable([&command](data::Character& character)
  {
    character.guildUid() = data::InvalidUid;
    // TODO: check if player is the last player in the guild
    // otherwise guild stays soft locked forever if not deleted
  });

  protocol::AcCmdCRWithdrawGuildMemberOK response{
    .unk0 = 0
  };
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
  protocol::AcCmdRCUpdatePet response{
    .petInfo = command.petInfo
  };

  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  auto petUid = data::InvalidUid;

  characterRecord.Mutable(
    [this, &command, &petUid, &response](data::Character& character)
    {
      
      response.petInfo.characterUid = character.uid();
      // The pets of the character.
      const auto storedPetRecords = GetServerInstance().GetDataDirector().GetPetCache().Get(
        character.pets());

      if (not storedPetRecords || storedPetRecords->empty())
      {
        // No pets found for the character.
        spdlog::warn("No pets found for character {}", character.uid());
        return;
      }
      bool petExists = false;
      // Find the pet record based on the item used.
      for (const auto& petRecord : *storedPetRecords)
      {
        petRecord.Immutable(
          [&command, &petUid, &petExists](const data::Pet& pet)
          {
            if (pet.itemUid() == command.petInfo.itemUid)
            {
              petUid = pet.uid();
              petExists = true;
            }
          });
      }
      
      if (!petExists)
      {
        spdlog::warn("Character {} has no pet with petId {}", character.uid(), command.petInfo.pet.petId);
        //probably should send a cancel here
        return;
      }

      auto itemRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(
        character.inventory());
      if (not itemRecords || itemRecords->empty())
      {
        spdlog::warn("No items found for character {}", character.uid());
        return;
      }
      // Pet rename, find item in inventory
      if (std::ranges::contains(character.inventory(), command.itemUid))
      {
        // TODO: actually reduce the item count or remove it
        const auto petRecord = GetServerInstance().GetDataDirector().GetPet(petUid);
        petRecord.Mutable(
          [&command](data::Pet& pet)
          {
            pet.name() = command.petInfo.pet.name;
          });
      }
      //just summoning the pet
      else
      {
        character.petUid = petUid;
      }
      if (petUid != 0)
      {
        const auto petRecord = GetServerInstance().GetDataDirector().GetPet(petUid);
        petRecord.Immutable(
          [&response](const data::Pet& pet)
          {
            response.petInfo.pet.name = pet.name();
            response.petInfo.pet.birthDate = util::TimePointToAliciaTime(pet.birthDate());
          });
      }
    });

  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];

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
  const auto& clientContext = GetClientContext(clientId);
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
      auto storedPetRecords = GetServerInstance().GetDataDirector().GetPetCache().Get(
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
void RanchDirector::HandleIncubateEgg(
  ClientId clientId,
  const protocol::AcCmdCRIncubateEgg& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCRIncubateEggOK response{
    response.incubatorSlot = command.incubatorSlot,
  };

  characterRecord.Mutable(
    [this, &command, &response, clientId](data::Character& character)
    {
      const std::optional<registry::EggInfo> eggTemplate = _serverInstance.GetPetRegistry().GetEggInfo(
        command.itemTid);
      if (not eggTemplate)
      {
        //not tested
        protocol::AcCmdCRIncubateEggCancel cancel{
          cancel.cancel = 0,
          cancel.itemUid = command.itemUid,
          cancel.itemTid = command.itemUid,
          cancel.incubatorSlot = command.incubatorSlot};

        _commandServer.QueueCommand<decltype(cancel)>(
          clientId,
          [cancel]()
          {
            return cancel;
          });
        spdlog::warn("User tried to incubate something that is not an egg");
        return;
      }

      const auto eggRecord = GetServerInstance().GetDataDirector().CreateEgg();
      eggRecord.Mutable([&command, &response, &character, &eggTemplate](data::Egg& egg)
        {
          
          egg.incubatorSlot = command.incubatorSlot;
          egg.incubatedAt = data::Clock::now();
          egg.boostsUsed = 0;
          egg.itemTid = command.itemTid;
          egg.itemUid = command.itemUid;

          character.eggs().emplace_back(egg.uid());

          // Fill the response with egg information.
          auto eggUid = egg.uid();
          auto eggItemTid = egg.itemTid();
          auto eggHatchDuration = eggTemplate.value().hatchDuration;

          response.egg.uid = eggUid;
          response.egg.itemTid = eggItemTid;
          response.egg.timeRemaining = std::chrono::duration_cast<std::chrono::seconds>(eggHatchDuration).count();
          response.egg.boost = 400000;
          response.egg.totalHatchingTime = std::chrono::duration_cast<std::chrono::seconds>(eggHatchDuration).count();
        });
    });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

   protocol::AcCmdCRIncubateEggNotify notify{
    .characterUid = clientContext.characterUid,
    .incubatorSlot = command.incubatorSlot,
    .egg = response.egg,
  };

  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];
  // Broadcast the egg incubation to all ranch clients.
  for (ClientId ranchClient : ranchInstance.clients)
  {
    // Prevent broadcasting to self.
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

void RanchDirector::HandleBoostIncubateEgg(
  ClientId clientId,
  const protocol::AcCmdCRBoostIncubateEgg& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCRBoostIncubateEggOK response{
    .incubatorSlot = command.incubatorSlot
  };

  characterRecord.Mutable(
    [this, &command, &response](data::Character& character)
    {
      // find the Item record for Crystal
      const auto itemRecord = GetServerInstance().GetDataDirector().GetItemCache().Get(
        command.itemUid);
      if (not itemRecord)
        throw std::runtime_error("Item not found");
      
      itemRecord->Immutable([&command, &response](const data::Item& item)
      {
        response.item = {
          .uid = item.uid(),
          .tid = item.tid(),
          .count = item.count()};
      });

      // Find the Egg record through the incubater slot.
      const auto eggRecord = GetServerInstance().GetDataDirector().GetEggCache().Get(
        character.eggs());
      if (not eggRecord)
        throw std::runtime_error("Egg not found");

      for (const auto& egg : *eggRecord)
      {

        egg.Mutable([this, &command, &response](data::Egg& eggData)
          {
            if (eggData.incubatorSlot() == command.incubatorSlot)
            {
              // retrieve egg template for the hatchDuration
              const registry::EggInfo eggTemplate = _serverInstance.GetPetRegistry().GetEggInfo(
                eggData.itemTid());

              eggData.boostsUsed() += 1;
              response.egg = {
                .uid = eggData.uid(),
                .itemTid = eggData.itemTid(),
                .timeRemaining = static_cast<uint32_t>(
                  std::chrono::duration_cast<std::chrono::seconds>(
                                    eggTemplate.hatchDuration -
                                    (std::chrono::system_clock::now() - eggData.incubatedAt()) -
                                    (eggData.boostsUsed() * std::chrono::hours(8))).count()),
                .boost = 400000,
                .totalHatchingTime = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                    eggTemplate.hatchDuration).count())};
            };
          });
      };
    });
    _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
};

void RanchDirector::HandleBoostIncubateInfoList(
  ClientId clientId,
  const protocol::AcCmdCRBoostIncubateInfoList& command)
{
  protocol::AcCmdCRBoostIncubateInfoListOK response{
    .member1 = 0,
    .count = 0
  // for loop with a vector
  };
  
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleRequestPetBirth(
  ClientId clientId,
  const protocol::AcCmdCRRequestPetBirth& command)
{
  // TODO: implement pity based on egg level provided by the client

  const auto& clientContext = GetClientContext(clientId);

  protocol::AcCmdCRRequestPetBirthOK response{
    .petBirthInfo = {
      .petInfo = {
        .characterUid = clientContext.characterUid,}
    },
  };

  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
  characterRecord.Mutable(
    [this, &command, &response](data::Character& character)
    {
      auto hatchingEggUid{data::InvalidUid};
      auto hatchingEggItemUid{data::InvalidUid};
      auto hatchingEggTid{data::InvalidTid};

      const auto eggRecord = GetServerInstance().GetDataDirector().GetEggCache().Get(
        character.eggs());
      if (not eggRecord)
        throw std::runtime_error("Egg records not available");

      // Find the egg that has hatched.
      for (const auto& egg : *eggRecord)
      {
        egg.Immutable(
          [&command, &response, &hatchingEggTid, &hatchingEggItemUid, &hatchingEggUid](
            const data::Egg& eggData)
          {
            if (eggData.incubatorSlot() == command.incubatorSlot)
            {
              hatchingEggUid = eggData.uid();
              hatchingEggTid = eggData.itemTid();
              hatchingEggItemUid = eggData.itemUid();

              response.petBirthInfo.petInfo.itemUid = hatchingEggUid;
            };
          });
      }

      // TODO: reduce the incubator durability (if it is a double incubator)

      // Remove the hatched egg from the incubator and from the character's inventory.
      if (auto it = std::ranges::find(character.eggs(), hatchingEggUid);
        it != character.eggs().end())
      {
        character.eggs().erase(it);
      }

      if (auto it = std::ranges::find(character.inventory(), hatchingEggItemUid);
        it != character.inventory().end())
      {
        character.inventory().erase(it);
      }

      //Delete the Item and Egg records
      GetServerInstance().GetDataDirector().GetEggCache().Delete(hatchingEggUid);
      GetServerInstance().GetDataDirector().GetItemCache().Delete(hatchingEggItemUid);

      const registry::EggInfo eggTemplate = _serverInstance.GetPetRegistry().GetEggInfo(
        hatchingEggTid);

      const auto& hatchablePets = eggTemplate.hatchablePets;
      std::uniform_int_distribution<size_t> dist(0, hatchablePets.size() - 1);
      const data::Tid petItemTid = hatchablePets[dist(_randomDevice)];

      const registry::PetInfo petTemplate = _serverInstance.GetPetRegistry().GetPetInfo(
        petItemTid);
      const auto petId = petTemplate.petId;

      bool petAlreadyExists = false;

      const auto petRecords = GetServerInstance().GetDataDirector().GetPetCache().Get(
        character.pets());

      // Figure out whether the character already has this pet
      for (const auto& petRecord : *petRecords)
      {
        petRecord.Immutable([&petAlreadyExists, petId](const data::Pet& pet)
        {
          petAlreadyExists = (pet.petId() == petId);
        });

        if (petAlreadyExists == true)
          break;
      }

      if (petAlreadyExists)
      {
        // todo: stacking
        const auto pityItem = GetServerInstance().GetDataDirector().CreateItem();
        pityItem.Mutable([&character, &response](data::Item& item)
        {
          item.tid() = 46019;
          item.count() = 1;
          // write Pity item into response
          response.petBirthInfo.eggItem = {
            .uid = item.uid(),
            .tid = item.tid(),
            .count = item.count()};
          // write the item into the character items
          character.inventory().emplace_back(item.uid());
        });
        return;
      }

      auto petUid = data::InvalidUid;
      auto petItemUid = data::InvalidUid;

      // Create the pet and the associated item.
      const auto petItem = GetServerInstance().GetDataDirector().CreateItem();
      const auto bornPet = GetServerInstance().GetDataDirector().CreatePet();

      petItem.Mutable([&response, &petItemUid, petId, petItemTid](data::Item& item)
      {
        item.tid() = petItemTid;
        item.count() = 1;
        // Fill the response with the born item information.
        response.petBirthInfo.eggItem = {
          .uid = item.uid(),
          .tid = item.tid(),
          .count = item.count()};
        petItemUid = item.uid();
      });

      bornPet.Mutable([&response, &character, &petUid, &petItemUid, petId](data::Pet& pet)
      {
        pet.itemUid() = petItemUid;
        pet.name() = "";
        pet.petId() = petId;
        pet.birthDate() = data::Clock::now();
    
        // Fill the response with the born pet.
        response.petBirthInfo.petInfo.pet = {
          .petId = pet.petId(),
          .name = pet.name(),
          .birthDate = util::TimePointToAliciaTime(pet.birthDate())};
        petUid = pet.uid();
      });

      character.inventory().emplace_back(petItemUid);
      character.pets().emplace_back(petUid);
    });

  protocol::AcCmdCRRequestPetBirthNotify notify{
    .petBirthInfo = response.petBirthInfo
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
  
  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];
  // Broadcast the egg hatching to all ranch clients.
  for (ClientId ranchClient : ranchInstance.clients)
  {
    // Prevent broadcasting to self.
    if (ranchClient == clientId)
      continue;

    _commandServer.QueueCommand<decltype(notify)>(
      ranchClient,
      [notify]()
      {
        return notify;
      });
  }
};

void RanchDirector::BroadcastEquipmentUpdate(ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::AcCmdCRUpdateEquipmentNotify notify{
    .characterUid = clientContext.characterUid};

  characterRecord.Immutable([this, &notify](const data::Character& character)
  {
    // Character equipment
    const auto characterEquipment = GetServerInstance().GetDataDirector().GetItemCache().Get(
      character.characterEquipment());
    protocol::BuildProtocolItems(notify.characterEquipment, *characterEquipment);

    // Mount equipment
    const auto mountEquipment = GetServerInstance().GetDataDirector().GetItemCache().Get(
      character.mountEquipment());
    protocol::BuildProtocolItems(notify.mountEquipment, *mountEquipment);

    // Mount record
    const auto mountRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(
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

bool RanchDirector::HandleUseFoodItem(
  const data::Uid characterUid,
  const data::Uid mountUid,
  const data::Tid usedItemTid,
  protocol::AcCmdCRUseItemOK& response)
{
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    characterUid);
  const auto mountRecord = _serverInstance.GetDataDirector().GetHorse(
    mountUid);
  const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(
    usedItemTid);
  assert(itemTemplate && itemTemplate->foodParameters);

  // Update plenitude and friendliness points according to the item used.
  mountRecord.Mutable([&itemTemplate](data::Horse& horse)
  {
    // todo: there's a ranch skill which gives bonus to these points
    horse.mountCondition.plenitude() += itemTemplate->foodParameters->plenitudePoints;
    horse.mountCondition.friendliness() += itemTemplate->foodParameters->friendlinessPoints;
  });

  response.type = protocol::AcCmdCRUseItemOK::ActionType::Feed;
  response.experiencePoints = 0xFF;
  response.playSuccessLevel = protocol::AcCmdCRUseItemOK::PlaySuccessLevel::Bad;

  // todo: award experiences gained
  // todo: client-side update of plenitude and friendliness stats

  return true;
}

bool RanchDirector::HandleUseCleanItem(
  const data::Uid characterUid,
  const data::Uid mountUid,
  const data::Tid usedItemTid,
  protocol::AcCmdCRUseItemOK& response)
{
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    characterUid);
  const auto mountRecord = _serverInstance.GetDataDirector().GetHorse(
    mountUid);
  const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(
    usedItemTid);
  assert(itemTemplate && itemTemplate->careParameters);

  // Update clean and polish points according to the item used.
  mountRecord.Mutable([&itemTemplate](data::Horse& horse)
  {
    // todo: there's a ranch skill which gives bonus to these points

    switch (itemTemplate->careParameters->parts)
    {
      case registry::Item::CareParameters::Part::Body:
      {
        horse.mountCondition.bodyPolish() += itemTemplate->careParameters->polishPoints;
        break;
      }
      case registry::Item::CareParameters::Part::Mane:
      {
        horse.mountCondition.manePolish() += itemTemplate->careParameters->polishPoints;
        break;
      }
      case registry::Item::CareParameters::Part::Tail:
      {
        horse.mountCondition.tailPolish() += itemTemplate->careParameters->polishPoints;
        break;
      }
    }
  });

  response.type = protocol::AcCmdCRUseItemOK::ActionType::Wash;
  response.playSuccessLevel = protocol::AcCmdCRUseItemOK::PlaySuccessLevel::CriticalGood;

  // todo: award experiences gained
  // todo: client-side update of clean and polish stats

  return true;
}

bool RanchDirector::HandleUsePlayItem(
  const data::Uid characterUid,
  const data::Uid mountUid,
  const data::Tid usedItemTid,
  const protocol::AcCmdCRUseItem::PlaySuccessLevel successLevel,
  protocol::AcCmdCRUseItemOK& response)
{
  const auto characterRecord = _serverInstance.GetDataDirector().GetCharacter(
    characterUid);
  const auto mountRecord = _serverInstance.GetDataDirector().GetHorse(
    mountUid);
  const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(
    usedItemTid);
  assert(itemTemplate && itemTemplate->playParameters);

  // TODO: Make critical chance configurable. Currently 0->1 is 50% chance.
  std::uniform_int_distribution<uint32_t> critRandomDist(0, 1);
  auto crit = critRandomDist(_randomDevice);

  response.type = protocol::AcCmdCRUseItemOK::ActionType::Play;
  switch (successLevel)
  {
    case protocol::AcCmdCRUseItem::PlaySuccessLevel::Bad:
      response.playSuccessLevel = protocol::AcCmdCRUseItemOK::PlaySuccessLevel::Bad;
      break;
    case protocol::AcCmdCRUseItem::PlaySuccessLevel::Good:
      response.playSuccessLevel = crit ?
        protocol::AcCmdCRUseItemOK::PlaySuccessLevel::CriticalGood :
        protocol::AcCmdCRUseItemOK::PlaySuccessLevel::Good;
      break;
    case protocol::AcCmdCRUseItem::PlaySuccessLevel::Perfect:
      response.playSuccessLevel = crit ?
        protocol::AcCmdCRUseItemOK::PlaySuccessLevel::CriticalPerfect :
        protocol::AcCmdCRUseItemOK::PlaySuccessLevel::Perfect;
      break;
  }

  // TODO: Update the horse's stats based on the play item used.
  return true;
}

bool RanchDirector::HandleUseCureItem(
  const data::Uid characterUid,
  const data::Uid mountUid,
  const data::Tid usedItemTid,
  protocol::AcCmdCRUseItemOK& response)
{
  // No info

  response.type = protocol::AcCmdCRUseItemOK::ActionType::Cure;
  response.experiencePoints = 0;

  // TODO: Update the horse's stats based on the cure item used.
  return true;
}

void RanchDirector::HandleUseItem(
  ClientId clientId,
  const protocol::AcCmdCRUseItem& command)
{
  protocol::AcCmdCRUseItemOK response{
    response.itemUid = command.itemUid,
    response.updatedItemCount = command.always1,
    response.type = protocol::AcCmdCRUseItemOK::ActionType::Generic};

  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  const auto usedItemUid = command.itemUid;
  const auto horseUid = command.horseUid;

  bool hasItem = false;
  bool hasHorse = false;
  std::string characterName;
  characterRecord.Immutable([&characterName, &usedItemUid, &horseUid, &hasItem, &hasHorse](
    const data::Character& character)
  {
    hasItem = std::ranges::contains(character.inventory(), usedItemUid);;
    hasHorse = std::ranges::contains(character.horses(), horseUid)
      || character.mountUid() == horseUid;

    characterName = character.name();
  });

  if (not hasItem || not hasHorse)
    throw std::runtime_error("Item or horse not owned by the character");

  const auto mountRecord = GetServerInstance().GetDataDirector().GetHorse(
    command.horseUid);
  const auto itemRecord = GetServerInstance().GetDataDirector().GetItem(
    command.itemUid);

  auto usedItemTid = data::InvalidTid;
  itemRecord.Immutable([&usedItemTid](const data::Item& item)
  {
    usedItemTid = item.tid();
  });

  const auto itemTemplate = _serverInstance.GetItemRegistry().GetItem(
    usedItemTid);
  if (not itemTemplate)
    throw std::runtime_error("Item tempate not available");

  bool consumeItem = false;
  if (itemTemplate->foodParameters)
  {
    consumeItem = HandleUseFoodItem(
      clientContext.characterUid,
      horseUid,
      usedItemTid,
      response);
  }
  else if (itemTemplate->careParameters)
  {
    HandleUseCleanItem(
      clientContext.characterUid,
      horseUid,
      usedItemTid,
      response);
  }
  else if (itemTemplate->playParameters)
  {
    HandleUsePlayItem(
      clientContext.characterUid,
      horseUid,
      usedItemTid,
      command.playSuccessLevel,
      response);
  }
  else if (itemTemplate->cureParameters)
  {
    HandleUseCureItem(
      clientContext.characterUid,
      horseUid,
      usedItemTid,
      response);
  }
  else
  {
    throw std::runtime_error(
      std::format("Unknown use of item tid {} for item uid {}", usedItemTid, command.itemUid));
    return;
  }

  if (consumeItem)
  {
    bool isUsedItemEmpty = false;
    itemRecord.Mutable([&isUsedItemEmpty, &response](data::Item& item)
    {
      item.count() -= 1;
      response.updatedItemCount = item.count();

      isUsedItemEmpty = item.count() <= 0;
    });

    if (isUsedItemEmpty)
    {
      characterRecord.Mutable([usedItemUid = command.itemUid](data::Character& character)
      {
        const auto removedItems = std::ranges::remove(character.inventory(), usedItemUid);
        character.inventory().erase(removedItems.begin(), removedItems.end());
      });

      _serverInstance.GetDataDirector().GetItemCache().Delete(command.itemUid);
    }
  }

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // Perform a mount update

  protocol::AcCmdRCUpdateMountInfoNotify notify{
    protocol::AcCmdRCUpdateMountInfoNotify::Action::UpdateConditionAndName,
    };

  const auto horseRecord = _serverInstance.GetDataDirector().GetHorse(
    horseUid);

  horseRecord.Immutable([&notify](const data::Horse& horse)
  {
    protocol::BuildProtocolHorse(notify.horse, horse);
  });

  const auto& ranchInstance = _ranches[clientContext.visitingRancherUid];
  for (auto client : ranchInstance.clients)
  {
    _commandServer.QueueCommand<decltype(notify)>(
      client,
      [notify](){return notify;});
  }
}

void RanchDirector::HandleHousingBuild(
  ClientId clientId,
  const protocol::AcCmdCRHousingBuild& command)
{
  //! The double incubator does not utilize the HousingRepair,
  //! instead it just creates a new double incubator
  //! TODO: make the check if the incubator already exists and set the durability back to 10

  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // todo: catalogue housing uids and handle transaction

  protocol::AcCmdCRHousingBuildOK response{
    .member1 = clientContext.characterUid,
    .housingTid = command.housingTid,
    .member3 = 10,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  auto housingUid = data::InvalidUid;

  // TODO: add a duplication check for double incubator, since rebuilding triggers HousingBuild and not HousingRepair
  const auto housingRecord = GetServerInstance().GetDataDirector().CreateHousing();
  housingRecord.Mutable([housingId = command.housingTid, &housingUid](data::Housing& housing)
  {
    housing.housingId = housingId;
    housingUid = housing.uid();

    if (housingId == DoubleIncubatorId)
      housing.durability = 10;
    else
      housing.expiresAt = std::chrono::system_clock::now() + std::chrono::days(20);
  });

  characterRecord.Mutable([&housingUid](data::Character& character)
  {
    character.housing().emplace_back(housingUid);
  });

  assert(clientContext.visitingRancherUid == clientContext.characterUid);

  protocol::AcCmdCRHousingBuildNotify notify{
    .member1 = 1,
    .housingId = command.housingTid,
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
  const protocol::AcCmdCRHousingRepair& command)
{
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
  
  uint16_t housingId;
  const auto housingRecord = GetServerInstance().GetDataDirector().GetHousingCache(
    command.housingUid);

  housingRecord.Mutable([&housingId](data::Housing& housing){
    housing.expiresAt = std::chrono::system_clock::now() + std::chrono::days(20);
    housingId = housing.housingId();
  });

  // todo: implement transaction for the repair

  protocol::AcCmdCRHousingRepairOK response{
    .housingUid = command.housingUid,
    .member2 = 1,
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  assert(clientContext.visitingRancherUid == clientContext.characterUid);

  protocol::AcCmdCRHousingBuildNotify notify{
    .member1 = 1,
    .housingId = housingId,
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
};

void RanchDirector::HandleOpCmd(
  ClientId clientId,
  const protocol::RanchCommandOpCmd& command)
{
  const auto& clientContext = GetClientContext(clientId);

  std::vector<std::string> feedback;

  const auto result = GetServerInstance().GetChatSystem().ProcessChatMessage(
    clientContext.characterUid, "//" + command.command);

  if (not result.commandVerdict)
  {
    return;
  }

  for (const auto response : result.commandVerdict->result)
  {
    _commandServer.QueueCommand<protocol::RanchCommandOpCmdOK>(
      clientId,
      [response = std::move(response)]()
      {
        return protocol::RanchCommandOpCmdOK{
          .feedback = response};
      });
  }
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

void RanchDirector::HandleRecoverMount(
  ClientId clientId,
  const protocol::AcCmdCRRecoverMount command)
{
  protocol::AcCmdCRRecoverMountOK response{
    .horseUid = command.horseUid};

  bool horseValid = false;
  const auto& characterUid = GetClientContext(clientId).characterUid;
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(characterUid);
  
  characterRecord.Mutable([this, &response, &horseValid](data::Character& character)
  {
    const bool ownsHorse = character.mountUid() == response.horseUid ||
      std::ranges::contains(character.horses(), response.horseUid);

    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorse(
      response.horseUid);

    // Check if the character owns the horse or exists in the data director
    if (not ownsHorse || character.carrots() <= 0 || not horseRecord.IsAvailable())
    {
      spdlog::warn("Character {} unsuccessfully tried to recover horse {} stamina with {} carrots",
        character.name(), response.horseUid, character.carrots());
      return;
    }

    horseValid = true;
    horseRecord.Mutable([&character, &response](data::Horse& horse)
    {
      // Seems to always be 4000.
      constexpr uint16_t MaxHorseStamina = 4'000;
      // Each stamina point costs one carrot.
      constexpr double StaminaPointPrice = 1.0;
      
      // The stamina points the horse needs to recover to reach maximum stamina.
      const int32_t recoverableStamina = MaxHorseStamina - horse.mountCondition.stamina();
      
      // Recover as much required stamina as the user can afford with
      // the threshold being the max recoverable stamina.
      const int32_t staminaToRecover = std::min(
        recoverableStamina,
        static_cast<int32_t>(std::floor(character.carrots() / StaminaPointPrice)));
      
      horse.mountCondition.stamina() += staminaToRecover;
      character.carrots() -= static_cast<int32_t>(
        std::floor(staminaToRecover * StaminaPointPrice));
  
      response.stamina = horse.mountCondition.stamina();
      response.updatedCarrots = character.carrots();
    });
  });

  if (not horseValid)
  {
    const protocol::AcCmdCRRecoverMountCancel cancelResponse{
      .horseUid = command.horseUid};

    _commandServer.QueueCommand<decltype(cancelResponse)>(
      clientId,
      [cancelResponse]()
      {
        return cancelResponse;
      });
    
    return;
  }
  
  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleMountFamilyTree(
  ClientId clientId,
  const protocol::RanchCommandMountFamilyTree& command)
{
  // todo: implement horse family tree

  protocol::RanchCommandMountFamilyTreeOK response{
    .ancestors = {
      protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem {
        .id = 1,
        .name = "1",
        .grade = 1,
        .skinId = 1
      },
      protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem {
        .id = 2,
        .name = "2",
        .grade = 4,
        .skinId = 1
      },
      protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem {
        .id = 3,
        .name = "3",
        .grade = 1,
        .skinId = 1
      },
      protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem {
        .id = 4,
        .name = "4",
        .grade = 1,
        .skinId = 1
      },
      protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem {
        .id = 5,
        .name = "5",
        .grade = 1,
        .skinId = 1
      },
      protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem {
        .id = 6,
        .name = "6",
        .grade = 1,
        .skinId = 1
      }}
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleCheckStorageItem(
  ClientId clientId,
  const protocol::AcCmdCRCheckStorageItem command)
{
  // No need to respond, only indicate to the server that
  // a stored item has been viewed
  const auto& characterUid = GetClientContext(clientId).characterUid;
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(characterUid);

  bool characterHasStoredItem = false;
  characterRecord.Immutable([&characterHasStoredItem, command](const data::Character& character)
  {
    characterHasStoredItem = 
      std::ranges::contains(character.purchases(), command.storedItemUid) ||
      std::ranges::contains(character.gifts(), command.storedItemUid);
  });

  if (not characterHasStoredItem)
  {
    spdlog::warn("Character {} tried to check a stored item {} they do not have",
      characterUid, command.storedItemUid);
    return;
  }

  const auto& storedItemRecord = GetServerInstance().GetDataDirector().GetStorageItemCache(command.storedItemUid);
  storedItemRecord.Mutable([](data::StorageItem& storedItem)
  {
    storedItem.checked() = true;
  });
}

void RanchDirector::HandleChangeAge(
  ClientId clientId,
  const protocol::AcCmdCRChangeAge command)
{
  const auto& clientContext = GetClientContext(clientId);
  GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid).Mutable([age = command.age](data::Character& character)
  {
    character.age() = static_cast<uint8_t>(age);
  });

  protocol::AcCmdCRChangeAgeOK response {
    .age = command.age
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  BroadcastChangeAgeNotify(
    clientContext.characterUid,
    clientContext.visitingRancherUid,
    command.age
  );
}

void RanchDirector::HandleHideAge(
  ClientId clientId,
  const protocol::AcCmdCRHideAge command)
{
  const auto& clientContext = GetClientContext(clientId);
  GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid).Mutable([option = command.option](data::Character& character)
  {
    character.hideGenderAndAge() = option == protocol::AcCmdCRHideAge::Option::Hidden;
  });

  protocol::AcCmdCRHideAgeOK response {
    .option = command.option
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  BroadcastHideAgeNotify(
    clientContext.characterUid,
    clientContext.visitingRancherUid,
    command.option
  );
}

void RanchDirector::HandleStatusPointApply(
  ClientId clientId,
  const protocol::AcCmdCRStatusPointApply command)
{
  protocol::AcCmdCRStatusPointApplyOK response {};

  const auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(command.horseUid);
  horseRecord->Mutable([&command](data::Horse& horse)
  {
    horse.stats.agility = command.stats.agility;
    horse.stats.ambition = command.stats.ambition;
    horse.stats.rush = command.stats.rush;
    horse.stats.endurance = command.stats.endurance;
    horse.stats.courage = command.stats.courage;

    horse.growthPoints() -= 1;
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

} // namespace server