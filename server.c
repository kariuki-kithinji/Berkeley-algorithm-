#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>


#define BUFFER_LEN 1024
#define PORT 8080
#define NUM_CLIENTS 3

int calculate_average(int *clocks, int clocks_len, int server_clock);
void save_data(int *client_clocks, int count_clients);

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    int server_fd, new_socket, valread;
    int count_clients = 0;
    int *client_socket = malloc(sizeof(int) * count_clients);
    int *client_clocks = malloc(sizeof(int) * count_clients);
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;
    char buffer[BUFFER_LEN];
    int r = rand() % 10;
    time_t start = time(NULL);
    time_t end = time(NULL);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("socket configuration failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("waiting for a client to connect...\n");

    while (true)
    {
        memset(buffer, '\0', BUFFER_LEN);

        while (count_clients < NUM_CLIENTS)
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                     (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("connection accept\n");
            count_clients++;
            client_socket = realloc(client_socket, sizeof(int) * count_clients);
            client_clocks = realloc(client_clocks, sizeof(int) * count_clients);
            client_socket[count_clients - 1] = new_socket;
        }

        // measure and save the local clock value of 3 random clients every 100ms and save to file
        end = time(NULL);
        if (difftime(end, start) >= 0.1) {
            start = end;
            int clients_to_measure[] = {0, 1, 2}; // Indexes of the clients to measure
            int num_clients_to_measure = 3;

            for (int i = 0; i < num_clients_to_measure; i++)
            {
                int index = clients_to_measure[i];
                valread = read(client_socket[index], buffer, BUFFER_LEN);
                printf("message payload: %s\n", buffer);
                int n = atoi(buffer);
                client_clocks[index] = n;
            }

            save_data(client_clocks, count_clients);
        }

        char *avg = calloc(BUFFER_LEN, sizeof(char));
        snprintf(avg, BUFFER_LEN, "%d", calculate_average(client_clocks, count_clients, r));

        for (int i = 0; i < count_clients; i++)
        {
            send(client_socket[i], avg, strlen(avg), 0);
            printf("%s message sent to client %d\n", avg, i);

            close(client_socket[i]);
            printf("client disconnected\n");
        }

        free(avg);
        count_clients = 0;
    }

    printf("closing server\n");
    shutdown(server_fd, SHUT_RDWR);
}

int calculate_average(int *clocks, int clocks_len, int server_clock)
{
    double avg = 0;
    for (int i = 0; i < clocks_len; i++)
    {
        avg += clocks[i];
    }

    return avg / clocks_len;
}

void save_data(int *client_clocks, int count_clients) {
    FILE *fp;
    char filename[100];
    sprintf(filename, "data_%ld.csv", time(NULL));
    fp = fopen(filename, "w");
    fprintf(fp, "A,B,C\n");
    for (int i = 0; i < count_clients; i++) {
        fprintf(fp, "%d", client_clocks[i]);
        if (i != count_clients - 1) {
            fprintf(fp, ",");
        }
    }
    fclose(fp);
}
