﻿#pragma once

#include <string>

class Expr {
public:
  static double DOUBLE_PRECISION;

  Expr(int64_t value);
  Expr(double value);

  int64_t GetInt() const;
  double GetDouble() const;

  bool IsInt() const { return is_integer_; }
  int64_t GetIntUnsafe() const { return value_.i; }
  double GetDoubleUnsafe() const { return value_.d; }

  virtual std::string ToString() const = 0;

private:
  bool is_integer_;

  union {
    int64_t i;
    double d;
  } value_;
};


class LiteralExpr : public Expr {
public:
  LiteralExpr(std::string&& literal);

  virtual std::string ToString() const;

private:
  std::string literal_;
};


class BinaryExpr : public Expr {
public:
  BinaryExpr(char oper, const Expr* left, const Expr* right, double value);

  virtual std::string LeftToString() const;
  virtual std::string RightToString() const;
  virtual std::string ToString() const;

protected:
  const char oper_;
  const Expr* left_;
  const Expr* right_;
};


class AddExpr : public BinaryExpr {
public:
  AddExpr(const Expr* left, const Expr* right);
};


class SubExpr : public BinaryExpr {
public:
  SubExpr(const Expr* left, const Expr* right);
};


class MulExpr : public BinaryExpr {
public:
  MulExpr(const Expr* left, const Expr* right);
};


class DivExpr : public BinaryExpr {
public:
  DivExpr(const Expr* left, const Expr* right);
};


class PowExpr : public BinaryExpr {
public:
  PowExpr(const Expr* left, const Expr* right);
};


class NegPowExpr : public BinaryExpr {
public:
  NegPowExpr(const Expr* left, const Expr* right);

  virtual std::string ToString() const;
};


class MultiSqrtPowExpr : public BinaryExpr {
public:
  MultiSqrtPowExpr(int sqrt_times, const Expr* left, const Expr* right);

  virtual std::string LeftToString() const;

private:
  int sqrt_times_;
};


class NegMultiSqrtPowExpr : public BinaryExpr {
public:
  NegMultiSqrtPowExpr(int sqrt_times, const Expr* left, const Expr* right);

  virtual std::string LeftToString() const;
  virtual std::string ToString() const;

private:
  int sqrt_times_;
};


class SqrtMulExpr : public BinaryExpr {
public:
  SqrtMulExpr(const Expr* left, const Expr* right);

  virtual std::string RightToString() const;
};


class FactorialExpr : public Expr {
public:
  static int64_t Factorial(int64_t n);

  FactorialExpr(const Expr* expr);

  virtual std::string ToString() const;

private:
  const Expr* child_;
};


class SqrtExpr : public Expr {
public:
  SqrtExpr(const Expr* expr);

  virtual std::string ToString() const;

private:
  const Expr* child_;
};


class DoubleSqrtExpr : public Expr {
public:
  DoubleSqrtExpr(const Expr* expr);

  virtual std::string ToString() const;

private:
  const Expr* child_;
};
