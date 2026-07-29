# sys_calculator

Системный сервис-калькулятор для Linux. Принимает вычислительные запросы через **D-Bus** и/или **TCP** — режим выбирается в JSON конфиге.

build:
```
sudo apt install libsdbus-c++-dev libhiredis-dev libspdlog-dev libpq-dev \
                 libboost-system-dev postgresql postgresql-contrib

git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus && mkdir build && cd build
cmake ..
make && sudo make install
```

Usage: `./calculator [--config PATH]`

Options:
  -h, --help          Show this help message and exit
  -c, --config PATH   Path to JSON config (default: /usr/local/etc/calculator.json)

Пример конфига — `etc/calculator.json`. Поле `mode`: `dbus_only` | `tcp_only` | `both`.

## Запуск

```
./calculator --config etc/calculator.json
```

### Вызов через D-Bus (mode = dbus_only | both)
```
 busctl call com.example.CalculatorService \
             /com/example/CalculatorObject \
             com.example.CalculatorInterface \
             Calculate \
              s '{"firstValue": 5, "operation": "+", "secondValue": 3}'
```

### Вызов через TCP (mode = tcp_only | both)

Запросы — JSON, завершённые нулевым байтом (`\0`). Клиентская утилита:
```
# автоматический тест (10 запросов)
./calculator_client --host 127.0.0.1 --port 1234 --test

# интерактивный режим (JSON построчно, Ctrl+D для выхода)
./calculator_client --host 127.0.0.1 --port 1234
```

Подробности протокола, формат конфига и описание всех классов — в [AGENT.md](AGENT.md).
