CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lrt -pthread

TARGETS = producer consumer

all: $(TARGETS)

producer: producer.cpp
	$(CXX) $(CXXFLAGS) -o producer producer.cpp $(LDFLAGS)

consumer: consumer.cpp
	$(CXX) $(CXXFLAGS) -o consumer consumer.cpp $(LDFLAGS)

clean:
	rm -f $(TARGETS)
	rm -f /dev/mqueue/ipc_demo_queue 2>/dev/null || true

.PHONY: all clean
