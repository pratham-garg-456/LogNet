#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>

#define BUFFER_LENGTH 1024
#define LOG_FILE_PATH "server.log"
#define SERVER_PORT_NUMBER 9000

static int serverSocket;
static bool serverRunning = true;
static pthread_t receiverThread;
static pthread_mutex_t fileMutex;
static struct sockaddr_in clientAddress;
static socklen_t clientAddressLength = sizeof(clientAddress);

// Handles the shutdown signal (e.g., SIGINT) to gracefully stop the server.
void handleShutdownSignal(int);

// Thread function to receive log messages and write them to the log file.
void *logReceiver(void *);

// Main function to initialize the server, handle user input, and manage server lifecycle.
int main() {
    if (pthread_mutex_init(&fileMutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handleShutdownSignal);

    serverSocket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        pthread_mutex_destroy(&fileMutex);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT_NUMBER);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        pthread_mutex_destroy(&fileMutex);
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&receiverThread, NULL, logReceiver, NULL) != 0) {
        perror("Thread creation failed");
        close(serverSocket);
        pthread_mutex_destroy(&fileMutex);
        exit(EXIT_FAILURE);
    }

    int userChoice, logLevel;
    char buffer[BUFFER_LENGTH];

    while (serverRunning) {
        printf("\nMenu:\n1. Set log level\n2. Dump log\n0. Shutdown\nChoice: ");
        if (scanf("%d", &userChoice) != 1) {
            fprintf(stderr, "Invalid input\n");
            getchar();
            continue;
        }
        getchar();

        if (userChoice == 1) {
            printf("Enter new log level (0=DEBUG to 3=CRITICAL): ");
            if (scanf("%d", &logLevel) != 1) {
                fprintf(stderr, "Invalid log level\n");
                getchar();
                continue;
            }
            getchar();

            snprintf(buffer, BUFFER_LENGTH, "Set Log Level=%d", logLevel);
            if (sendto(serverSocket, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&clientAddress, clientAddressLength) < 0) {
                perror("Failed to send log level");
            }
        } else if (userChoice == 2) {
            pthread_mutex_lock(&fileMutex);
            FILE *logFile = fopen(LOG_FILE_PATH, "r");
            if (!logFile) {
                perror("Failed to open log file");
            } else {
                char character;
                while ((character = fgetc(logFile)) != EOF) putchar(character);
                if (ferror(logFile)) {
                    perror("Error reading log file");
                }
                if (fclose(logFile) != 0) {
                    perror("Failed to close log file");
                }
            }
            pthread_mutex_unlock(&fileMutex);
            printf("\nPress any key to continue:");
            getchar();
        } else if (userChoice == 0) {
            printf("Shutting down server...\n");
            serverRunning = false;
        } else {
            fprintf(stderr, "Invalid choice\n");
        }
    }

    if (pthread_join(receiverThread, NULL) != 0) {
        perror("Failed to join thread");
    }

    if (close(serverSocket) < 0) {
        perror("Failed to close socket");
    }

    if (pthread_mutex_destroy(&fileMutex) != 0) {
        perror("Failed to destroy mutex");
    }

    return 0;
}

// Handles the shutdown signal (e.g., SIGINT) to gracefully stop the server.
void handleShutdownSignal(int) {
    serverRunning = false;
}

// Thread function to receive log messages and write them to the log file.
void *logReceiver(void *) {
    char buffer[BUFFER_LENGTH];
    FILE *logFile = fopen(LOG_FILE_PATH, "a");

    if (!logFile) {
        perror("Log file open failed");
        return NULL;
    }

    while (serverRunning) {
        ssize_t receivedLength = recvfrom(serverSocket, buffer, BUFFER_LENGTH, 0,
                                          (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (receivedLength > 0) {
            pthread_mutex_lock(&fileMutex);
            buffer[receivedLength] = '\0';
            if (fprintf(logFile, "%s", buffer) < 0) {
                perror("Failed to write to log file");
            }
            fflush(logFile);
            pthread_mutex_unlock(&fileMutex);
        } else if (receivedLength < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom failed");
            }
            sleep(1);
        }
    }

    if (fclose(logFile) != 0) {
        perror("Failed to close log file");
    }
    return NULL;
}


