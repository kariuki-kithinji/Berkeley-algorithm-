#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#include "config.h"

#define PORT 8080
#define BUFFER_LEN 1024
#define NUM_CLIENTS 3

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    int r = rand() % 10 + 1;

    char *message = malloc(sizeof(char) * 2);
    sprintf(message, "%d", r);

    int sock = 0,
        valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_LEN];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
                             sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock, message, strlen(message), 0);
    printf("message %s sent\n", message);
    valread = read(sock, buffer, BUFFER_LEN);
    printf("%s\n", buffer);

    // closing the connected socket
    close(client_fd);
    return 0;
}
