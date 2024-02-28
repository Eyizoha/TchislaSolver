# Tchisla Solver
## Overview
[Tchisla求解器](https://blog.csdn.net/Eyizoha/article/details/136352253)
Tchisla Solver is a Python-based mathematical expression solver specifically designed to solve the Tchisla game.
The Tchisla game challenges players to use a given seed number and mathematical operations like addition, subtraction, multiplication, division, exponentiation, factorial, and square root to reach a target number.
## Usage
To use the Tchisla Solver, you need to create an instance of TchislaSolver by passing the target number and the seed number. Then, call the solve method with the desired search depth. If a solution is found, it will be returned; otherwise, the method will return None.

Example:
``` python
solver = TchislaSolver(target=42, seed=3)
solution = solver.solve(search_depth=10)
print(solution) # 3! + (3! * 3!)
```
