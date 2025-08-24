//
// Created by rgnter on 31/05/2025.
//

#include "libserver/data/helper/ProtocolHelper.hpp"

namespace server
{

namespace protocol
{

void BuildProtocolCharacter(
  Character& protocolCharacter,
  const data::Character& character)
{
  // Set the character parts.
  // These serial ID's can be found in the `_ClientCharDefaultPartInfo` table.
  // Each character has specific part serial IDs for each part type.
  protocolCharacter.parts = {
    .charId = static_cast<uint8_t>(character.parts.modelId()),
    .mouthSerialId = static_cast<uint8_t>(character.parts.mouthId()),
    .faceSerialId = static_cast<uint8_t>(character.parts.faceId()),
  };

  // Set the character appearance.
  protocolCharacter.appearance = {
    .voiceId = static_cast<uint16_t>(character.appearance.voiceId()),
    .headSize = static_cast<uint16_t>(character.appearance.headSize()),
    .height = static_cast<uint16_t>(character.appearance.height()),
    .thighVolume = static_cast<uint16_t>(character.appearance.thighVolume()),
    .legVolume = static_cast<uint16_t>(character.appearance.legVolume()),
    .emblemId = static_cast<uint16_t>(character.appearance.emblemId())
  };
}

void BuildProtocolHorse(
  Horse& protocolHorse,
  const data::Horse& horse)
{
  protocolHorse.uid = horse.uid();
  protocolHorse.tid = horse.tid();
  protocolHorse.name = horse.name();

  protocolHorse.rating = horse.rating();
  protocolHorse.clazz = static_cast<uint8_t>(horse.clazz());
  protocolHorse.val0 = 1;
  protocolHorse.grade = static_cast<uint8_t>(horse.grade());
  protocolHorse.growthPoints = static_cast<uint16_t>(horse.growthPoints());

  protocolHorse.val16 = 0xb8a167e4,
  protocolHorse.val17 = 0;

  protocolHorse.mountCondition = {
    .stamina = 0xFFFF,
    .charmPoint = 0x3DE,
    .friendlyPoint = 0x3DE,
    .injuryPoint = 0x0000,
    .plenitude = 0x500, 
    .bodyDirtiness = 0x160, 
    .maneDirtiness = 0x160, 
    .tailDirtiness = 0x160,
    .attachment = 0x1F5, 
    .boredom = 0x15,
    .bodyPolish = 0x2710, //does nothing rn?? 
    .manePolish = 0xA,
    .tailPolish = 0xA,
    .stopAmendsPoint = 0x0000
  };

  protocolHorse.vals1 = {
    .type = Horse::HorseType::Adult,
    .val1 = 0x00,
    .dateOfBirth = util::TimePointToAliciaTime(horse.dateOfBirth()),
    .tendency = 0x02,
    .spirit = 0x00,
    .classProgression = static_cast<uint32_t>(horse.clazzProgress()),
    .val5 = 0x00,
    .potentialLevel = static_cast<uint8_t>(horse.potentialLevel()),
    .hasPotential = static_cast<uint8_t>(horse.potentialType() != 0),
    .potentialValue = static_cast<uint8_t>(horse.potentialLevel()),
    .val9 = 0x00,
    .luck = static_cast<uint8_t>(horse.luckState()),
    .injury = Horse::Injury::None,
    .val12 = 0x00,
    .fatigue = 0x00,
    .val14 = 0x00,
    .emblem = static_cast<uint16_t>(horse.emblemUid())};

  BuildProtocolHorseParts(protocolHorse.parts, horse.parts);
  BuildProtocolHorseAppearance(protocolHorse.appearance, horse.appearance);
  BuildProtocolHorseStats(protocolHorse.stats, horse.stats);
  BuildProtocolHorseMastery(protocolHorse.mastery, horse.mastery);
}

void BuildProtocolHorseParts(
  Horse::Parts& protocolHorseParts,
  const data::Horse::Parts& parts)
{
  protocolHorseParts = {
    .skinId = static_cast<uint8_t>(parts.skinTid()),
    .maneId = static_cast<uint8_t>(parts.maneTid()),
    .tailId = static_cast<uint8_t>(parts.tailTid()),
    .faceId = static_cast<uint8_t>(parts.faceTid())};
}

void BuildProtocolHorseAppearance(
  Horse::Appearance& protocolHorseAppearance,
  const data::Horse::Appearance& appearance)
{
  protocolHorseAppearance = {
    .scale = static_cast<uint8_t>(appearance.scale()),
    .legLength = static_cast<uint8_t>(appearance.legLength()),
    .legVolume = static_cast<uint8_t>(appearance.legVolume()),
    .bodyLength = static_cast<uint8_t>(appearance.bodyLength()),
    .bodyVolume = static_cast<uint8_t>(appearance.bodyVolume())};
}

void BuildProtocolHorseStats(
  Horse::Stats& protocolHorseStats,
  const data::Horse::Stats& stats)
{
  protocolHorseStats = {
    .agility = stats.agility(),
    .ambition = stats.ambition(),
    .rush = stats.rush(),
    .endurance = stats.endurance(),
    .courage = stats.courage()};
}

void BuildProtocolHorseMastery(
  Horse::Mastery& protocolHorseMastery,
  const data::Horse::Mastery& mastery)
{
  protocolHorseMastery = {
    .spurMagicCount = mastery.spurMagicCount(),
    .jumpCount = mastery.jumpCount(),
    .slidingTime = mastery.slidingTime(),
    .glidingDistance = mastery.glidingDistance(),
  };
}

void BuildProtocolHorses(
  std::vector<Horse>& protocolHorses,
  const std::vector<Record<data::Horse>>& horseRecords)
{
  for (const auto& horse : horseRecords)
  {
    auto& protocolHorse = protocolHorses.emplace_back();
    horse.Immutable([&protocolHorse](const auto& horse)
    {
      BuildProtocolHorse(protocolHorse, horse);
    });
  }
}

void BuildProtocolItem(
  Item& protocolItem,
  const data::Item& item)
{
  protocolItem.uid = item.uid();
  protocolItem.tid = item.tid();
  protocolItem.expiresAt = util::TimePointToAliciaTime(item.expiresAt());
  protocolItem.count = item.count();
}

void BuildProtocolItems(
  std::vector<Item>& protocolItems,
  const std::vector<Record<data::Item>>& itemRecords)
{
  for (const auto& item : itemRecords)
  {
    auto& protocolItem = protocolItems.emplace_back();
    item.Immutable([&protocolItem](const auto& item)
    {
      BuildProtocolItem(protocolItem, item);
    });
  }
}

void BuildProtocolStoredItem(
  StoredItem& protocolStoredItem,
  const data::StorageItem& storedItem)
{
  protocolStoredItem.uid = storedItem.uid();
  protocolStoredItem.sender = storedItem.sender();
  protocolStoredItem.message = storedItem.message();
  protocolStoredItem.dateAndTime = util::TimePointToAliciaTime(storedItem.created());
}

void BuildProtocolStoredItems(
  std::vector<StoredItem>& protocolStoredItems,
  const std::span<const Record<data::StorageItem>>& storedItemRecords)
{
  for (const auto& storedItem : storedItemRecords)
  {
    auto& protocolStoredItem = protocolStoredItems.emplace_back();
    storedItem.Immutable([&protocolStoredItem](const auto& storedItem)
      {
        BuildProtocolStoredItem(protocolStoredItem, storedItem);
      });
  }
}

void BuildProtocolGuild(Guild& protocolGuild, const data::Guild& guildRecord)
{
  protocolGuild.name = guildRecord.name();
  protocolGuild.uid = guildRecord.uid();
}

void BuildProtocolPet(Pet& protocolPet, const data::Pet& petRecord)
{
  protocolPet.petId = petRecord.petId();
  protocolPet.member2 = 0; // Unused
  protocolPet.name = petRecord.name();
  protocolPet.member4 = 0; // Unused
}

void BuildProtocolPets(
  std::vector<Pet>& protocolPets,
  const std::span<const Record<data::Pet>>& storedPets)
{
  for (const auto& storedPet : storedPets)
  {
    auto& protocolPet = protocolPets.emplace_back();
    storedPet.Immutable([&protocolPet](const auto& storedPet)
    {
      BuildProtocolPet(protocolPet, storedPet);
    });
  }
}

void BuildProtocolHousing(
  Housing& protocolHousing,
  const data::Housing& housingRecord)
{
  protocolHousing.uid = housingRecord.uid();
  protocolHousing.tid = housingRecord.housingId();
  protocolHousing.durability = housingRecord.durability();
}

void BuildProtocolHousing(
  std::vector<Housing>& protocolHousings,
  const std::vector<Record<data::Housing>>& housingRecords)
{
  for (const auto& housingRecord : housingRecords)
  {
    auto& protocolHousing = protocolHousings.emplace_back();
    housingRecord.Immutable(
      [&protocolHousing](const auto& housingRecord)
      {
        BuildProtocolHousing(protocolHousing, housingRecord);
      });
  }
}

} // namespace protocol

} // namespace server
