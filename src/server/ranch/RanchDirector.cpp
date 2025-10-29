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
#include <random>
#include <unordered_map>
#include <fstream>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

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

// Global stallion tracking
static std::vector<data::Uid> g_stallions;
static std::unordered_map<data::Uid, data::Uid> g_horseToStallionMap; // Maps horseUid -> stallionUid

// Cached stallion data to avoid async loading issues
struct CachedStallionData
{
  data::Uid ownerUid;
  uint32_t breedingCharge;
  util::Clock::time_point expiresAt;
};
static std::unordered_map<data::Uid, CachedStallionData> g_stallionDataCache; // Maps stallionUid -> cached data

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

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRCheckStallionCharge>(
    [this](ClientId clientId, auto& command)
    {
      HandleCheckStallionCharge(clientId, command);
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

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRBreedingFailureCard>(
    [this](ClientId clientId, auto& command)
    {
      HandleBreedingFailureCard(clientId, command);
    });

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRBreedingFailureCardChoose>(
    [this](ClientId clientId, auto& command)
    {
      HandleBreedingFailureCardChoose(clientId, command);
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

  _commandServer.RegisterCommandHandler<protocol::AcCmdCRChangeSkillCardPreset>(
    [this](ClientId clientId, const auto& command)
    {
      HandleChangeSkillCardPreset(clientId, command);
    });
}

void RanchDirector::Initialize()
{
  spdlog::debug(
    "Ranch server listening on {}:{}",
    GetConfig().listen.address.to_string(),
    GetConfig().listen.port);

  _commandServer.BeginHost(GetConfig().listen.address, GetConfig().listen.port);
  
  // Load registered stallions from database
  LoadRegisteredStallions();
}

void RanchDirector::LoadRegisteredStallions()
{
  // Scan the stallions directory and load all registered stallions
  // The stallions are stored in the data/stallions directory relative to server root
  const std::filesystem::path stallionsPath = "data/stallions";
  
  if (!std::filesystem::exists(stallionsPath))
  {
    spdlog::info("Stallions directory does not exist, skipping stallion loading");
    return;
  }

  int loadedCount = 0;
  int expiredCount = 0;
  std::vector<data::Uid> horseUidsToPreload;
  std::vector<data::Uid> stallionUidsToLoad;
  
  for (const auto& entry : std::filesystem::directory_iterator(stallionsPath))
  {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
      continue;
      
    try
    {
      // Extract stallion UID from filename (e.g., "123.json" -> 123)
      data::Uid stallionUid = std::stoul(entry.path().stem().string());
      
      // Read the JSON file directly to get stallion info
      std::ifstream file(entry.path());
      if (!file.is_open())
      {
        spdlog::warn("Failed to open stallion file {}", entry.path().string());
        continue;
      }
      
      nlohmann::json json = nlohmann::json::parse(file);
      
      spdlog::debug("Loading stallion {} from {}", stallionUid, entry.path().string());
      
      if (!json.contains("horseUid") || !json.contains("expiresAt") || 
          !json.contains("ownerUid") || !json.contains("breedingCharge"))
      {
        spdlog::warn("Stallion file {} is missing required fields, skipping", entry.path().string());
        continue;
      }
      
      data::Uid horseUid = json["horseUid"];
      uint64_t expiresAtSeconds = json["expiresAt"];
      util::Clock::time_point expiresAt = util::Clock::time_point(std::chrono::seconds(expiresAtSeconds));
      
      // Check if stallion has expired
      if (util::Clock::now() >= expiresAt)
      {
        spdlog::info("Stallion {} (horse {}) has expired, removing from database", 
          stallionUid, horseUid);
        
        // Queue stallion UID for retrieval to delete it
        stallionUidsToLoad.push_back(stallionUid);
        
        // Reset horse type back to Adult
        auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(horseUid);
        if (horseRecord)
        {
          horseRecord->Mutable([](data::Horse& horse)
          {
            horse.horseType() = 0; // Adult
          });
        }
        
        // Delete expired stallion file
        std::filesystem::remove(entry.path());
        expiredCount++;
        continue;
      }
      
      // Cache the stallion data to avoid async loading issues
      data::Uid ownerUid = json["ownerUid"];
      uint32_t breedingCharge = json["breedingCharge"];
      
      CachedStallionData cachedData{
        .ownerUid = ownerUid,
        .breedingCharge = breedingCharge,
        .expiresAt = expiresAt
      };
      g_stallionDataCache[stallionUid] = cachedData;
      
      // Add to in-memory lists for active stallions
      g_stallions.emplace_back(horseUid);
      g_horseToStallionMap[horseUid] = stallionUid;
      horseUidsToPreload.push_back(horseUid);
      
      loadedCount++;
    }
    catch (const std::exception& e)
    {
      spdlog::warn("Failed to load stallion from {}: {}", 
        entry.path().string(), e.what());
    }
  }
  
  // Pre-load all stallions
  if (!horseUidsToPreload.empty())
  {
    GetServerInstance().GetDataDirector().GetHorseCache().Get(horseUidsToPreload);
  }
  
  spdlog::info("Loaded {} registered stallion(s) from database ({} expired and removed)", 
    loadedCount, expiredCount);
  
  // Debug: Log the first few stallions
  if (!g_stallions.empty())
  {
    spdlog::debug("g_stallions count: {}, first entry: {}", g_stallions.size(), g_stallions[0]);
  }
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

void RanchDirector::SendInventoryUpdate(ClientId clientId)
{
  const auto& clientContext = GetClientContext(clientId);
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  if (not characterRecord)
    return;

  protocol::AcCmdCRGetItemFromStorageOK response{
    .storageItemUid = 0,
    .items = {},
    .updatedCarrots = 0};

  characterRecord.Immutable(
    [this, &response](const data::Character& character)
    {
      const auto itemRecords = GetServerInstance().GetDataDirector().GetItemCache().Get(
        character.inventory());
      protocol::BuildProtocolItems(response.items, *itemRecords);
      response.updatedCarrots = character.carrots();
    });

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
    throw std::runtime_error("Ranch client is not available");

  auto& clientContext = clientIter->second;
  if (requireAuthentication && not clientContext.isAuthenticated)
    throw std::runtime_error("Ranch client is not authenticated");

  return clientContext;
}

ClientId RanchDirector::GetClientIdByCharacterUid(data::Uid characterUid)
{
  for (auto& [clientId, clientContext] : _clients)
  {
    if (clientContext.characterUid == characterUid
      && clientContext.isAuthenticated)
      return clientId;
  }

  throw std::runtime_error("Character not associated with any client");
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
      .type = protocol::League::Type::Platinum,
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
  protocol::RanchCharacter characterEnteringRanch;

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
        ? protocol::RanchCharacter::Role::GameMaster
        : character.role() == data::Character::Role::Op
          ? protocol::RanchCharacter::Role::Op
          : protocol::RanchCharacter::Role::User;
      protocolCharacter.age = character.hideGenderAndAge() ? 0 : character.age();
      // todo: use model constant
      protocolCharacter.gender = character.parts.modelId() == 10
          ? protocol::RanchCharacter::Gender::Boy
          : protocol::RanchCharacter::Gender::Girl;

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
  auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  protocol::RanchCommandEnterBreedingMarketOK response;

  characterRecord.Immutable(
    [this, &response, &clientContext](const data::Character& character)
    {
      // Include all horses in the response
      const auto horseRecords = GetServerInstance().GetDataDirector().GetHorseCache().Get(
        character.horses());

      for (const auto& horseRecord : *horseRecords)
      {
        auto& protocolHorse = response.stallions.emplace_back();

        // Get the horse data (EnterBreedingMarket has simpler struct)
        horseRecord.Immutable([&protocolHorse](const data::Horse& horse)
        {
          protocolHorse.uid = horse.uid();
          protocolHorse.tid = horse.tid();
          protocolHorse.combo = 0;  // Combo/success streak count
          protocolHorse.isRegistered = (horse.horseType() == 2) ? 1 : 0;  // 1 if registered as stallion, 0 if not
          protocolHorse.unk2 = 0;   // Unknown field
          protocolHorse.lineage = 0; // Ancestor coat lineage score
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

void RanchDirector::HandleSearchStallion(
  ClientId clientId,
  const protocol::AcCmdCRSearchStallion& command)
{
  auto& clientContext = GetClientContext(clientId);

  spdlog::debug("SearchStallion: unk0={}, flags=[{},{},{},{},{},{},{},{}], "
    "filterLists=[{},{},{}], unk10={}",
    command.unk0,
    command.unk1, command.unk2, command.unk3, command.unk4,
    command.unk5, command.unk6, command.unk7, command.unk8,
    command.unk9[0].size(), command.unk9[1].size(), command.unk9[2].size(),
    command.unk10);

  spdlog::debug("g_stallions size: {}", g_stallions.size());
  
  protocol::RanchCommandSearchStallionOK response{
    .unk0 = 0,
    .unk1 = 0};

  for (const data::Uid& horseUid : g_stallions)
  {
    spdlog::debug("Processing stallion horseUid: {}", horseUid);
    
    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(horseUid);
    if (!horseRecord)
    {
      spdlog::warn("Horse record not found for horseUid: {}", horseUid);
      continue;
    }

    // Look up the stallion registration record using the map
    auto it = g_horseToStallionMap.find(horseUid);
    if (it == g_horseToStallionMap.end())
    {
      spdlog::warn("Stallion mapping not found for horseUid: {}", horseUid);
      continue; // Horse not properly registered
    }
    
    data::Uid stallionUid = it->second;
    spdlog::debug("Found stallion mapping: horseUid={} -> stallionUid={}", horseUid, stallionUid);
    
    // Get cached stallion data to avoid async loading issues
    auto cacheIt = g_stallionDataCache.find(stallionUid);
    if (cacheIt == g_stallionDataCache.end())
    {
      spdlog::warn("Stallion data not found in cache for stallionUid: {}", stallionUid);
      continue;
    }
    
    const CachedStallionData& cachedData = cacheIt->second;
    spdlog::debug("Successfully retrieved cached data for stallion {}", stallionUid);

    auto& protocolStallion = response.stallions.emplace_back();
    
    std::string ownerName = "unknown";
    
    // Get owner name
    const auto ownerRecord = GetServerInstance().GetDataDirector().GetCharacter(cachedData.ownerUid);
    if (ownerRecord)
    {
      ownerRecord.Immutable([&ownerName](const data::Character& owner)
      {
        ownerName = owner.name();
      });
    }

    horseRecord->Immutable([&protocolStallion, &ownerName, &cachedData](const data::Horse& horse)
    {
      protocolStallion.member1 = ownerName;
      protocolStallion.uid = horse.uid();
      protocolStallion.tid = horse.tid();
      protocolStallion.name = horse.name();
      protocolStallion.grade = horse.grade();
      protocolStallion.chance = 0;  // TODO: Calculate breeding chances based on bonus, linage, etc.
      protocolStallion.matePrice = cachedData.breedingCharge;
      protocolStallion.unk7 = 0;    // Unknown field
      protocolStallion.expiresAt = util::TimePointToAliciaTime(cachedData.expiresAt);

      protocol::BuildProtocolHorseStats(protocolStallion.stats, horse.stats);
      protocol::BuildProtocolHorseParts(protocolStallion.parts, horse.parts);
      protocol::BuildProtocolHorseAppearance(protocolStallion.appearance, horse.appearance);
      
      protocolStallion.unk11 = 0;   // Unknown field
      protocolStallion.lineage = 0; // TODO: Calculate lineage
    });
  }
  
  spdlog::debug("SearchStallion: Found {} stallions", response.stallions.size());

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
  spdlog::info("RegisterStallion: horseUid={}, breedingCharge={}", command.horseUid, command.carrots);
  
  const auto& clientContext = GetClientContext(clientId);
  
  // Get horse data to validate grade
  auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(command.horseUid);
  if (!horseRecord)
  {
    spdlog::warn("RegisterStallion: Horse {} not found", command.horseUid);
    return; // TODO: Send cancel response
  }

  uint8_t horseGrade = 0;
  horseRecord->Immutable([&horseGrade](const data::Horse& horse)
  {
    horseGrade = horse.grade();
  });

  spdlog::debug("RegisterStallion: Horse grade is {}", horseGrade);

  // Only allow grades 4-8 to be registered
  if (horseGrade < 4 || horseGrade > 8)
  {
    spdlog::warn("RegisterStallion: Horse {} grade {} is not allowed for breeding (must be 4-8)", 
      command.horseUid, horseGrade);
    return; // TODO: Send cancel response
  }

  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  // Calculate registration fee (50% of breeding charge)
  uint32_t registrationFee = command.carrots / 2;

  spdlog::debug("RegisterStallion: Deducting registration fee of {} carrots", registrationFee);

  // Deduct the registration fee from player's carrots
  characterRecord.Mutable([registrationFee](data::Character& character)
  {
    if (character.carrots() >= registrationFee)
    {
      character.carrots() = character.carrots() - registrationFee;
    }
  });

  spdlog::debug("RegisterStallion: Setting horse type to Stallion (2)");

  // Mark horse as registered stallion
  horseRecord->Mutable([](data::Horse& horse)
    {
      horse.horseType() = 2; // HorseType::Stallion
  });

  spdlog::debug("RegisterStallion: Creating stallion database record");

  // Create persistent stallion registration
  auto stallionRecord = GetServerInstance().GetDataDirector().CreateStallion();

  data::Uid stallionUid = data::InvalidUid;
  stallionRecord.Mutable([&command, &clientContext, &stallionUid](data::Stallion& stallion)
  {
    stallion.horseUid() = command.horseUid;
    stallion.ownerUid() = clientContext.characterUid;
    stallion.breedingCharge() = command.carrots;
    stallion.registeredAt() = server::util::Clock::now();
    stallion.expiresAt() = server::util::Clock::now() + std::chrono::hours(24 * 7); // 7 days
    stallion.timesBreeded() = 0;
    stallionUid = stallion.uid();
  });

  spdlog::info("RegisterStallion: Created stallion record with UID {}", stallionUid);

  // Add in-memory list for backwards compatibility
  g_stallions.emplace_back(command.horseUid);
  
  // Map horseUid to stallionUid for quick lookup
  g_horseToStallionMap[command.horseUid] = stallionUid;
  
  // Cache the stallion data
  CachedStallionData cachedData{
    .ownerUid = clientContext.characterUid,
    .breedingCharge = command.carrots,
    .expiresAt = server::util::Clock::now() + std::chrono::hours(24 * 7)
  };
  g_stallionDataCache[stallionUid] = cachedData;

  spdlog::debug("RegisterStallion: Sending OK response to client");

  protocol::AcCmdCRRegisterStallionOK response{
    .horseUid = command.horseUid};

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  spdlog::debug("RegisterStallion: Sending inventory update");

  // Update client's inventory/carrot display
  SendInventoryUpdate(clientId);

  spdlog::info("RegisterStallion: Successfully registered horse {} as stallion {}", command.horseUid, stallionUid);
}

void RanchDirector::HandleUnregisterStallion(
  ClientId clientId,
  const protocol::AcCmdCRUnregisterStallion& command)
{
  // Reset horse type back to Adult
  auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(command.horseUid);
  if (horseRecord)
  {
    horseRecord->Mutable([](data::Horse& horse)
    {
      horse.horseType() = 0; // HorseType::Adult
    });
  }

  // Remove from in-memory list
  g_stallions.erase(std::ranges::find(g_stallions, command.horseUid));

  // Look up and delete the stallion registration record
  auto it = g_horseToStallionMap.find(command.horseUid);
  if (it != g_horseToStallionMap.end())
  {
    data::Uid stallionUid = it->second;
    
    // Delete the stallion record from database
    GetServerInstance().GetDataDirector().GetStallionCache().Delete(stallionUid);
    
    // Remove from map
    g_horseToStallionMap.erase(it);
  }

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

void RanchDirector::HandleCheckStallionCharge(
  ClientId clientId,
  const protocol::AcCmdCRCheckStallionCharge& command)
{
  // Get horse data to determine grade
  auto horseRecord = GetServerInstance().GetDataDirector().GetHorseCache().Get(command.horseUid);
  if (!horseRecord)
  {
    spdlog::warn("CheckStallionCharge: Horse {} not found", command.horseUid);
    return; // TODO: Send cancel/error response
  }

  uint8_t horseGrade = 0;
  horseRecord->Immutable([&horseGrade](const data::Horse& horse)
  {
    horseGrade = horse.grade();
  });

  // Only allow grades 4-8 to be registered
  if (horseGrade < 4 || horseGrade > 8)
  {
    spdlog::warn("CheckStallionCharge: Horse {} grade {} is not allowed for breeding (must be 4-8)", 
      command.horseUid, horseGrade);
    
    // Return error response
    protocol::AcCmdCRCheckStallionChargeOK response{
      .status = 1,            // 1 = error/not allowed
      .minCharge = 0,
      .maxCharge = 0,
      .registrationFee = 0,
      .charge = command.horseUid
    };
    
    _commandServer.QueueCommand<decltype(response)>(
      clientId,
      [response]() { return response; });
    return;
  }

  // Fallback values
  uint32_t minCharge = 1;
  uint32_t maxCharge = 100000;
  uint32_t registrationFee = 0;

  // TODO: Replace the temporary hardcoded values for grades 4-7 with real values
  if (horseGrade == 4)
  {
    minCharge = 4000;
    maxCharge = 12000;
  }
  else if (horseGrade == 5)
  {
    minCharge = 5000;
    maxCharge = 15000;
  }
  else if (horseGrade == 6)
  {
    minCharge = 6000;
    maxCharge = 18000;
  }
  else if (horseGrade == 7)
  {
    minCharge = 8000;
    maxCharge = 24000;
  }
  else if (horseGrade == 8)
  {
    minCharge = 10000;
    maxCharge = 40000;
  }

  // Validate and return breeding charge information
  protocol::AcCmdCRCheckStallionChargeOK response{
    .status = 0,                // 0 = success
    .minCharge = minCharge,     // Grade-specific minimum
    .maxCharge = maxCharge,     // Grade-specific maximum
    .registrationFee = registrationFee,  // TODO: Calculate based on grade
    .charge = command.horseUid  // Echo back the horseUid
  };

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

void RanchDirector::HandleBreedingFailureCard(
  ClientId clientId,
  const protocol::AcCmdCRBreedingFailureCard& command)
{
  spdlog::info("BreedingFailureCard: statusOrFlag = {}", command.statusOrFlag);
  
  protocol::AcCmdCRBreedingFailureCardOK response{
    .choiceOrFlag = 0  // Default choice/flag value
  };

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
}

void RanchDirector::HandleBreedingFailureCardChoose(
  ClientId clientId,
  const protocol::AcCmdCRBreedingFailureCardChoose& command)
{
  spdlog::info("BreedingFailureCardChoose: statusOrFlag = {}", command.statusOrFlag);
  
  const auto& clientContext = GetClientContext(clientId);
  auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);
  
  protocol::AcCmdCRBreedingFailureCardChooseOK response;
  
  static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
  
  uint32_t moneySpent = 100000;
  
  // Breeding Failure Card reward grade probability structure
  // Maps total money spent on breeding to reward grade probabilities
  struct ProbData {
    uint32_t moneySpent;  // Total money spent threshold (in game currency)
    int probA;            // Probability % for Grade A (common/low-tier rewards)
    int probB;            // Probability % for Grade B (uncommon/mid-tier rewards)
    int probC;            // Probability % for Grade C (rare/high-tier rewards)
  };
  
  // Breeding Failure Card Probability Table
  // Source: libconfig_c.dat -> BreedingFailureCardProb table (XML)
  // 
  // This table determines the reward grade (quality tier) based on cumulative money spent on breeding.
  // As players spend more money, probabilities shift from Grade A (common) -> Grade B (uncommon) -> Grade C (rare)
  //
  // Format: {MoneySpent threshold, Prob_A%, Prob_B%, Prob_C%}
  // - At 4k-8k spent: 100% Grade A (only common rewards)
  // - At 10k spent: 90% A, 10% B (mostly common, some uncommon)
  // - At 100k+ spent: 0% A, 0% B, 100% C (guaranteed rare/high-tier rewards)
  //
  // The grade determines which reward tier is selected from the Normal/Chance reward tables
  static const std::vector<ProbData> probTable = {
    {4000, 100, 0, 0}, {5000, 100, 0, 0}, {6000, 100, 0, 0}, {7000, 100, 0, 0}, {8000, 100, 0, 0},
    {9000, 96, 4, 0}, {10000, 90, 10, 0}, {11000, 89, 10, 1}, {12000, 87, 11, 2}, {13000, 86, 11, 3},
    {16000, 77, 18, 5}, {19000, 68, 25, 7}, {22000, 57, 34, 9}, {25000, 50, 39, 11}, {28000, 40, 47, 13},
    {31000, 33, 52, 15}, {35000, 22, 60, 18}, {39000, 16, 62, 22}, {43000, 8, 66, 26}, {47000, 10, 59, 31},
    {51000, 7, 57, 36}, {55000, 6, 52, 42}, {59000, 7, 45, 48}, {65000, 10, 35, 55}, {71000, 10, 28, 62},
    {77000, 10, 20, 70}, {83000, 7, 15, 78}, {89000, 4, 11, 85}, {95000, 1, 8, 91}, {100000, 0, 0, 100},
    {125000, 0, 0, 100}, {140000, 0, 0, 100}, {155000, 0, 0, 100}
  };
  
  // Find probability entry for money spent
  const ProbData& probEntry = [&]() -> const ProbData& {
    for (const auto& entry : probTable) {
      if (moneySpent <= entry.moneySpent) {
        return entry;
      }
    }
    return probTable.back(); // Default to highest spending tier
  }();
  
  std::uniform_int_distribution<int> gradeDist(1, 100);
  int gradeRoll = gradeDist(gen);
  
  int rewardGrade = 0;
  if (gradeRoll <= probEntry.probA) {
    rewardGrade = 0;
  } else if (gradeRoll <= probEntry.probA + probEntry.probB) {
    rewardGrade = 1;
  } else {
    rewardGrade = 2;
  }
  
  // Determine card type: 50/50 chance between Normal (RED) and Chance (YELLOW) cards
  // TODO: Figure out the exact/approximate probability of each card type.
  std::uniform_int_distribution<int> cardTypeDist(0, 1);
  bool isChanceCard = (cardTypeDist(gen) == 1);
  
  // Breeding Failure Card reward data structure
  struct RewardData {
    uint32_t itemTid;    // Item Template ID (identifies the reward item type)
    uint32_t itemCount;  // Number of items to award
    uint32_t gameMoney;  // Bonus carrots (in-game currency) to award
  };
  
  uint32_t rewardId;
  const RewardData* rewardData = nullptr;
  
  if (isChanceCard) {
    // GOLD "Chance" Cards
    // Source: libconfig_c.dat -> BreedingFailureCard_Chance table (48 entries)
    // These cards give HIGHER rewards than normal cards
    
    // Map grade to RewardId ranges within the Chance table:
    // Grade A (common tier): RewardId 1-16   (300-1000 carrots, basic items)
    // Grade B (uncommon):    RewardId 17-32  (1400-4000 carrots, better items)
    // Grade C (rare tier):   RewardId 33-48  (7000-25000 carrots, premium items)
    uint32_t minReward, maxReward;
    if (rewardGrade == 0) {
      minReward = 1; maxReward = 16;
    } else if (rewardGrade == 1) {
      minReward = 17; maxReward = 32;
    } else {
      minReward = 33; maxReward = 48;
    }
    
    std::uniform_int_distribution<uint32_t> chanceDist(minReward, maxReward);
    rewardId = chanceDist(gen);
    
    // Chance Card Reward Table (YELLOW cards)
    // Format: {RewardId, {ItemTid, ItemCount, CarrotBonus}}
    // RewardId 1-16: Grade A rewards (low-tier for yellow cards)
    // RewardId 17-32: Grade B rewards (mid-tier)
    // RewardId 33-48: Grade C rewards (high-tier, up to 25k carrots!)
    static const std::unordered_map<uint32_t, RewardData> chanceRewardTable = {
      {1, {45001, 1, 300}}, {2, {45001, 1, 350}}, {3, {45001, 1, 400}}, {4, {45001, 1, 420}},
      {5, {45001, 1, 450}}, {6, {45001, 1, 550}}, {7, {45001, 1, 600}}, {8, {44006, 1, 620}},
      {9, {44005, 1, 700}}, {10, {44004, 1, 800}}, {11, {44003, 1, 800}}, {12, {44002, 1, 900}},
      {13, {44001, 1, 900}}, {14, {43001, 1, 950}}, {15, {44002, 1, 1000}}, {16, {43001, 2, 1000}},
      {17, {42002, 7, 1400}}, {18, {42001, 10, 1800}}, {19, {43001, 1, 2000}}, {20, {44006, 1, 2000}},
      {21, {44004, 2, 2000}}, {22, {44002, 2, 2000}}, {23, {43001, 1, 2100}}, {24, {45001, 2, 2200}},
      {25, {45001, 2, 2300}}, {26, {45001, 2, 2500}}, {27, {45001, 2, 2800}}, {28, {45001, 2, 3000}},
      {29, {45001, 2, 3500}}, {30, {45001, 2, 3800}}, {31, {45001, 2, 4000}}, {32, {45001, 3, 4000}},
      {33, {45001, 3, 7000}}, {34, {45001, 3, 8000}}, {35, {45001, 3, 9000}}, {36, {45001, 3, 10000}},
      {37, {45001, 3, 11000}}, {38, {45001, 3, 12000}}, {39, {45001, 3, 13000}}, {40, {45001, 3, 14000}},
      {41, {44006, 3, 15000}}, {42, {44004, 3, 16000}}, {43, {44002, 3, 17000}}, {44, {44001, 3, 18000}},
      {45, {44003, 3, 19000}}, {46, {45001, 3, 20000}}, {47, {45001, 3, 25000}}, {48, {45001, 3, 25000}}
    };
    
    auto it = chanceRewardTable.find(rewardId);
    if (it != chanceRewardTable.end()) {
      rewardData = &it->second;
    }
  } else {
    // RED "Normal" Cards
    // Source: libconfig_c.dat -> BreedingFailureCard_Normal table (63 entries)
    // These cards give LOWER rewards compared to chance cards
    
    // Map grade to RewardId ranges within the Normal table:
    // Grade A (common tier): RewardId 1-20   (100-350 carrots, basic items)
    // Grade B (uncommon):    RewardId 21-38  (300-1000 carrots, better items)
    // Grade C (rare tier):   RewardId 39-63  (2000-13000 carrots, premium items)
    uint32_t minReward, maxReward;
    if (rewardGrade == 0) {
      minReward = 1; maxReward = 20;
    } else if (rewardGrade == 1) {
      minReward = 21; maxReward = 38;
    } else {
      minReward = 39; maxReward = 63;
    }
    
    std::uniform_int_distribution<uint32_t> normalDist(minReward, maxReward);
    rewardId = normalDist(gen);
    
    // Normal Card Reward Table (RED cards)
    // Format: {RewardId, {ItemTid, ItemCount, CarrotBonus}}
    // RewardId 1-20: Grade A rewards (low-tier, 100-350 carrots)
    // RewardId 21-38: Grade B rewards (mid-tier, 300-1000 carrots)
    // RewardId 39-63: Grade C rewards (high-tier, 2000-13000 carrots)
    // Note: Even at Grade C, normal cards give less than chance cards!
    static const std::unordered_map<uint32_t, RewardData> normalRewardTable = {
      {1, {45001, 1, 100}}, {2, {45001, 1, 100}}, {3, {45001, 1, 120}}, {4, {45001, 1, 140}},
      {5, {45001, 1, 120}}, {6, {45001, 1, 100}}, {7, {45001, 1, 120}}, {8, {45001, 1, 100}},
      {9, {45001, 1, 120}}, {10, {41001, 5, 120}}, {11, {41009, 3, 150}}, {12, {41007, 3, 150}},
      {13, {40002, 2, 150}}, {14, {41004, 2, 150}}, {15, {41003, 2, 180}}, {16, {41002, 3, 180}},
      {17, {41001, 4, 200}}, {18, {41009, 2, 240}}, {19, {41008, 2, 300}}, {20, {40002, 3, 350}},
      {21, {45001, 3, 300}}, {22, {45001, 2, 350}}, {23, {45001, 2, 400}}, {24, {45001, 2, 450}},
      {25, {45001, 1, 500}}, {26, {45001, 1, 550}}, {27, {45001, 1, 600}}, {28, {45001, 1, 650}},
      {29, {45001, 1, 700}}, {30, {45001, 1, 700}}, {31, {45001, 1, 800}}, {32, {45001, 1, 800}},
      {33, {45001, 1, 900}}, {34, {45001, 1, 900}}, {35, {44001, 1, 1000}}, {36, {44005, 1, 1000}},
      {37, {44003, 1, 1000}}, {38, {44001, 1, 1000}}, {39, {44006, 3, 2000}}, {40, {44004, 3, 2100}},
      {41, {44002, 3, 2200}}, {42, {43001, 3, 2300}}, {43, {43001, 1, 2400}}, {44, {43001, 1, 2500}},
      {45, {44006, 2, 2600}}, {46, {44004, 2, 2700}}, {47, {44002, 2, 2800}}, {48, {43001, 2, 2900}},
      {49, {43001, 2, 3000}}, {50, {43001, 2, 3200}}, {51, {43001, 2, 3400}}, {52, {43001, 2, 3600}},
      {53, {45001, 3, 3800}}, {54, {45001, 3, 4300}}, {55, {45001, 3, 4800}}, {56, {45001, 2, 5300}},
      {57, {45001, 2, 5800}}, {58, {45001, 2, 6300}}, {59, {45001, 3, 6800}}, {60, {45001, 3, 7300}},
      {61, {45001, 2, 7800}}, {62, {45001, 3, 10000}}, {63, {45001, 3, 13000}}
    };
    
    auto it = normalRewardTable.find(rewardId);
    if (it != normalRewardTable.end()) {
      rewardData = &it->second;
    }
  }
  
  static const RewardData fallbackReward = {45001, 1, 120};
  if (!rewardData) {
    rewardData = &fallbackReward;
  }
  
  data::Uid itemUid = 0;
  bool foundExistingItem = false;
  
  characterRecord.Immutable([&itemUid, &foundExistingItem, rewardData, this](const data::Character& character) {
    for (const auto& existingItemUid : character.inventory()) {
      const auto existingItemRecord = GetServerInstance().GetDataDirector().GetItem(existingItemUid);
      if (existingItemRecord) {
        existingItemRecord.Immutable([&itemUid, &foundExistingItem, rewardData](const data::Item& existingItem) {
          if (existingItem.tid() == rewardData->itemTid) {
            itemUid = existingItem.uid();
            foundExistingItem = true;
          }
        });
        if (foundExistingItem) break;
      }
    }
  });
  
  if (foundExistingItem) {
    const auto existingItemRecord = GetServerInstance().GetDataDirector().GetItem(itemUid);
    existingItemRecord.Mutable([rewardData, &response](data::Item& item) {
      item.count() += rewardData->itemCount;
      response.item.uid = item.uid();
      response.item.tid = item.tid();
      response.item.expiresAt = 0;
      response.item.count = item.count();
    });
  } else {
    const auto newItem = GetServerInstance().GetDataDirector().CreateItem();
    newItem.Mutable([&itemUid, rewardData, &response](data::Item& item) {
      item.tid() = rewardData->itemTid;
      item.count() = rewardData->itemCount;
      itemUid = item.uid();
      response.item.uid = item.uid();
      response.item.tid = item.tid();
      response.item.expiresAt = 0;
      response.item.count = item.count();
    });
    characterRecord.Mutable([itemUid](data::Character& character) {
      character.inventory().emplace_back(itemUid);
    });
  }
  
  characterRecord.Mutable([rewardData](data::Character& character) {
    character.carrots() += rewardData->gameMoney;
  });
  
  response.member1 = 0;
  response.rewardId = rewardId;
  response.member3 = 0;
  response.member4 = {1, 0};
  response.member5 = 0;
  response.member6 = rewardData->gameMoney;
  
  spdlog::info("BreedingFailureCard: {} CARD (Grade {})! MoneySpent: {}, GradeRoll: {}, RewardId {}, gave {} carrots + item {} x{}",
    isChanceCard ? "CHANCE (YELLOW)" : "NORMAL (RED)",
    rewardGrade, moneySpent, gradeRoll, rewardId,
    rewardData->gameMoney, rewardData->itemTid, rewardData->itemCount);

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });
  
  // Send inventory update notification to refresh client inventory
  SendInventoryUpdate(clientId);
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
  const auto characterRecord = GetServerInstance().GetDataDirector().GetCharacter(
    clientContext.characterUid);

  if (not characterRecord)
  {
    throw std::runtime_error(
      std::format("Character [{}] not available", clientContext.characterUid));
  }

  // NOTE: This handler is being repurposed to grant a breeding failure card reward
  // until the full card choice system is implemented. This is a temporary measure.

  // 1. Define the reward item (e.g., 5 Carrots, TID 45001)
  const uint32_t rewardItemTid = 45001;
  const uint16_t rewardItemCount = 5;

  // 2. Create the item and add it to the character's inventory
  const auto newItem = GetServerInstance().GetDataDirector().CreateItem();
  data::Uid newItemUid = 0;
  newItem.Mutable([&](data::Item& item) {
    item.tid() = rewardItemTid;
    item.count() = rewardItemCount;
    newItemUid = item.uid();
  });

  characterRecord.Mutable([newItemUid](data::Character& character) {
    character.inventory().emplace_back(newItemUid);
  });

  // 3. Send the response for the original action (even though we are repurposing it)
  // We will use AcCmdCRGetItemFromStorageOK for now to signal success to the client action that triggered this.
  protocol::AcCmdCRGetItemFromStorageOK response;
  response.storageItemUid = command.storedItemUid; // Acknowledge the original request
  
  protocol::Item responseItem;
  responseItem.uid = newItemUid;
  responseItem.tid = rewardItemTid;
  responseItem.count = rewardItemCount;
  response.items.push_back(responseItem);

  characterRecord.Immutable([&](const data::Character& character) {
      response.updatedCarrots = character.carrots();
  });

  _commandServer.QueueCommand<decltype(response)>(
    clientId,
    [response]()
    {
      return response;
    });

  // 4. Send the inventory update notification to refresh the client's UI
  SendInventoryUpdate(clientId);

  spdlog::info("Awarded temporary breeding failure item {} (x{}) to character {}", rewardItemTid, rewardItemCount, clientContext.characterUid);
}

void RanchDirector::HandleRequestNpcDressList(
  ClientId clientId,
  const protocol::RanchCommandRequestNpcDressList& requestNpcDressList)
{
  protocol::RanchCommandRequestNpcDressListOK response{
    .unk0 = requestNpcDressList.unk0,
    .dressList = {
    protocol::Item{
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
  protocol::RanchCommandMountFamilyTreeOK response{};

  const auto& horseRecord = GetServerInstance().GetDataDirector().GetHorse(command.horseUid);
  if (not horseRecord.IsAvailable())
  {
    _commandServer.QueueCommand<decltype(response)>(clientId, [response]() { return response; });
    return;
  }

  std::vector<data::Uid> parents;
  horseRecord.Immutable([&parents](const data::Horse& horse) { parents = horse.ancestors(); });

  // If there are not exactly two parents, return empty response
  if (parents.size() != 2)
  {
    _commandServer.QueueCommand<decltype(response)>(clientId, [response]() { return response; });
    return;
  }

  // Map of position ID to UID for family tree
  using Position = protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem::Position;
  std::map<Position, data::Uid> ancestorPositions;
  
  // BFS queue in Position enum order: Father(1) → Mother(2) → grandparents
  std::queue<std::pair<data::Uid, Position>> bfs;
  bfs.emplace(parents[0], Position::Father);
  bfs.emplace(parents[1], Position::Mother);

  // BFS for 2 generations
  while (not bfs.empty())
  {
    auto [uid, pos] = bfs.front();
    bfs.pop();
    ancestorPositions[pos] = uid;

    if (pos == Position::Father || pos == Position::Mother)
    {
      const auto horseRecord = GetServerInstance().GetDataDirector().GetHorse(uid);
      if (horseRecord.IsAvailable())
      {
        std::vector<data::Uid> ancestors;
        horseRecord.Immutable([&ancestors](const data::Horse& h)
          {
            ancestors = h.ancestors();
          });
        if (ancestors.size() == 2)
        {
          auto [grandfather, grandmother] = pos == Position::Father
          ? std::make_pair(Position::PaternalGrandfather, Position::PaternalGrandmother)
          : std::make_pair(Position::MaternalGrandfather, Position::MaternalGrandmother);

          bfs.emplace(ancestors[0], grandfather);
          bfs.emplace(ancestors[1], grandmother);
        }
      }
    }
  }
  // Build response from ancestor positions
  for (const auto& [positionId, horseUid] : ancestorPositions)
  {
    const auto horseRecord = GetServerInstance().GetDataDirector().GetHorse(horseUid);
    if (horseRecord.IsAvailable())
    {
      horseRecord.Immutable([&](const data::Horse& horse) {
        auto item = protocol::RanchCommandMountFamilyTreeOK::MountFamilyTreeItem{};
        item.id = positionId;
        item.name = horse.name();
        item.grade = horse.grade();
        item.skinId = horse.parts.skinTid();
        response.ancestors.push_back(item);
      });
    }
  }

  _commandServer.QueueCommand<decltype(response)>(clientId, [response]() { return response; });
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

void RanchDirector::HandleChangeSkillCardPreset(
  ClientId clientId,
  const protocol::AcCmdCRChangeSkillCardPreset command)
{
  const auto& clientContext = GetClientContext(clientId);
  if (command.skillSet.setId > 2)
  {
    // TODO: character tried to update skill set exceeding range, return?
    spdlog::warn("Character {} tried to update their skill set {} but character cannot have more than 2 skill sets",
      clientContext.characterUid, command.skillSet.setId);
    return;
  }
  else if (command.skillSet.skills.size() > 2)
  {
    spdlog::warn("Character {} tried to save more skills ({} skills) than a skill set can hold (2 skills)",
      clientContext.characterUid, command.skillSet.skills.size());
    return;
  }

  const auto& characterRecord = GetServerInstance().GetDataDirector().GetCharacter(clientContext.characterUid);
  characterRecord.Mutable(
    [&command](data::Character& character)
    {
      auto selectSkillSets = [&character](protocol::GameMode gamemode)
      { 
        switch (gamemode)
        {
          case protocol::GameMode::Magic:
            return &character.skills.magic();
          case protocol::GameMode::Speed:
            return &character.skills.speed();
          default:
            throw std::runtime_error("Gamemode is not recognised");
        }
      };

      const auto& skillSets = selectSkillSets(command.skillSet.gamemode);
      auto& skillSet = command.skillSet.setId == 0 ? skillSets->set1 : skillSets->set2;
      skillSet.slot1 = command.skillSet.skills[0];
      skillSet.slot2 = command.skillSet.skills[1];
    });
}

} // namespace server