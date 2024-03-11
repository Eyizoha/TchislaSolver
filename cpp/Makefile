CXX = g++
CXXFLAGS = -Wall -O3 -std=c++17
LDFLAGS = -pthread -static-libstdc++

TARGET1 = tchisla-solver
TARGET1_SRCS = expr.cc tchisla-solver.cc main.cc
TARGET1_OBJS = $(TARGET1_SRCS:.cc=.o)

TARGET2 = test
TARGET2_SRCS = expr.cc tchisla-solver.cc test.cc
TARGET2_OBJS = $(TARGET2_SRCS:.cc=.o)

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(TARGET1_OBJS)
	$(CXX) $(CXXFLAGS) $(TARGET1_OBJS) -o $@ $(LDFLAGS)

$(TARGET2): $(TARGET2_OBJS)
	$(CXX) $(CXXFLAGS) $(TARGET2_OBJS) -o $@ $(LDFLAGS)

expr.o: expr.cc expr.h
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cc argh.h tchisla-solver.h
	$(CXX) $(CXXFLAGS) -c $<

tchisla-solver.o: tchisla-solver.cc tchisla-solver.h
	$(CXX) $(CXXFLAGS) -c $<

test.o: test.cc tchisla-solver.h
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(TARGET1_OBJS) $(TARGET2_OBJS) $(TARGET1) $(TARGET2)
