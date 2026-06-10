# 📋 План рефакторинга sys_calculator в ООП стиль

## 🎯 Цель задания

Переписать проект `sys_calculator` из процедурного стиля (C-style) в объектно-ориентированный (C++ OOP), добавить новые функции и улучшить архитектуру согласно современным стандартам C++.

---

## 📊 Этап 0: Подготовка

### 0.1 Создание ветки разработки

```bash
git checkout -b feat/oop-refactoring
```

**Conventional Commit:** `chore: create feature branch for OOP refactoring`

### 0.2 Анализ текущей архитектуры

**Текущие компоненты:**
- `src/main.cpp` - точка входа, парсинг CLI аргументов
- `src/task.hpp/cpp` - структура данных Task и функции её создания/парсинга
- `src/calculator.hpp/cpp` - логика вычислений
- `src/dbus.hpp/cpp` - работа с DBus (низкоуровневый C API)
- `src/args_parser.hpp/cpp` - парсинг строковых выражений
- Внешняя библиотека `libmath` - математические операции (C-style)

**Проблемы текущей реализации:**
- ❌ Свободные функции вместо методов классов
- ❌ Ручное управление памятью (malloc/free)
- ❌ Нет обработки исключений
- ❌ Нет логирования
- ❌ Нет тестов
- ❌ Использование C-style DBus API вместо sdbus-cpp
- ❌ Нет поддержки JSON формата
- ❌ Нет обработки системных сигналов

---

## 🏗️ Этап 1: Проектирование архитектуры

### 1.1 Проработка архитектуры приложения

**Цель:** Создать детальную архитектуру приложения перед началом кодирования.

**Задачи:**
1. Определить все необходимые классы и их ответственность
2. Спроектировать взаимодействие между компонентами
3. Определить интерфейсы и контракты
4. Спланировать паттерны проектирования
5. Подготовить структуру проекта

**Результат:** Документ с архитектурным решением (этот файл)

### 1.2 Архитектурная концепция

**Предлагаемая архитектура:**

```
namespace calculator {

// ===== Core Domain =====
class Task                    // Инкапсуляция задачи вычисления
class Calculator              // Бизнес-логика вычислений
class ExpressionParser        // Парсинг математических выражений

// ===== Math Library (refactored from libmath) =====
class MathOperations          // ООП обёртка над математическими операциями

// ===== Infrastructure =====
class Logger                  // Singleton для логирования (обёртка над spdlog)
class JsonConverter           // Конвертация Task <-> JSON (nlohmann/json)
class DbusServer              // DBus сервер (sdbus-cpp)
class SignalHandler           // Обработка системных сигналов

// ===== Application =====
class Application             // Главный класс приложения (Facade)

} // namespace calculator
```

### 1.3 Детальное описание классов

#### Класс `Task` (Value Object)

**Ответственность:** Хранение данных задачи вычисления и сериализация в JSON.

```cpp
class Task {
public:
    enum class Operation { Add, Subtract, Multiply, Divide, Power, Factorial };
    enum class Status { Success, DivisionByZero, UnknownOperation, 
                       Overflow, InvalidArguments, ParseError };
    
    // Конструкторы
    Task(int firstValue, Operation op, int secondValue = 0);
    Task(const nlohmann::json& json);  // Конструктор из JSON
    
    // Геттеры
    int getFirstValue() const;
    Operation getOperation() const;
    int getSecondValue() const;
    int getResult() const;
    Status getStatus() const;
    
    // Сеттеры
    void setResult(int result);
    void setStatus(Status status);
    
    // Сериализация
    nlohmann::json toJson() const;
    static Task fromJson(const nlohmann::json& json);
    
private:
    int firstValue_;
    Operation operation_;
    int secondValue_;
    int result_ = 0;
    Status status_ = Status::Success;
};
```

**JSON формат:**
```json
// Input
{
    "firstValue": 5,
    "operation": "+",
    "secondValue": 3
}

// Output
{
    "firstValue": 5,
    "operation": "+",
    "secondValue": 3,
    "result": 8,
    "status": "success"
}
```

#### Класс `MathOperations` (Refactored libmath)

**Ответственность:** Выполнение математических операций с обработкой ошибок через исключения.

```cpp
class MathOperations {
public:
    static int add(int a, int b);
    static int subtract(int a, int b);
    static int multiply(int a, int b);
    static int divide(int a, int b);      // Бросает DivisionByZeroException
    static int power(int base, int exp);  // Бросает OverflowException
    static int factorial(int n);          // Бросает OverflowException или InvalidArgumentException
    
private:
    // Вспомогательные методы для проверки переполнения
    static void checkOverflow(long long result);
    static void checkNegativeForFactorial(int n);
};
```

**Кастомные исключения:**
```cpp
class CalculationException : public std::exception {
public:
    explicit CalculationException(const std::string& message);
    const char* what() const noexcept override;
protected:
    std::string message_;
};

class DivisionByZeroException : public CalculationException { ... };
class OverflowException : public CalculationException { ... };
class InvalidArgumentException : public CalculationException { ... };
```

#### Класс `Calculator` (Business Logic)

**Ответственность:** Координация вычислений, использование MathOperations.

```cpp
class Calculator {
public:
    void calculate(Task& task);  // Выполняет вычисление, может бросать исключения
    
private:
    int performCalculation(int a, Task::Operation op, int b) const;
};
```

#### Класс `ExpressionParser` (Parsing Logic)

**Ответственность:** Парсинг строковых выражений типа "5 + 3" или "5!".

```cpp
class ExpressionParser {
public:
    Task parse(const std::string& expression);  // Парсит выражение
    
private:
    std::vector<std::string> tokenize(const std::string& expr) const;
    Task::Operation parseOperation(const std::string& op) const;
    bool isValidNumber(const std::string& str) const;
};
```

#### Класс `Logger` (Singleton - Meyer's)

**Ответственность:** Централизованное логирование через spdlog (скрытая реализация).

```cpp
class Logger {
public:
    static Logger& getInstance();
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    template<typename... Args>
    void debug(const std::string& format, const Args&... args);
    template<typename... Args>
    void info(const std::string& format, const Args&... args);
    template<typename... Args>
    void warning(const std::string& format, const Args&... args);
    template<typename... Args>
    void error(const std::string& format, const Args&... args);
    
    // Запрещаем копирование и перемещение
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
private:
    Logger();  // Инициализирует spdlog::logger внутри
    ~Logger();
    
    std::unique_ptr<spdlog::logger> logger_;  // Скрытая реализация
};
```

**Важно:** Пользовательский код не должен знать о spdlog! Все детали реализации скрыты.

**Настройка вывода:** Логи выводятся в syslog/journalctl через `spdlog::syslog_logger`.

**Формат логов:** `[timestamp] [level] message`

**Примеры:**
```
[2024-01-15 10:30:45.123] [info] Application starting...
[2024-01-15 10:30:45.125] [info] DBus server initialized on com.example.CalculatorService
[2024-01-15 10:30:50.456] [info] Received calculation request: {"firstValue":5,"operation":"+","secondValue":3}
[2024-01-15 10:30:50.457] [debug] Calculating: 5 + 3
[2024-01-15 10:30:50.458] [info] Result: 8
[2024-01-15 10:31:00.789] [warning] Received SIGTERM signal, shutting down...
[2024-01-15 10:31:00.790] [info] Application stopped gracefully
```

#### Класс `JsonConverter` (Utility)

**Ответственность:** Конвертация между Task и JSON.

```cpp
class JsonConverter {
public:
    static nlohmann::json taskToJson(const Task& task);
    static Task jsonToTask(const nlohmann::json& json);
    
    static std::string serialize(const nlohmann::json& json);
    static nlohmann::json deserialize(const std::string& jsonString);
    
private:
    static Task::Operation stringToOperation(const std::string& op);
    static std::string operationToString(Task::Operation op);
    static Task::Status stringToStatus(const std::string& status);
    static std::string statusToString(Task::Status status);
};
```

#### Класс `DbusServer` (Infrastructure)

**Ответственность:** DBus сервер на базе sdbus-cpp.

```cpp
class DbusServer {
public:
    using CalculateCallback = std::function<std::string(const std::string&)>;
    
    explicit DbusServer(CalculateCallback callback);
    ~DbusServer();
    
    void start();   // Запускает event loop
    void stop();    // Останавливает event loop
    
    // Запрещаем копирование
    DbusServer(const DbusServer&) = delete;
    DbusServer& operator=(const DbusServer&) = delete;
    
private:
    void setupDBusInterface();
    
    std::unique_ptr<sdbus::IConnection> connection_;
    std::unique_ptr<sdbus::IObject> object_;
    CalculateCallback callback_;
    bool running_ = false;
};
```

**Константы DBus:**
```cpp
constexpr const char* SERVICE_NAME = "com.example.CalculatorService";
constexpr const char* OBJECT_PATH = "/com/example/CalculatorObject";
constexpr const char* INTERFACE_NAME = "com.example.CalculatorInterface";
constexpr const char* METHOD_NAME = "Calculate";
```

#### Класс `SignalHandler` (System Integration)

**Ответственность:** Обработка системных сигналов SIGTERM и SIGHUP.

```cpp
class SignalHandler {
public:
    using SignalCallback = std::function<void(int)>;
    
    explicit SignalHandler(SignalCallback callback);
    ~SignalHandler();
    
    void registerSignals();  // Регистрирует SIGTERM, SIGHUP
    void waitForSignal();    // Блокирующий вызов, ждёт сигнал
    
    // Запрещаем копирование
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    
private:
    sigset_t signalSet_;
    SignalCallback callback_;
    int signalFd_;
};
```

#### Класс `Application` (Facade/Orchestrator)

**Ответственность:** Главный класс приложения, координирует все компоненты.

```cpp
class Application {
public:
    Application();
    ~Application();
    
    void run(int argc, char** argv);
    void stop();
    
private:
    void parseCliArguments(int argc, char** argv);
    void initializeComponents();
    void setupSignalHandlers();
    void startDBusServer();
    void waitForShutdownSignal();
    void gracefulShutdown();
    
    // Callback для DBus
    std::string handleCalculateRequest(const std::string& jsonInput);
    
    // Вспомогательные методы
    Task parseTaskFromJson(const std::string& jsonInput);
    void executeCalculation(Task& task);
    std::string formatResultAsJson(const Task& task);
    
    // Компоненты
    Logger& logger_;
    std::unique_ptr<ExpressionParser> parser_;
    std::unique_ptr<Calculator> calculator_;
    std::unique_ptr<DbusServer> dbusServer_;
    std::unique_ptr<SignalHandler> signalHandler_;
    bool isRunning_ = false;
};
```

### 1.4 Диаграммы взаимодействия

#### Class Diagram (будет создана clang-uml после реализации)

Структура классов и их отношений:
- `Application` агрегирует: `Logger`, `ExpressionParser`, `Calculator`, `DbusServer`, `SignalHandler`
- `Calculator` использует: `MathOperations`
- `Task` используется всеми компонентами
- `JsonConverter` работает с `Task` и `nlohmann::json`
- `Logger` - Singleton, используется всеми компонентами

#### Sequence Diagram: Обработка запроса Calculate

```
Client -> DBus: Calculate('{"firstValue":5,"operation":"+","secondValue":3}')
DBus -> Application: handleCalculateRequest(json)
Application -> Logger: info("Received calculation request")
Application -> JsonConverter: deserialize(json)
JsonConverter --> Application: Task object
Application -> Logger: debug("Calculating: 5 + 3")
Application -> Calculator: calculate(task)
Calculator -> MathOperations: add(5, 3)
MathOperations --> Calculator: 8
Calculator --> Application: task with result=8
Application -> Logger: info("Result: 8")
Application -> JsonConverter: serialize(task)
JsonConverter --> Application: json string
Application --> DBus: return '{"result":8,"status":"success",...}'
DBus --> Client: response
```

#### Sequence Diagram: Graceful Shutdown

```
systemd -> SignalHandler: SIGTERM signal
SignalHandler -> Application: callback(SIGTERM)
Application -> Logger: warning("Received SIGTERM, shutting down")
Application -> DbusServer: stop()
DbusServer -> Application: event loop stopped
Application -> Logger: info("Application stopped gracefully")
Application --> systemd: exit(0)
```

### 1.5 Структура проекта

```
sys_calculator/
├── src/
│   ├── main.cpp                      # Точка входа
│   ├── application.hpp               # Application class
│   ├── application.cpp
│   ├── task.hpp                      # Task class
│   ├── task.cpp
│   ├── calculator.hpp                # Calculator class
│   ├── calculator.cpp
│   ├── math_operations.hpp           # MathOperations class (refactored libmath)
│   ├── math_operations.cpp
│   ├── expression_parser.hpp         # ExpressionParser class
│   ├── expression_parser.cpp
│   ├── logger.hpp                    # Logger singleton
│   ├── logger.cpp
│   ├── json_converter.hpp            # JsonConverter utility
│   ├── json_converter.cpp
│   ├── dbus_server.hpp               # DbusServer class
│   ├── dbus_server.cpp
│   ├── signal_handler.hpp            # SignalHandler class
│   ├── signal_handler.cpp
│   └── exceptions.hpp                # Custom exceptions
├── tests/
│   ├── CMakeLists.txt
│   ├── test_task.cpp
│   ├── test_calculator.cpp
│   ├── test_math_operations.cpp
│   └── test_expression_parser.cpp
├── docs/
│   └── uml/
│       ├── class_diagram.svg
│       └── sequence_diagram.svg
├── etc/
│   └── dbus-1/
│       └── system.d/
│           └── com.example.CalculatorService.conf
├── deb-configs/
│   ├── preinst
│   ├── postinst
│   └── prerm
├── CMakeLists.txt
├── README.md
├── coding_plan.md                    # Этот файл
├── .clang-format
└── .clang-tidy
```

---

## 🔧 Этап 2: Настройка зависимостей

### 2.1 Установка системных зависимостей (Ubuntu 24.04)

```bash
sudo apt update
sudo apt install -y \
    libsdbus-c++-dev \
    nlohmann-json3-dev \
    libspdlog-dev \
    libgtest-dev \
    cmake \
    build-essential \
    pkg-config \
    libdbus-1-dev \
    clang-uml
```

**Conventional Commit:** `build: install system dependencies via apt`

### 2.2 Обновление CMakeLists.txt

**Изменения:**
- Убрать FetchContent для внешних библиотек (теперь устанавливаются через apt)
- Добавить find_package для всех зависимостей
- Обновить target_link_libraries
- Добавить цель для тестов

```cmake
# Найти зависимости
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

find_package(sdbus-c++ REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)

# Обновить линковку
target_link_libraries(calculator
    PRIVATE
        sdbus-c++
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        ${DBUS_LIBRARIES}
)

# Добавить тесты
enable_testing()
add_subdirectory(tests)
```

---

## 💻 Этап 3: Реализация исключений и утилит

### 3.1 Коммит: `feat: implement custom exception hierarchy`

**Файлы:**
- `src/exceptions.hpp` (новый)

**Что реализовать:**
```cpp
namespace calculator {

class CalculationException : public std::exception {
public:
    explicit CalculationException(const std::string& message);
    const char* what() const noexcept override;
    
protected:
    std::string message_;
};

class DivisionByZeroException : public CalculationException {
public:
    DivisionByZeroException();
};

class OverflowException : public CalculationException {
public:
    explicit OverflowException(const std::string& operation);
};

class InvalidArgumentException : public CalculationException {
public:
    explicit InvalidArgumentException(const std::string& reason);
};

} // namespace calculator
```

### 3.2 Коммит: `feat: implement Logger singleton with spdlog backend`

**Файлы:**
- `src/logger.hpp` (новый)
- `src/logger.cpp` (новый)

**Что реализовать:**
- Meyer's Singleton паттерн
- Скрыть spdlog детали за unique_ptr
- Настроить syslog logger для journalctl
- Реализовать уровни: debug, info, warning, error
- Поддержка форматированных сообщений (variadic templates)

**Важно:** Пользовательский код НЕ должен видеть spdlog типы!

---

## 🏗️ Этап 4: Реализация Core Domain (3 коммита)

### 4.1 Коммит: `feat: implement MathOperations class with exception handling`

**Файлы:**
- `src/math_operations.hpp` (новый)
- `src/math_operations.cpp` (новый)

**Что сделать:**
- Переписать все функции из libmath в статические методы класса
- Заменить возврат статусов на исключения
- Добавить проверки переполнения
- Добавить валидацию входных данных
- Логировать ошибки через Logger

**Методы:**
- `add(int a, int b)` - сложение
- `subtract(int a, int b)` - вычитание
- `multiply(int a, int b)` - умножение
- `divide(int a, int b)` - деление (бросает DivisionByZeroException)
- `power(int base, int exp)` - возведение в степень (бросает OverflowException)
- `factorial(int n)` - факториал (бросает OverflowException, InvalidArgumentException)

### 4.2 Коммит: `feat: implement Task class with JSON serialization`

**Файлы:**
- `src/task.hpp` (полностью переписать)
- `src/task.cpp` (полностью переписать)

**Что сделать:**
- Создать enum классы `Operation` и `Status`
- Реализовать конструкторы, геттеры, сеттеры
- Добавить сериализацию/десериализацию JSON через nlohmann/json
- Использовать исключения для ошибок валидации
- Удалить старый C-style код (malloc/free)

### 4.3 Коммит: `feat: implement Calculator class orchestrating MathOperations`

**Файлы:**
- `src/calculator.hpp` (полностью переписать)
- `src/calculator.cpp` (полностью переписать)

**Что сделать:**
- Создать метод `calculate(Task& task)`
- Использовать MathOperations для вычислений
- Обрабатывать исключения от MathOperations и устанавливать статус в Task
- Логировать процесс вычисления

---

## 🔍 Этап 5: Реализация парсера выражений

### 5.1 Коммит: `feat: implement ExpressionParser for string expressions`

**Файлы:**
- `src/expression_parser.hpp` (новый)
- `src/expression_parser.cpp` (новый)

**Что сделать:**
- Переписать логику из `args_parser.cpp` в ООП стиль
- Парсинг выражений типа `"5 + 3"` или `"5!"`
- Токенизация строки
- Валидация входных данных
- Использовать исключения для ошибок парсинга
- Интегрировать с классом Task

**Поддерживаемые форматы:**
- `"5 + 3"` → Task(5, Add, 3)
- `"10 - 4"` → Task(10, Subtract, 4)
- `"5!"` → Task(5, Factorial, 0)

---

## 🛠️ Этап 6: Реализация Infrastructure (3 коммита)

### 6.1 Коммит: `feat: implement JsonConverter utility`

**Файлы:**
- `src/json_converter.hpp` (новый)
- `src/json_converter.cpp` (новый)

**Что сделать:**
- Статические методы для конвертации Task <-> JSON
- Маппинг enum Operation ↔ string ("+", "-", "*", "/", "^", "!")
- Маппинг enum Status ↔ string ("success", "error_division_by_zero", ...)
- Валидация JSON структуры
- Обработка ошибок через исключения

### 6.2 Коммит: `feat: implement DbusServer with sdbus-cpp`

**Файлы:**
- `src/dbus_server.hpp` (новый)
- `src/dbus_server.cpp` (новый)

**Что сделать:**
- Переписать на sdbus-cpp API (вместо низкоуровневого C API)
- Асинхронный event loop
- Callback механизм для вызова бизнес-логики
- Методы start()/stop() для управления lifecycle
- Логирование событий DBus
- Удалить старые файлы `dbus.hpp` и `dbus.cpp`

**Пример использования sdbus-cpp:**
```cpp
auto connection = sdbus::createSystemBusConnection(SERVICE_NAME);
auto object = sdbus::createObject(*connection, OBJECT_PATH);

object->registerMethod(INTERFACE_NAME, "Calculate")
      .onInterface(INTERFACE_NAME)
      .withInputParamNames("expression")
      .withOutputParamNames("result")
      .implementedAs([this](const std::string& json) {
          return callback_(json);
      });

object->finishRegistration();
connection->enterEventLoopAsync();
```

### 6.3 Коммит: `feat: implement SignalHandler for graceful shutdown`

**Файлы:**
- `src/signal_handler.hpp` (новый)
- `src/signal_handler.cpp` (новый)

**Что сделать:**
- Регистрация обработчиков SIGTERM и SIGHUP через signalfd
- Блокирующий метод waitForSignal()
- Callback для уведомления Application о необходимости остановки
- Логирование полученных сигналов
- Корректная очистка ресурсов в деструкторе

---

## 🎯 Этап 7: Реализация Application facade

### 7.1 Коммит: `feat: implement Application class as main orchestrator`

**Файлы:**
- `src/application.hpp` (новый)
- `src/application.cpp` (новый)
- `src/main.cpp` (переписать)

**Что сделать:**
- Реализовать метод `run(int argc, char** argv)`
- Инициализация всех компонентов в конструкторе или отдельном методе
- Парсинг CLI аргументов (--help)
- Запуск DBus server в асинхронном режиме
- Ожидание сигналов в главном потоке через SignalHandler
- Graceful shutdown: остановка DBus, освобождение ресурсов
- Логирование всех этапов запуска/остановки
- Обработка исключений на верхнем уровне

**Новый main.cpp:**
```cpp
#include "application.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv)
{
    try {
        calculator::Application app;
        app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```

---

## 🧪 Этап 8: Unit тесты

### 8.1 Коммит: `test: add unit tests for MathOperations`

**Файлы:**
- `tests/test_math_operations.cpp` (новый)
- `tests/CMakeLists.txt` (новый)

**Что тестировать:**
- ✅ Сложение: 2 + 3 = 5
- ✅ Вычитание: 10 - 4 = 6
- ✅ Умножение: 3 * 7 = 21
- ✅ Деление: 10 / 2 = 5
- ✅ Деление на ноль: должно бросать DivisionByZeroException
- ✅ Возведение в степень: 2 ^ 3 = 8
- ✅ Переполнение при возведении в степень: должно бросать OverflowException
- ✅ Факториал: 5! = 120
- ✅ Факториал отрицательного числа: должно бросать InvalidArgumentException
- ✅ Переполнение факториала: должно бросать OverflowException

### 8.2 Коммит: `test: add unit tests for Task and JsonConverter`

**Файлы:**
- `tests/test_task.cpp` (новый)

**Что тестировать:**
- ✅ Создание Task с валидными параметрами
- ✅ Геттеры возвращают правильные значения
- ✅ Сеттеры изменяют состояние
- ✅ Сериализация в JSON корректна
- ✅ Десериализация из JSON восстанавливает объект
- ✅ Round-trip: toJson → fromJson даёт тот же объект
- ✅ Некорректный JSON бросает исключение
- ✅ Отсутствующие поля в JSON бросают исключение

### 8.3 Коммит: `test: add unit tests for Calculator and ExpressionParser`

**Файлы:**
- `tests/test_calculator.cpp` (новый)
- `tests/test_expression_parser.cpp` (новый)

**test_calculator.cpp:**
- ✅ Calculator корректно вычисляет все операции через Task
- ✅ Статус Task обновляется после вычисления
- ✅ Результат Task заполняется правильно
- ✅ Исключения от MathOperations обрабатываются корректно

**test_expression_parser.cpp:**
- ✅ Парсинг "5 + 3" → Task(5, Add, 3)
- ✅ Парсинг "10 - 4" → Task(10, Subtract, 4)
- ✅ Парсинг "3 * 7" → Task(3, Multiply, 7)
- ✅ Парсинг "10 / 2" → Task(10, Divide, 2)
- ✅ Парсинг "2 ^ 3" → Task(2, Power, 3)
- ✅ Парсинг "5!" → Task(5, Factorial, 0)
- ✅ Пустая строка: должно бросать исключение
- ✅ Некорректное выражение "abc": должно бросать исключение
- ✅ Неизвестная операция "5 % 3": должно бросать исключение

### 8.4 Настройка CMake для тестов

**tests/CMakeLists.txt:**
```cmake
include(GoogleTest)

# Создать библиотеку с исходниками для тестирования
add_library(calculator_core
    ../src/task.cpp
    ../src/calculator.cpp
    ../src/math_operations.cpp
    ../src/expression_parser.cpp
    ../src/json_converter.cpp
    ../src/logger.cpp
)

target_link_libraries(calculator_core
    PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

# Создать исполняемый файл тестов
add_executable(calculator_tests
    test_math_operations.cpp
    test_task.cpp
    test_calculator.cpp
    test_expression_parser.cpp
)

target_link_libraries(calculator_tests
    PRIVATE
        calculator_core
        GTest::gtest_main
        GTest::gmock_main
)

# Обнаружить тесты автоматически
gtest_discover_tests(calculator_tests)
```

**Обновить корневой CMakeLists.txt:**
```cmake
enable_testing()
add_subdirectory(tests)
```

---

## 📝 Этап 9: Интеграция логирования

### 9.1 Коммит: `feat: integrate logging throughout the application`

**Что сделать:**
Добавить логирование во все ключевые точки приложения:

**Application:**
- `run()` - старт приложения, инициализация компонентов
- `handleCalculateRequest()` - получение запроса, результат
- `gracefulShutdown()` - процесс остановки

**Calculator:**
- `calculate()` - начало вычисления, результат, ошибки

**DbusServer:**
- `start()` - запуск сервера
- `stop()` - остановка сервера
- Обработка каждого запроса

**SignalHandler:**
- `registerSignals()` - регистрация сигналов
- `waitForSignal()` - получение сигнала

**ExpressionParser:**
- `parse()` - ошибки парсинга

**MathOperations:**
- Все операции - ошибки (деление на ноль, переполнение)

**Примеры логов:**
```
[2024-01-15 10:30:45.123] [info] Application starting...
[2024-01-15 10:30:45.125] [info] DBus server initialized on com.example.CalculatorService
[2024-01-15 10:30:50.456] [info] Received calculation request: {"firstValue":5,"operation":"+","secondValue":3}
[2024-01-15 10:30:50.457] [debug] Parsing expression: 5 + 3
[2024-01-15 10:30:50.458] [debug] Calculating: 5 + 3
[2024-01-15 10:30:50.459] [info] Calculation result: 8
[2024-01-15 10:30:50.460] [info] Response sent: {"result":8,"status":"success"}
[2024-01-15 10:31:00.789] [warning] Received SIGTERM signal, initiating graceful shutdown...
[2024-01-15 10:31:00.790] [info] DBus server stopped
[2024-01-15 10:31:00.791] [info] Application stopped gracefully
```

---

## 🗑️ Этап 10: Удаление старого кода

### 10.1 Коммит: `refactor: remove legacy C-style code`

**Удалить файлы:**
- ❌ `src/args_parser.hpp` (заменён ExpressionParser)
- ❌ `src/args_parser.cpp` (заменён ExpressionParser)
- ❌ Старые версии `src/task.hpp`, `src/task.cpp` (полностью переписаны)
- ❌ Старые версии `src/calculator.hpp`, `src/calculator.cpp` (полностью переписаны)
- ❌ `src/dbus.hpp` (заменён DbusServer)
- ❌ `src/dbus.cpp` (заменён DbusServer)

**Обновить CMakeLists.txt:**
- Убрать удалённые файлы из add_executable
- Добавить новые файлы

**Проверить:**
- ✅ Нет свободных функций (кроме main)
- ✅ Нет глобальных переменных
- ✅ Нет malloc/free (использовать RAII)
- ✅ Нет C-style строк (использовать std::string)

---

## 📊 Этап 11: UML диаграммы

### 11.1 Коммит: `docs: generate UML diagrams with clang-uml`

**Что сделать:**

1. **Установить clang-uml** (если ещё не установлен):
   ```bash
   sudo apt install clang-uml
   ```

2. **Создать конфигурацию `.clang-uml.yml`:**
   ```yaml
   compilation_database_dir: build/
   output_directory: docs/uml
   
   diagrams:
     class_diagram:
       type: class
       include:
         - src/*.hpp
       exclude:
         - tests/*
       title: "sys_calculator Class Diagram"
       
     sequence_diagram_calculate:
       type: sequence
       start_from: calculator::Application::handleCalculateRequest
       title: "Calculate Request Sequence"
       
     sequence_diagram_shutdown:
       type: sequence
       start_from: calculator::SignalHandler::waitForSignal
       title: "Graceful Shutdown Sequence"
   ```

3. **Сгенерировать компиляционную базу данных:**
   ```bash
   mkdir -p build && cd build
   cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
   ```

4. **Сгенерировать диаграммы:**
   ```bash
   cd ..
   clang-uml
   ```

5. **Добавить диаграммы в репозиторий:**
   ```
   docs/
   └── uml/
       ├── class_diagram.svg
       ├── sequence_diagram_calculate.svg
       └── sequence_diagram_shutdown.svg
   ```

6. **Обновить README.md** с ссылками на диаграммы

---

## ✅ Этап 12: Финальная проверка и документация

### 12.1 Коммит: `docs: update README with new architecture and usage`

**Обновить README.md:**

```markdown
# sys_calculator

System calculator service with DBus interface, implemented in modern C++ with OOP principles.

## Architecture

The application follows a clean architecture pattern with separation of concerns:

- **Core Domain**: Task, Calculator, MathOperations, ExpressionParser
- **Infrastructure**: DbusServer, SignalHandler, Logger, JsonConverter
- **Application**: Application facade orchestrating all components

See [UML diagrams](docs/uml/) for detailed architecture.

## Features

- ✅ Object-Oriented Design (no free functions, no global variables)
- ✅ Exception-based error handling
- ✅ JSON input/output format
- ✅ DBus integration using sdbus-cpp
- ✅ System signal handling (SIGTERM, SIGHUP) for graceful shutdown
- ✅ Comprehensive logging to journalctl via spdlog
- ✅ Unit tests with Google Test
- ✅ UML documentation

## Building

### Prerequisites (Ubuntu 24.04)

```bash
sudo apt install -y \
    libsdbus-c++-dev \
    nlohmann-json3-dev \
    libspdlog-dev \
    libgtest-dev \
    cmake \
    build-essential \
    pkg-config \
    libdbus-1-dev
```

### Build

```bash
mkdir -p build && cd build
cmake ..
make
```

### Run Tests

```bash
ctest --verbose
```

### Install

```bash
sudo make install
sudo systemctl daemon-reload
sudo systemctl enable calculator
sudo systemctl start calculator
```

## Usage

### Send calculation request

```bash
busctl call com.example.CalculatorService \
           /com/example/CalculatorObject \
           com.example.CalculatorInterface \
           Calculate \
           s '{"firstValue":5,"operation":"+","secondValue":3}'
```

### Supported operations

- Addition: `+`
- Subtraction: `-`
- Multiplication: `*`
- Division: `/`
- Power: `^`
- Factorial: `!`

### JSON Format

**Input:**
```json
{
    "firstValue": 5,
    "operation": "+",
    "secondValue": 3
}
```

**Output:**
```json
{
    "firstValue": 5,
    "operation": "+",
    "secondValue": 3,
    "result": 8,
    "status": "success"
}
```

### Viewing Logs

```bash
journalctl -u calculator -f
```

### Graceful Shutdown

```bash
sudo systemctl stop calculator
```

The service handles SIGTERM and SIGHUP signals for clean resource cleanup.

## Project Structure

```
src/
├── application.*          # Main orchestrator
├── task.*                 # Task value object
├── calculator.*           # Business logic
├── math_operations.*      # Math operations (refactored libmath)
├── expression_parser.*    # String expression parser
├── logger.*               # Logging singleton
├── json_converter.*       # JSON serialization utilities
├── dbus_server.*          # DBus server implementation
├── signal_handler.*       # System signal handler
└── exceptions.*           # Custom exception hierarchy

tests/
├── test_math_operations.cpp
├── test_task.cpp
├── test_calculator.cpp
└── test_expression_parser.cpp

docs/uml/
├── class_diagram.svg
├── sequence_diagram_calculate.svg
└── sequence_diagram_shutdown.svg
```

## Development

### Code Style

- Follow C++ OOP principles (no free functions except main)
- Use exceptions for error handling
- No user-defined global variables
- Use RAII for resource management
- Follow Conventional Commits for commit messages

### Running clang-format

```bash
make format
```

### Running clang-tidy

```bash
make tidy
```

## License

[Your License]
```

### 12.2 Финальная проверка требований

**Чеклист:**

- ✅ **Нет свободных функций** (кроме main)
- ✅ **Нет глобальных переменных**
- ✅ **Используются исключения** для обработки ошибок
- ✅ **Обработка SIGTERM и SIGHUP** для graceful shutdown
- ✅ **JSON вход/выход** формат
- ✅ **sdbus-cpp** для DBus коммуникации
- ✅ **nlohmann/json** для JSON парсинга
- ✅ **spdlog** для логирования (скрыт за Logger singleton)
- ✅ **Google Test** для unit тестов
- ✅ **Singleton (Meyer's)** для Logger
- ✅ **UML диаграммы** (class diagram + sequence diagrams)
- ✅ **libmath переписан** в ООП стиль как MathOperations
- ✅ **Тесты покрывают** только бизнес-логику (Calculator, Task, MathOperations, ExpressionParser)
- ✅ **Логи пишутся** в journalctl через syslog
- ✅ **Conventional Commits** для всех коммитов
- ✅ **Отдельная ветка** для разработки

### 12.3 Тестирование работы сервиса

```bash
# Сборка
mkdir -p build && cd build
cmake ..
make

# Запуск тестов
ctest --verbose

# Установка
sudo make install
sudo systemctl daemon-reload

# Запуск сервиса
sudo systemctl start calculator

# Проверка статуса
systemctl status calculator

# Отправка запроса
busctl call com.example.CalculatorService \
           /com/example/CalculatorObject \
           com.example.CalculatorInterface \
           Calculate \
           s '{"firstValue":5,"operation":"+","secondValue":3}'

# Просмотр логов
journalctl -u calculator -f

# Graceful shutdown
sudo systemctl stop calculator

# Проверка что сервис остановился
systemctl status calculator
```

---

## 📦 Итоговый список коммитов (Conventional Commits)

```
1. chore: create feature branch for OOP refactoring
2. build: install system dependencies via apt
3. build: update CMakeLists.txt with new dependencies
4. feat: implement custom exception hierarchy
5. feat: implement Logger singleton with spdlog backend
6. feat: implement MathOperations class with exception handling
7. feat: implement Task class with JSON serialization
8. feat: implement Calculator class orchestrating MathOperations
9. feat: implement ExpressionParser for string expressions
10. feat: implement JsonConverter utility
11. feat: implement DbusServer with sdbus-cpp
12. feat: implement SignalHandler for graceful shutdown
13. feat: implement Application class as main orchestrator
14. test: add unit tests for MathOperations
15. test: add unit tests for Task and JsonConverter
16. test: add unit tests for Calculator and ExpressionParser
17. feat: integrate logging throughout the application
18. refactor: remove legacy C-style code
19. docs: generate UML diagrams with clang-uml
20. docs: update README with new architecture and usage
```

---

## ⚠️ Риски и рекомендации

### Потенциальные проблемы:

1. **sdbus-cpp API**:
   - API может отличаться в разных версиях
   - Проверить документацию для установленной версии
   - Команда: `dpkg -l | grep sdbus`

2. **Статическая vs динамическая линковка**:
   - Зависимости установлены через apt как shared libraries
   - CMake автоматически подключит правильные файлы
   - При необходимости можно форсировать: `set(BUILD_SHARED_LIBS OFF)`

3. **Логирование в journalctl**:
   - spdlog::syslog_logger может требовать дополнительных настроек
   - Альтернатива: писать в stderr, systemd сам перехватит
   - Проверить: `journalctl -u calculator`

4. **Тестирование без DBus**:
   - Тесты не должны зависеть от DBus
   - Тестируем только бизнес-логику
   - DbusServer не входит в покрытие тестами

5. **Совместимость с Ubuntu 24.04**:
   - Все пакеты должны быть доступны в репозиториях
   - Проверить версии пакетов перед установкой

### Рекомендации по порядку выполнения:

1. **Сначала ядро**: Task → MathOperations → Calculator
2. **Затем тесты**: написать тесты для ядра (TDD подход)
3. **Инфраструктура**: Logger → JsonConverter → ExpressionParser
4. **Интеграция**: DbusServer → SignalHandler → Application
5. **Финализация**: удаление старого кода → UML → документация

### Полезные команды:

```bash
# Проверить установленные пакеты
dpkg -l | grep -E "sdbus|nlohmann|spdlog|gtest"

# Просмотреть логи сервиса
journalctl -u calculator -f

# Проверить DBus сервис
busctl list | grep Calculator

# Запустить тесты с подробным выводом
ctest --verbose --output-on-failure

# Сгенерировать UML диаграммы
clang-uml

# Проверить код на утечки памяти
valgrind --leak-check=full ./calculator
```

---

## 🎓 Ключевые принципы рефакторинга

1. **Инкапсуляция**: Все данные и поведение инкапсулированы в классах
2. **RAII**: Ресурсы управляются через время жизни объектов
3. **Исключения**: Ошибки передаются через исключения, не через коды возврата
4. **Single Responsibility**: Каждый класс отвечает за одну вещь
5. **Dependency Injection**: Зависимости передаются через конструктор
6. **Interface Segregation**: Чёткие интерфейсы между компонентами
7. **Testability**: Код легко тестируется изолированно
8. **No Global State**: Никаких глобальных переменных (кроме Logger Singleton)

---

## 📚 Дополнительные ресурсы

- [sdbus-cpp Documentation](https://github.com/Kistler-Group/sdbus-cpp)
- [nlohmann/json Documentation](https://json.nlohmann.me/)
- [spdlog Documentation](https://github.com/gabime/spdlog)
- [Google Test Documentation](https://google.github.io/googletest/)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Semantic Versioning](https://semver.org/)

---

**Дата создания плана:** 2026-06-07  
**Автор:** AI Assistant  
**Статус:** Готов к выполнению
