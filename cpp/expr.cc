#include "expr.h"

#include <array>
#include <cmath>
#include <sstream>

using std::abs;
using std::pow;
using std::round;
using std::sqrt;
using std::stoll;

using std::array;
using std::index_sequence;
using std::make_index_sequence;
using std::string;
using std::ostringstream;

double Expr::DOUBLE_PRECISION = 1e-8;

template <typename Derived>
static bool IsInstanceOf(const Expr* ptr) {
  return dynamic_cast<const Derived*>(ptr) != nullptr;
}

Expr::Expr(int64_t value) : is_integer_(true) {
  value_.i = value;
}

Expr::Expr(double value) {
  double nearby_int = round(value);
  is_integer_ = abs(value - nearby_int) < DOUBLE_PRECISION;
  if (is_integer_) {
    value_.i = static_cast<int64_t>(nearby_int);
  } else {
    value_.d = value;
  }
}

int64_t Expr::GetInt() const {
  if (is_integer_) return value_.i;
  else return static_cast<int64_t>(value_.d);
}

double Expr::GetDouble() const {
  if (is_integer_) return static_cast<double>(value_.i);
  else return value_.d;
}

LiteralExpr::LiteralExpr(string&& literal)
  : Expr(static_cast<int64_t>(stoll(literal))), literal_(literal) {
}

string LiteralExpr::ToString() const {
  return literal_;
}

BinaryExpr::BinaryExpr(char oper, const Expr* left, const Expr* right, double value)
  : Expr(value), oper_(oper), left_(left), right_(right) {
}

string BinaryExpr::ToString() const {
  ostringstream ss;
  if (IsInstanceOf<BinaryExpr>(left_)) {
    ss << '(' << left_->ToString() << ')';
  } else {
    ss << left_->ToString();
  }
  ss << ' ' << oper_ << ' ';
  if (IsInstanceOf<BinaryExpr>(right_)) {
    ss << '(' << right_->ToString() << ')';
  } else {
    ss << right_->ToString();
  }
  return ss.str();
}

AddExpr::AddExpr(const Expr* left, const Expr* right)
  : BinaryExpr('+', left, right, left->GetDouble() + right->GetDouble()) {
}

SubExpr::SubExpr(const Expr* left, const Expr* right)
  : BinaryExpr('-', left, right, left->GetDouble() - right->GetDouble()) {
}

MulExpr::MulExpr(const Expr* left, const Expr* right)
  : BinaryExpr('*', left, right, left->GetDouble()* right->GetDouble()) {
}

DivExpr::DivExpr(const Expr* left, const Expr* right)
  : BinaryExpr('/', left, right, left->GetDouble() / right->GetDouble()) {
}

PowExpr::PowExpr(const Expr* left, const Expr* right)
  : BinaryExpr('^', left, right, pow(left->GetDouble(), right->GetDouble())) {
}

constexpr static int64_t FactorialRaw(int64_t n) {
  int64_t res = 1;
  while (n > 0) res *= n--;
  return res;
}

template <class T, size_t... Is>
constexpr static auto GenerateTable(T(*func)(int64_t), index_sequence<Is...>) {
  return array<T, sizeof...(Is)>{func(Is)...};
}

FactorialExpr::FactorialExpr(const Expr* expr)
  : Expr(Factorial(expr->GetInt())), child_(expr) {
}

int64_t FactorialExpr::Factorial(int64_t n) {
  constexpr int table_size = 21;
  constexpr static auto factorial_table =
    GenerateTable(FactorialRaw, make_index_sequence<table_size>{});
  if (n < table_size) return factorial_table[n];
  else return FactorialRaw(n);
}

string FactorialExpr::ToString() const {
  ostringstream ss;
  if (IsInstanceOf<LiteralExpr>(child_) || IsInstanceOf<FactorialExpr>(child_)) {
    ss << child_->ToString() << '!';
  } else {
    ss << '(' << child_->ToString() << ")!";
  }
  return ss.str();
}

SqrtExpr::SqrtExpr(const Expr* expr)
  : Expr(sqrt(expr->GetDouble())), child_(expr) {
}

string SqrtExpr::ToString() const {
  ostringstream ss;
  if (IsInstanceOf<LiteralExpr>(child_) || IsInstanceOf<FactorialExpr>(child_)) {
    ss << "√" << child_->ToString();
  } else {
    ss << "√(" << child_->ToString() << ')';
  }
  return ss.str();
}