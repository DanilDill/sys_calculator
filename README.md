# sys_calculator
Usage: ./calculator 

Options:
  -h, --help     Show this help message and exit

Run: ./calculator
In other terminal run:
  busctl call com.example.CalculatorService \
             /com/example/CalculatorObject \
             com.example.CalculatorInterface \
             Calculate \
             s "1 + 4"