CXX = g++
CXXFLAGS = -Wall -g
TARGET = tchisla_solver
SRCS = expr.cc tchisla-solver.cc test.cc
OBJS = $(SRCS:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) -o $@ $^

expr.o: expr.cc expr.h
    $(CXX) $(CXXFLAGS) -c $<

tchisla-solver.o: tchisla-solver.cc tchisla-solver.h
    $(CXX) $(CXXFLAGS) -c $<

test.o: test.cc tchisla-solver.h
    $(CXX) $(CXXFLAGS) -c $<

.PHONY: clean
clean:
    rm -f $(OBJS) $(TARGET)
