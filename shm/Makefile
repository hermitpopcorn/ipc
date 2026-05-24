CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
LDFLAGS = -pthread -lrt
TARGETS = shm_sem shm_mutex

all: $(TARGETS)

shm_sem: shm_sem.cpp shm_data.h common_shm.h
	$(CXX) $(CXXFLAGS) -o shm_sem shm_sem.cpp $(LDFLAGS)

shm_mutex: shm_mutex.cpp shm_data.h common_shm.h
	$(CXX) $(CXXFLAGS) -o shm_mutex shm_mutex.cpp $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
