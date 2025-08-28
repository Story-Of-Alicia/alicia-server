//
// Created by rgnter on 31/05/2025.
//

#ifndef PROTOCOLHELPER_HPP
#define PROTOCOLHELPER_HPP

#include "libserver/data/DataDefinitions.hpp"
#include "libserver/data/DataStorage.hpp"
#include "libserver/network/command/proto/CommonStructureDefinitions.hpp"
#include "libserver/network/command/proto/LobbyMessageDefinitions.hpp"
#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"
#include "server/ServerInstance.hpp"

namespace server
{

namespace protocol
{

void BuildProtocolCharacter(
  Character& protocolCharacter,
  const data::Character& character);

void BuildProtocolHorse(
  Horse& protocolHorse,
  const data::Horse& horse);

void BuildProtocolHorseParts(
  Horse::Parts& protocolHorseParts,
  const data::Horse::Parts& parts);

void BuildProtocolHorseAppearance(
  Horse::Appearance& protocolHorseAppearance,
  const data::Horse::Appearance& appearance);

void BuildProtocolHorseStats(
  Horse::Stats& protocolHorseStats,
  const data::Horse::Stats& stats);

void BuildProtocolHorseMastery(
  Horse::Mastery& protocolHorseMastery,
  const data::Horse::Mastery& mastery);

void BuildProtocolHorses(
    std::vector<Horse>& protocolHorses,
    const std::vector<Record<data::Horse>>& horseRecords);

void BuildProtocolItem(
  Item& protocolItem,
  const data::Item& item);

void BuildProtocolItems(
  std::vector<Item>& protocolItems,
  const std::vector<Record<data::Item>>& itemRecords);

void BuildProtocolStoredItem(
  StoredItem& protocolStoredItem,
  const data::StorageItem& storedItem);

void BuildProtocolStoredItems(
  std::vector<StoredItem>& protocolStoredItems,
  const std::span<const Record<data::StorageItem>>& storedItemRecords);

void BuildProtocolGuild(
  Guild& protocolGuild,
  const data::Guild& guildRecord);

void BuildProtocolPet(
  Pet& protocolPet,
  const data::Pet& petRecord);

void BuildProtocolPets(
  std::vector<Pet>& protocolPets,
  const std::span<const Record<data::Pet>>& storedPets);

void BuildProtocolHousing(
  Housing& protocolHousing,
  const data::Housing& housingRecord,
  bool hasDurability = false);

void BuildProtocolHousing(
  std::vector<Housing>& protocolHousing,
  const std::vector<Record<data::Housing>>& housingRecords);

} // namespace protocol

} // namespace server

#endif //PROTOCOLHELPER_HPP
