create schema if not exists "data";

-- horses and items share the equipment identifier namespace.
create sequence "data"."equipmentUidSeq" as bigint;

create table "data"."users"
(
    "name" text not null primary key,
    "token" text not null default '',
    "characterUid" bigint not null default 0,
    "lastSeenOnline" bigint not null default 0
);

create sequence "data"."infractionUidSeq" as bigint;

create table "data"."infractions"
(
    "uid" bigint not null default nextval('"data"."infractionUidSeq"') primary key,
    "description" text not null default '',
    "punishment" smallint not null default 0,
    "duration" bigint not null default 0,
    "createdAt" bigint not null default 0
);

alter sequence "data"."infractionUidSeq" owned by "data"."infractions"."uid";

create table "data"."items"
(
    "uid" bigint not null default nextval('"data"."equipmentUidSeq"') primary key,
    "tid" bigint not null default 0,
    "count" bigint not null default 0,
    "duration" bigint not null default 0,
    "createdAt" bigint not null default 0
);

create sequence "data"."storageItemUidSeq" as bigint;

create table "data"."storageItems"
(
    "uid" bigint not null default nextval('"data"."storageItemUidSeq"') primary key,
    "sender" text not null default '',
    "message" text not null default '',
    "carrots" integer not null default 0,
    "checked" boolean not null default false,
    "createdAt" bigint not null default 0,
    "duration" bigint not null default 0,
    "goodsSq" bigint not null default 0,
    "priceId" bigint not null default 0
);

alter sequence "data"."storageItemUidSeq" owned by "data"."storageItems"."uid";

create sequence "data"."eggUidSeq" as bigint;

create table "data"."eggs"
(
    "uid" bigint not null default nextval('"data"."eggUidSeq"') primary key,
    "itemUid" bigint not null default 0,
    "itemTid" bigint not null default 0,
    "incubatedAt" bigint not null default 0,
    "incubatorSlot" bigint not null default 0,
    "boostsUsed" bigint not null default 0
);

alter sequence "data"."eggUidSeq" owned by "data"."eggs"."uid";

create sequence "data"."petUidSeq" as bigint;

create table "data"."pets"
(
    "uid" bigint not null default nextval('"data"."petUidSeq"') primary key,
    "itemUid" bigint not null default 0,
    "petId" bigint not null default 0,
    "name" text not null default '',
    "birthDate" bigint not null default 0
);

alter sequence "data"."petUidSeq" owned by "data"."pets"."uid";

create sequence "data"."housingUidSeq" as bigint;

create table "data"."housing"
(
    "uid" bigint not null default nextval('"data"."housingUidSeq"') primary key,
    "housingId" bigint not null default 0,
    "expiresAt" bigint not null default 0,
    "durability" bigint not null default 0
);

alter sequence "data"."housingUidSeq" owned by "data"."housing"."uid";

create sequence "data"."guildUidSeq" as bigint;

create table "data"."guilds"
(
    "uid" bigint not null default nextval('"data"."guildUidSeq"') primary key,
    "name" text not null default '',
    "description" text not null default '',
    "owner" bigint not null default 0,
    "rank" bigint not null default 0,
    "totalWins" bigint not null default 0,
    "totalLosses" bigint not null default 0,
    "seasonalWins" bigint not null default 0,
    "seasonalLosses" bigint not null default 0
);

alter sequence "data"."guildUidSeq" owned by "data"."guilds"."uid";

create sequence "data"."settingsUidSeq" as bigint;

create table "data"."settings"
(
    "uid" bigint not null default nextval('"data"."settingsUidSeq"') primary key,
    "hasKeyboardBindings" boolean not null default false,
    "hasMacros" boolean not null default false,
    "hasGamepadBindings" boolean not null default false,
    "age" bigint not null default 0,
    "hideAge" boolean not null default true
);

alter sequence "data"."settingsUidSeq" owned by "data"."settings"."uid";

create sequence "data"."dailyQuestGroupUidSeq" as bigint;

create table "data"."dailyQuestGroups"
(
    "uid" bigint not null default nextval('"data"."dailyQuestGroupUidSeq"') primary key,
    "rewardId" bigint not null default 0,
    "rewardType" bigint not null default 0,
    "rewardPoints" bigint not null default 0,
    "carrotsClaimed" boolean not null default false
);

alter sequence "data"."dailyQuestGroupUidSeq" owned by "data"."dailyQuestGroups"."uid";

create sequence "data"."mailUidSeq" as bigint;

create table "data"."mails"
(
    "uid" bigint not null default nextval('"data"."mailUidSeq"') primary key,
    "senderUid" bigint not null default 0,
    "recipientUid" bigint not null default 0,
    "isRead" boolean not null default false,
    "isDeleted" boolean not null default false,
    "type" smallint not null default 0,
    "claimUid" bigint not null default 0,
    "createdAt" bigint not null default 0,
    "body" text not null default ''
);

alter sequence "data"."mailUidSeq" owned by "data"."mails"."uid";

create sequence "data"."questUidSeq" as bigint;

create table "data"."quests"
(
    "uid" bigint not null default nextval('"data"."questUidSeq"') primary key,
    "questId" bigint not null default 0,
    "isCompleted" smallint not null default 0,
    "progress" bigint not null default 0
);

alter sequence "data"."questUidSeq" owned by "data"."quests"."uid";

create sequence "data"."stallionUidSeq" as bigint;

create table "data"."stallions"
(
    "uid" bigint not null default nextval('"data"."stallionUidSeq"') primary key,
    "horseUid" bigint not null default 0,
    "ownerUid" bigint not null default 0,
    "breedingCharge" bigint not null default 0,
    "timesMated" bigint not null default 0,
    "registeredAt" bigint not null default 0,
    "expiresAt" bigint not null default 0
);

alter sequence "data"."stallionUidSeq" owned by "data"."stallions"."uid";

create sequence "data"."rewardUidSeq" as bigint;

create table "data"."rewards"
(
    "claimUid" bigint not null default nextval('"data"."rewardUidSeq"') primary key,
    "characterUid" bigint not null default 0,
    "type" smallint not null default 0,
    "carrots" bigint not null default 0,
    "isClaimed" boolean not null default false,
    "createdAt" bigint not null default 0,
    "claimedAt" bigint not null default 0
);

alter sequence "data"."rewardUidSeq" owned by "data"."rewards"."claimUid";

create sequence "data"."characterUidSeq" as bigint;

create table "data"."characters"
(
    "uid" bigint not null default nextval('"data"."characterUidSeq"') primary key,
    "name" text not null default '',
    "introduction" text not null default '',
    "level" bigint not null default 0,
    "experience" bigint not null default 0,
    "carrots" integer not null default 0,
    "cash" integer not null default 0,
    "role" smallint not null default 0,
    "staffRank" smallint not null default 0,
    "partsModelId" bigint not null default 0,
    "partsMouthId" bigint not null default 0,
    "partsFaceId" bigint not null default 0,
    "appearanceVoiceId" bigint not null default 0,
    "appearanceHeadSize" bigint not null default 0,
    "appearanceHeight" bigint not null default 0,
    "appearanceThighVolume" bigint not null default 0,
    "appearanceLegVolume" bigint not null default 0,
    "appearanceEmblemId" bigint not null default 0,
    "guildUid" bigint not null default 0,
    "horseSlotCount" bigint not null default 0,
    "mountUid" bigint not null default 0,
    "petUid" bigint not null default 0,
    "ranchExperience" bigint not null default 0,
    "totalRaces" bigint not null default 0,
    "isRanchLocked" boolean not null default false,
    "isIntroCompleted" boolean not null default false,
    "settingsUid" bigint not null default 0,
    "dailyQuestGroupUid" bigint not null default 0,
    "mailboxHasNewMail" boolean not null default false
);

alter sequence "data"."characterUidSeq" owned by "data"."characters"."uid";

create table "data"."horses"
(
    "uid" bigint not null default nextval('"data"."equipmentUidSeq"') primary key,
    "tid" bigint not null default 0,
    "name" text not null default '',
    "partsSkinTid" bigint not null default 0,
    "partsFaceTid" bigint not null default 0,
    "partsManeTid" bigint not null default 0,
    "partsTailTid" bigint not null default 0,
    "appearanceScale" bigint not null default 0,
    "appearanceLegLength" bigint not null default 0,
    "appearanceLegVolume" bigint not null default 0,
    "appearanceBodyLength" bigint not null default 0,
    "appearanceBodyVolume" bigint not null default 0,
    "statsAgility" bigint not null default 0,
    "statsCourage" bigint not null default 0,
    "statsRush" bigint not null default 0,
    "statsEndurance" bigint not null default 0,
    "statsAmbition" bigint not null default 0,
    "masterySpurMagicCount" bigint not null default 0,
    "masteryJumpCount" bigint not null default 0,
    "masterySlidingTime" bigint not null default 0,
    "masteryGlidingDistance" bigint not null default 0,
    "rating" bigint not null default 0,
    "clazz" bigint not null default 0,
    "clazzProgress" bigint not null default 0,
    "grade" bigint not null default 0,
    "growthPoints" bigint not null default 0,
    "breedingCount" bigint not null default 0,
    "breedingCombo" bigint not null default 0,
    "type" smallint not null default 0,
    "dateOfBirth" bigint not null default 0,
    "tendency" bigint not null default 0,
    "spirit" bigint not null default 0,
    "potentialType" bigint not null default 0,
    "potentialLevel" bigint not null default 0,
    "potentialValue" bigint not null default 0,
    "luckState" bigint not null default 0,
    "fatigue" bigint not null default 0,
    "emblemUid" bigint not null default 0,
    "mountConditionStamina" bigint not null default 0,
    "mountConditionCharm" bigint not null default 0,
    "mountConditionFriendliness" bigint not null default 0,
    "mountConditionInjury" bigint not null default 0,
    "mountConditionPlenitude" bigint not null default 0,
    "mountConditionBodyDirtiness" bigint not null default 0,
    "mountConditionManeDirtiness" bigint not null default 0,
    "mountConditionTailDirtiness" bigint not null default 0,
    "mountConditionBodyPolish" bigint not null default 0,
    "mountConditionManePolish" bigint not null default 0,
    "mountConditionTailPolish" bigint not null default 0,
    "mountConditionAttachment" bigint not null default 0,
    "mountConditionBoredom" bigint not null default 0,
    "mountConditionLastDailyCareTick" bigint not null default 0,
    "mountConditionStopAmendsPoint" bigint not null default 0,
    "mountInfoBoostsInARow" bigint not null default 0,
    "mountInfoWinsSpeedSingle" bigint not null default 0,
    "mountInfoWinsSpeedTeam" bigint not null default 0,
    "mountInfoWinsMagicSingle" bigint not null default 0,
    "mountInfoWinsMagicTeam" bigint not null default 0,
    "mountInfoTotalDistance" bigint not null default 0,
    "mountInfoTopSpeed" bigint not null default 0,
    "mountInfoLongestGlideDistance" bigint not null default 0,
    "mountInfoParticipated" bigint not null default 0,
    "mountInfoCumulativePrize" bigint not null default 0,
    "mountInfoBiggestPrize" bigint not null default 0,
    "ancestorsFather" bigint not null default 0,
    "ancestorsMother" bigint not null default 0,
    "lineage" bigint not null default 1
);

-- empty character and guild names are placeholders while new records are initialized.
create unique index "usersNameUnique" on "data"."users" (lower("name"));
create unique index "charactersNameUnique" on "data"."characters" (lower("name")) where "name" <> '';
create unique index "guildsNameUnique" on "data"."guilds" (lower("name")) where "name" <> '';

create table "data"."userInfractions"
(
    "userName" text not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("userName", "position"),
    foreign key ("userName")
        references "data"."users" ("name") on delete cascade
);

create table "data"."storageItemContents"
(
    "storageItemUid" bigint not null,
    "position" bigint not null,
    "tid" bigint not null,
    "count" bigint not null,
    "duration" bigint not null,
    primary key ("storageItemUid", "position"),
    foreign key ("storageItemUid")
        references "data"."storageItems" ("uid") on delete cascade
);

create table "data"."guildOfficers"
(
    "guildUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("guildUid", "position"),
    foreign key ("guildUid")
        references "data"."guilds" ("uid") on delete cascade
);

create table "data"."guildMembers"
(
    "guildUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("guildUid", "position"),
    foreign key ("guildUid")
        references "data"."guilds" ("uid") on delete cascade
);

create table "data"."settingsKeyboardBindings"
(
    "settingsUid" bigint not null,
    "position" bigint not null,
    "primaryKey" bigint not null,
    "type" bigint not null,
    "secondaryKey" bigint not null,
    primary key ("settingsUid", "position"),
    foreign key ("settingsUid")
        references "data"."settings" ("uid") on delete cascade
);

create table "data"."settingsGamepadBindings"
(
    "settingsUid" bigint not null,
    "position" bigint not null,
    "primaryKey" bigint not null,
    "type" bigint not null,
    "secondaryKey" bigint not null,
    primary key ("settingsUid", "position"),
    foreign key ("settingsUid")
        references "data"."settings" ("uid") on delete cascade
);

create table "data"."settingsMacros"
(
    "settingsUid" bigint not null,
    "position" bigint not null,
    "value" text not null,
    primary key ("settingsUid", "position"),
    foreign key ("settingsUid")
        references "data"."settings" ("uid") on delete cascade
);

create table "data"."dailyQuestEntries"
(
    "dailyQuestGroupUid" bigint not null,
    "position" bigint not null,
    "questId" bigint not null,
    "progress" bigint not null,
    primary key ("dailyQuestGroupUid", "position"),
    foreign key ("dailyQuestGroupUid")
        references "data"."dailyQuestGroups" ("uid") on delete cascade
);

create table "data"."characterContactsPending"
(
    "characterUid" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "valueUid"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterBreedingWishlist"
(
    "characterUid" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "valueUid"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterGifts"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterPurchases"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterInventory"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterEquipment"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterHorses"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterPets"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterEggs"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterHousing"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterMailboxInbox"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterMailboxSent"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterQuests"
(
    "characterUid" bigint not null,
    "position" bigint not null,
    "valueUid" bigint not null,
    primary key ("characterUid", "position"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterContactGroups"
(
    "characterUid" bigint not null,
    "groupKey" bigint not null,
    "uid" bigint not null,
    "name" text not null,
    "createdAt" bigint not null,
    primary key ("characterUid", "groupKey"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterContactGroupMembers"
(
    "characterUid" bigint not null,
    "groupKey" bigint not null,
    "memberUid" bigint not null,
    primary key ("characterUid", "groupKey", "memberUid"),
    foreign key ("characterUid", "groupKey")
        references "data"."characterContactGroups" ("characterUid", "groupKey") on delete cascade
);

create table "data"."characterSkills"
(
    "characterUid" bigint not null,
    "mode" text not null,
    "activeSetId" bigint not null,
    "set1Slot1" bigint not null,
    "set1Slot2" bigint not null,
    "set2Slot1" bigint not null,
    "set2Slot2" bigint not null,
    primary key ("characterUid", "mode"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterMissions"
(
    "characterUid" bigint not null,
    "missionKey" bigint not null,
    "id" bigint not null,
    primary key ("characterUid", "missionKey"),
    foreign key ("characterUid")
        references "data"."characters" ("uid") on delete cascade
);

create table "data"."characterMissionProgress"
(
    "characterUid" bigint not null,
    "missionKey" bigint not null,
    "position" bigint not null,
    "id" bigint not null,
    "value" bigint not null,
    primary key ("characterUid", "missionKey", "position"),
    foreign key ("characterUid", "missionKey")
        references "data"."characterMissions" ("characterUid", "missionKey") on delete cascade
);
