#include <iostream>

#include "argh.h"
#include "tchisla-solver.h"

using std::cout;
using std::cerr;
using std::endl;

void PrintUsage() {
  cout << "Usage: tchisla_solver target [seed]\n"
    << "Options:\n"
    << "  -h, --help                          Show this help message\n"
    << "  -t, --trace                         Print trace of current search generation and the number of reachable values\n"
    << "  -d, --deep_search                   Enable deep search mode, slow but will activate extra search strategies\n"
    << "  -dd, --deeper_search                Enable deeper search mode, slower but will activate all search strategies\n"
    << "  --precision=double_value            Set precision for double's approximation integer and existence test (default: 1e-8)\n"
    << "  --value-max-limit=double_value      Set maximum limit for reachable values during search, larger values will be ignored (default: 1e12)\n"
    << "  --value-min-limit=double_value      Set minimum limit for reachable values during search, smaller values will be ignored (default: 1e-8)\n"
    << "  --power-limit=int_value             Set the maximum exponent value for power calculations (default: 40)\n"
    << "  --factorial-limit=int_value         Set the maximum original value for factorial calculations (default: 15)\n"
    << "  --muilt-threads-threshold=int_value Set the threshold for enabling multi-threading in next generation search when a generation reachable values exceeds this number (default: 10000)\n"
    << "  --search-depth=DEPTH                Set the maximum number of iterations for searching a target value (default: 20)\n"
    << "\n"
    << "Examples:\n"
    << "  tchisla_solver 1234                 Search using digits 1 to 9 to calculate 1234\n"
    << "  tchisla_solver 1234 5               Search using digit 5 to calculate 1234\n";
}


int main(int argc, char* argv[]) {
  argh::parser cmdl;
  cmdl.parse(argc, argv);

  if (cmdl[{ "-h", "--help" }]) {
    PrintUsage();
    return 0;
  }

  bool trace = cmdl[{ "-t", "--trace" }];
  bool deep_search = cmdl[{ "-d", "--deep_search" }];
  bool deeper_search = cmdl[{ "-dd", "--deeper_search" }];
  int search_mode = deeper_search ? 2 : deep_search;

  double dvalue;
  if (cmdl("precision")) {
    cmdl("precision") >> dvalue;
    if (0 < dvalue && dvalue < 1) Expr::DOUBLE_PRECISION = dvalue;
  }
  if (cmdl("value-max-limit")) {
    cmdl("value-max-limit") >> dvalue;
    if (0 < dvalue) TchislaSolver::VALUE_MAX_LIMIT = dvalue;
  }
  if (cmdl("value-min-limit")) {
    cmdl("value-min-limit") >> dvalue;
    if (0 < dvalue) TchislaSolver::VALUE_MIN_LIMIT = dvalue;
  }

  int64_t ivalue;
  if (cmdl("power-limit")) {
    cmdl("power-limit") >> ivalue;
    if (0 < ivalue) TchislaSolver::POWER_LIMIT = ivalue;
  }
  if (cmdl("factorial-limit")) {
    cmdl("factorial-limit") >> ivalue;
    if (0 < ivalue) TchislaSolver::FACTORIAL_LIMIT = ivalue;
  }
  if (cmdl("muilt-threads-threshold")) {
    cmdl("muilt-threads-threshold") >> ivalue;
    if (0 < ivalue) TchislaSolver::MUILT_THREADS_THRESHOLD = ivalue;
  }
  int64_t search_depth = -1;
  if (cmdl("search-depth")) {
    cmdl("search-depth") >> ivalue;
    if (0 < ivalue) search_depth = ivalue;
  }

  int64_t target;
  if (!(cmdl(1) >> target) || target <= 0) {
      cerr << "Error: A positive target value is required!" << endl;
      return 1;
  }

  int64_t seed = 0;
  if (cmdl(2)) {
      if (!(cmdl(2) >> seed) || seed <= 0) {
          cerr << "Error: Seed value must be a positive integer!" << endl;
          return 1;
      }
  }

  if (seed != 0) {
    TchislaSolver ts(target, seed, search_mode, trace ? &cout : nullptr);
    bool found = ts.Solve(search_depth > 0 ? search_depth : 20);
    cout << target << " = " << (found ? ts.Result() : "Not Found") << '\n' << endl;
  } else {
    for (int i = 1; i <= 9; ++i) {
      TchislaSolver ts(target, i, search_mode, trace ? &cout : nullptr);
      bool found = ts.Solve(search_depth > 0 ? search_depth : 20);
      cout << target << " = " << (found ? ts.Result() : "Not Found") << '\n' << endl;
    }
  }

  return 0;
}
