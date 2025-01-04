create table if not exists horse
(
    uid          integer PRIMARY KEY,
    tid          integer,
    name         text,

    skinId       integer,
    maneId       integer,
    tailId       integer,
    faceId       integer,
    scale        integer,
    legLength    integer,
    legVolume    integer,
    bodyLength   integer,
    bodyVolume   integer,

    agility      integer,
    spirit       integer,
    speed        integer,
    strength     integer,
    ambition     integer,

    rating       integer,
    class        integer,
    grade        integer,
    growthPoints integer,

    magic        integer,
    jumping      integer,
    sliding      integer,
    gliding      integer
);

create table if not exists ranch
(
    uid  integer PRIMARY KEY,
    name text
);

create table if not exists character
(
    uid           integer PRIMARY KEY,

    name          text,
    status        text,
    ageGroup      integer,
    gender        integer,

    level         integer,
    carrots       integer,
    cash          integer,

    charId        integer,
    mouthSerialId integer,
    faceSerialId  integer,
    headVolume    integer,
    height        integer,
    thighVolume   integer,
    legVolume     integer,

    horse_uid     integer
        REFERENCES horse (uid),
    ranch_uid     integer
        REFERENCES ranch (uid)
);

create table if not exists item
(
    uid           integer PRIMARY KEY,
    tid           integer,
    count         integer,
    character_uid integer
        REFERENCES character (uid)
);

create table if not exists "user"
(
    uid           integer PRIMARY KEY,
    character_uid integer
        REFERENCES character (uid)
);

create table if not exists token
(
    login    text PRIMARY KEY,
    token    text UNIQUE,
    user_uid integer
        REFERENCES "user" (uid)
);