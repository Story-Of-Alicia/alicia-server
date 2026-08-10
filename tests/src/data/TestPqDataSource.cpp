/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2026 Story Of Alicia
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

//! Round-trips every record through a real Postgres data source, which is the
//! only way to tell that the column lists, the parameter order and the child
//! tables all agree with scripts/schema/001_initial.sql.
//!
//! The connection URI is taken from the ALICIA_TEST_PQ_URI environment
//! variable. It defaults to a scratch database rather than the development one
//! from resources/config/server/config.yaml, because the test writes records
//! and advances sequences, and neither belongs in a database holding a real
//! world. Create it once with:
//!
//!   CREATE DATABASE alicia_test OWNER storyofalicia;
//!
//! The schema does not need applying, the data source creates it on its own.
//! The test reports a skip and succeeds when no database is reachable, so that
//! it does not fail a build on a machine without one.

#include "libserver/data/pq/PqDataSource.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{

using namespace server;

constexpr const char* DefaultUri =
  "postgresql://storyofalicia:dev@localhost:5432/alicia_test";

std::string ResolveUri()
{
  if (const auto* uri = std::getenv("ALICIA_TEST_PQ_URI"))
    return uri;
  return DefaultUri;
}

//! A time point truncated to whole seconds, which is the resolution the data
//! source stores. Comparing against an untruncated `now()` would always fail.
data::Clock::time_point SecondsFromNow(const int64_t offsetSeconds)
{
  const auto now = std::chrono::floor<std::chrono::seconds>(
    data::Clock::now().time_since_epoch());
  return data::Clock::time_point(now + std::chrono::seconds(offsetSeconds));
}

void TestUser(PqDataSource& dataSource)
{
  data::User stored;
  stored.name = std::string("test_user_roundtrip");
  stored.characterUid = 4242u;
  stored.lastSeenOnline = SecondsFromNow(-90);
  stored.infractions = std::vector<data::Uid>{7, 3, 11};

  dataSource.StoreUser(stored.name(), stored);

  data::User loaded;
  dataSource.RetrieveUser(stored.name(), loaded);

  assert(loaded.name() == stored.name());
  assert(loaded.characterUid() == stored.characterUid());
  assert(loaded.lastSeenOnline() == stored.lastSeenOnline());
  // The infraction list is ordered, so the order has to survive the round trip.
  assert(loaded.infractions() == stored.infractions());

  // The uniqueness check has to ignore the case of the name.
  assert(not dataSource.IsUserNameUnique("TEST_USER_ROUNDTRIP"));
  assert(dataSource.IsUserNameUnique("test_user_roundtrip_other"));

  // A later sighting has to replace the stored one.
  stored.lastSeenOnline = SecondsFromNow(0);
  dataSource.StoreUser(stored.name(), stored);

  data::User reloaded;
  dataSource.RetrieveUser(stored.name(), reloaded);
  assert(reloaded.lastSeenOnline() == stored.lastSeenOnline());
  assert(reloaded.characterUid() == stored.characterUid());
  assert(reloaded.infractions() == stored.infractions());

  std::cout << "  user ok\n";
}

void TestEquipmentSequenceIsShared(PqDataSource& dataSource)
{
  // Items and horses drew from one counter in the file data source, so their
  // UIDs have never collided. That has to keep holding.
  data::Item firstItem;
  data::Horse horse;
  data::Item secondItem;

  dataSource.CreateItem(firstItem);
  dataSource.CreateHorse(horse);
  dataSource.CreateItem(secondItem);

  assert(firstItem.uid() != horse.uid());
  assert(secondItem.uid() != horse.uid());
  assert(firstItem.uid() != secondItem.uid());

  std::cout << "  shared equipment sequence ok\n";
}

void TestHorse(PqDataSource& dataSource)
{
  data::Horse stored;
  dataSource.CreateHorse(stored);

  // Distinct values in every column, so that a parameter appended in the wrong
  // order cannot pass by coincidence.
  uint32_t value = 1;
  const auto next = [&value]() { return value++; };

  stored.tid = next();
  stored.name = std::string("test_horse");
  stored.parts.skinTid = next();
  stored.parts.faceTid = next();
  stored.parts.maneTid = next();
  stored.parts.tailTid = next();
  stored.appearance.scale = next();
  stored.appearance.legLength = next();
  stored.appearance.legVolume = next();
  stored.appearance.bodyLength = next();
  stored.appearance.bodyVolume = next();
  stored.stats.agility = next();
  stored.stats.courage = next();
  stored.stats.rush = next();
  stored.stats.endurance = next();
  stored.stats.ambition = next();
  stored.mastery.spurMagicCount = next();
  stored.mastery.jumpCount = next();
  stored.mastery.slidingTime = next();
  stored.mastery.glidingDistance = next();
  stored.rating = next();
  stored.clazz = next();
  stored.clazzProgress = next();
  stored.grade = next();
  stored.growthPoints = next();
  stored.breedingCount = next();
  stored.breedingCombo = next();
  stored.type = data::Horse::Type::Stallion;
  stored.dateOfBirth = SecondsFromNow(-100000);
  stored.tendency = next();
  stored.spirit = next();
  stored.potential.type = next();
  stored.potential.level = next();
  stored.potential.value = next();
  stored.luckState = next();
  stored.fatigue = next();
  stored.emblemUid = next();
  stored.mountCondition.stamina = next();
  stored.mountCondition.charm = next();
  stored.mountCondition.friendliness = next();
  stored.mountCondition.injury = next();
  stored.mountCondition.plenitude = next();
  stored.mountCondition.bodyDirtiness = next();
  stored.mountCondition.maneDirtiness = next();
  stored.mountCondition.tailDirtiness = next();
  stored.mountCondition.bodyPolish = next();
  stored.mountCondition.manePolish = next();
  stored.mountCondition.tailPolish = next();
  stored.mountCondition.attachment = next();
  stored.mountCondition.boredom = next();
  stored.mountCondition.stopAmendsPoint = next();
  stored.mountInfo.boostsInARow = next();
  stored.mountInfo.winsSpeedSingle = next();
  stored.mountInfo.winsSpeedTeam = next();
  stored.mountInfo.winsMagicSingle = next();
  stored.mountInfo.winsMagicTeam = next();
  stored.mountInfo.totalDistance = next();
  stored.mountInfo.topSpeed = next();
  stored.mountInfo.longestGlideDistance = next();
  stored.mountInfo.participated = next();
  stored.mountInfo.cumulativePrize = next();
  stored.mountInfo.biggestPrize = next();
  stored.ancestors.father = next();
  stored.ancestors.mother = next();
  stored.lineage = next();

  dataSource.StoreHorse(stored.uid(), stored);

  data::Horse loaded;
  dataSource.RetrieveHorse(stored.uid(), loaded);

  assert(loaded.uid() == stored.uid());
  assert(loaded.tid() == stored.tid());
  assert(loaded.name() == stored.name());
  assert(loaded.parts.skinTid() == stored.parts.skinTid());
  assert(loaded.parts.faceTid() == stored.parts.faceTid());
  assert(loaded.parts.maneTid() == stored.parts.maneTid());
  assert(loaded.parts.tailTid() == stored.parts.tailTid());
  assert(loaded.appearance.scale() == stored.appearance.scale());
  assert(loaded.appearance.legLength() == stored.appearance.legLength());
  assert(loaded.appearance.legVolume() == stored.appearance.legVolume());
  assert(loaded.appearance.bodyLength() == stored.appearance.bodyLength());
  assert(loaded.appearance.bodyVolume() == stored.appearance.bodyVolume());
  assert(loaded.stats.agility() == stored.stats.agility());
  assert(loaded.stats.courage() == stored.stats.courage());
  assert(loaded.stats.rush() == stored.stats.rush());
  assert(loaded.stats.endurance() == stored.stats.endurance());
  assert(loaded.stats.ambition() == stored.stats.ambition());
  assert(loaded.mastery.spurMagicCount() == stored.mastery.spurMagicCount());
  assert(loaded.mastery.jumpCount() == stored.mastery.jumpCount());
  assert(loaded.mastery.slidingTime() == stored.mastery.slidingTime());
  assert(loaded.mastery.glidingDistance() == stored.mastery.glidingDistance());
  assert(loaded.rating() == stored.rating());
  assert(loaded.clazz() == stored.clazz());
  assert(loaded.clazzProgress() == stored.clazzProgress());
  assert(loaded.grade() == stored.grade());
  assert(loaded.growthPoints() == stored.growthPoints());
  assert(loaded.breedingCount() == stored.breedingCount());
  assert(loaded.breedingCombo() == stored.breedingCombo());
  assert(loaded.type() == stored.type());
  assert(loaded.dateOfBirth() == stored.dateOfBirth());
  assert(loaded.tendency() == stored.tendency());
  assert(loaded.spirit() == stored.spirit());
  assert(loaded.potential.type() == stored.potential.type());
  assert(loaded.potential.level() == stored.potential.level());
  assert(loaded.potential.value() == stored.potential.value());
  assert(loaded.luckState() == stored.luckState());
  assert(loaded.fatigue() == stored.fatigue());
  assert(loaded.emblemUid() == stored.emblemUid());
  assert(loaded.mountCondition.stamina() == stored.mountCondition.stamina());
  assert(loaded.mountCondition.charm() == stored.mountCondition.charm());
  assert(loaded.mountCondition.friendliness() == stored.mountCondition.friendliness());
  assert(loaded.mountCondition.injury() == stored.mountCondition.injury());
  assert(loaded.mountCondition.plenitude() == stored.mountCondition.plenitude());
  assert(loaded.mountCondition.bodyDirtiness() == stored.mountCondition.bodyDirtiness());
  assert(loaded.mountCondition.maneDirtiness() == stored.mountCondition.maneDirtiness());
  assert(loaded.mountCondition.tailDirtiness() == stored.mountCondition.tailDirtiness());
  assert(loaded.mountCondition.bodyPolish() == stored.mountCondition.bodyPolish());
  assert(loaded.mountCondition.manePolish() == stored.mountCondition.manePolish());
  assert(loaded.mountCondition.tailPolish() == stored.mountCondition.tailPolish());
  assert(loaded.mountCondition.attachment() == stored.mountCondition.attachment());
  assert(loaded.mountCondition.boredom() == stored.mountCondition.boredom());
  assert(loaded.mountCondition.stopAmendsPoint() == stored.mountCondition.stopAmendsPoint());
  assert(loaded.mountInfo.boostsInARow() == stored.mountInfo.boostsInARow());
  assert(loaded.mountInfo.winsSpeedSingle() == stored.mountInfo.winsSpeedSingle());
  assert(loaded.mountInfo.winsSpeedTeam() == stored.mountInfo.winsSpeedTeam());
  assert(loaded.mountInfo.winsMagicSingle() == stored.mountInfo.winsMagicSingle());
  assert(loaded.mountInfo.winsMagicTeam() == stored.mountInfo.winsMagicTeam());
  assert(loaded.mountInfo.totalDistance() == stored.mountInfo.totalDistance());
  assert(loaded.mountInfo.topSpeed() == stored.mountInfo.topSpeed());
  assert(loaded.mountInfo.longestGlideDistance() == stored.mountInfo.longestGlideDistance());
  assert(loaded.mountInfo.participated() == stored.mountInfo.participated());
  assert(loaded.mountInfo.cumulativePrize() == stored.mountInfo.cumulativePrize());
  assert(loaded.mountInfo.biggestPrize() == stored.mountInfo.biggestPrize());
  assert(loaded.ancestors.father == stored.ancestors.father);
  assert(loaded.ancestors.mother == stored.ancestors.mother);
  assert(loaded.lineage() == stored.lineage());

  dataSource.DeleteHorse(stored.uid());

  std::cout << "  horse ok (66 columns)\n";
}

void TestCharacter(PqDataSource& dataSource)
{
  data::Character stored;
  dataSource.CreateCharacter(stored);

  stored.name = std::string("test_character_") + std::to_string(stored.uid());
  stored.introduction = std::string("an introduction");
  stored.level = 42u;
  stored.experience = 123456u;
  stored.carrots = -50;
  stored.cash = 900;
  stored.role = data::Character::Role::GameMaster;
  stored.roleRank = data::Character::RoleRank::Moderator;
  stored.parts.modelId = 1u;
  stored.parts.mouthId = 2u;
  stored.parts.faceId = 3u;
  stored.appearance.voiceId = 4u;
  stored.appearance.headSize = 5u;
  stored.appearance.height = 6u;
  stored.appearance.thighVolume = 7u;
  stored.appearance.legVolume = 8u;
  stored.appearance.emblemId = 9u;
  stored.guildUid = 11u;
  stored.horseSlotCount = uint8_t{5};
  stored.mountUid = 12u;
  stored.petUid = 13u;
  stored.isRanchLocked = true;
  stored.settingsUid = 14u;
  stored.dailyQuestGroupUid = 15u;
  stored.skills.speed().set1 = {.slot1 = 21, .slot2 = 22};
  stored.skills.speed().set2 = {.slot1 = 23, .slot2 = 24};
  stored.skills.speed().activeSetId = 1;
  stored.skills.magic().set1 = {.slot1 = 31, .slot2 = 32};
  stored.skills.magic().set2 = {.slot1 = 33, .slot2 = 34};
  stored.skills.magic().activeSetId = 2;
  stored.mailbox.hasNewMail = true;

  // Every list gets distinct contents, so that a list written to the wrong
  // discriminator shows up as a mismatch rather than passing silently.
  stored.inventory = std::vector<data::Uid>{101, 102, 103};
  stored.characterEquipment = std::vector<data::Uid>{201, 202};
  stored.gifts = std::vector<data::Uid>{401, 402};
  stored.purchases = std::vector<data::Uid>{501};
  stored.horses = std::vector<data::Uid>{601, 602, 603};
  stored.pets = std::vector<data::Uid>{701};
  stored.eggs = std::vector<data::Uid>{801, 802};
  stored.housing = std::vector<data::Uid>{901};
  stored.quests = std::vector<data::Uid>{1001, 1002};
  stored.mailbox.inbox = std::vector<data::Uid>{1101, 1102};
  stored.mailbox.sent = std::vector<data::Uid>{1201};
  stored.breedingWishlist = std::set<data::Uid>{1301, 1302};
  stored.contacts.pending = std::set<data::Uid>{1401};

  std::map<data::Uid, data::Character::Contacts::Group> groups;
  groups.try_emplace(
    0u,
    data::Character::Contacts::Group{
      .uid = 0u,
      .name = "",
      .members = {1501, 1502},
      .createdAt = SecondsFromNow(-500)});
  groups.try_emplace(
    5u,
    data::Character::Contacts::Group{
      .uid = 5u,
      .name = "rivals",
      .members = {1601},
      .createdAt = SecondsFromNow(-250)});
  stored.contacts.groups = std::move(groups);

  dataSource.StoreCharacter(stored.uid(), stored);

  data::Character loaded;
  dataSource.RetrieveCharacter(stored.uid(), loaded);

  assert(loaded.uid() == stored.uid());
  assert(loaded.name() == stored.name());
  assert(loaded.introduction() == stored.introduction());
  assert(loaded.level() == stored.level());
  assert(loaded.experience() == stored.experience());
  assert(loaded.carrots() == stored.carrots());
  assert(loaded.cash() == stored.cash());
  assert(loaded.role() == stored.role());
  assert(loaded.roleRank() == stored.roleRank());
  assert(loaded.parts.modelId() == stored.parts.modelId());
  assert(loaded.parts.mouthId() == stored.parts.mouthId());
  assert(loaded.parts.faceId() == stored.parts.faceId());
  assert(loaded.appearance.voiceId() == stored.appearance.voiceId());
  assert(loaded.appearance.headSize() == stored.appearance.headSize());
  assert(loaded.appearance.height() == stored.appearance.height());
  assert(loaded.appearance.thighVolume() == stored.appearance.thighVolume());
  assert(loaded.appearance.legVolume() == stored.appearance.legVolume());
  assert(loaded.appearance.emblemId() == stored.appearance.emblemId());
  assert(loaded.guildUid() == stored.guildUid());
  assert(loaded.horseSlotCount() == stored.horseSlotCount());
  assert(loaded.mountUid() == stored.mountUid());
  assert(loaded.petUid() == stored.petUid());
  assert(loaded.isRanchLocked() == stored.isRanchLocked());
  assert(loaded.settingsUid() == stored.settingsUid());
  assert(loaded.dailyQuestGroupUid() == stored.dailyQuestGroupUid());

  assert(loaded.skills.speed().set1.slot1 == stored.skills.speed().set1.slot1);
  assert(loaded.skills.speed().set1.slot2 == stored.skills.speed().set1.slot2);
  assert(loaded.skills.speed().set2.slot1 == stored.skills.speed().set2.slot1);
  assert(loaded.skills.speed().set2.slot2 == stored.skills.speed().set2.slot2);
  assert(loaded.skills.speed().activeSetId == stored.skills.speed().activeSetId);
  assert(loaded.skills.magic().set1.slot1 == stored.skills.magic().set1.slot1);
  assert(loaded.skills.magic().set1.slot2 == stored.skills.magic().set1.slot2);
  assert(loaded.skills.magic().set2.slot1 == stored.skills.magic().set2.slot1);
  assert(loaded.skills.magic().set2.slot2 == stored.skills.magic().set2.slot2);
  assert(loaded.skills.magic().activeSetId == stored.skills.magic().activeSetId);

  assert(loaded.mailbox.hasNewMail() == stored.mailbox.hasNewMail());

  assert(loaded.inventory() == stored.inventory());
  assert(loaded.characterEquipment() == stored.characterEquipment());
  assert(loaded.gifts() == stored.gifts());
  assert(loaded.purchases() == stored.purchases());
  assert(loaded.horses() == stored.horses());
  assert(loaded.pets() == stored.pets());
  assert(loaded.eggs() == stored.eggs());
  assert(loaded.housing() == stored.housing());
  assert(loaded.quests() == stored.quests());
  assert(loaded.mailbox.inbox() == stored.mailbox.inbox());
  assert(loaded.mailbox.sent() == stored.mailbox.sent());
  assert(loaded.breedingWishlist() == stored.breedingWishlist());
  assert(loaded.contacts.pending() == stored.contacts.pending());

  assert(loaded.contacts.groups().size() == 2);
  // Group 0 is the default group created at character creation, and is a real
  // group rather than a missing value.
  const auto& defaultGroup = loaded.contacts.groups().at(0u);
  assert(defaultGroup.uid == 0u);
  assert(defaultGroup.name.empty());
  assert((defaultGroup.members == std::set<data::Uid>{1501, 1502}));
  assert(defaultGroup.createdAt == stored.contacts.groups().at(0u).createdAt);

  const auto& rivalsGroup = loaded.contacts.groups().at(5u);
  assert(rivalsGroup.uid == 5u);
  assert(rivalsGroup.name == "rivals");
  assert((rivalsGroup.members == std::set<data::Uid>{1601}));

  // A second store has to replace the children rather than duplicate them.
  data::Character reordered;
  dataSource.RetrieveCharacter(stored.uid(), reordered);
  reordered.horses = std::vector<data::Uid>{603, 601};
  dataSource.StoreCharacter(stored.uid(), reordered);

  data::Character afterRestore;
  dataSource.RetrieveCharacter(stored.uid(), afterRestore);
  assert((afterRestore.horses() == std::vector<data::Uid>{603, 601}));

  assert(dataSource.RetrieveCharacterUidByName(stored.name()) == stored.uid());
  assert(dataSource.RetrieveCharacterUidByName("no_such_character") == data::InvalidUid);
  assert(not dataSource.IsCharacterNameUnique(stored.name()));

  // Deleting the character has to take every child row with it.
  dataSource.DeleteCharacter(stored.uid());
  assert(dataSource.RetrieveCharacterUidByName(stored.name()) == data::InvalidUid);

  std::cout << "  character ok (36 columns, 12 child tables)\n";
}

void TestSettingsOptionals(PqDataSource& dataSource)
{
  data::Settings stored;
  dataSource.CreateSettings(stored);

  stored.age = 21u;
  stored.hideAge = false;

  // Absent, so the client is told the section was never configured.
  stored.keyboardBindings().reset();
  // Present but empty, which is a different thing entirely.
  stored.gamepadBindings().emplace();
  stored.macros().emplace() = std::array<std::string, 8>{
    "a", "b", "", "d", "", "", "", "h"};

  dataSource.StoreSettings(stored.uid(), stored);

  data::Settings loaded;
  dataSource.RetrieveSettings(stored.uid(), loaded);

  assert(loaded.age() == stored.age());
  assert(loaded.hideAge() == stored.hideAge());

  // This is the distinction the presence flags exist for.
  assert(not loaded.keyboardBindings().has_value());
  assert(loaded.gamepadBindings().has_value());
  assert(loaded.gamepadBindings()->empty());
  assert(loaded.macros().has_value());
  assert(loaded.macros().value() == stored.macros().value());

  // Populating the previously absent bindings has to engage the optional.
  auto& bindings = stored.keyboardBindings().emplace();
  bindings.emplace_back(data::Settings::Option{
    .primaryKey = 1, .type = 2, .secondaryKey = 3});
  bindings.emplace_back(data::Settings::Option{
    .primaryKey = 4, .type = 5, .secondaryKey = 6});
  dataSource.StoreSettings(stored.uid(), stored);

  data::Settings reloaded;
  dataSource.RetrieveSettings(stored.uid(), reloaded);
  assert(reloaded.keyboardBindings().has_value());
  assert(reloaded.keyboardBindings()->size() == 2);
  assert((*reloaded.keyboardBindings())[0].primaryKey == 1);
  assert((*reloaded.keyboardBindings())[0].type == 2);
  assert((*reloaded.keyboardBindings())[0].secondaryKey == 3);
  assert((*reloaded.keyboardBindings())[1].secondaryKey == 6);
  // The gamepad bindings must still be present and still empty.
  assert(reloaded.gamepadBindings().has_value());
  assert(reloaded.gamepadBindings()->empty());

  std::cout << "  settings optionals ok\n";
}

void TestStorageItem(PqDataSource& dataSource)
{
  data::StorageItem stored;
  dataSource.CreateStorageItem(stored);

  stored.sender = std::string("a sender");
  stored.message = std::string("a message");
  stored.carrots = -12;
  stored.checked = true;
  stored.createdAt = SecondsFromNow(-3600);
  stored.duration = std::chrono::seconds(7200);
  stored.goodsSq = 55u;
  stored.priceId = 66u;
  stored.items = std::vector<data::StorageItem::Item>{
    {.tid = 10, .count = 1, .duration = std::chrono::seconds(100)},
    {.tid = 20, .count = 2, .duration = std::chrono::seconds(200)}};

  dataSource.StoreStorageItem(stored.uid(), stored);

  data::StorageItem loaded;
  dataSource.RetrieveStorageItem(stored.uid(), loaded);

  assert(loaded.sender() == stored.sender());
  assert(loaded.message() == stored.message());
  assert(loaded.carrots() == stored.carrots());
  assert(loaded.checked() == stored.checked());
  assert(loaded.createdAt() == stored.createdAt());
  assert(loaded.duration() == stored.duration());
  assert(loaded.goodsSq() == stored.goodsSq());
  assert(loaded.priceId() == stored.priceId());
  assert(loaded.items().size() == 2);
  assert(loaded.items()[0].tid == 10);
  assert(loaded.items()[0].count == 1);
  assert(loaded.items()[0].duration == std::chrono::seconds(100));
  assert(loaded.items()[1].tid == 20);

  std::cout << "  storage item ok\n";
}

void TestDailyQuestGroup(PqDataSource& dataSource)
{
  data::DailyQuestGroup stored;
  dataSource.CreateDailyQuestGroup(stored);

  stored.rewardId = uint8_t{3};
  stored.rewardType = uint8_t{2};
  stored.rewardPoints = 1234u;
  stored.carrotsClaimed = true;
  stored.quests = std::array<data::DailyQuestEntry, 3>{
    data::DailyQuestEntry{.questId = 11, .progress = 1},
    data::DailyQuestEntry{.questId = 22, .progress = 2},
    data::DailyQuestEntry{.questId = 33, .progress = 3}};

  dataSource.StoreDailyQuestGroup(stored.uid(), stored);

  data::DailyQuestGroup loaded;
  dataSource.RetrieveDailyQuestGroup(stored.uid(), loaded);

  assert(loaded.rewardId() == stored.rewardId());
  assert(loaded.rewardType() == stored.rewardType());
  assert(loaded.rewardPoints() == stored.rewardPoints());
  assert(loaded.carrotsClaimed() == stored.carrotsClaimed());
  for (size_t slot = 0; slot < 3; ++slot)
  {
    assert(loaded.quests()[slot].questId == stored.quests()[slot].questId);
    assert(loaded.quests()[slot].progress == stored.quests()[slot].progress);
  }

  std::cout << "  daily quest group ok\n";
}

void TestGuild(PqDataSource& dataSource)
{
  data::Guild stored;
  dataSource.CreateGuild(stored);

  stored.name = std::string("test_guild_") + std::to_string(stored.uid());
  stored.description = std::string("a description");
  stored.owner = 77u;
  stored.rank = 1u;
  stored.totalWins = 2u;
  stored.totalLosses = 3u;
  stored.seasonalWins = 4u;
  stored.seasonalLosses = 5u;
  stored.officers = std::vector<data::Uid>{81, 82};
  stored.members = std::vector<data::Uid>{91, 92, 93};

  dataSource.StoreGuild(stored.uid(), stored);

  data::Guild loaded;
  dataSource.RetrieveGuild(stored.uid(), loaded);

  assert(loaded.name() == stored.name());
  assert(loaded.description() == stored.description());
  assert(loaded.owner() == stored.owner());
  assert(loaded.rank() == stored.rank());
  assert(loaded.totalWins() == stored.totalWins());
  assert(loaded.totalLosses() == stored.totalLosses());
  assert(loaded.seasonalWins() == stored.seasonalWins());
  assert(loaded.seasonalLosses() == stored.seasonalLosses());
  assert(loaded.officers() == stored.officers());
  assert(loaded.members() == stored.members());
  assert(not dataSource.IsGuildNameUnique(stored.name()));

  dataSource.DeleteGuild(stored.uid());

  std::cout << "  guild ok\n";
}

void TestLeafRecords(PqDataSource& dataSource)
{
  {
    data::Item stored;
    dataSource.CreateItem(stored);
    stored.tid = 5u;
    stored.count = 9u;
    stored.duration = std::chrono::seconds(60);
    stored.createdAt = SecondsFromNow(-10);
    dataSource.StoreItem(stored.uid(), stored);

    data::Item loaded;
    dataSource.RetrieveItem(stored.uid(), loaded);
    assert(loaded.tid() == stored.tid());
    assert(loaded.count() == stored.count());
    assert(loaded.duration() == stored.duration());
    assert(loaded.createdAt() == stored.createdAt());

    dataSource.DeleteItem(stored.uid());

    // A retrieval of a missing record has to throw, because that is what makes
    // the data director count the retrieval as failed.
    bool threw = false;
    try
    {
      data::Item missing;
      dataSource.RetrieveItem(stored.uid(), missing);
    }
    catch (const std::exception&)
    {
      threw = true;
    }
    assert(threw);
  }

  {
    data::Mail stored;
    dataSource.CreateMail(stored);
    stored.from = 1u;
    stored.to = 2u;
    stored.isRead = true;
    stored.isDeleted = false;
    stored.type = data::Mail::MailType::BreedingReward;
    stored.claimUid = 3u;
    stored.createdAt = SecondsFromNow(-20);
    stored.body = std::string("a body");
    dataSource.StoreMail(stored.uid(), stored);

    data::Mail loaded;
    dataSource.RetrieveMail(stored.uid(), loaded);
    assert(loaded.from() == stored.from());
    assert(loaded.to() == stored.to());
    assert(loaded.isRead() == stored.isRead());
    assert(loaded.isDeleted() == stored.isDeleted());
    assert(loaded.type() == stored.type());
    assert(loaded.claimUid() == stored.claimUid());
    assert(loaded.createdAt() == stored.createdAt());
    assert(loaded.body() == stored.body());
  }

  {
    data::Stallion stored;
    dataSource.CreateStallion(stored);
    stored.horseUid = 4u;
    stored.ownerUid = 5u;
    stored.breedingCharge = 600u;
    stored.timesMated = 2u;
    stored.registeredAt = SecondsFromNow(-30);
    stored.expiresAt = SecondsFromNow(3000);
    dataSource.StoreStallion(stored.uid(), stored);

    data::Stallion loaded;
    dataSource.RetrieveStallion(stored.uid(), loaded);
    assert(loaded.horseUid() == stored.horseUid());
    assert(loaded.breedingCharge() == stored.breedingCharge());
    assert(loaded.registeredAt() == stored.registeredAt());
    assert(loaded.expiresAt() == stored.expiresAt());

    const auto registered = dataSource.ListRegisteredStallions();
    assert(std::ranges::find(registered, stored.uid()) != registered.end());

    // Removed again, because the server scans the registered stallions during
    // startup and would report the test's dangling horse and owner as errors.
    dataSource.DeleteStallion(stored.uid());
  }

  {
    data::Reward stored;
    dataSource.CreateReward(stored);
    stored.characterUid = 6u;
    stored.type = data::Reward::Type::Carnival;
    stored.carrots = 700u;
    stored.isClaimed = true;
    stored.createdAt = SecondsFromNow(-40);
    stored.claimedAt = SecondsFromNow(-35);
    dataSource.StoreReward(stored.claimUid(), stored);

    data::Reward loaded;
    dataSource.RetrieveReward(stored.claimUid(), loaded);
    assert(loaded.characterUid() == stored.characterUid());
    assert(loaded.type() == stored.type());
    assert(loaded.carrots() == stored.carrots());
    assert(loaded.isClaimed() == stored.isClaimed());
    assert(loaded.createdAt() == stored.createdAt());
    assert(loaded.claimedAt() == stored.claimedAt());

    dataSource.DeleteReward(stored.claimUid());
  }

  {
    data::Infraction stored;
    dataSource.CreateInfraction(stored);
    stored.description = std::string("a description");
    stored.punishment = data::Infraction::Punishment::Ban;
    stored.duration = std::chrono::seconds(86400);
    stored.createdAt = SecondsFromNow(-50);
    dataSource.StoreInfraction(stored.uid(), stored);

    data::Infraction loaded;
    dataSource.RetrieveInfraction(stored.uid(), loaded);
    assert(loaded.description() == stored.description());
    assert(loaded.punishment() == stored.punishment());
    assert(loaded.duration() == stored.duration());
    assert(loaded.createdAt() == stored.createdAt());
  }

  {
    data::Pet stored;
    dataSource.CreatePet(stored);
    stored.itemUid = 7u;
    stored.petId = 8u;
    stored.name = std::string("a pet");
    stored.birthDate = SecondsFromNow(-60);
    dataSource.StorePet(stored.uid(), stored);

    data::Pet loaded;
    dataSource.RetrievePet(stored.uid(), loaded);
    assert(loaded.itemUid() == stored.itemUid());
    assert(loaded.petId() == stored.petId());
    assert(loaded.name() == stored.name());
    assert(loaded.birthDate() == stored.birthDate());
  }

  {
    data::Egg stored;
    dataSource.CreateEgg(stored);
    stored.itemUid = 9u;
    stored.itemTid = 10u;
    stored.incubatedAt = SecondsFromNow(-70);
    stored.incubatorSlot = 2u;
    stored.boostsUsed = 3u;
    dataSource.StoreEgg(stored.uid(), stored);

    data::Egg loaded;
    dataSource.RetrieveEgg(stored.uid(), loaded);
    assert(loaded.itemUid() == stored.itemUid());
    assert(loaded.itemTid() == stored.itemTid());
    assert(loaded.incubatedAt() == stored.incubatedAt());
    assert(loaded.incubatorSlot() == stored.incubatorSlot());
    assert(loaded.boostsUsed() == stored.boostsUsed());
  }

  {
    data::Housing stored;
    dataSource.CreateHousing(stored);
    stored.housingId = 11u;
    stored.expiresAt = SecondsFromNow(-80);
    stored.durability = 12u;
    dataSource.StoreHousing(stored.uid(), stored);

    data::Housing loaded;
    dataSource.RetrieveHousing(stored.uid(), loaded);
    assert(loaded.housingId() == stored.housingId());
    assert(loaded.expiresAt() == stored.expiresAt());
    assert(loaded.durability() == stored.durability());
  }

  {
    data::Quest stored;
    dataSource.CreateQuest(stored);
    stored.questId = 13u;
    stored.isCompleted = data::Quest::Status::Completed;
    stored.progress = 14u;
    dataSource.StoreQuest(stored.uid(), stored);

    data::Quest loaded;
    dataSource.RetrieveQuest(stored.uid(), loaded);
    assert(loaded.questId() == stored.questId());
    assert(loaded.isCompleted() == stored.isCompleted());
    assert(loaded.progress() == stored.progress());
  }

  std::cout << "  leaf records ok\n";
}

} // anon namespace

int main()
{
  const auto uri = ResolveUri();

  PqDataSource dataSource;
  try
  {
    dataSource.Initialize(uri);
  }
  catch (const std::exception& x)
  {
    std::cout
      << "SKIP: no reachable database at the test URI (" << x.what() << ")\n"
      << "Apply scripts/schema/001_initial.sql and set ALICIA_TEST_PQ_URI to run.\n";
    return 0;
  }

  std::cout << "Running Postgres data source round trips\n";

  TestUser(dataSource);
  TestEquipmentSequenceIsShared(dataSource);
  TestLeafRecords(dataSource);
  TestStorageItem(dataSource);
  TestDailyQuestGroup(dataSource);
  TestSettingsOptionals(dataSource);
  TestGuild(dataSource);
  TestHorse(dataSource);
  TestCharacter(dataSource);

  dataSource.Terminate();

  std::cout << "All Postgres data source round trips passed\n";
  return 0;
}
