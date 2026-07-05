# sys_calculator
build: 
```
sudo apt install libsdbus-c++-dev libhiredis-dev libspdlog-dev libpq-dev postgresql postgresql-contrib

git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus && mkdir build && cd build
cmake ..
make && sudo make install
```
Usage: ./calculator 

Options:
  -h, --help     Show this help message and exit

Run: ./calculator
In other terminal run:
```
 busctl call com.example.CalculatorService \
             /com/example/CalculatorObject \
             com.example.CalculatorInterface \
             Calculate \
              s '{"firstValue": 5, "operation": "+", "secondValue": 3}'
```              