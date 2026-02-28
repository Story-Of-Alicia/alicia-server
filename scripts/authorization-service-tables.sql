create table sessions
(
    username VARCHAR(32) NOT NULL PRIMARY KEY,
    token VARCHAR(64) NOT NULL,
    expires_at TIMESTAMP
);