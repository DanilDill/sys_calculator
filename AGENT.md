# Calculator — системный сервис

## Назначение

Calculator — это системный сервис для Linux (Ubuntu 24), предоставляющий вычислительные операции через D-Bus и/или TCP интерфейс. Сервис поддерживает базовые арифметические операции (+, -, *, /, ^, !), кэширование результатов в Redis инициализацию истории записей в PostgreSQL. Управляется через systemd/systemctl, работает как system bus D-Bus сервис. Режим работы (только D-Bus, только TCP или оба сразу) и все параметры подключения задаются в JSON конфиге.

## Технологический стек

- **Язык/стандарт**: C++17
- **Сборка**: CMake 3.16+
- **Управление зависимостями**: FetchContent (для libmath), find_package (для остальных)
- **Ключевые библиотеки**:
  - `sdbus-c++` — D-Bus коммуникация
  - `Boost.Asio` (`libboost-system-dev`) — TCP сервер и клиент
  - `nlohmann_json` — JSON сериализация/десериализация
  - `spdlog` + `fmt` — логирование
  - `libpq` (PostgreSQL) - инициализацию хранение истории вычислений
  - `redis-plus-plus` + `hiredis` — кэширование результатов
  - `libmath` (внешний репозиторий: https://github.com/DanilDill/libmath.git) — математические операции
  - `GTest` — unit/integration тестирование
- **Дополнительные инструменты**:
  - `clang-tidy` (опционально, ENABLE_CLANG_TIDY) — статический анализ
  - `clang-format` (по умолчанию включен) — форматирование кода
  - `Doxygen` — генерация документации
  - `Valgrind` — проверка утечек памяти (интегрирован в тесты)

## Архитектура

### Основные компоненты и их ответственность

```
┌─────────────────────────────────────────────────┐
│              D-Bus Client (busctl)               │
└──────────────┬──────────────────────────────────┘
               │ JSON request/response
               ▼
┌─────────────────────────────────────────────────┐
│              DBusServer                          │
│  - SERVICE_NAME: com.example.CalculatorService   │
│  - OBJECT_PATH: /com/example/CalculatorObject    │
│  - INTERFACE: com.example.CalculatorInterface    │
│  - METHOD: Calculate(string) -> string           │
└──────────────┬──────────────────────────────────┘
               │ парсинг JSON → Task
               ▼
┌─────────────────────────────────────────────────┐
│            HistoryService                        │
│  1. Проверка кэша (RedisCache::get)             │
│  2. Если cache miss → Calculator::execute       │
│  3. Сохранение в БД (PostgresStorage::insert)   │
│  4. Сохранение в кэш (RedisCache::put)          │
└──────┬──────────────────────────┬───────────────┘
       │                          │
       ▼                          ▼
┌──────────────────┐   ┌──────────────────────┐
│  RedisCache      │   │  PostgresStorage     │
│  - Кэш результатов│   │  - История вычислений│
│  - Warmup при    │   │  - Схема: calc_history│
│    старте        │   │                      │
└──────────────────┘   └──────────────────────┘
       │
       ▼ (при cache miss)
┌──────────────────────────┐
│   Calculator::execute    │
│  - math::add/sub/mul/    │
│    div/pow/factorial     │
│  - Обработка ошибок      │
└──────────────────────────┘
```

### Классы и их роль

| Класс | Файлы | Ответственность |
|-------|-------|-----------------|
| `calculator::app` | `src/app.hpp`, `src/app.cpp` | Точка входа, управление жизненным циклом, обработка CLI аргументов (`-h`, `-c`), загрузка конфига, запуск серверов по режиму, обработка сигналов (SIGINT/SIGTERM) |
| `calculator::Config` | `src/config.hpp`, `src/config.cpp` | Структура конфигурации и парсер JSON: режим работы, TCP address/port, уровень логирования, строки подключения к PostgreSQL/Redis |
| `calculator::DBusServer` | `src/dbus_server.hpp`, `src/dbus_server.cpp` | Регистрация D-Bus сервиса, обработка вызовов метода `Calculate`, async event loop |
| `calculator::TCPServer` | `src/tcp_server.hpp`, `src/tcp_server.cpp` | TCP сервер на Boost.Asio (Pimpl): async accept, обработка сессий, разбор сообщений по `\0`-разделителю, тот же handler что и D-Bus |
| `calculator::TCPClient` | `src/tcp_client.hpp`, `src/tcp_client.cpp` | Синхронный TCP клиент: connect/send_request/disconnect, `\0`-разделитель |
| `HistoryService` | `src/history_service.hpp`, `src/history_service.cpp` | Координация между кэшем и БД: проверка кэша → вычисление → сохранение |
| `calculator::Calculator` | `src/calculator.hpp`, `src/calculator.cpp` | Бизнес-логика: выполнение математических операций через libmath |
| `calculator::Task` | `src/task.hpp`, `src/task.cpp` | DTO для передачи данных: value1, value2, operation, result, status; JSON сериализация (nlohmann) |
| `storage::RedisCache` | `src/redis_cache.hpp` | Кэширование результатов с ключом формата `calc:{a}:{op}:{b}`, warmup из БД при старте |
| `storage::PostgresStorage` | `src/postgres_storage.hpp`, `src/postgres_storage.cpp` | Работа с PostgreSQL: создание схемы, вставка записей, загрузка истории |
| `Logger` | `src/logger.hpp`, `src/logger.cpp` | Singleton logger на базе spdlog, уровни: debug/info/warning/error |

### Поток выполнения запроса

1. Клиент вызывает D-Bus метод `Calculate` с JSON строкой
2. `DBusServer` принимает запрос, передает JSON обработчику в `app::run()`
3. JSON парсится в `calculator::Task` через `from_json()`
4. `HistoryService::process(task)`:
   - Проверяет кэш по ключу `calc:{value1}:{operation}:{value2}`
   - При hit: возвращает результат из кэша
   - При miss: вызывает `Calculator::execute(task)` → сохраняет в PostgreSQL → сохраняет в Redis
5. Результат сериализуется в JSON через `to_json()` и возвращается клиенту

## Структура репозитория

```
sys_calculator/
├── CMakeLists.txt              # Корневой CMake: таргеты, зависимости, CPack, Doxygen
├── AGENT.md                    # Документация для ИИ-агентов и разработчиков
├── README.md                   # Краткое руководство пользователя
├── Doxyfile.in                 # Шаблон конфигурации Doxygen
├── .clang-format               # Конфигурация clang-format
├── .clang-tidy                 # Конфигурация clang-tidy (пустой)
├── .clang-uml                  # Конфигурация Clang-UML для диаграмм
│
├── src/                        # Исходный код приложения
│   ├── main.cpp                # Точка входа: создает app и вызывает run()
│   ├── app.hpp/cpp             # Управление жизненным циклом, CLI args, signal handling
│   ├── dbus_server.hpp/cpp     # D-Bus сервер: регистрация интерфейса, event loop
│   ├── tcp_server.hpp/cpp      # TCP сервер на Boost.Asio (Pimpl)
│   ├── tcp_client.hpp/cpp      # TCP клиент (Pimpl)
│   ├── tcp_client_main.cpp     # main() утилиты calculator_client
│   ├── config.hpp/cpp          # Конфигурация: структура Config + парсер JSON
│   ├── calculator.hpp/cpp      # Бизнес-логика: выполнение операций через libmath
│   ├── task.hpp/cpp            # DTO: структура Task, JSON сериализация
│   ├── history_service.hpp/cpp # Координация кэша и БД
│   ├── redis_cache.hpp         # Redis кэширование (только header)
│   ├── postgres_storage.hpp/cpp# PostgreSQL хранилище
│   └── logger.hpp/cpp          # Singleton logger на spdlog
│
├── tests/                      # Интеграционные тесты
│   ├── CMakeLists.txt          # Настройка GTest, Valgrind memcheck
│   └── integration_test.cpp    # Integration test: D-Bus вызов сложения
│
├── etc/                        # Конфигурационные файлы для установки
│   ├── calculator.json         # Пример JSON конфига (устанавливается в /usr/local/etc)
│   ├── dbus-1/
│   │   └── system.d/
│   │       └── com.example.CalculatorService.conf  # D-Bus policy: доступ root и default
│   └── systemd/
│       └── system/
│           └── calculator.service  # systemd unit: Type=dbus, Restart=always
│
├── deb-configs/                # Скрипты Debian пакета
│   ├── preinst                 # Остановка сервиса перед установкой (systemctl stop calculator)
│   ├── postinst                # Настройка БД (роль calc, база calc), daemon-reload, enable сервиса
│   └── prerm                   # Остановка и отключение сервиса перед удалением (stop + disable)
│
├── cmake/                      # Вспомогательные CMake модули
│   └── hiredisConfig.cmake     # Конфиг для поиска hiredis
│
├── docs/                       # Генерируемая документация
│   └── html/                   # Doxygen HTML output
```

## Сборка и запуск

### Зависимости системы

```bash
sudo apt install libsdbus-c++-dev libhiredis-dev libspdlog-dev libpq-dev \
                 libboost-system-dev \
                 postgresql postgresql-contrib redis-server \
                 cmake g++ pkg-config
```

**Важно**: `redis-plus-plus` не доступен в apt, требуется ручная сборка:

```bash
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus && mkdir build && cd build
cmake ..
make && sudo make install
```

### Сборка проекта

```bash
mkdir build && cd build
cmake ..
make
```

**Опции сборки**:
- `-DENABLE_CLANG_TIDY=ON` — включить статический анализ clang-tidy (требует установленный clang-tidy)
- `-DENABLE_CLANG_FORMAT=ON` (по умолчанию) — добавить цель `format` для clang-format

**Дополнительные цели**:
- `make format` — форматирование кода через clang-format
- `make tidy` — запуск clang-tidy (если включен ENABLE_CLANG_TIDY)
- `make doc_doxygen` — генерация Doxygen документации
- `make package_calculator` — создание DEB пакета через CPack

### Локальный запуск

```bash
# Запуск вручную (требуются запущенные PostgreSQL и Redis)
# Режим и параметры берутся из конфига (по умолчанию /usr/local/etc/calculator.json)
./build/calculator --config etc/calculator.json

# Уровень логирования задаётся полем "log_level" в конфиге ("debug" | "info")

# В другом терминале — вызов через D-Bus (если mode = dbus_only|both)
busctl call com.example.CalculatorService \
            /com/example/CalculatorObject \
            com.example.CalculatorInterface \
            Calculate \
            s '{"firstValue": 5, "operation": "+", "secondValue": 3}'

# Или через TCP (если mode = tcp_only|both)
./build/calculator_client --host 127.0.0.1 --port 1234 --test
```

### Установка как системный сервис

```bash
# Создание DEB пакета
cd build
make package_calculator

# Установка пакета
sudo dpkg -i sys-calculator-1.3.0-Linux.deb

# postinst скрипт автоматически:
# 1. Создаст роль и БД в PostgreSQL (user: calc, db: calc, password: calc_password)
# 2. Выполнит daemon-reload
# 3. Включит и запустит сервис calculator
```

### Управление через systemctl

```bash
# Старт/стоп/рестарт
sudo systemctl start calculator
sudo systemctl stop calculator
sudo systemctl restart calculator

# Просмотр статуса
sudo systemctl status calculator

# Просмотр логов
sudo journalctl -u calculator -f

# Автозапуск при загрузке
sudo systemctl enable calculator
```

**Конфигурация сервиса** (`/etc/systemd/system/calculator.service`):
- `Type=dbus` — systemd ждет появления BusName на D-Bus перед считанием сервиса активным
- `BusName=com.example.CalculatorService` — имя D-Bus сервиса
- `Restart=always` — автоматический перезапуск при падении
- `RestartSec=3s` — задержка перед перезапуском
- Работает от root (требуется для system bus D-Bus)

## D-Bus интерфейс

### Параметры подключения

| Параметр | Значение |
|----------|----------|
| **Service Name** | `com.example.CalculatorService` |
| **Object Path** | `/com/example/CalculatorObject` |
| **Interface Name** | `com.example.CalculatorInterface` |
| **Bus Type** | System Bus (не Session Bus) |

### Методы

#### Calculate

Выполняет вычисление и возвращает результат.

**Сигнатура**:
```
Calculate(IN String json_request, OUT String json_response)
```

**Входной параметр** (JSON строка):
```json
{
  "firstValue": 5,
  "operation": "+",
  "secondValue": 3
}
```

Для унарной операции факториал (`!`) поле `secondValue` не требуется:
```json
{
  "firstValue": 5,
  "operation": "!"
}
```

**Поддерживаемые операции**:
- `+` — сложение
- `-` — вычитание
- `*` — умножение
- `/` — деление
- `^` — возведение в степень
- `!` — факториал (унарная операция, используется только firstValue)

**Выходной параметр** (JSON строка):

При успехе:
```json
{
  "firstValue": 5,
  "operation": "+",
  "secondValue": 3,
  "result": 8,
  "status": "success"
}
```

При ошибке:
```json
{
  "firstValue": 5,
  "operation": "/",
  "secondValue": 0,
  "status": "Error! Division by zero!"
}
```

**Коды статусов** (`Task::Status`):
| Код | Константа | Сообщение |
|-----|-----------|-----------|
| 0 | `OK` | "success" |
| -1 | `DIV_BY_ZERO` | "Error! Division by zero!" |
| -2 | `OVERFLOW` | "Error! Overflow!" |
| -3 | `INCORRECT_ARGUMENTS` | "Error! Incorrect arguments!" |
| 1 | `UNKNOWN_OPERATION` | "Error! Unknown error!" |
| 2 | `UNKNOWN` | "UNKNOWN STATUS" |

### Примеры вызова

```bash
# Сложение
busctl call com.example.CalculatorService \
            /com/example/CalculatorObject \
            com.example.CalculatorInterface \
            Calculate \
            s '{"firstValue": 5, "operation": "+", "secondValue": 3}'

# Деление на ноль (ошибка)
busctl call com.example.CalculatorService \
            /com/example/CalculatorObject \
            com.example.CalculatorInterface \
            Calculate \
            s '{"firstValue": 10, "operation": "/", "secondValue": 0}'

# Факториал
busctl call com.example.CalculatorService \
            /com/example/CalculatorObject \
            com.example.CalculatorInterface \
            Calculate \
            s '{"firstValue": 5, "operation": "!"}'
```

### Политика доступа D-Bus

Файл: `/etc/dbus-1/system.d/com.example.CalculatorService.conf`

- `root` может владеть сервисом (`own`)
- Все пользователи могут отправлять сообщения сервису (`send_destination`)

## TCP интерфейс

Помимо D-Bus, сервис может принимать те же вычислительные запросы по TCP. Логика обработки полностью переиспользуется: и D-Bus, и TCP вызывают один и тот же handler (`HistoryService::process`), поэтому кэширование, сохранение в БД и коды статусов идентичны.

### Параметры подключения

| Параметр | Значение по умолчанию | Описание |
|----------|-----------------------|----------|
| `tcp_address` | `0.0.0.0` | Адрес прослушивания |
| `tcp_port` | `1234` | Порт прослушивания |

### Протокол обмена

- Транспорт: raw TCP.
- Формат сообщения: JSON строка (тот же формат, что и в D-Bus методе `Calculate`), завершённая **нулевым байтом** (`\0`).
- Ответ: JSON строка результата, также завершённая `\0`.
- В рамках одного соединения можно отправлять несколько запросов подряд — сервер обрабатывает каждое сообщение по мере получения полного (до `\0`), корректно переживая фрагментацию TCP.

Пример запроса (без учёта завершающего `\0`):
```json
{"firstValue": 5, "operation": "+", "secondValue": 3}
```
Пример ответа:
```json
{"firstValue":5,"operation":"+","secondValue":3,"result":8,"status":"success"}
```

Поддерживаемые операции и коды статусов совпадают с D-Bus интерфейсом (см. таблицу выше). Поле `result` присутствует только при `status == "success"`.

### Клиентская утилита `calculator_client`

Отдельный исполняемый файл для обращения к TCP серверу.

```bash
# Интерактивный режим: JSON-запросы построчно (Ctrl+D для выхода)
calculator_client --host 127.0.0.1 --port 1234

# Автоматический тест: 10 запросов, включая edge cases
calculator_client --host 127.0.0.1 --port 1234 --test

# С отладочным логом
calculator_client --host 127.0.0.1 --port 1234 --debug
```

## Конфигурация

Все параметры сервиса задаются в JSON файле. Путь указывается флагом `--config` (по умолчанию `/usr/local/etc/calculator.json`). Пример-шаблон лежит в репозитории: `etc/calculator.json`.

```json
{
    "mode": "both",
    "tcp_address": "0.0.0.0",
    "tcp_port": 1234,
    "log_level": "info",
    "postgres_connection": "host=localhost dbname=calc user=calc password=calc_password",
    "redis_uri": "tcp://127.0.0.1:6379"
}
```

| Поле | Тип | По умолчанию | Описание |
|------|-----|--------------|----------|
| `mode` | string | `dbus_only` | Режим: `dbus_only` (или `dbus`), `tcp_only` (или `tcp`), `both` |
| `tcp_address` | string | `0.0.0.0` | Адрес TCP сервера (используется в `tcp_only`/`both`) |
| `tcp_port` | int | `1234` | Порт TCP сервера (1..65535) |
| `log_level` | string | `info` | `debug` включает отладочный лог, иначе `info` |
| `postgres_connection` | string | `host=localhost dbname=calc user=calc password=calc_password` | Строка подключения к PostgreSQL |
| `redis_uri` | string | `tcp://127.0.0.1:6379` | URI подключения к Redis |

**Поведение парсера** (`Config::load_from_file`):
- Файл не существует / не читается → `std::runtime_error` с понятным сообщением, сервис не стартует.
- Невалидный JSON → `std::runtime_error` (перехват `nlohmann::json::exception`).
- Отсутствующие поля → используются значения по умолчанию.
- Некорректное значение `mode` или `tcp_port` вне диапазона → ошибка валидации.

### Запуск в разных режимах

```bash
# Режим задаётся полем "mode" в конфиге
./calculator --config etc/calculator.json          # both (по умолчанию в примере)

# Только TCP: в конфиге "mode": "tcp_only"
# Только D-Bus: в конфиге "mode": "dbus_only"
```

## Хранение данных

### PostgreSQL

**Подключение** (hardcoded в `app.cpp`):
```
host=localhost dbname=calc user=calc password=calc_password
```

**Схема** (создается автоматически при старте через `PostgresStorage::createSchema()`):

```sql
CREATE TABLE IF NOT EXISTS calc_history(
    value1    INTEGER NOT NULL,
    operation CHAR(1) NOT NULL,
    value2    INTEGER NOT NULL,
    status    INTEGER NOT NULL,
    result    INTEGER NOT NULL,
    PRIMARY KEY (value1, operation, value2)
);
```

**Поля таблицы `calc_history`**:
- `value1` — первый операнд
- `operation` — операция (один символ: +, -, *, /, ^, !)
- `value2` — второй операнд (для унарных операций обычно 0)
- `status` — код статуса операции (целое число, соответствует `Task::Status`)
- `result` — результат вычисления
- **Primary Key**: `(value1, operation, value2)` — обеспечивает уникальность записи для каждого набора параметров

**Особенности**:
- `INSERT ... ON CONFLICT ... DO NOTHING` — идемпотентная вставка, предотвращает дубликаты
- Автоматическое переподключение при потере соединения (`ensureConnection()`)
- Метод `loadHistory()` загружает всю историю для warmup кэша

**Настройка при установке**:
Скрипт `deb-configs/postinst` создает:
- Роль PostgreSQL: `calc` с паролем `calc_password`
- Базу данных: `calc` с владельцем `calc`

### Redis

**Подключение** (hardcoded в `app.cpp`):
```
tcp://127.0.0.1:6379
```

**Формат ключа**:
```
calc:{a}:{operation}:{b}
```
Для коммутативных операций (`+`, `*`) значения сортируются: если `a > b`, они меняются местами для обеспечения консистентности кэша.

**Формат значения**:
```
{result}:{status_code}
```
Пример: `"8:0"` означает результат 8 со статусом OK (0).

**Warmup**:
При старте `HistoryService` загружает всю историю из PostgreSQL и заполняет Redis через pipeline для ускорения последующих запросов.

## Тестирование

### Запуск интеграционных тестов

```bash
cd build
make integration_tests
ctest --verbose
# или напрямую
./tests/integration_tests
```

**Текущие тесты**:
- `IntegrationTest.AdditionOperation` — проверяет сложение 5 + 3 = 8 через реальный D-Bus вызов

### Valgrind Memcheck

Если установлен Valgrind, автоматически создается тест `integration_tests_memcheck`:

```bash
ctest -R memcheck --verbose
# или
valgrind --leak-check=full \
         --show-leak-kinds=definite,indirect \
         --track-origins=yes \
         --error-exitcode=1 \
         ./tests/integration_tests
```

**Параметры проверки**:
- `--leak-check=full` — полная проверка утечек
- `--show-leak-kinds=definite,indirect` — показывать определенные и косвенные утечки
- `--track-origins=yes` — отслеживать источники неинициализированных значений
- `--error-exitcode=1` — вернуть код ошибки 1 при обнаружении проблем
- `TIMEOUT 300` — таймаут 5 минут (Valgrind замедляет выполнение в 10-30 раз)

### Статический анализ (clang-tidy)

```bash
cmake -DENABLE_CLANG_TIDY=ON ..
make tidy
```


### Форматирование кода (clang-format)

```bash
make format
```

Конфигурация: `.clang-format` (присутствует, 3.3KB)

## Соглашения и принципы разработки

### Управление памятью

- **RAII**: повсеместное использование `std::unique_ptr` для управления ресурсами
  - `DBusServer::Impl` — unique_ptr
  - `app::Impl` — unique_ptr
  - `PGconnPtr`, `PGresultPtr` — unique_ptr с custom deleters (`PQfinish`, `PQclear`)
  - `HistoryService` владеет `unique_ptr<PostgresStorage>` и `unique_ptr<RedisCache>`
- **Запрет копирования**: классы `app`, `Logger` имеют deleted copy constructor/assignment
- **Move semantics**: частично реализован (app имеет deleted move, но Impl перемещается через unique_ptr)

### Обработка ошибок

- **Fail-fast на старте**: если PostgreSQL недоступен при инициализации, сервис не запускается (systemd перезапустит)
- **Graceful degradation**: если Redis недоступен во время работы, ошибки логируются, но сервис продолжает работать (только без кэша)
- **Исключения**: используются `std::exception`, `sdbus::Error`, `nlohmann::json::exception`
- **Статусы операций**: все ошибки вычислений кодируются в `Task::Status` и передаются клиенту

### Логирование

- **Singleton паттерн**: `Logger::instance()` возвращает единственный экземпляр
- **Уровни**: debug (включается флагом `-d`), info, warning, error
- **Библиотека**: spdlog с fmt-style форматированием
- **Инициализация**: `Logger::init(is_debug)` вызывается в `app::run()`

### Стиль кода

- **Пространства имен**: 
  - `calculator::` — основные классы приложения
  - `storage::` — классы хранения данных
- **Именование**:
  - Классы: PascalCase (`DBusServer`, `HistoryService`)
  - Методы: snake_case (`async_run`, `createSchema`)
  - Приватные члены: префикс `m_` (`m_impl`, `m_connection`)
  - Константы: UPPER_CASE (`SERVICE_NAME`, `OBJECT_PATH`)
- **Pimpl идиома**: используется в `app` и `DBusServer` для скрытия реализации
- **Header-only**: `RedisCache` реализован полностью в header file
- **Форматирование**: clang-format (конфиг `.clang-format`)

### Thread Safety

- `Logger::instance()` — thread-safe singleton (static local variable, C++11 guarantee)
- `DBusServer::async_run()` — запускает event loop в отдельном потоке
- Signal handling через `signalfd` и `sigwait` — безопасная обработка сигналов в многопоточной среде

## Известные ограничения и TODO

### Конфигурация вынесена в JSON

Ранее захардкоженные в `app.cpp` параметры (PostgreSQL connection string, Redis URI) теперь читаются из JSON конфига (`Config`, см. раздел «Конфигурация»). Значения по умолчанию совпадают с прежними хардкодами, так что поведение без конфига не меняется.

### Возможные улучшения

- TLS/аутентификация для TCP соединений (сейчас соединение открытое).
- `Type=dbus` в systemd unit ждёт появления BusName — для чистого `tcp_only` режима стоит переключить на `Type=simple`.


## Быстрый старт для разработчиков

### 1. Настройка окружения

```bash
# Установка зависимостей
sudo apt install libsdbus-c++-dev libhiredis-dev libspdlog-dev libpq-dev \
                 libboost-system-dev \
                 postgresql postgresql-contrib redis-server \
                 cmake g++ pkg-config clang-format valgrind

# Сборка redis-plus-plus
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus && mkdir build && cd build
cmake .. && make && sudo make install
```

### 2. Клонирование и сборка

```bash
git clone https://github.com/DanilDill/sys_calculator.git
cd sys_calculator
mkdir build && cd build
cmake ..
make
```

### 3. Запуск сервисов

```bash
# Запуск PostgreSQL и Redis
sudo systemctl start postgresql
sudo systemctl start redis-server

# Создание БД и пользователя (если не через пакет)
sudo -u postgres psql -c "CREATE USER calc WITH PASSWORD 'calc_password'"
sudo -u postgres createdb -O calc calc
```

### 4. Запуск приложения

```bash
# В режиме debug с подробными логами
./calculator -d

# В другом терминале — тестовый вызов
busctl call com.example.CalculatorService \
            /com/example/CalculatorObject \
            com.example.CalculatorInterface \
            Calculate \
            s '{"firstValue": 10, "operation": "*", "secondValue": 5}'
```

### 5. Запуск тестов

```bash
cd build
make integration_tests
ctest --verbose

# С Valgrind
ctest -R memcheck --verbose
```

### 6. Сборка пакета

```bash
cd build
make package_calculator
ls -lh sys-calculator-*.deb
```

## Полезные команды

```bash
# Просмотр D-Bus сервиса
busctl list | grep Calculator
busctl tree com.example.CalculatorService
busctl introspect com.example.CalculatorService /com/example/CalculatorObject

# Мониторинг логов сервиса
sudo journalctl -u calculator -f

# Проверка подключения к БД
psql -h localhost -U calc -d calc -c "SELECT * FROM calc_history LIMIT 5;"

# Проверка Redis
redis-cli keys "calc:*"
redis-cli get "calc:5:+:3"

# Форматирование кода
make format

# Статический анализ (если включен)
make tidy
```
