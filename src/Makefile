SRCS = $(shell find . -iname "*.cpp")
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

PROC = "acproxy"

CXXFLAGS = -std=c++1y -g -pthread

all: LINK

LINK: $(OBJS)
	$(CXX) -o $(PROC) $(OBJS) $(CXXFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm $(OBJS)
