#ifndef LOGGER_H
#define LOGGER_H

// IDs: -1 for Producer, 0-3 for Workers
#define ID_PRODUCER -1

void init_logger(void);
int is_logging_enabled(void);
void log_event(int entity_id, int client_fd, const char* state, const char* comment);

#endif
