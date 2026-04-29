CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
LDFLAGS = -pthread
TARGETS = hub client

all: $(TARGETS)

hub: hub.cpp message.hpp
	$(CXX) $(CXXFLAGS) -o hub hub.cpp

client: client.cpp message.hpp
	$(CXX) $(CXXFLAGS) -o client client.cpp $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
