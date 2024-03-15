#include "tchisla-solver.h"

#include <iostream>
#include <chrono> 

using namespace std;

int main() {
  constexpr int64_t target = 2016;
  auto loop_start = std::chrono::high_resolution_clock::now();

  for (int i = 1; i <= 9; ++i) {
    TchislaSolver ts(target, i, false, &std::cout);
    std::cout << target << " = " << (ts.Solve() ? ts.Result() : "Not Found") << std::endl << std::endl;
  }

  auto loop_end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(loop_end - loop_start).count();
  std::cout << "Total time for loop: " << duration << "ms" << std::endl;

  return 0;
}