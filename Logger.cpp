#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#include "logger.h"

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT_NUMBER 9000
#define BUFFER_LENGTH 1024

static int loggerSocket;
static struct sockaddr_in serverAddress;
static LOG_LEVEL currentLogLevel = DEBUG;
static bool loggerRunning = true;
static pthread_t receiverThread;
static pthread_mutex_t logMutex;

// Function to handle receiving log messages and setting log levels dynamically.
void *logReceiverThread(void *) {
    char buffer[BUFFER_LENGTH];
    while (loggerRunning) {
        socklen_t addressLength = sizeof(serverAddress);
        ssize_t receivedLength = recvfrom(loggerSocket, buffer, BUFFER_LENGTH, 0, NULL, NULL);
        if (receivedLength > 0) {
            buffer[receivedLength] = '\0';
            if (strncmp(buffer, "Set Log Level=", 14) == 0) {
                int level = atoi(buffer + 14);
                SetLogLevel((LOG_LEVEL)level);
                printf("Log level set to %d\n", LOG_LEVEL(level));
            }
        } else if (receivedLength < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom failed");
            }
            sleep(1);
          
        }
    }
    return NULL;
}

// Initializes the logger, including socket creation and thread setup.
int InitializeLog() {
    if (pthread_mutex_init(&logMutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return -1;
    }

    loggerSocket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (loggerSocket < 0) {
        perror("Socket creation failed");
        pthread_mutex_destroy(&logMutex);
        return -1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT_NUMBER);
    if (inet_pton(AF_INET, SERVER_IP_ADDRESS, &serverAddress.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(loggerSocket);
        pthread_mutex_destroy(&logMutex);
        return -1;
    }

    loggerRunning = true;
    if (pthread_create(&receiverThread, NULL, logReceiverThread, NULL) != 0) {
        perror("Thread creation failed");
        close(loggerSocket);
        pthread_mutex_destroy(&logMutex);
        return -1;
    }

    return 0;
}

// Sets the current log level for filtering log messages.
void SetLogLevel(LOG_LEVEL level) {
    pthread_mutex_lock(&logMutex);
    currentLogLevel = level;
    pthread_mutex_unlock(&logMutex);
}

// Logs a message with the specified log level, file, function, and line information.
void Log(LOG_LEVEL level, const char *file, const char *function, int line, const char *message) {
    pthread_mutex_lock(&logMutex);
    if (level < currentLogLevel) {
        pthread_mutex_unlock(&logMutex);
        return;
    }

    time_t now = time(0);
    if (now == ((time_t)-1)) {
        perror("Failed to get current time");
        pthread_mutex_unlock(&logMutex);
        return;
    }

    char *dateTime = ctime(&now);
    if (dateTime == NULL) {
        perror("Failed to convert time to string");
        pthread_mutex_unlock(&logMutex);
        return;
    }
    dateTime[strlen(dateTime) - 1] = '\0';

    char buffer[BUFFER_LENGTH];
    const char *levelStrings[] = {"DEBUG", "WARNING", "ERROR", "CRITICAL"};
    int length = snprintf(buffer, BUFFER_LENGTH, "%s %s %s:%s:%d %s\n",
                          dateTime, levelStrings[level], file, function, line, message);
    if (length < 0 || length >= BUFFER_LENGTH) {
        perror("Failed to format log message");
        pthread_mutex_unlock(&logMutex);
        return;
    }

    if (sendto(loggerSocket, buffer, length, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to send log message");
    }

    pthread_mutex_unlock(&logMutex);
}

// Cleans up resources used by the logger, including threads and sockets.
void ExitLog() {
    loggerRunning = false;
    if (pthread_join(receiverThread, NULL) != 0) {
        perror("Failed to join receiver thread");
    }

    if (close(loggerSocket) < 0) {
        perror("Failed to close logger socket");
    }

    if (pthread_mutex_destroy(&logMutex) != 0) {
        perror("Failed to destroy mutex");
    }
}



