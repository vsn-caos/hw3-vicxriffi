#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

static int send_all(int sockfd, const unsigned char *buffer, size_t size) {
    size_t sent = 0;

    while (sent < size) {
        ssize_t result = send(sockfd, buffer + sent, size - sent, 0);

        if (result > 0) {
            sent += (size_t)result;
        } else if (result == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }

    return 0;
}

static int receive_all(int sockfd, unsigned char *buffer, size_t size) {
    size_t received = 0;

    while (received < size) {
        ssize_t result = recv(sockfd, buffer + received, size - received, 0);

        if (result > 0) {
            received += (size_t)result;
        } else if (result == 0) {
            return 0;
        } else if (errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IPv4-address> <port>\n", argv[0]);
        return 1;
    }

    char *end = NULL;
    long port = strtol(argv[2], &end, 10);

    if (
        argv[2][0] == '\0' ||
        *end != '\0' ||
        port < 1 ||
        port > 65535
    ) {
        fprintf(stderr, "Invalid port\n");
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in address = {0};

    address.sin_family = AF_INET;
    address.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, argv[1], &address.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address\n");
        close(sockfd);
        return 1;
    }

    if (
        connect(
            sockfd,
            (struct sockaddr *)&address,
            sizeof(address)
        ) == -1
    ) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    int64_t input;

    while (scanf("%" SCNd64, &input) == 1) {
        if (input < INT32_MIN || input > INT32_MAX) {
            fprintf(stderr, "Input is outside int32 range\n");
            close(sockfd);
            return 1;
        }

        uint32_t raw = (uint32_t)(int32_t)input;

        unsigned char request[4] = {
            (unsigned char)(raw & 0xffu),
            (unsigned char)((raw >> 8) & 0xffu),
            (unsigned char)((raw >> 16) & 0xffu),
            (unsigned char)((raw >> 24) & 0xffu)
        };

        if (send_all(sockfd, request, sizeof(request)) == -1) {
            if (errno == EPIPE || errno == ECONNRESET) {
                close(sockfd);
                return 0;
            }

            perror("send");
            close(sockfd);
            return 1;
        }

        unsigned char response[4];

        int receive_status = receive_all(
            sockfd,
            response,
            sizeof(response)
        );

        if (receive_status == 0) {
            close(sockfd);
            return 0;
        }

        if (receive_status == -1) {
            if (errno == ECONNRESET) {
                close(sockfd);
                return 0;
            }

            perror("recv");
            close(sockfd);
            return 1;
        }

        uint32_t response_raw =
            (uint32_t)response[0] |
            ((uint32_t)response[1] << 8) |
            ((uint32_t)response[2] << 16) |
            ((uint32_t)response[3] << 24);

        int64_t signed_response =
            (response_raw & 0x80000000u)
                ? (int64_t)response_raw - 0x100000000LL
                : (int64_t)response_raw;

        printf("%" PRId64 "\n", signed_response);
    }

    close(sockfd);
    return 0;
}
