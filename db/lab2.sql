-- SELECT
-- №1.1
SELECT first_name, last_name, father_name FROM auto_personal ORDER BY last_name, first_name DESC;
-- №1.2
SELECT COUNT(*) FROM auto WHERE num LIKE 'р%';
-- №2.1
SELECT num FROM auto WHERE personal_id IN (SELECT id FROM auto_personal WHERE last_name='Смирнов');
-- №2.2
SELECT id, name FROM routes WHERE id IN (SELECT route_id FROM journal WHERE time_in IS NULL);
-- №3.1
SELECT
    r.name,
    COALESCE(j.time_out::text, 'Нет рейса') time_out
FROM routes r
LEFT JOIN journal j ON j.route_id = r.id;
-- №3.2
SELECT
    COALESCE(r.name::text, 'Нет направления') route,
    COALESCE(j.time_out::text, 'Нет отправки') time_out,
    COALESCE(j.time_in::text, 'Нет прибытия') time_in,
    COALESCE(a.num::text, 'Нет автомобиля') num
FROM routes r
LEFT JOIN journal j ON j.route_id = r.id
FULL JOIN auto a ON j.auto_id = a.id;
-- №4.1
SELECT
    r.name route,
    a.num car_number,
    j.time_in - j.time_out duration
FROM journal j
JOIN routes r ON j.route_id = r.id
JOIN auto a ON a.id = j.auto_id
WHERE time_in IS NOT NULL AND r.name = 'Центр'
ORDER BY duration LIMIT 1;
-- №4.2
SELECT COUNT(*) FROM journal WHERE time_in IS NULL AND time_out IS NOT NULL AND route_id IN (
    SELECT id FROM routes WHERE name = 'Аэропорт'
);
-- №5.1
SELECT
    ap.first_name,
    ap.last_name,
    ap.father_name,
    j.id journal_id,
    j.time_out,
    j.time_in,
    row_number() over (partition by ap.id order by j.time_out) route_number,
    extract(epoch from (j.time_out - lag(j.time_in) over (partition by ap.id order by j.time_out))) / 60 minutes_since_prev_arrival
FROM journal j
JOIN auto a ON a.id = j.auto_id
JOIN auto_personal ap ON ap.id = a.personal_id
ORDER BY ap.last_name, ap.first_name, j.time_out;






-- INSERT
-- №1.1
INSERT INTO auto (num, color, mark, personal_id)
SELECT 'номер', 'цвет', 'марка', id
FROM auto_personal
WHERE last_name = 'Кузнецов'
UNION
SELECT 'номерр', 'цветт', 'маркаa', id
FROM auto_personal
WHERE last_name = 'Кузнецов';

SELECT * FROM auto
JOIN auto_personal ON auto.personal_id = auto_personal.id;

-- №1.2
INSERT INTO journal (time_out, route_id, auto_id)
SELECT now(), 1, a.id
FROM auto a
JOIN auto_personal ap ON a.personal_id = ap.id
WHERE a.color = 'желтый' AND ap.last_name = 'Смирнов';

SELECT * FROM journal;

-- №1.3
INSERT INTO auto (num, color, mark, personal_id)
VALUES ('тестовый номер', 'красный', 'тайота', 4)
RETURNING *;

INSERT INTO auto (num, color, mark, personal_id)
VALUES ('тестовый номер', 'синий', 'мерседес', 4)
ON CONFLICT (num) DO UPDATE SET color = EXCLUDED.color
RETURNING *;

-- №2.1
SELECT * FROM journal;

DO $$
BEGIN
    INSERT INTO journal (time_out, route_id, auto_id)
    SELECT NOW(), r.id, a.id
    FROM routes r, auto a
    WHERE r.name = 'Аэропорт' AND a.num = 'м888мм';

    IF (SELECT COUNT(DISTINCT j.auto_id)
        FROM journal j
        JOIN routes r ON r.id = j.route_id
        WHERE r.name = 'Аэропорт' AND j.time_in IS NULL) > 2 THEN
        RAISE EXCEPTION 'invalid count';
    END IF;
END $$;

SELECT * FROM journal;



-- DELETE
-- №1.1
DELETE FROM journal WHERE route_id = 1;
DELETE FROM routes  WHERE id = 1;

SELECT COUNT(*) FROM journal WHERE id = 1;
-- №1.2
DELETE FROM auto_personal ap WHERE NOT EXISTS (
SELECT 1 FROM auto a WHERE a.personal_id = AP.id
) RETURNING *;

-- №2.1
BEGIN;
DELETE FROM auto WHERE personal_id IN (SELECT id FROM auto_personal WHERE last_name = 'Иванов') RETURNING *;
DELETE FROM auto_personal WHERE last_name = 'Иванов' RETURNING *;
COMMIT;

SELECT * FROM auto a JOIN auto_personal ap ON a.personal_id = ap.id
where last_name = 'Иванов';

-- №2.2
BEGIN;
DELETE FROM auto WHERE personal_id IN (SELECT id FROM auto_personal WHERE last_name = 'Иванов') RETURNING *;
DELETE FROM auto_personal WHERE last_name = 'Иванов' RETURNING *;
ROLLBACK;

SELECT * FROM auto a JOIN auto_personal ap ON a.personal_id = ap.id
where last_name = 'Иванов';



-- UPDATE
-- №1.1
UPDATE journal SET time_in = '2026-10-26 10:30' WHERE id = 10 RETURNING *;

-- №1.2
UPDATE journal SET
    route_id = (SELECT id FROM routes WHERE name = 'Центр')
WHERE auto_id = (SELECT id FROM auto WHERE num = 'р001ее') AND time_in IS NULL RETURNING *;

-- №2.1
BEGIN;
UPDATE journal SET route_id = (SELECT id FROM routes WHERE name = 'Центр')
WHERE route_id = (SELECT id FROM routes WHERE name = 'Вокзал') RETURNING *;

DELETE FROM routes WHERE name = 'Вокзал' RETURNING *;
COMMIT;

-- №2.2
BEGIN;
UPDATE journal SET route_id = (SELECT id FROM routes WHERE name = 'Центр')
WHERE route_id = (SELECT id FROM routes WHERE name = 'Вокзал') RETURNING *;

DELETE FROM routes WHERE name = 'Вокзал' RETURNING *;
ROLLBACK;

SELECT p.id, p.last_name, p.first_name, p.father_name
FROM auto_personal p
WHERE NOT EXISTS (
    SELECT 1
    FROM auto a
    JOIN journal j ON j.auto_id = a.id
    JOIN routes r  ON r.id = j.route_id
    WHERE a.personal_id = p.id AND r.name IN ('Аэропорт', 'Вокзал', 'Центр', 'Университет')
);

DELETE FROM auto_personal p
WHERE NOT EXISTS (
    SELECT 1
    FROM auto a
    JOIN journal j ON j.auto_id = a.id
    JOIN routes r  ON r.id = j.route_id
    WHERE a.personal_id = p.id AND r.name = 'Аэропорт'
);

SELECT
    j.route_id,
    r.name
FROM journal j
LEFT JOIN routes r ON j.route_id = r.id
WHERE route_id IS NOT NULL;

DELETE FROM routes r WHERE NOT EXISTS (
    SELECT 1
    FROM journal j
    WHERE j.route_id = r.id
);

SELECT
    p.id AS personal_id,
    p.last_name,
    p.first_name,
    p.father_name,
    a.id AS auto_id,
    a.num,
    a.color,
    a.mark,
    r.name AS route,
    j.time_out,
    j.time_in
FROM auto_personal p
LEFT JOIN auto a ON a.personal_id = p.id
LEFT JOIN journal j ON j.auto_id = a.id
LEFT JOIN routes r ON r.id = j.route_id
ORDER BY p.id, a.id, j.time_out;

DELETE FROM auto_personal p
WHERE NOT EXISTS (
    SELECT 1
    FROM auto a
             JOIN journal j ON j.auto_id = a.id
             JOIN routes r  ON r.id = j.route_id
    WHERE a.personal_id = p.id AND r.name IN ('Аэропорт', 'Вокзал', 'Центр', 'Университет')
);
