//
// Created by rgnter on 31/05/2025.
//

#ifndef PROTOCOLHELPER_HPP
#define PROTOCOLHELPER_HPP

#include "libserver/data/DataDefinitions.hpp"
#include "libserver/data/Record.hpp"

#include "libserver/network/command/proto/CommonStructureDefinitions.hpp"
#include "libserver/network/command/proto/LobbyMessageDefinitions.hpp"
#include "libserver/network/command/proto/RanchMessageDefinitions.hpp"

namespace server
{

namespace protocol
{

//! Packs a moment into the date format the achievement page expects, which is
//! `year | (month << 12) | (day << 16)`. A year of zero is how the client marks
//! a date as absent, so an unset moment packs to zero.
//! The client builds this format from its own local time, so the conversion has
//! to leave UTC here rather than in storage. It uses the time zone of the
//! server, which is the closest the server gets without knowing the one of the
//! player.
//! @param timePoint The moment to pack.
//! @returns The packed date, or zero when the moment is unset.
uint32_t BuildProtocolAchievementDate(data::Clock::time_point timePoint);

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

void BuildProtocolStorageItem(
  StoredItem& protocolStorageItem,
  const data::StorageItem& storageItem);

void BuildProtocolStorageItems(
  std::vector<StoredItem>& protocolStoredItems,
  const std::span<const Record<data::StorageItem>>& storageItemRecords);

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

void BuildProtocolEgg(
  Egg& protocolEgg,
  const data::Egg& eggRecord,
  const data::Clock::duration hatchDuration);

void BuildProtocolSettings(
  Settings& settings,
  const data::Settings& settingsRecord);

void BuildProtocolQuest(
  Quest& protocolQuest,
  const data::Quest& quest);

void BuildProtocolQuests(
  std::vector<Quest>& protocolQuests,
  const std::vector<Record<data::Quest>>& questRecords);


} // namespace protocol

} // namespace server

#endif //PROTOCOLHELPER_HPP
