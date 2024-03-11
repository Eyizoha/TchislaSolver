from math import factorial


class Expr:
  threshold = 1e-10

  def __init__(self, value):
    if isinstance(value, float):
      if value.is_integer():
        value = int(value)
      elif abs(value - round(value)) < Expr.threshold:
        value = round(value)
    self.value = value

  def __str__(self):
    raise TypeError("Should not call Expr.__str__()")

  def evaluable(self):
    return str(self)


class LiteralExpr(Expr):
  def __init__(self, value):
    Expr.__init__(self, value)
    self.literal = str(value)

  def __str__(self):
    return self.literal


class FactorialExpr(Expr):
  def __init__(self, expr: Expr):
    Expr.__init__(self, factorial(expr.value))
    self.child = expr

  def __str__(self):
    if isinstance(self.child, BinaryExpr):
      return '({})!'.format(str(self.child))
    else:
      return str(self.child) + '!'

  def evaluable(self):
    return 'factorial(round({}))'.format(self.child.evaluable())


class SqrtExpr(Expr):
  def __init__(self, expr: Expr):
    Expr.__init__(self, expr.value ** 0.5)
    self.child = expr

  def __str__(self):
    if isinstance(self.child, LiteralExpr):
      return '√{}'.format(str(self.child))
    else:
      return '√({})'.format(str(self.child))

  def evaluable(self):
    return '(round({}) ** 0.5)'.format(self.child.evaluable())


class BinaryExpr(Expr):
  def __init__(self, oper: str, left: Expr, right: Expr, value):
    Expr.__init__(self, value)
    self.oper = oper
    self.left = left
    self.right = right

  def build(self, oper, evaluable):
    format_str = '({})' if isinstance(self.left, BinaryExpr) else '{}'
    format_str += ' {} '
    format_str += '({})' if isinstance(self.right, BinaryExpr) else '{}'
    left = self.left.evaluable() if evaluable else str(self.left)
    right = self.right.evaluable() if evaluable else str(self.right)
    return format_str.format(left, oper, right)

  def __str__(self):
    return self.build(oper=self.oper, evaluable=False)

  def evaluable(self):
    return self.build(oper=self.oper, evaluable=True)


class AddExpr(BinaryExpr):
  def __init__(self, left: Expr, right: Expr):
    BinaryExpr.__init__(self, '+', left, right, left.value + right.value)


class SubExpr(BinaryExpr):
  def __init__(self, left: Expr, right: Expr):
    BinaryExpr.__init__(self, '-', left, right, left.value - right.value)


class MulExpr(BinaryExpr):
  def __init__(self, left: Expr, right: Expr):
    BinaryExpr.__init__(self, '*', left, right, left.value * right.value)


class DivExpr(BinaryExpr):
  def __init__(self, left: Expr, right: Expr):
    BinaryExpr.__init__(self, '/', left, right, left.value / right.value)


class PowExpr(BinaryExpr):
  def __init__(self, left: Expr, right: Expr):
    BinaryExpr.__init__(self, '^', left, right, left.value ** right.value)

  def evaluable(self):
    return self.build(oper='**', evaluable=True)


class TchislaSolver:
  value_max_limit = 1e8
  value_min_limit = 1e-8
  power_limit = 30
  factorial_limit = 15

  class Found(BaseException):
    pass

  def __init__(self, target: int, seed: int):
    self.target = target
    self.seed = seed

    self.all_candidates = set()
    self.current_min = None
    self.current_max = None
    self.current_candidates = []
    self.generations = []
    self.result = None

  def solve(self, search_depth=10, trace=False):
    try:
      while search_depth > 0:
        begin, end = 0, len(self.generations) - 1
        while begin <= end:
          self.cross_candidates(self.generations[begin], self.generations[end])
          begin += 1
          end -= 1
        self.add_literal(len(self.generations) + 1)
        self.next_generation(trace)
        search_depth -= 1
    except TchislaSolver.Found:
      pass
    return self.result

  def cross_candidates(self, c1, c2):
    for expr1 in c1:
      for expr2 in c2:
        self.add_addition(expr1, expr2)
        self.add_subtraction(expr1, expr2)
        self.add_multiplication(expr1, expr2)
        self.add_division(expr1, expr2)
        self.add_power(expr1, expr2)

  def add_candidate(self, expr):
    if expr.value == self.target:
      self.result = expr
      raise TchislaSolver.Found()
    if expr.value < TchislaSolver.value_min_limit:
      return
    if expr.value > TchislaSolver.value_max_limit:
      return
    if expr.value not in self.all_candidates:
      self.all_candidates.add(expr.value)
      self.current_candidates.append(expr)
      self.current_min = min(expr.value, self.current_min or expr.value)
      self.current_max = max(expr.value, self.current_max or expr.value)
      self.add_factorial(expr)
      self.add_square_root(expr)

  def next_generation(self, trace):
    if trace:
      print('Seed: {}, G{}: size={} min={} max={}'.format(self.seed,
                                                          len(self.generations) + 1,
                                                          len(self.current_candidates),
                                                          self.current_min,
                                                          self.current_max))
    self.generations.append(self.current_candidates)
    self.current_min = None
    self.current_max = None
    self.current_candidates = []

  def add_literal(self, repeats: int):
    self.add_candidate(LiteralExpr(int(str(self.seed) * repeats)))

  def add_addition(self, expr1: Expr, expr2: Expr):
    self.add_candidate(AddExpr(expr1, expr2))

  def add_subtraction(self, expr1: Expr, expr2: Expr):
    if expr1.value > expr2.value:
      self.add_candidate(SubExpr(expr1, expr2))
    else:
      self.add_candidate(SubExpr(expr2, expr1))

  def add_multiplication(self, expr1: Expr, expr2: Expr):
    self.add_candidate(MulExpr(expr1, expr2))

  def add_division(self, expr1: Expr, expr2: Expr):
    if expr1.value == 0 or expr2.value == 0:
      return
    self.add_candidate(DivExpr(expr1, expr2))
    self.add_candidate(DivExpr(expr2, expr1))

  def add_power(self, expr1: Expr, expr2: Expr):
    if isinstance(expr2.value, int) and expr2.value <= TchislaSolver.power_limit:
      self.add_candidate(PowExpr(expr1, expr2))
    if isinstance(expr1.value, int) and expr1.value <= TchislaSolver.power_limit:
      self.add_candidate(PowExpr(expr2, expr1))

  def add_factorial(self, expr: Expr):
    if isinstance(expr.value, int) and expr.value <= TchislaSolver.factorial_limit:
      self.add_candidate(FactorialExpr(expr))

  def add_square_root(self, expr: Expr):
    if isinstance(expr.value, int) and expr.value > 0:
      self.add_candidate(SqrtExpr(expr))
