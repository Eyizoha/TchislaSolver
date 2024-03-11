#include "tchisla-solver.h"

#include <sstream>
#include <thread>

using std::ostringstream;
using std::thread;
using std::vector;

#define RETURN_IF_TRUE(expr) if (expr) return true

double TchislaSolver::VALUE_MAX_LIMIT = 1e8;
double TchislaSolver::VALUE_MIN_LIMIT = 1e-8;
int64_t TchislaSolver::POWER_LIMIT = 30;
int64_t TchislaSolver::FACTORIAL_LIMIT = 15;
size_t TchislaSolver::MUILT_THREADS_THRESHOLD = 10000;

TchislaSolver::TchislaSolver(int64_t target, int64_t seed, std::ostream* trace_os)
  : target_(target), seed_(seed), trace_os_(trace_os) { }

bool TchislaSolver::Solve(int search_depth) {
  vector<thread> extra_threads;
  vector<GenerationCreator> extra_creators;
  GenerationCreator creator(*this, 0);
  while (search_depth-- > 0) {
    size_t num_loops = (generations_.size() + 1) / 2;
    if (UseMultiThread()) {
      size_t num_extra_threads = num_loops - 1;
      size_t part_id = extra_creators.size() > 0 ? extra_creators.back().part_id : 0;
      while (num_extra_threads > extra_creators.size()) {
        extra_creators.emplace_back(*this, ++part_id);
      }
      NewGeneration(num_loops);
      extra_threads.clear();
      for (size_t i = 0; i < num_loops - 1; ++i) {
        const GenerationPtr* g1 = &generations_[i];
        const GenerationPtr* g2 = &generations_[generations_.size() - i - 1];
        GenerationCreator* thread_creator = &extra_creators[i];
        extra_threads.emplace_back([=]() { thread_creator->CrossGeneration(*g1, *g2); });
      }
      const GenerationPtr& g1 = generations_[num_loops - 1];
      const GenerationPtr& g2 = generations_[generations_.size() - num_loops];
      creator.CrossGeneration(g1, g2);
      for (auto& t : extra_threads) {
        t.join();
      }
      RETURN_IF_TRUE(found.load());
    } else {
      NewGeneration(1);
      for (size_t i = 0; i < num_loops; ++i) {
        const GenerationPtr& g1 = generations_[i];
        const GenerationPtr& g2 = generations_[generations_.size() - i - 1];
        RETURN_IF_TRUE(creator.CrossGeneration(g1, g2));
      }
    }
    RETURN_IF_TRUE(creator.AddLiteral(generations_.size() + 1));
    EndGeneration();
  }
  return false;
}

bool TchislaSolver::UseMultiThread() const {
  return !generations_.empty() && generations_.back()->size() > MUILT_THREADS_THRESHOLD;
}

bool TchislaSolver::AddReachableValueIfNotExist(const Expr& expr) {
  if (expr.IsInt()) {
    return reachable_ints_.InsertIfNotExist(expr.GetIntUnsafe());
  } else {
    return reachable_doubles_.InsertIfNotExist(expr.GetDoubleUnsafe());
  }
}

bool TchislaSolver::GenerationCreator::CrossGeneration(
    const GenerationPtr& g1, const GenerationPtr& g2) {
  for (const auto& expr1 : *g1) {
    for (const auto& expr2 : *g2) {
      RETURN_IF_TRUE(AddAddition(expr1.get(), expr2.get()));
      RETURN_IF_TRUE(AddSubtraction(expr1.get(), expr2.get()));
      RETURN_IF_TRUE(AddMultiplication(expr1.get(), expr2.get()));
      RETURN_IF_TRUE(AddDivision(expr1.get(), expr2.get()));
      RETURN_IF_TRUE(AddPower(expr1.get(), expr2.get()));
    }
  }
  return false;
}

void TchislaSolver::NewGeneration(size_t num_new_parts) {
  current_generation_ = std::make_unique<PartitionedList<ExprPtr>>(num_new_parts);
}


void TchislaSolver::EndGeneration() {
  if (trace_os_ != nullptr) {
    *trace_os_ << "Seed: " << seed_
      << ", G" << generations_.size() + 1
      << " size: " << current_generation_->size() << std::endl;
  }
  generations_.push_back(std::move(current_generation_));
}

bool TchislaSolver::GenerationCreator::AddCandidate(const Expr* expr) {
  ExprPtr expr_ptr(expr);
  RETURN_IF_TRUE(solver.found.load());
  if (expr->IsInt() && expr->GetInt() == solver.target_) {
    solver.found.store(true);
    solver.result_ = expr->ToString();
    return true;
  }
  if (expr->GetDouble() < VALUE_MIN_LIMIT) return false;
  if (expr->GetDouble() > VALUE_MAX_LIMIT) return false;
  if (solver.AddReachableValueIfNotExist(*expr)) {
    solver.current_generation_->push_back(part_id, std::move(expr_ptr));
    RETURN_IF_TRUE(AddFactorial(expr));
    RETURN_IF_TRUE(AddSquareRoot(expr));
  }
  return false;
}

bool TchislaSolver::GenerationCreator::AddLiteral(size_t repeats) {
  ostringstream ss;
  while (repeats-- > 0) ss << solver.seed_;
  return AddCandidate(new LiteralExpr(ss.str()));
}

bool TchislaSolver::GenerationCreator::AddAddition(const Expr* expr1, const Expr* expr2) {
  return AddCandidate(new AddExpr(expr1, expr2));
}

bool TchislaSolver::GenerationCreator::AddSubtraction(const Expr* expr1, const Expr* expr2) {
  if (expr1->GetDouble() > expr2->GetDouble()) {
    return AddCandidate(new SubExpr(expr1, expr2));
  } else {
    return AddCandidate(new SubExpr(expr2, expr1));
  }
}

bool TchislaSolver::GenerationCreator::AddMultiplication(const Expr* expr1, const Expr* expr2) {
  return AddCandidate(new MulExpr(expr1, expr2));
}

bool TchislaSolver::GenerationCreator::AddDivision(const Expr* expr1, const Expr* expr2) {
  if (expr1->GetDouble() < Expr::FLOAT_THRESHOLD ||
    expr2->GetDouble() < Expr::FLOAT_THRESHOLD) return false;
  RETURN_IF_TRUE(AddCandidate(new DivExpr(expr1, expr2)));
  return AddCandidate(new DivExpr(expr2, expr1));
}

bool TchislaSolver::GenerationCreator::AddPower(const Expr* expr1, const Expr* expr2) {
  if (expr2->IsInt() && expr2->GetIntUnsafe() <= POWER_LIMIT) {
    return AddCandidate(new PowExpr(expr1, expr2));
  }
  if (expr1->IsInt() && expr1->GetIntUnsafe() <= POWER_LIMIT) {
    return AddCandidate(new PowExpr(expr2, expr1));
  }
  return false;
}

bool TchislaSolver::GenerationCreator::AddFactorial(const Expr* expr) {
  if (expr->IsInt() && expr->GetIntUnsafe() <= FACTORIAL_LIMIT) {
    return AddCandidate(new FactorialExpr(expr));
  }
  return false;
}

bool TchislaSolver::GenerationCreator::AddSquareRoot(const Expr* expr) {
  if (expr->IsInt() && expr->GetIntUnsafe() > 0) {
    return AddCandidate(new SqrtExpr(expr));
  }
  return false;
}

