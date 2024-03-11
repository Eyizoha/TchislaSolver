from tchisla import TchislaSolver
from math import factorial

target = 2016
optimal_solutions = [None, 9, 6, 4, 4, 6, 4, 6, 5, 4]

for i in range(1, 10):
  ts = TchislaSolver(target, i)
  result = ts.solve(trace=True)
  assert str(result).count(str(i)) == optimal_solutions[i]
  assert target == round(eval(result.evaluable()))
  print('{} = {}'.format(target, result))
  print()
