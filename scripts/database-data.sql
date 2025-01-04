

INSERT INTO soa_dev.public.user (uid)
    VALUES (1);
INSERT INTO soa_dev.public.user (uid)
    VALUES (2);

INSERT INTO soa_dev.public.token (login, token, user_uid)
    VALUES ('regent', 'test', 1);
INSERT INTO soa_dev.public.token (login, token, user_uid)
    VALUES ('laith', 'test', 2);
