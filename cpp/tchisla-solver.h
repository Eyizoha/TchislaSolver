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
	static int64_t MUILT_THREADS_THRESHOLD;

	TchislaSolver(int64_t target, int64_t seed, std::ostream* trace_os = nullptr);

	bool Solve(int search_depth = 20);

	std::string Result() const { return result_; }

private:
	const int64_t target_;
	const int64_t seed_;
	std::ostream* trace_os_ = nullptr;

	ConcurrentSet<int64_t> reachable_ints_;
	ConcurrentSet<double> reachable_doubles_;

	using ExprPtr = std::unique_ptr<const Expr>;
	using GenerationPtr = std::unique_ptr<PartitionedList<ExprPtr>>;
	GenerationPtr current_generation_;
	std::vector<GenerationPtr> generations_;
	std::atomic_bool found = false;
	std::string result_;

	bool UseMultiThread() const;

	bool AddReachableValueIfNotExist(const Expr& expr);

	void NewGeneration(size_t num_new_parts);
	void EndGeneration();

	struct GenerationCreator {
		TchislaSolver& solver;
		size_t part_id;

		GenerationCreator(TchislaSolver& solver, size_t part_id)
			: solver(solver), part_id(part_id) { }

		bool CrossGeneration(const GenerationPtr& g1, const GenerationPtr& g2);

		bool AddCandidate(const Expr* expr);

		bool AddLiteral(size_t repeats);
		bool AddAddition(const Expr* expr1, const Expr* expr2);
		bool AddSubtraction(const Expr* expr1, const Expr* expr2);
		bool AddMultiplication(const Expr* expr1, const Expr* expr2);
		bool AddDivision(const Expr* expr1, const Expr* expr2);
		bool AddPower(const Expr* expr1, const Expr* expr2);
		bool AddFactorial(const Expr* expr);
		bool AddSquareRoot(const Expr* expr);
	};
};
