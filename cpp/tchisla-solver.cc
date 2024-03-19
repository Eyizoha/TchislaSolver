#include "tchisla-solver.h"

#include <sstream>
#include <thread>

using std::ostringstream;
using std::thread;
using std::vector;

#define RETURN_IF_TRUE(expr) if (expr) return true

double TchislaSolver::VALUE_MAX_LIMIT = 1e12;
double TchislaSolver::VALUE_MIN_LIMIT = 1e-8;
int64_t TchislaSolver::POWER_LIMIT = 40;
int64_t TchislaSolver::FACTORIAL_LIMIT = 15;
size_t TchislaSolver::MUILT_THREADS_THRESHOLD = 10000;

TchislaSolver::TchislaSolver(int64_t target, int64_t seed, bool deep_search, std::ostream* trace_os)
  : target_(target), seed_(seed), deep_search_(deep_search), trace_os_(trace_os),
  reachable_values_(Expr::DOUBLE_PRECISION) {
  expr_pools_.emplace_back(new ObjectPool<OBJ_POOL_SIZE>);
  creators_.emplace_back(*this, 0);
}

bool TchislaSolver::Solve(int search_depth) {
  while (search_depth-- > 0) {
    size_t num_loops = (generations_.size() + 1) / 2;
    if (UseMultiThread()) {
      MultiThreadCrossGeneration(num_loops);
      RETURN_IF_TRUE(found.load());
    } else {
      NewGeneration(1);
      for (size_t i = 0; i < num_loops; ++i) {
        const GenerationPtr& g1 = generations_[i];
        const GenerationPtr& g2 = generations_[generations_.size() - i - 1];
        RETURN_IF_TRUE(creators_[0].CrossGeneration(g1, g2));
      }
    }
    RETURN_IF_TRUE(creators_[0].AddLiteral(generations_.size() + 1));
    EndGeneration();
  }
  return false;
}

bool TchislaSolver::UseMultiThread() const {
  return !generations_.empty() && generations_.back()->size() > MUILT_THREADS_THRESHOLD;
}

void TchislaSolver::MultiThreadCrossGeneration(size_t num_loops) {
  vector<thread> extra_threads;
  size_t num_extra_threads = num_loops - 1;
  size_t part_id = creators_.back().part_id;
  while (num_extra_threads > expr_pools_.size() - 1) {
    expr_pools_.emplace_back(new ObjectPool<OBJ_POOL_SIZE>);
  }
  while (num_extra_threads > creators_.size() - 1) {
    creators_.emplace_back(*this, ++part_id);
  }
  NewGeneration(num_loops);
  for (size_t i = 0; i < num_loops - 1; ++i) {
    const GenerationPtr* g1 = &generations_[i];
    const GenerationPtr* g2 = &generations_[generations_.size() - i - 1];
    GenerationCreator* thread_creator = &creators_[i + 1];
    extra_threads.emplace_back([=]() { thread_creator->CrossGeneration(*g1, *g2); });
  }
  const GenerationPtr& g1 = generations_[num_loops - 1];
  const GenerationPtr& g2 = generations_[generations_.size() - num_loops];
  creators_[0].CrossGeneration(g1, g2);
  for (auto& t : extra_threads) {
    t.join();
  }
}

bool TchislaSolver::AddReachableValueIfNotExist(const Expr& expr) {
  if (expr.IsInt()) {
    return reachable_values_.InsertIfNotExist(expr.GetIntUnsafe());
  } else {
    return reachable_values_.InsertIfNotExist(expr.GetDoubleUnsafe());
  }
}

bool TchislaSolver::GenerationCreator::CrossGeneration(
    const GenerationPtr& g1, const GenerationPtr& g2) {
  for (const auto& expr1 : *g1) {
    for (const auto& expr2 : *g2) {
      RETURN_IF_TRUE(AddAddition(expr1, expr2));
      RETURN_IF_TRUE(AddSubtraction(expr1, expr2));
      RETURN_IF_TRUE(AddMultiplication(expr1, expr2));
      RETURN_IF_TRUE(AddDivision(expr1, expr2));
      RETURN_IF_TRUE(AddPower(expr1, expr2));
    }
  }
  return false;
}

void TchislaSolver::NewGeneration(size_t num_new_parts) {
  current_generation_ = std::make_unique<PartitionedList<const Expr*>>(num_new_parts);
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
  RETURN_IF_TRUE(solver.found.load());
  if (expr->IsInt() && expr->GetIntUnsafe() == solver.target_) {
    solver.found.store(true);
    solver.result_ = expr->ToString();
    return true;
  }
  if (expr->GetDouble() < VALUE_MIN_LIMIT) return false;
  if (expr->GetDouble() > VALUE_MAX_LIMIT) return false;
  if (solver.AddReachableValueIfNotExist(*expr)) {
    expr_pool.CommitLastObject();
    solver.current_generation_->push_back(part_id, expr);
    RETURN_IF_TRUE(AddFactorial(expr));
    RETURN_IF_TRUE(AddSquareRoot(expr));
  }
  return false;
}

bool TchislaSolver::GenerationCreator::AddLiteral(size_t repeats) {
  ostringstream ss;
  while (repeats-- > 0) ss << solver.seed_;
  return AddCandidate(expr_pool.EmplaceObject<LiteralExpr>(ss.str()));
}

bool TchislaSolver::GenerationCreator::AddAddition(const Expr* expr1, const Expr* expr2) {
  return AddCandidate(expr_pool.EmplaceObject<AddExpr>(expr1, expr2));
}

bool TchislaSolver::GenerationCreator::AddSubtraction(const Expr* expr1, const Expr* expr2) {
  if (expr1->GetDouble() > expr2->GetDouble()) {
    return AddCandidate(expr_pool.EmplaceObject<SubExpr>(expr1, expr2));
  } else {
    return AddCandidate(expr_pool.EmplaceObject<SubExpr>(expr2, expr1));
  }
}

bool TchislaSolver::GenerationCreator::AddMultiplication(const Expr* expr1, const Expr* expr2) {
  return AddCandidate(expr_pool.EmplaceObject<MulExpr>(expr1, expr2));
}

bool TchislaSolver::GenerationCreator::AddDivision(const Expr* expr1, const Expr* expr2) {
  if (expr1->GetDouble() < Expr::DOUBLE_PRECISION ||
    expr2->GetDouble() < Expr::DOUBLE_PRECISION) return false;
  RETURN_IF_TRUE(AddCandidate(expr_pool.EmplaceObject<DivExpr>(expr1, expr2)));
  return AddCandidate(expr_pool.EmplaceObject<DivExpr>(expr2, expr1));
}

bool TchislaSolver::GenerationCreator::AddPower(const Expr* expr1, const Expr* expr2) {
  if (expr2->IsInt() && expr2->GetIntUnsafe() <= POWER_LIMIT) {
    RETURN_IF_TRUE(AddCandidate(expr_pool.EmplaceObject<PowExpr>(expr1, expr2)));
    RETURN_IF_TRUE(AddCandidate(expr_pool.EmplaceObject<NegPowExpr>(expr1, expr2)));
    RETURN_IF_TRUE(AddMultiSqrtPower(expr1, expr2));
  }
  if (expr1->IsInt() && expr1->GetIntUnsafe() <= POWER_LIMIT) {
    RETURN_IF_TRUE(AddCandidate(expr_pool.EmplaceObject<PowExpr>(expr2, expr1)));
    RETURN_IF_TRUE(AddCandidate(expr_pool.EmplaceObject<NegPowExpr>(expr2, expr1)));
    return AddMultiSqrtPower(expr2, expr1);
  }
  return false;
}

bool TchislaSolver::GenerationCreator::AddMultiSqrtPower(const Expr* expr1, const Expr* expr2) {
  int64_t power = expr2->GetIntUnsafe();
  int sqrt_times = 1;
  while ((power & 1) == 0) {
    power >>= 1;
    const Expr* expr = expr_pool.EmplaceObject<MultiSqrtPowExpr>(sqrt_times++, expr1, expr2);
    if (solver.deep_search_ || expr->IsInt()) {
      RETURN_IF_TRUE(AddCandidate(expr));
      RETURN_IF_TRUE(AddCandidate(expr_pool.EmplaceObject<NegMultiSqrtPowExpr>(sqrt_times, expr1, expr2)));
    }
  }
  return false;
}

bool TchislaSolver::GenerationCreator::AddFactorial(const Expr* expr) {
  if (expr->IsInt() && expr->GetIntUnsafe() <= FACTORIAL_LIMIT) {
    return AddCandidate(expr_pool.EmplaceObject<FactorialExpr>(expr));
  }
  return false;
}

bool TchislaSolver::GenerationCreator::AddSquareRoot(const Expr* expr) {
  if (expr->IsInt() && expr->GetIntUnsafe() > 0) {
    if (solver.deep_search_ || expr->GetIntUnsafe() == solver.seed_) {
      RETURN_IF_TRUE(AddCandidate(expr_pool.EmplaceObject<SqrtExpr>(expr)));
      return AddCandidate(expr_pool.EmplaceObject<DoubleSqrtExpr>(expr));
    } else {
      expr = expr_pool.EmplaceObject<SqrtExpr>(expr);
      if (expr->IsInt()) {
        return AddCandidate(expr);
      }
    }
  }
  return false;
}
