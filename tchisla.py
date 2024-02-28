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


class SqrtExpr(Expr):
  def __init__(self, expr: Expr):
    Expr.__init__(self, expr.value ** 0.5)
    self.child = expr

  def __str__(self):
    if isinstance(self.child, LiteralExpr):
      return '√{}'.format(str(self.child))
    else:
      return '√({})'.format(str(self.child))


class BinaryExpr(Expr):
  def __init__(self, oper: str, left: Expr, right: Expr, value):
    Expr.__init__(self, value)
    self.oper = oper
    self.left = left
    self.right = right

  def __str__(self):
    format_str = '({})' if isinstance(self.left, BinaryExpr) else '{}'
    format_str += ' {} '
    format_str += '({})' if isinstance(self.right, BinaryExpr) else '{}'
    return format_str.format(str(self.left), self.oper, str(self.right))


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


class TchislaSolver:
  value_limit = 10 ** 8
  power_limit = 30
  factorial_limit = 15

  class Found(BaseException):
    pass

  def __init__(self, target: int, seed: int):
    self.target = target
    self.seed = seed

    self.all_candidates = set()
    self.current_candidates = []
    self.generations = []
    self.result = None

  def solve(self, search_depth=10):
    try:
      while search_depth > 0:
        begin, end = 0, len(self.generations) - 1
        while begin <= end:
          self.cross_candidates(self.generations[begin], self.generations[end])
          begin += 1
          end -= 1
        self.add_literal(len(self.generations) + 1)
        self.next_generation()
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
    if expr.value > TchislaSolver.value_limit:
      return
    if expr.value not in self.all_candidates:
      self.all_candidates.add(expr.value)
      self.current_candidates.append(expr)
      self.add_factorial(expr)
      self.add_square_root(expr)

  def next_generation(self):
    self.generations.append(self.current_candidates)
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

