#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include "common_shm.h"

#define SHM_NAME "/shm_demo_mutex"

struct SharedDataWithMutex {
    pthread_mutex_t mutex;
    SharedData actual;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <master|slave>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    bool is_master = (mode == "master");

    // Create or open shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        return 1;
    }

    // Set size
    if (is_master) {
        if (ftruncate(shm_fd, sizeof(SharedDataWithMutex)) < 0) {
            perror("ftruncate");
            return 1;
        }
    }

    // Map to memory
    SharedDataWithMutex* shm_data = static_cast<SharedDataWithMutex*>(mmap(nullptr, sizeof(SharedDataWithMutex),
                                                                PROT_READ | PROT_WRITE,
                                                                MAP_SHARED, shm_fd, 0));
    if (shm_data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    close(shm_fd);

    if (is_master) {
        // Initialize mutex for shared memory (process-shared)
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shm_data->mutex, &attr);
        pthread_mutexattr_destroy(&attr);

        shm_data->actual.message_counter = 0;
    }

    // Define lock/unlock functions for mutex
    auto lock = [shm_data]() { pthread_mutex_lock(&shm_data->mutex); };
    auto unlock = [shm_data]() { pthread_mutex_unlock(&shm_data->mutex); };

    // Run main loop
    run_common_process(&(shm_data->actual), mode, lock, unlock);

    // Cleanup
    if (is_master) {
        pthread_mutex_destroy(&shm_data->mutex);
        shm_unlink(SHM_NAME);
    }

    munmap(shm_data, sizeof(SharedDataWithMutex));
    return 0;
}
