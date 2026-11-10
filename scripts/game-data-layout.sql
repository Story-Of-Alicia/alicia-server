create schema if not exists "data";

-- the persistence layer only uses 32-bit integers,
-- unsigned values are stored in this domain to keep them in the range of `uint32_t`.
-- time points and durations are stored as 64-bit count of seconds.
create domain "data"."uint32" as bigint
    check (value between 0 and 4294967295);

-- references to other records are nullable foreign keys,
-- an invalid UID (zero) is stored as null.

-- horses and items share the equipment identifier namespace.
create sequence "data"."equipmentUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."user"
(
    "name" text not null primary key,
    "token" text not null default '',
    "characterUid" "data"."uint32",
    "lastSeenOnline" bigint not null default 0
);

create sequence "data"."infractionUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."infraction"
(
    "uid" "data"."uint32" not null default nextval('"data"."infractionUidSeq"') primary key,
    "description" text not null default '',
    "punishment" integer not null default 0,
    "duration" bigint not null default 0,
    "createdAt" bigint not null default 0
);

alter sequence "data"."infractionUidSeq" owned by "data"."infraction"."uid";

create table "data"."item"
(
    "uid" "data"."uint32" not null default nextval('"data"."equipmentUidSeq"') primary key,
    -- the character owning the item, assigned once and never changed.
    "ownerUid" "data"."uint32",
    "tid" "data"."uint32" not null default 0,
    "count" "data"."uint32" not null default 0,
    "duration" bigint not null default 0,
    "createdAt" bigint not null default 0,
    unique ("uid", "ownerUid")
);

create sequence "data"."storageItemUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."storageItem"
(
    "uid" "data"."uint32" not null default nextval('"data"."storageItemUidSeq"') primary key,
    "sender" text not null default '',
    "message" text not null default '',
    "carrots" integer not null default 0,
    "checked" boolean not null default false,
    "createdAt" bigint not null default 0,
    "duration" bigint not null default 0,
    "goodsSq" "data"."uint32" not null default 0,
    "priceId" "data"."uint32" not null default 0
);

alter sequence "data"."storageItemUidSeq" owned by "data"."storageItem"."uid";

create sequence "data"."eggUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."egg"
(
    "uid" "data"."uint32" not null default nextval('"data"."eggUidSeq"') primary key,
    "itemUid" "data"."uint32",
    -- kept apart from the item, the item does not outlive the incubation.
    "itemTid" "data"."uint32" not null default 0,
    "incubatedAt" bigint not null default 0,
    "incubatorSlot" "data"."uint32" not null default 0,
    "boostsUsed" "data"."uint32" not null default 0
);

alter sequence "data"."eggUidSeq" owned by "data"."egg"."uid";

create sequence "data"."petUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."pet"
(
    "uid" "data"."uint32" not null default nextval('"data"."petUidSeq"') primary key,
    "itemUid" "data"."uint32",
    "petId" "data"."uint32" not null default 0,
    "name" text not null default '',
    "birthDate" bigint not null default 0
);

alter sequence "data"."petUidSeq" owned by "data"."pet"."uid";

create sequence "data"."housingUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."housing"
(
    "uid" "data"."uint32" not null default nextval('"data"."housingUidSeq"') primary key,
    "housingId" "data"."uint32" not null default 0,
    "expiresAt" bigint not null default 0,
    "durability" "data"."uint32" not null default 0
);

alter sequence "data"."housingUidSeq" owned by "data"."housing"."uid";

create sequence "data"."guildUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."guild"
(
    "uid" "data"."uint32" not null default nextval('"data"."guildUidSeq"') primary key,
    "name" text not null default '',
    "description" text not null default '',
    "owner" "data"."uint32",
    "rank" "data"."uint32" not null default 0,
    "totalWins" "data"."uint32" not null default 0,
    "totalLosses" "data"."uint32" not null default 0,
    "seasonalWins" "data"."uint32" not null default 0,
    "seasonalLosses" "data"."uint32" not null default 0
);

alter sequence "data"."guildUidSeq" owned by "data"."guild"."uid";

create sequence "data"."settingsUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."settings"
(
    "uid" "data"."uint32" not null default nextval('"data"."settingsUidSeq"') primary key,
    -- distinguish absent bindings and macros from empty ones.
    "hasKeyboardBindings" boolean not null default false,
    "hasMacros" boolean not null default false,
    "hasGamepadBindings" boolean not null default false,
    "age" "data"."uint32" not null default 0,
    "hideAge" boolean not null default true
);

alter sequence "data"."settingsUidSeq" owned by "data"."settings"."uid";

create sequence "data"."dailyQuestGroupUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."dailyQuestGroup"
(
    "uid" "data"."uint32" not null default nextval('"data"."dailyQuestGroupUidSeq"') primary key,
    "rewardId" "data"."uint32" not null default 0,
    "rewardType" "data"."uint32" not null default 0,
    "rewardPoints" "data"."uint32" not null default 0,
    "carrotsClaimed" boolean not null default false
);

alter sequence "data"."dailyQuestGroupUidSeq" owned by "data"."dailyQuestGroup"."uid";

create sequence "data"."mailUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."mail"
(
    "uid" "data"."uint32" not null default nextval('"data"."mailUidSeq"') primary key,
    "senderUid" "data"."uint32",
    "recipientUid" "data"."uint32",
    "isRead" boolean not null default false,
    "isDeleted" boolean not null default false,
    "type" "data"."uint32" not null default 0,
    "claimUid" "data"."uint32",
    "createdAt" bigint not null default 0,
    "body" text not null default ''
);

alter sequence "data"."mailUidSeq" owned by "data"."mail"."uid";

create sequence "data"."questUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."quest"
(
    "uid" "data"."uint32" not null default nextval('"data"."questUidSeq"') primary key,
    "questId" "data"."uint32" not null default 0,
    "isCompleted" "data"."uint32" not null default 0,
    "progress" "data"."uint32" not null default 0
);

alter sequence "data"."questUidSeq" owned by "data"."quest"."uid";

create sequence "data"."stallionUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."stallion"
(
    "uid" "data"."uint32" not null default nextval('"data"."stallionUidSeq"') primary key,
    "horseUid" "data"."uint32",
    "ownerUid" "data"."uint32",
    "breedingCharge" "data"."uint32" not null default 0,
    "timesMated" "data"."uint32" not null default 0,
    "registeredAt" bigint not null default 0,
    "expiresAt" bigint not null default 0
);

alter sequence "data"."stallionUidSeq" owned by "data"."stallion"."uid";

create sequence "data"."rewardUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."reward"
(
    "claimUid" "data"."uint32" not null default nextval('"data"."rewardUidSeq"') primary key,
    "characterUid" "data"."uint32",
    "type" "data"."uint32" not null default 0,
    "carrots" "data"."uint32" not null default 0,
    "isClaimed" boolean not null default false,
    "createdAt" bigint not null default 0,
    "claimedAt" bigint not null default 0
);

alter sequence "data"."rewardUidSeq" owned by "data"."reward"."claimUid";

create sequence "data"."characterUidSeq" as bigint minvalue 1 maxvalue 4294967295;

create table "data"."character"
(
    "uid" "data"."uint32" not null default nextval('"data"."characterUidSeq"') primary key,
    "name" text not null default '',
    "introduction" text not null default '',
    "level" "data"."uint32" not null default 0,
    "experience" "data"."uint32" not null default 0,
    "carrots" integer not null default 0,
    "cash" integer not null default 0,
    "role" integer not null default 0,
    "staffRank" integer not null default 0,
    "partsModelId" "data"."uint32" not null default 0,
    "partsMouthId" "data"."uint32" not null default 0,
    "partsFaceId" "data"."uint32" not null default 0,
    "appearanceVoiceId" "data"."uint32" not null default 0,
    "appearanceHeadSize" "data"."uint32" not null default 0,
    "appearanceHeight" "data"."uint32" not null default 0,
    "appearanceThighVolume" "data"."uint32" not null default 0,
    "appearanceLegVolume" "data"."uint32" not null default 0,
    "appearanceEmblemId" "data"."uint32" not null default 0,
    "guildUid" "data"."uint32",
    "horseSlotCount" "data"."uint32" not null default 0,
    "mountUid" "data"."uint32",
    "petUid" "data"."uint32",
    "ranchExperience" "data"."uint32" not null default 0,
    "totalRaces" "data"."uint32" not null default 0,
    "isRanchLocked" boolean not null default false,
    "isIntroCompleted" boolean not null default false,
    "settingsUid" "data"."uint32",
    "dailyQuestGroupUid" "data"."uint32",
    "mailboxHasNewMail" boolean not null default false
);

alter sequence "data"."characterUidSeq" owned by "data"."character"."uid";

-- horses are never deleted, they stay as the ancestors of their foals.
create table "data"."horse"
(
    "uid" "data"."uint32" not null default nextval('"data"."equipmentUidSeq"') primary key,
    -- the character owning the horse, assigned once and never changed.
    "ownerUid" "data"."uint32",
    "tid" "data"."uint32" not null default 0,
    "name" text not null default '',
    "partsSkinTid" "data"."uint32" not null default 0,
    "partsFaceTid" "data"."uint32" not null default 0,
    "partsManeTid" "data"."uint32" not null default 0,
    "partsTailTid" "data"."uint32" not null default 0,
    "appearanceScale" "data"."uint32" not null default 0,
    "appearanceLegLength" "data"."uint32" not null default 0,
    "appearanceLegVolume" "data"."uint32" not null default 0,
    "appearanceBodyLength" "data"."uint32" not null default 0,
    "appearanceBodyVolume" "data"."uint32" not null default 0,
    "statsAgility" "data"."uint32" not null default 0,
    "statsCourage" "data"."uint32" not null default 0,
    "statsRush" "data"."uint32" not null default 0,
    "statsEndurance" "data"."uint32" not null default 0,
    "statsAmbition" "data"."uint32" not null default 0,
    "masterySpurMagicCount" "data"."uint32" not null default 0,
    "masteryJumpCount" "data"."uint32" not null default 0,
    "masterySlidingTime" "data"."uint32" not null default 0,
    "masteryGlidingDistance" "data"."uint32" not null default 0,
    "rating" "data"."uint32" not null default 0,
    "clazz" "data"."uint32" not null default 0,
    "clazzProgress" "data"."uint32" not null default 0,
    "grade" "data"."uint32" not null default 0,
    "growthPoints" "data"."uint32" not null default 0,
    "breedingCount" "data"."uint32" not null default 0,
    "breedingCombo" "data"."uint32" not null default 0,
    "type" integer not null default 0,
    "dateOfBirth" bigint not null default 0,
    "tendency" "data"."uint32" not null default 0,
    "spirit" "data"."uint32" not null default 0,
    "potentialType" "data"."uint32" not null default 0,
    "potentialLevel" "data"."uint32" not null default 0,
    "potentialValue" "data"."uint32" not null default 0,
    "luckState" "data"."uint32" not null default 0,
    "fatigue" "data"."uint32" not null default 0,
    -- an emblem identifier, not a reference to a record.
    "emblemUid" "data"."uint32" not null default 0,
    "mountConditionStamina" "data"."uint32" not null default 0,
    "mountConditionCharm" "data"."uint32" not null default 0,
    "mountConditionFriendliness" "data"."uint32" not null default 0,
    "mountConditionInjury" "data"."uint32" not null default 0,
    "mountConditionPlenitude" "data"."uint32" not null default 0,
    "mountConditionBodyDirtiness" "data"."uint32" not null default 0,
    "mountConditionManeDirtiness" "data"."uint32" not null default 0,
    "mountConditionTailDirtiness" "data"."uint32" not null default 0,
    "mountConditionBodyPolish" "data"."uint32" not null default 0,
    "mountConditionManePolish" "data"."uint32" not null default 0,
    "mountConditionTailPolish" "data"."uint32" not null default 0,
    "mountConditionAttachment" "data"."uint32" not null default 0,
    "mountConditionBoredom" "data"."uint32" not null default 0,
    "mountConditionLastDailyCareTick" bigint not null default 0,
    "mountConditionStopAmendsPoint" "data"."uint32" not null default 0,
    "mountInfoBoostsInARow" "data"."uint32" not null default 0,
    "mountInfoWinsSpeedSingle" "data"."uint32" not null default 0,
    "mountInfoWinsSpeedTeam" "data"."uint32" not null default 0,
    "mountInfoWinsMagicSingle" "data"."uint32" not null default 0,
    "mountInfoWinsMagicTeam" "data"."uint32" not null default 0,
    "mountInfoTotalDistance" "data"."uint32" not null default 0,
    "mountInfoTopSpeed" "data"."uint32" not null default 0,
    "mountInfoLongestGlideDistance" "data"."uint32" not null default 0,
    "mountInfoParticipated" "data"."uint32" not null default 0,
    "mountInfoCumulativePrize" "data"."uint32" not null default 0,
    "mountInfoBiggestPrize" "data"."uint32" not null default 0,
    "ancestorsFather" "data"."uint32",
    "ancestorsMother" "data"."uint32",
    "lineage" "data"."uint32" not null default 1,
    unique ("uid", "ownerUid")
);

-- the ownership of items and horses is permanent,
-- they can not be moved from one character to another.
-- the owner is cleared only when the owning character is deleted.
create function "data"."preventOwnerChange"() returns trigger
    language plpgsql as
$$
begin
    if old."ownerUid" is not null and new."ownerUid" is not null
        and new."ownerUid" <> old."ownerUid" then
        raise exception 'owner of % % can not change', tg_table_name, old."uid";
    end if;
    return new;
end;
$$;

create trigger "itemOwnerPermanent"
    before update of "ownerUid" on "data"."item"
    for each row execute function "data"."preventOwnerChange"();

create trigger "horseOwnerPermanent"
    before update of "ownerUid" on "data"."horse"
    for each row execute function "data"."preventOwnerChange"();

create function "data"."preventHorseDelete"() returns trigger
    language plpgsql as
$$
begin
    raise exception 'horse % can not be deleted', old."uid";
end;
$$;

create trigger "horsePermanent"
    before delete on "data"."horse"
    for each row execute function "data"."preventHorseDelete"();

-- user names are case-sensitive and kept unique by the primary key.
-- empty character and guild names are placeholders while new records are initialized.
create unique index "characterNameUnique" on "data"."character" (lower("name")) where "name" <> '';
create unique index "guildNameUnique" on "data"."guild" (lower("name")) where "name" <> '';

-- one-to-one relations.
create unique index "userCharacterUnique" on "data"."user" ("characterUid");
create unique index "characterSettingsUnique" on "data"."character" ("settingsUid");
create unique index "characterDailyQuestGroupUnique" on "data"."character" ("dailyQuestGroupUid");

-- references between the records,
-- the reference is cleared when the referenced record is deleted.
-- horses are never deleted, so the references to horses are never cleared.
alter table "data"."user"
    add foreign key ("characterUid") references "data"."character" ("uid") on delete set null;

alter table "data"."egg"
    add foreign key ("itemUid") references "data"."item" ("uid") on delete set null;

alter table "data"."pet"
    add foreign key ("itemUid") references "data"."item" ("uid") on delete set null;

alter table "data"."guild"
    add foreign key ("owner") references "data"."character" ("uid") on delete set null;

alter table "data"."mail"
    add foreign key ("senderUid") references "data"."character" ("uid") on delete set null,
    add foreign key ("recipientUid") references "data"."character" ("uid") on delete set null,
    add foreign key ("claimUid") references "data"."reward" ("claimUid") on delete set null;

alter table "data"."stallion"
    add foreign key ("horseUid") references "data"."horse" ("uid"),
    add foreign key ("ownerUid") references "data"."character" ("uid") on delete set null;

alter table "data"."reward"
    add foreign key ("characterUid") references "data"."character" ("uid") on delete set null;

alter table "data"."character"
    add foreign key ("guildUid") references "data"."guild" ("uid") on delete set null,
    -- the mount is a horse owned by the character.
    add foreign key ("mountUid", "uid") references "data"."horse" ("uid", "ownerUid"),
    add foreign key ("petUid") references "data"."pet" ("uid") on delete set null,
    add foreign key ("settingsUid") references "data"."settings" ("uid") on delete set null,
    add foreign key ("dailyQuestGroupUid") references "data"."dailyQuestGroup" ("uid") on delete set null;

alter table "data"."horse"
    add foreign key ("ancestorsFather") references "data"."horse" ("uid"),
    add foreign key ("ancestorsMother") references "data"."horse" ("uid");

-- the items are deleted with their owner.
alter table "data"."item"
    add foreign key ("ownerUid") references "data"."character" ("uid") on delete cascade;

-- the horses are unlinked from their owner, as horses are never deleted.
alter table "data"."horse"
    add foreign key ("ownerUid") references "data"."character" ("uid") on delete set null;

-- collections owned by the records,
-- the entry is removed when either the owner or the referenced record is deleted.
create table "data"."userInfraction"
(
    "userName" text not null references "data"."user" ("name") on delete cascade,
    "infractionUid" "data"."uint32" not null references "data"."infraction" ("uid") on delete cascade,
    primary key ("userName", "infractionUid")
);

create table "data"."storageItemContent"
(
    "storageItemUid" "data"."uint32" not null references "data"."storageItem" ("uid") on delete cascade,
    "position" "data"."uint32" not null,
    "tid" "data"."uint32" not null,
    "count" "data"."uint32" not null,
    "duration" bigint not null,
    primary key ("storageItemUid", "position")
);

create table "data"."guildOfficer"
(
    "guildUid" "data"."uint32" not null references "data"."guild" ("uid") on delete cascade,
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    primary key ("guildUid", "characterUid")
);

create table "data"."guildMember"
(
    "guildUid" "data"."uint32" not null references "data"."guild" ("uid") on delete cascade,
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    primary key ("guildUid", "characterUid")
);

create table "data"."settingsKeyboardBinding"
(
    "settingsUid" "data"."uint32" not null references "data"."settings" ("uid") on delete cascade,
    "position" "data"."uint32" not null,
    "primaryKey" "data"."uint32" not null,
    "type" "data"."uint32" not null,
    "secondaryKey" "data"."uint32" not null,
    primary key ("settingsUid", "position")
);

create table "data"."settingsGamepadBinding"
(
    "settingsUid" "data"."uint32" not null references "data"."settings" ("uid") on delete cascade,
    "position" "data"."uint32" not null,
    "primaryKey" "data"."uint32" not null,
    "type" "data"."uint32" not null,
    "secondaryKey" "data"."uint32" not null,
    primary key ("settingsUid", "position")
);

create table "data"."settingsMacro"
(
    "settingsUid" "data"."uint32" not null references "data"."settings" ("uid") on delete cascade,
    "position" "data"."uint32" not null check ("position" < 8),
    "value" text not null,
    primary key ("settingsUid", "position")
);

create table "data"."dailyQuestEntry"
(
    "dailyQuestGroupUid" "data"."uint32" not null references "data"."dailyQuestGroup" ("uid") on delete cascade,
    "position" "data"."uint32" not null check ("position" < 3),
    "questId" "data"."uint32" not null,
    "progress" "data"."uint32" not null,
    primary key ("dailyQuestGroupUid", "position")
);

create table "data"."characterContactPending"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "contactUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    primary key ("characterUid", "contactUid")
);

create table "data"."characterBreedingWishlist"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "horseUid" "data"."uint32" not null references "data"."horse" ("uid"),
    primary key ("characterUid", "horseUid")
);

create table "data"."characterGift"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "storageItemUid" "data"."uint32" not null references "data"."storageItem" ("uid") on delete cascade,
    primary key ("characterUid", "storageItemUid")
);

create table "data"."characterPurchase"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "storageItemUid" "data"."uint32" not null references "data"."storageItem" ("uid") on delete cascade,
    primary key ("characterUid", "storageItemUid")
);

-- the inventory, equipment and horses only hold the records owned by the character.
create table "data"."characterInventory"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "itemUid" "data"."uint32" not null,
    primary key ("characterUid", "itemUid"),
    foreign key ("itemUid", "characterUid")
        references "data"."item" ("uid", "ownerUid") on delete cascade
);

create table "data"."characterEquipment"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "itemUid" "data"."uint32" not null,
    primary key ("characterUid", "itemUid"),
    foreign key ("itemUid", "characterUid")
        references "data"."item" ("uid", "ownerUid") on delete cascade
);

-- a horse removed from the list stays with its owner.
create table "data"."characterHorse"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "horseUid" "data"."uint32" not null,
    primary key ("characterUid", "horseUid"),
    foreign key ("horseUid", "characterUid")
        references "data"."horse" ("uid", "ownerUid")
);

create table "data"."characterPet"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "petUid" "data"."uint32" not null references "data"."pet" ("uid") on delete cascade,
    primary key ("characterUid", "petUid")
);

create table "data"."characterEgg"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "eggUid" "data"."uint32" not null references "data"."egg" ("uid") on delete cascade,
    primary key ("characterUid", "eggUid")
);

create table "data"."characterHousing"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "housingUid" "data"."uint32" not null references "data"."housing" ("uid") on delete cascade,
    primary key ("characterUid", "housingUid")
);

create table "data"."characterMailboxInbox"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "position" "data"."uint32" not null,
    "mailUid" "data"."uint32" not null references "data"."mail" ("uid") on delete cascade,
    primary key ("characterUid", "position")
);

create table "data"."characterMailboxSent"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "position" "data"."uint32" not null,
    "mailUid" "data"."uint32" not null references "data"."mail" ("uid") on delete cascade,
    primary key ("characterUid", "position")
);

create table "data"."characterQuest"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "questUid" "data"."uint32" not null references "data"."quest" ("uid") on delete cascade,
    primary key ("characterUid", "questUid")
);

-- the group key is also the group UID.
create table "data"."characterContactGroup"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "groupUid" "data"."uint32" not null,
    "name" text not null,
    "createdAt" bigint not null,
    primary key ("characterUid", "groupUid")
);

create table "data"."characterContactGroupMember"
(
    "characterUid" "data"."uint32" not null,
    "groupUid" "data"."uint32" not null,
    "memberUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    primary key ("characterUid", "groupUid", "memberUid"),
    foreign key ("characterUid", "groupUid")
        references "data"."characterContactGroup" ("characterUid", "groupUid") on delete cascade
);

create table "data"."characterSkillSet"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "mode" text not null check ("mode" in ('speed', 'magic')),
    "activeSetId" "data"."uint32" not null,
    "set1Slot1" "data"."uint32" not null,
    "set1Slot2" "data"."uint32" not null,
    "set2Slot1" "data"."uint32" not null,
    "set2Slot2" "data"."uint32" not null,
    primary key ("characterUid", "mode")
);

create table "data"."characterMission"
(
    "characterUid" "data"."uint32" not null references "data"."character" ("uid") on delete cascade,
    "missionKey" "data"."uint32" not null,
    "id" "data"."uint32" not null,
    primary key ("characterUid", "missionKey")
);

create table "data"."characterMissionProgress"
(
    "characterUid" "data"."uint32" not null,
    "missionKey" "data"."uint32" not null,
    "position" "data"."uint32" not null,
    "id" "data"."uint32" not null,
    "value" "data"."uint32" not null,
    primary key ("characterUid", "missionKey", "position"),
    foreign key ("characterUid", "missionKey")
        references "data"."characterMission" ("characterUid", "missionKey") on delete cascade
);
