

INSERT INTO data.user (uid)
    VALUES (1);
INSERT INTO data.user (uid)
    VALUES (2);

INSERT INTO data.token (login, token, user_uid)
    VALUES ('regent', 'rgnter-token', 1);
INSERT INTO data.token (login, token, user_uid)
    VALUES ('laith', 'laith-test', 2);
