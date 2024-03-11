CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17
LDFLAGS = -pthread -static-libstdc++
TARGET = tchisla_solver
SRCS = expr.cc tchisla-solver.cc test.cc
OBJS = $(SRCS:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)

expr.o: expr.cc expr.h
	$(CXX) $(CXXFLAGS) -c $<

tchisla-solver.o: tchisla-solver.cc tchisla-solver.h
	$(CXX) $(CXXFLAGS) -c $<

test.o: test.cc tchisla-solver.h
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
