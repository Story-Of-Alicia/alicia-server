-- =============================================================================
-- Alicia Server - primary data schema
--
-- This file holds the parts of the schema which cannot be derived from the
-- record definitions: the sequences, the child tables backing the collection
-- fields, and the indexes.
--
-- The 16 record tables themselves are NOT here. They are generated from the
-- column tables in src/libserver/data/pq/PqDataSource.cpp, which already have
-- to list every column in parameter order. Describing them twice is what lets
-- the schema and the code drift apart, so the column tables are the single
-- source of truth and PqDataSource::EnsureSchema creates the tables from them.
--
-- Everything here is idempotent, because EnsureSchema applies it on every
-- start-up rather than only against an empty database.
--
-- Conventions
-- -----------
-- * Column names are the snake_case form of the C++ field names. Nested
--   structs are flattened with an underscore-joined prefix.
-- * data::Uid and data::Tid map to `integer`. They are drawn from the sequences
--   below, starting at 1, and will never approach 2^31.
-- * std::chrono::seconds durations map to `bigint`, a count of seconds.
-- * data::Clock::time_point maps to `timestamptz`.
-- * data::InvalidUid (0) is stored literally as 0, never as NULL. Every column
--   is NOT NULL with a default, which keeps the C++ mapping mechanical.
-- * Enums are stored as smallint, the C++ enum staying the source of truth.
--
-- Referential integrity
-- ---------------------
-- Foreign keys exist ONLY from a child table to its owning record, and only
-- where both rows are written inside the same transaction by the same Store
-- call.
--
-- There are deliberately NO foreign keys between records (character.guild_uid,
-- user.character_uid, character_item.item_uid, mail.from_uid, ...). DataStorage
-- writes each record type independently, from separate queues drained in a
-- fixed order which is not dependency order, so a reference can legitimately be
-- stored before its target row exists. The references are also genuinely cyclic
-- (character.guild_uid -> guild.owner -> character), so no insertion order
-- could satisfy them all. Dangling references between records are an expected
-- state which src/libserver/data/DataRepair.cpp already reconciles.
-- =============================================================================

CREATE SCHEMA IF NOT EXISTS data;

-- -----------------------------------------------------------------------------
-- Sequences
--
-- One per counter which the file data source kept in meta.json. Create* draws a
-- UID with nextval() and does not insert a row; the row is written later, when
-- the store queue is drained. That is why the record tables carry no identity
-- columns.
--
-- The equipment sequence is shared by item and horse on purpose: FileDataSource
-- allocated both from a single _equipmentSequentialUid counter, so item and
-- horse UIDs have never collided in existing data. One sequence preserves that.
-- -----------------------------------------------------------------------------

CREATE SEQUENCE IF NOT EXISTS data.infraction_uid_seq        AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.character_uid_seq         AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.equipment_uid_seq         AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.storage_item_uid_seq      AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.egg_uid_seq               AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.pet_uid_seq               AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.housing_uid_seq           AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.guild_uid_seq             AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.settings_uid_seq          AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.daily_quest_group_uid_seq AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.mail_uid_seq              AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.quest_uid_seq             AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.stallion_uid_seq          AS integer START WITH 1;
CREATE SEQUENCE IF NOT EXISTS data.reward_uid_seq            AS integer START WITH 1;

-- -----------------------------------------------------------------------------
-- Indexes on the generated record tables
-- -----------------------------------------------------------------------------

-- Backs DataSource::IsUserNameUnique. The name is already the primary key, so
-- this exists purely to make the uniqueness check case-insensitive. Note that
-- this is stricter than FileDataSource, whose regex-over-filenames check also
-- rejected names sharing a prefix with an existing one.
CREATE UNIQUE INDEX IF NOT EXISTS user_name_lower_key
    ON data."user" (lower(name));

-- Backs DataSource::IsCharacterNameUnique and ::RetrieveCharacterUidByName.
CREATE UNIQUE INDEX IF NOT EXISTS character_name_lower_key
    ON data.character (lower(name));

-- Backs DataSource::IsGuildNameUnique.
CREATE UNIQUE INDEX IF NOT EXISTS guild_name_lower_key
    ON data.guild (lower(name));

-- =============================================================================
-- Child tables
--
-- One per collection field. A store deletes the children of the record and
-- inserts them again, inside the same transaction as the record itself, so the
-- record is always the sole writer of its own children.
-- =============================================================================

-- data::User::infractions
CREATE TABLE IF NOT EXISTS data.user_infraction
(
    user_name      text    NOT NULL
        REFERENCES data."user" (name) ON DELETE CASCADE,
    ordinal        integer NOT NULL,
    infraction_uid integer NOT NULL,
    PRIMARY KEY (user_name, ordinal)
);

-- data::StorageItem::items - an ordered vector of inline value objects, not
-- references to data.item.
CREATE TABLE IF NOT EXISTS data.storage_item_entry
(
    storage_item_uid integer NOT NULL
        REFERENCES data.storage_item (uid) ON DELETE CASCADE,
    ordinal          integer NOT NULL,
    tid              integer NOT NULL DEFAULT 0,
    count            integer NOT NULL DEFAULT 0,
    duration         bigint  NOT NULL DEFAULT 0,
    PRIMARY KEY (storage_item_uid, ordinal)
);

-- data::DailyQuestGroup::quests - a fixed std::array of 3, so slot is 0..2.
CREATE TABLE IF NOT EXISTS data.daily_quest_entry
(
    group_uid integer  NOT NULL
        REFERENCES data.daily_quest_group (uid) ON DELETE CASCADE,
    slot      smallint NOT NULL CHECK (slot BETWEEN 0 AND 2),
    quest_id  integer  NOT NULL DEFAULT 0,
    progress  bigint   NOT NULL DEFAULT 0,
    PRIMARY KEY (group_uid, slot)
);

-- data::Settings::keyboardBindings and ::gamepadBindings, discriminated by
-- device. Whether the optionals are engaged at all is carried by the
-- has_keyboard_bindings and has_gamepad_bindings columns of data.settings,
-- because an absent optional and a present but empty one both yield zero rows
-- here and LobbyNetworkHandler maps that difference onto typeBitset.
CREATE TABLE IF NOT EXISTS data.settings_binding
(
    settings_uid  integer  NOT NULL
        REFERENCES data.settings (uid) ON DELETE CASCADE,
    -- 0 = keyboard, 1 = gamepad
    device        smallint NOT NULL CHECK (device IN (0, 1)),
    ordinal       integer  NOT NULL,
    primary_key   integer  NOT NULL DEFAULT 0,
    type          integer  NOT NULL DEFAULT 0,
    secondary_key integer  NOT NULL DEFAULT 0,
    PRIMARY KEY (settings_uid, device, ordinal)
);

-- data::Settings::macros - a fixed std::array of 8, so slot is 0..7.
CREATE TABLE IF NOT EXISTS data.settings_macro
(
    settings_uid integer  NOT NULL
        REFERENCES data.settings (uid) ON DELETE CASCADE,
    slot         smallint NOT NULL CHECK (slot BETWEEN 0 AND 7),
    body         text     NOT NULL DEFAULT '',
    PRIMARY KEY (settings_uid, slot)
);

-- data::Guild::officers
CREATE TABLE IF NOT EXISTS data.guild_officer
(
    guild_uid     integer NOT NULL
        REFERENCES data.guild (uid) ON DELETE CASCADE,
    ordinal       integer NOT NULL,
    character_uid integer NOT NULL,
    PRIMARY KEY (guild_uid, ordinal)
);

-- data::Guild::members
CREATE TABLE IF NOT EXISTS data.guild_member
(
    guild_uid     integer NOT NULL
        REFERENCES data.guild (uid) ON DELETE CASCADE,
    ordinal       integer NOT NULL,
    character_uid integer NOT NULL,
    PRIMARY KEY (guild_uid, ordinal)
);

-- data::Character's item lists, folded into one table. They are structurally
-- identical (ordered vectors of data.item UIDs) and always written together by
-- StoreCharacter, so a discriminator beats several identical tables.
CREATE TABLE IF NOT EXISTS data.character_item
(
    character_uid integer  NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    -- 0 = inventory, 1 = characterEquipment, 3 = gifts, 4 = purchases.
    -- 2 was expiredEquipment, which the character no longer carries; rows of
    -- that kind are never read again and the value must not be reused.
    kind          smallint NOT NULL CHECK (kind BETWEEN 0 AND 4),
    ordinal       integer  NOT NULL,
    item_uid      integer  NOT NULL,
    PRIMARY KEY (character_uid, kind, ordinal)
);

-- data::Character::horses
CREATE TABLE IF NOT EXISTS data.character_horse
(
    character_uid integer NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    ordinal       integer NOT NULL,
    horse_uid     integer NOT NULL,
    PRIMARY KEY (character_uid, ordinal)
);

-- data::Character::pets
CREATE TABLE IF NOT EXISTS data.character_pet
(
    character_uid integer NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    ordinal       integer NOT NULL,
    pet_uid       integer NOT NULL,
    PRIMARY KEY (character_uid, ordinal)
);

-- data::Character::eggs
CREATE TABLE IF NOT EXISTS data.character_egg
(
    character_uid integer NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    ordinal       integer NOT NULL,
    egg_uid       integer NOT NULL,
    PRIMARY KEY (character_uid, ordinal)
);

-- data::Character::housing
CREATE TABLE IF NOT EXISTS data.character_housing
(
    character_uid integer NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    ordinal       integer NOT NULL,
    housing_uid   integer NOT NULL,
    PRIMARY KEY (character_uid, ordinal)
);

-- data::Character::quests
CREATE TABLE IF NOT EXISTS data.character_quest
(
    character_uid integer NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    ordinal       integer NOT NULL,
    quest_uid     integer NOT NULL,
    PRIMARY KEY (character_uid, ordinal)
);

-- data::Character::Mailbox::inbox and ::sent. Partially redundant with
-- mail.from_uid / mail.to_uid, but the character owns these lists and
-- StoreCharacter must remain their sole writer.
CREATE TABLE IF NOT EXISTS data.character_mail
(
    character_uid integer  NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    -- 0 = inbox, 1 = sent
    box           smallint NOT NULL CHECK (box IN (0, 1)),
    ordinal       integer  NOT NULL,
    mail_uid      integer  NOT NULL,
    PRIMARY KEY (character_uid, box, ordinal)
);

-- data::Character::breedingWishlist is a std::set, so there is no ordinal.
CREATE TABLE IF NOT EXISTS data.character_breeding_wishlist
(
    character_uid integer NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    horse_uid     integer NOT NULL,
    PRIMARY KEY (character_uid, horse_uid)
);

-- data::Character::Contacts::pending is a std::set, so there is no ordinal.
CREATE TABLE IF NOT EXISTS data.character_contact_pending
(
    character_uid integer NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    contact_uid   integer NOT NULL,
    PRIMARY KEY (character_uid, contact_uid)
);

-- data::Character::Contacts::groups is a std::map keyed by Group::uid, and the
-- two are always equal, so group_uid serves as both. Note that group_uid 0 is
-- the valid default group created at character creation, not a missing value.
CREATE TABLE IF NOT EXISTS data.character_contact_group
(
    character_uid integer     NOT NULL
        REFERENCES data.character (uid) ON DELETE CASCADE,
    group_uid     integer     NOT NULL,
    name          text        NOT NULL DEFAULT '',
    created_at    timestamptz NOT NULL DEFAULT 'epoch',
    PRIMARY KEY (character_uid, group_uid)
);

-- data::Character::Contacts::Group::members, a std::set.
CREATE TABLE IF NOT EXISTS data.character_contact_group_member
(
    character_uid integer NOT NULL,
    group_uid     integer NOT NULL,
    member_uid    integer NOT NULL,
    PRIMARY KEY (character_uid, group_uid, member_uid),
    FOREIGN KEY (character_uid, group_uid)
        REFERENCES data.character_contact_group (character_uid, group_uid)
        ON DELETE CASCADE
);
