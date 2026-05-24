#ifndef SHM_DATA_H
#define SHM_DATA_H

#define MAX_DATA_SIZE 256

struct SharedData {
    char message[MAX_DATA_SIZE];
    char sender[64];
    int message_counter;
};

#endif
