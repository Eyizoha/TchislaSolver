# Tchisla Solver
## Overview
[Tchisla求解器](https://blog.csdn.net/Eyizoha/article/details/136352253)
Tchisla Solver is a mathematical expression solver specifically designed to solve the Tchisla game.
The Tchisla game challenges players to use a given seed number and mathematical operations like addition, subtraction, multiplication, division, exponentiation, factorial, and square root to reach a target number.

### Python Ver Usage
To use the Python Ver Tchisla Solver, you need to create an instance of TchislaSolver by passing the target number and the seed number. Then, call the solve method with the desired search depth. If a solution is found, it will be returned; otherwise, the method will return None.

Example:
``` python
solver = TchislaSolver(target=42, seed=3)
solution = solver.solve(search_depth=10)
print(solution) # 3! + (3! * 3!)
```

### C++ Ver Usage
The C++ version has a lower memory footprint and execution speed (over 1000% faster!) compared to the Python version. However, you need to compile it yourself by running make in the cpp directory. After compiling, you can run the resulting program by providing the target number like this:
``` shell
tchisla_solver 1234 5              # Search using digit 5 to calculate 1234
or
tchisla_solver 1234                # Search using digits 1 to 9 to calculate 1234
```
Execute --help to see more options.