#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define WORKER_COUNT 4
// 5 slots: 0 for Producer, 1-4 for Workers (mapped from ID 0-3)
#define TOTAL_SLOTS (WORKER_COUNT + 1)
#define PRODUCER_SLOT 0

// Global state tracking
static char current_states[TOTAL_SLOTS][32];
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Global logging flag
static int logging_enabled = 0;

void init_logger(void) {
    pthread_mutex_lock(&log_mutex);
    logging_enabled = 1;  // Enable logging
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        strcpy(current_states[i], "Ready");
    }
    // Print Header
    printf("%-12s | %-6s | %-10s | %-10s | %-10s | %-10s | %-10s | %s\n",
           "Timestamp", "FD", "Producer", "Worker 0", "Worker 1", "Worker 2", "Worker 3", "Comments");
    printf("------------------------------------------------------------------------------------------------------------------\n");
    pthread_mutex_unlock(&log_mutex);
}

int is_logging_enabled(void) {
    return logging_enabled;
}

void log_event(int entity_id, int client_fd, const char* state, const char* comment) {
    if (!logging_enabled) {
        return;  // Logging disabled, skip
    }
    pthread_mutex_lock(&log_mutex);

    // Map entity_id to slot index
    int slot_index;
    if (entity_id == ID_PRODUCER) {
        slot_index = PRODUCER_SLOT;
    } else if (entity_id >= 0 && entity_id < WORKER_COUNT) {
        slot_index = entity_id + 1;
    } else {
        // Unknown entity
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    // Update state
    if (state != NULL) {
        snprintf(current_states[slot_index], sizeof(current_states[slot_index]), "%s", state);
    }

    // Get formatted timestamp
    char time_buf[64];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);
    
    // Append milliseconds
    char timestamp[64];
    snprintf(timestamp, sizeof(timestamp), "%s.%03ld", time_buf, tv.tv_usec / 1000);

    // Format FD string
    char fd_str[8];
    if (client_fd >= 0) {
        snprintf(fd_str, sizeof(fd_str), "%d", client_fd);
    } else {
        strcpy(fd_str, "-");
    }

    // Print the row
    printf("%-12s | %-6s | %-10s | %-10s | %-10s | %-10s | %-10s | %s\n",
           timestamp,
           fd_str,
           current_states[0], // Producer
           current_states[1], // Worker 0
           current_states[2], // Worker 1
           current_states[3], // Worker 2
           current_states[4], // Worker 3
           comment ? comment : ""
    );

    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}
