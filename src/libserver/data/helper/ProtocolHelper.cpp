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
    .headSize = static_cast<uint8_t>(character.appearance.headSize()),
    .height = static_cast<uint8_t>(character.appearance.height()),
    .thighVolume = static_cast<uint8_t>(character.appearance.thighVolume()),
    .legVolume = static_cast<uint8_t>(character.appearance.legVolume()),
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

  protocolHorse.vals0 = {
    .stamina = 0xFFFF,
    .attractiveness = 0xFFFF,
    .hunger = 0xFFFF,
  };

  protocolHorse.vals1 = {
    .type = Horse::HorseType::Adult,
    .val1 = 0x00,
    .dateOfBirth = 0xb8a167e4,
    .tendency = 0x02,
    .spirit = 0x00,
    .classProgression = static_cast<uint32_t>(horse.clazzProgress()),
    .val5 = 0x00,
    .potentialLevel = static_cast<uint8_t>(horse.potentialLevel()),
    .hasPotential = static_cast<uint8_t>(horse.potentialType() != 0),
    .potentialValue = static_cast<uint8_t>(horse.potentialLevel()),
    .val9 = 0x00,
    .luck = static_cast<uint8_t>(horse.luckState()),
    .hasLuck = static_cast<uint8_t>(horse.luckState() != 0),
    .val12 = 0x00,
    .fatigue = 0x00,
    .val14 = 0x00,
    .emblem = static_cast<uint16_t>(horse.emblem())};

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
    .skinId = static_cast<uint8_t>(parts.skinId()),
    .maneId = static_cast<uint8_t>(parts.maneId()),
    .tailId = static_cast<uint8_t>(parts.tailId()),
    .faceId = static_cast<uint8_t>(parts.faceId())};
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
    .control = stats.control(),
    .speed = stats.speed(),
    .strength = stats.strength(),
    .spirit = stats.spirit()};
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
  const std::vector<Record<data::Horse>>& horses)
{
  for (const auto& horse : horses)
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
  protocolItem.count = item.count();
}

void BuildProtocolItems(
  std::vector<Item>& protocolItems,
  const std::vector<Record<data::Item>>& items)
{
  for (const auto& item : items)
  {
    auto& protocolItem = protocolItems.emplace_back();
    item.Immutable([&protocolItem](const auto& item)
    {
      BuildProtocolItem(protocolItem, item);
    });
  }
}

void BuildProtocolStoredItem(StoredItem& protocolStoredItem, const data::StoredItem& storedItem)
{
  protocolStoredItem.uid = storedItem.uid();
  protocolStoredItem.sender = storedItem.sender();
  protocolStoredItem.message = storedItem.message();
}

void BuildProtocolStoredItems(
  std::vector<StoredItem>& protocolStoredItems,
  const std::span<const Record<data::StoredItem>>& storedItems)
{
  for (const auto& storedItem : storedItems)
  {
    auto& protocolStoredItem = protocolStoredItems.emplace_back();
    storedItem.Immutable([&protocolStoredItem](const auto& storedItem)
    {
      BuildProtocolStoredItem(protocolStoredItem, storedItem);
    });
  }
}

} // namespace protocol

} // namespace server
