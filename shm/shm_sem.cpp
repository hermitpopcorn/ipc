#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include "shm_data.h"
#include "common_shm.h"

#define SHM_NAME "/shm_demo"
#define SEM_NAME "/shm_demo_sem"

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
        if (ftruncate(shm_fd, sizeof(SharedData)) < 0) {
            perror("ftruncate");
            return 1;
        }
    }

    // Map to memory
    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData),
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED, shm_fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    close(shm_fd);

    // Create or open semaphore
    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    // Define lock/unlock functions for semaphore
    auto lock = [sem]() { sem_wait(sem); };
    auto unlock = [sem]() { sem_post(sem); };

    // Run main loop
    run_common_process(data, mode, lock, unlock);

    // Cleanup
    if (is_master) {
        sem_close(sem);
        sem_unlink(SEM_NAME);
        shm_unlink(SHM_NAME);
    } else {
        sem_close(sem);
    }

    munmap(data, sizeof(SharedData));
    return 0;
}
