#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    DEBUG = 0,
    WARNING,
    ERROR,
    CRITICAL
} LOG_LEVEL;

int InitializeLog();
void SetLogLevel(LOG_LEVEL level);
void Log(LOG_LEVEL level, const char *file, const char *func, int line, const char *message);
void ExitLog();
void *logReceiverThread(void *);

#endif // LOGGER_H
