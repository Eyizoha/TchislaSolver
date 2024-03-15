#pragma once

#include <atomic>
#include <memory>

#include "expr.h"
#include "util.h"


class TchislaSolver {
public:
  static double VALUE_MAX_LIMIT;
  static double VALUE_MIN_LIMIT;
  static int64_t POWER_LIMIT;
  static int64_t FACTORIAL_LIMIT;
  static size_t MUILT_THREADS_THRESHOLD;

  TchislaSolver(int64_t target, int64_t seed, bool deep_search = false,
      std::ostream* trace_os = nullptr);

  bool Solve(int search_depth = 20);

  std::string Result() const { return result_; }

private:
  struct GenerationCreator;

  TchislaSolver(const TchislaSolver&) = delete;
  TchislaSolver& operator=(const TchislaSolver&) = delete;

  const int64_t target_;
  const int64_t seed_;
  const bool deep_search_;
  std::ostream* trace_os_ = nullptr;

  ConcurrentNumericSet<11> reachable_values_;

  std::vector<GenerationCreator> creators_;

  static constexpr size_t OBJ_POOL_SIZE = 1024 * 1024;
  std::vector<std::unique_ptr<ObjectPool<OBJ_POOL_SIZE>>> expr_pools_;

  using GenerationPtr = std::unique_ptr<PartitionedList<const Expr*>>;
  GenerationPtr current_generation_;
  std::vector<GenerationPtr> generations_;
  std::atomic_bool found = false;
  std::string result_;

  bool UseMultiThread() const;
  void MultiThreadCrossGeneration(size_t num_loops);

  bool AddReachableValueIfNotExist(const Expr& expr);

  void NewGeneration(size_t num_new_parts);
  void EndGeneration();

  struct GenerationCreator {
    TchislaSolver& solver;
    ObjectPool<OBJ_POOL_SIZE>& expr_pool;
    size_t part_id;

    GenerationCreator(TchislaSolver& solver, size_t part_id)
      : solver(solver), expr_pool(*solver.expr_pools_[part_id]), part_id(part_id) { }

    bool CrossGeneration(const GenerationPtr& g1, const GenerationPtr& g2);

    bool AddCandidate(const Expr* expr);

    bool AddLiteral(size_t repeats);
    bool AddAddition(const Expr* expr1, const Expr* expr2);
    bool AddSubtraction(const Expr* expr1, const Expr* expr2);
    bool AddMultiplication(const Expr* expr1, const Expr* expr2);
    bool AddDivision(const Expr* expr1, const Expr* expr2);
    bool AddPower(const Expr* expr1, const Expr* expr2);
    bool AddMultiSqrtPower(const Expr* expr1, const Expr* expr2);
    bool AddFactorial(const Expr* expr);
    bool AddSquareRoot(const Expr* expr);
  };
};
