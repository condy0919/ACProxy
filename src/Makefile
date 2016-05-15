SRCS = $(shell find . -iname "*.cpp")
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

PROC = acproxy

LIBS = -pthread -lboost_system -lhiredis

CXXFLAGS = -std=c++17 -g -ggdb

all: LINK

LINK: $(OBJS)
	$(CXX) -o $(PROC) $(OBJS) $(CXXFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm $(OBJS) -f
