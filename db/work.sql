-- =====================================================
-- Урок 1. Группировка и агрегатные функции
-- Исходные данные (PostgreSQL; также работает в SQLite)
-- =====================================================

DROP TABLE IF EXISTS employees;
DROP TABLE IF EXISTS orders;

-- Сотрудники компании
CREATE TABLE employees (
                           id          INTEGER PRIMARY KEY,
                           name        VARCHAR(50) NOT NULL,
                           department  VARCHAR(30) NOT NULL,
                           salary      INTEGER     NOT NULL,
                           bonus       INTEGER,              -- может быть NULL (премии нет)
                           hire_date   DATE        NOT NULL
);

INSERT INTO employees (id, name, department, salary, bonus, hire_date) VALUES
                                                                           (1,  'Анна Иванова',      'IT',        150000, 20000, '2021-03-15'),
                                                                           (2,  'Борис Петров',      'IT',        120000, NULL,  '2022-07-01'),
                                                                           (3,  'Виктор Сидоров',    'IT',        180000, 30000, '2019-11-20'),
                                                                           (4,  'Галина Кузнецова',  'Sales',      90000, 15000, '2020-02-10'),
                                                                           (5,  'Дмитрий Смирнов',   'Sales',      85000, 10000, '2023-01-09'),
                                                                           (6,  'Елена Попова',      'Sales',      95000, NULL,  '2021-09-30'),
                                                                           (7,  'Жанна Волкова',     'HR',         70000, NULL,  '2022-04-18'),
                                                                           (8,  'Игорь Морозов',     'HR',         75000, 5000,  '2018-06-25'),
                                                                           (9,  'Ксения Лебедева',   'Marketing', 110000, 12000, '2020-10-05'),
                                                                           (10, 'Леонид Козлов',     'Marketing', 100000, NULL,  '2023-05-22'),
                                                                           (11, 'Мария Новикова',    'IT',        135000, 15000, '2023-08-14'),
                                                                           (12, 'Никита Федоров',    'Finance',   125000, 20000, '2019-03-01');

-- Заказы интернет-магазина
CREATE TABLE orders (
                        order_id    INTEGER PRIMARY KEY,
                        customer    VARCHAR(50) NOT NULL,
                        city        VARCHAR(50) NOT NULL,
                        category    VARCHAR(30) NOT NULL,
                        product     VARCHAR(50) NOT NULL,
                        quantity    INTEGER     NOT NULL,
                        price       INTEGER     NOT NULL,  -- цена за 1 штуку
                        order_date  DATE        NOT NULL,
                        status      VARCHAR(20) NOT NULL   -- delivered / cancelled / pending
);

INSERT INTO orders (order_id, customer, city, category, product, quantity, price, order_date, status) VALUES
                                                                                                          (1,  'Алексей', 'Москва',          'Электроника', 'Ноутбук',          1, 85000, '2024-01-05', 'delivered'),
                                                                                                          (2,  'Ольга',   'Москва',          'Книги',       'Роман',            3,   700, '2024-01-07', 'delivered'),
                                                                                                          (3,  'Сергей',  'Казань',          'Одежда',      'Куртка',           1,  9000, '2024-01-12', 'cancelled'),
                                                                                                          (4,  'Алексей', 'Москва',          'Книги',       'Учебник SQL',      2,  1500, '2024-01-15', 'delivered'),
                                                                                                          (5,  'Татьяна', 'Казань',          'Дом',         'Чайник',           1,  3500, '2024-01-20', 'delivered'),
                                                                                                          (6,  'Павел',   'Новосибирск',     'Электроника', 'Наушники',         2,  6000, '2024-01-25', 'pending'),
                                                                                                          (7,  'Ирина',   'Санкт-Петербург', 'Одежда',      'Платье',           2,  5500, '2024-02-02', 'delivered'),
                                                                                                          (8,  'Ольга',   'Москва',          'Дом',         'Плед',             1,  2500, '2024-02-05', 'cancelled'),
                                                                                                          (9,  'Сергей',  'Казань',          'Электроника', 'Смартфон',         1, 60000, '2024-02-10', 'delivered'),
                                                                                                          (10, 'Алексей', 'Москва',          'Одежда',      'Кроссовки',        1, 12000, '2024-02-14', 'delivered'),
                                                                                                          (11, 'Павел',   'Новосибирск',     'Книги',       'Детектив',         4,   600, '2024-02-18', 'delivered'),
                                                                                                          (12, 'Ирина',   'Санкт-Петербург', 'Электроника', 'Планшет',          1, 40000, '2024-02-22', 'cancelled'),
                                                                                                          (13, 'Татьяна', 'Казань',          'Книги',       'Сборник рецептов', 1,  1200, '2024-02-27', 'delivered'),
                                                                                                          (14, 'Ольга',   'Москва',          'Электроника', 'Умные часы',       1, 25000, '2024-03-03', 'delivered'),
                                                                                                          (15, 'Сергей',  'Казань',          'Дом',         'Сковорода',        2,  2800, '2024-03-08', 'delivered'),
                                                                                                          (16, 'Павел',   'Новосибирск',     'Одежда',      'Свитер',           1,  4500, '2024-03-12', 'delivered'),
                                                                                                          (17, 'Алексей', 'Москва',          'Дом',         'Лампа',            3,  1800, '2024-03-15', 'pending'),
                                                                                                          (18, 'Ирина',   'Санкт-Петербург', 'Книги',       'Фэнтези',          2,   900, '2024-03-20', 'delivered'),
                                                                                                          (19, 'Татьяна', 'Казань',          'Электроника', 'Фен',              1,  4500, '2024-03-25', 'cancelled'),
                                                                                                          (20, 'Ольга',   'Москва',          'Одежда',      'Шарф',             2,  1500, '2024-03-29', 'delivered');
































SELECT
    department,
    COUNT(*) cnt,
    AVG(salary) as avg_sal
FROM employees
GROUP BY department;

SELECT city, category, count(*) FROM orders GROUP BY city, category;

SELECT TO_CHAR(order_date, 'YYYY-MM') AS month, COUNT(*)
FROM orders
GROUP BY TO_CHAR(order_date, 'YYYY-MM');

select
    department,
    avg(salary)
from employees
where hire_date >= '2020-01-01'
group by department
having avg(salary) < 100000;

select
    city,
    count(*) as total,
    count(*) filter ( where status = 'cancelled' ) as cancelled
from orders
group by city;




select
    department,
    count(*) employees_count
from employees
group by department
order by employees_count desc, department;

select
    department,
    min(salary) as min_salary,
    max(salary) as max_salary,
    round(avg(salary)) as avg_salary
from employees
group by department
order by avg_salary desc;

select
    department,
    count(*) total,
    count(bonus) with_bonus,
    sum(bonus) bonus_sum
from employees
group by department
order by department;

select
    department,
    round(avg(salary)) as avg_salary
from employees
group by department
having avg(salary) > 100000
order by avg_salary desc ;

select
    category,
    sum(quantity * price) revenue
from orders
where status = 'delivered'
group by category
having sum(quantity * price) > 10000
order by revenue desc;

select
    to_char(order_date, 'YYYY-MM') as month,
    count(*)orders_count,
    count(distinct customer) customers,
    sum(quantity * price) revenue
from orders
where status != 'cancelled'
group by month
order by month;

select
    city,
    count(*) total,
    count(*) filter ( where status = 'delivered' ) delivered,
    count(*) filter ( where status = 'cancelled' ) cancelled,
    round((count(*) filter ( where status = 'cancelled' )) * 100.0 / count(status), 1) cancel_pct
from orders
group by city
order by cancel_pct desc, city;

select
    customer,
    count(distinct category) categories,
    sum(quantity * price) total_spent
from orders
group by customer
having count(distinct category) >= 3 and count(*) filter ( where status = 'cancelled' ) = 0
order by total_spent desc;

-- =====================================================
-- Урок 2. Подзапросы: IN, NOT IN, EXISTS, NOT EXISTS
-- Исходные данные (PostgreSQL; также работает в SQLite)
-- =====================================================

DROP TABLE IF EXISTS order_items;
DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS products;
DROP TABLE IF EXISTS customers;

-- Клиенты
CREATE TABLE customers (
                           id            INTEGER PRIMARY KEY,
                           name          VARCHAR(50) NOT NULL,
                           city          VARCHAR(50) NOT NULL,
                           registered_at DATE        NOT NULL
);

INSERT INTO customers (id, name, city, registered_at) VALUES
                                                          (1, 'Алексей', 'Москва',          '2023-02-10'),
                                                          (2, 'Ольга',   'Москва',          '2023-05-21'),
                                                          (3, 'Сергей',  'Казань',          '2023-07-03'),
                                                          (4, 'Татьяна', 'Казань',          '2023-09-15'),
                                                          (5, 'Павел',   'Новосибирск',     '2023-11-01'),
                                                          (6, 'Ирина',   'Санкт-Петербург', '2024-01-12'),
                                                          (7, 'Дмитрий', 'Екатеринбург',    '2024-02-20'),
                                                          (8, 'Наталья', 'Москва',          '2024-03-05');

-- Товары
CREATE TABLE products (
                          id        INTEGER PRIMARY KEY,
                          name      VARCHAR(50) NOT NULL,
                          category  VARCHAR(30) NOT NULL,
                          price     INTEGER     NOT NULL
);

INSERT INTO products (id, name, category, price) VALUES
                                                     (1,  'Ноутбук',     'Электроника', 85000),
                                                     (2,  'Смартфон',    'Электроника', 60000),
                                                     (3,  'Наушники',    'Электроника',  6000),
                                                     (4,  'Умные часы',  'Электроника', 25000),
                                                     (5,  'Учебник SQL', 'Книги',        1500),
                                                     (6,  'Роман',       'Книги',         700),
                                                     (7,  'Детектив',    'Книги',         600),
                                                     (8,  'Куртка',      'Одежда',       9000),
                                                     (9,  'Кроссовки',   'Одежда',      12000),
                                                     (10, 'Чайник',      'Дом',          3500),
                                                     (11, 'Плед',        'Дом',          2500),
                                                     (12, 'Лампа',       'Дом',          1800);

-- Заказы
-- customer_id = NULL означает гостевой заказ (без регистрации)
CREATE TABLE orders (
                        id           INTEGER PRIMARY KEY,
                        customer_id  INTEGER REFERENCES customers(id),
                        order_date   DATE        NOT NULL,
                        status       VARCHAR(20) NOT NULL   -- delivered / cancelled / pending
);

INSERT INTO orders (id, customer_id, order_date, status) VALUES
                                                             (1,  1,    '2024-01-05', 'delivered'),
                                                             (2,  2,    '2024-01-07', 'delivered'),
                                                             (3,  3,    '2024-01-12', 'cancelled'),
                                                             (4,  1,    '2024-01-15', 'delivered'),
                                                             (5,  4,    '2024-01-20', 'delivered'),
                                                             (6,  5,    '2024-01-25', 'pending'),
                                                             (7,  6,    '2024-02-02', 'delivered'),
                                                             (8,  NULL, '2024-02-05', 'delivered'),
                                                             (9,  3,    '2024-02-10', 'delivered'),
                                                             (10, 2,    '2024-02-14', 'cancelled'),
                                                             (11, 5,    '2024-02-18', 'delivered'),
                                                             (12, 6,    '2024-03-03', 'cancelled'),
                                                             (13, NULL, '2024-03-08', 'pending'),
                                                             (14, 1,    '2024-03-15', 'delivered');

-- Позиции заказов (какие товары и сколько штук в каждом заказе)
CREATE TABLE order_items (
                             order_id    INTEGER NOT NULL REFERENCES orders(id),
                             product_id  INTEGER NOT NULL REFERENCES products(id),
                             quantity    INTEGER NOT NULL,
                             PRIMARY KEY (order_id, product_id)
);

INSERT INTO order_items (order_id, product_id, quantity) VALUES
                                                             (1,  1, 1), (1,  3, 1),
                                                             (2,  6, 3),
                                                             (3,  8, 1),
                                                             (4,  5, 2), (4,  9, 1),
                                                             (5, 10, 1), (5,  7, 2),
                                                             (6,  3, 2),
                                                             (7,  4, 1),
                                                             (8,  2, 1),
                                                             (9,  2, 1), (9,  5, 1),
                                                             (10, 9, 1),
                                                             (11, 7, 4), (11, 8, 1),
                                                             (12, 1, 1),
                                                             (13, 6, 1),
                                                             (14, 4, 1), (14, 10, 1);







select name from products where price = (select max(price) from products);
select name from customers where id in (select customer_id from orders where order_date between '2024-01-01' and '2024-01-31')
select name from customers where id in (select customer_id from orders where id in (select order_id from order_items where product_id in (select id from products where category = 'Книги')))


select
    c.name
from customers c
where not exists (
    select 1
    from orders o
    where o.customer_id = c.id and status = 'pending'
);

select
    c.name,
    (select count(*) from orders o where o.customer_id = c.id) orders_count
from customers c
order by name;

select
    name,
    price
from products
where price > (select avg(price) from products)
order by price desc ;

select
    name
from customers
where id in (select customer_id from orders where status ='cancelled')
order by name;

select
    name,
    category
from products p
where not exists (
    select 1
    from order_items oi
    where oi.product_id = p.id
)
order by name;

-- a
select
    name
from customers
where id not in (select customer_id from orders where customer_id is not null)
-- b
select
    name
from customers c
where not exists (
    select 1
    from orders o
    where c.id = o.customer_id
);

select
    name,
    category,
    price,
    (select round(avg(pp.price)) from products pp where pp.category = p.category)
from products p
where p.price > (select avg(pp.price) from products pp where p.category = pp.category)
order by category, price desc;

select
    name
from customers c
where exists (
    select 1
    from orders o
    where status = 'delivered' and
    o.customer_id = c.id and
    o.id in (
        select oi.order_id from order_items oi where oi.product_id in (
            select p.id
            from products p
            where p.category = 'Электроника'
        )
    )
)
order by name;

select
    c.name
from customers c
where not exists (
    select 1
    from orders o
    where status != 'delivered' and o.customer_id = c.id
) and exists (
    select 1
    from orders o
    where o.customer_id = c.id
)
order by c.name







-- =====================================================
-- Урок 3. JOIN: INNER, LEFT, RIGHT, FULL, SELF, CROSS
-- Исходные данные (PostgreSQL; также работает в SQLite 3.39+)
-- =====================================================

DROP TABLE IF EXISTS employee_projects;
DROP TABLE IF EXISTS projects;
DROP TABLE IF EXISTS employees;
DROP TABLE IF EXISTS departments;

-- Отделы
CREATE TABLE departments (
                             id    INTEGER PRIMARY KEY,
                             name  VARCHAR(30) NOT NULL
);

INSERT INTO departments (id, name) VALUES
                                       (1, 'IT'),
                                       (2, 'Sales'),
                                       (3, 'HR'),
                                       (4, 'Marketing'),
                                       (5, 'Finance'),
                                       (6, 'Legal');          -- в отделе пока нет сотрудников

-- Сотрудники
-- department_id = NULL: сотрудника ещё не распределили в отдел
-- manager_id    = NULL: у сотрудника нет руководителя (это CEO)
CREATE TABLE employees (
                           id             INTEGER PRIMARY KEY,
                           name           VARCHAR(50) NOT NULL,
                           department_id  INTEGER REFERENCES departments(id),
                           manager_id     INTEGER REFERENCES employees(id),
                           salary         INTEGER NOT NULL,
                           hire_date      DATE    NOT NULL
);

INSERT INTO employees (id, name, department_id, manager_id, salary, hire_date) VALUES
                                                                                   (1,  'Анна Иванова',     1,    NULL, 250000, '2018-03-01'),
                                                                                   (2,  'Борис Петров',     1,    1,    180000, '2019-06-15'),
                                                                                   (3,  'Виктор Сидоров',   1,    2,    190000, '2021-09-01'),
                                                                                   (4,  'Галина Кузнецова', 1,    2,    140000, '2023-02-20'),
                                                                                   (5,  'Дмитрий Смирнов',  2,    1,    150000, '2019-01-10'),
                                                                                   (6,  'Елена Попова',     2,    5,     95000, '2022-04-05'),
                                                                                   (7,  'Жанна Волкова',    2,    5,    160000, '2023-07-12'),
                                                                                   (8,  'Игорь Морозов',    3,    1,    120000, '2020-11-30'),
                                                                                   (9,  'Ксения Лебедева',  3,    8,     80000, '2024-01-15'),
                                                                                   (10, 'Леонид Козлов',    4,    1,    130000, '2021-03-22'),
                                                                                   (11, 'Мария Новикова',   4,    10,   135000, '2024-05-06'),
                                                                                   (12, 'Никита Федоров',   NULL, 1,     90000, '2024-08-19'),
                                                                                   (13, 'Олег Соколов',     5,    1,    170000, '2017-05-11');

-- Проекты (у каждого проекта есть отдел-владелец)
CREATE TABLE projects (
                          id             INTEGER PRIMARY KEY,
                          name           VARCHAR(50) NOT NULL,
                          department_id  INTEGER NOT NULL REFERENCES departments(id)
);

INSERT INTO projects (id, name, department_id) VALUES
                                                   (1, 'CRM-система',          1),
                                                   (2, 'Мобильное приложение', 1),
                                                   (3, 'Рекламная кампания',   4),
                                                   (4, 'Найм 2024',            3),
                                                   (5, 'Миграция в облако',    1),   -- участников пока нет
                                                   (6, 'Аудит',                5);   -- участников пока нет

-- Участие сотрудников в проектах (связь многие-ко-многим)
-- hours: сколько часов сотрудник отработал на проекте
CREATE TABLE employee_projects (
                                   employee_id  INTEGER NOT NULL REFERENCES employees(id),
                                   project_id   INTEGER NOT NULL REFERENCES projects(id),
                                   hours        INTEGER NOT NULL,
                                   PRIMARY KEY (employee_id, project_id)
);

INSERT INTO employee_projects (employee_id, project_id, hours) VALUES
                                                                   (2,  1, 120),
                                                                   (3,  1, 200),
                                                                   (4,  1,  80),
                                                                   (7,  1,  30),
                                                                   (3,  2, 150),
                                                                   (4,  2, 100),
                                                                   (12, 2,  50),
                                                                   (10, 3,  90),
                                                                   (11, 3, 110),
                                                                   (6,  3,  40),
                                                                   (8,  4,  60),
                                                                   (9,  4,  70);


-- no pair - no select
-- row with n pair repeated n times
-- INNER JOIN - no null rows, must be pairs
SELECT
    e.name,
    d.name dept
FROM employees e
JOIN departments d ON d.id = e.department_id;

-- outer join - with null left side rows
SELECT
    e.name,
    d.name dept
from employees e
left join departments d on d.id = e.department_id;

select e.name, d.name dept from employees e right join departments d on d.id = e.department_id;

-- all rows with nulls
select e.name, d.name dept from employees e full join departments d on d.id = e.department_id;

-- all pairs with no nulls (generally for generation combinations)
select e.name, d.name dept from employees e cross join departments d;

-- self join for ties hierarchy or pairs
select e.name employee, m.name manager from employees e left join employees m on e.manager_id = m.id;





SELECT
    e.name,
    d.name
FROM employees e
JOIN departments d ON d.id = e.department_id
ORDER BY d.name, e.name;

SELECT
    e.name,
    d.name
FROM employees e
LEFT JOIN departments d ON d.id = e.department_id
ORDER BY e.id;

SELECT
    d.name
FROM departments d
LEFT JOIN employees e ON d.id = e.department_id
GROUP BY d.name
HAVING count(e.id) = 0;

SELECT
    e.name employee,
    COALESCE(m.name, 'Нет руководителя') manager
FROM employees e
LEFT JOIN employees m ON m.id = e.manager_id
ORDER BY e.id;

SELECT
    e.name employee,
    e.salary,
    m.name manager,
    m.salary manager_salary
FROM employees e
INNER JOIN employees m
ON m.id = e.manager_id
WHERE e.salary > m.salary
ORDER BY e.salary - m.salary DESC;

SELECT
    p.name project,
    d.name departament,
    count(ep.employee_id) participants,
    COALESCE(sum(ep.hours), 0) total_hours
FROM projects p
JOIN departments d ON p.department_id = d.id
LEFT JOIN employee_projects ep ON p.id = ep.project_id
GROUP BY p.name, d.name
ORDER BY total_hours DESC, p.name;

SELECT
    d.name departament,
    count(e.id) new_employees
FROM departments d
LEFT JOIN employees e ON e.department_id = d.id AND e.hire_date > '2023-01-01'
GROUP BY d.name
ORDER BY new_employees DESC, departament;

SELECT
    d.name dept,
    count(e.id) employees_cnt,
    COALESCE(sum(e.salary), 0) salary_fund,
    COALESCE(sum(h.hours), 0) project_hours
FROM departments d
LEFT JOIN employees e ON d.id = e.department_id
LEFT JOIN (
    SELECT employee_id, SUM(hours) hours FROM employee_projects GROUP BY employee_id
) h ON h.employee_id = e.id
GROUP BY dept
ORDER BY salary_fund DESC;
