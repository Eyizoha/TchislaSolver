from tchisla import TchislaSolver


for i in range(1, 10):
  ts = TchislaSolver(2016, i)
  print('2016 = ' + str(ts.solve()))
