#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// global flag to check that there are only 4 instances of clients served:
int MAX_CLIENTS = 4;
int CURR_CLIENTS = 0;

void revstr(char *str1)
{
    // declare variable
    int i, len, temp;
    len = strlen(str1); // use strlen() to get the length of str string

    // use for loop to iterate the string
    for (i = 0; i < len / 2; i++)
    {
        // temp variable use to temporary hold the string
        temp = str1[i];
        str1[i] = str1[len - i - 1];
        str1[len - i - 1] = temp;
    }
}

int main(int argc, char **argv)
{
    int portNumber;
    pid_t pid;
    portNumber = atoi(argv[1]);

    struct sockaddr_in server_address;

    int server_fd, new_socket;
    char buffer[1024] = {0}, copy[1024] = {0};

    int opt = 1, valread = 0;
    int addrlen = sizeof(server_address);

    /*
    Creating a socket file descriptor. This is the first step in creating a socket.
    args: AF_INET refers to addresses from the internet
    SOCK_STREAM refers to TCP protocol
    0 refers to default protocol
    */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // check if there was an error in server creation:
    if (server_fd == 0)
    {
        printf("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    /*
    Setting socket options:
    This is an optional configuration that binds address and helps in reuse of port.
    */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        printf("Socket attaching failed");
        exit(EXIT_FAILURE);
    }

    /*
    Initializing the address structure:
    attributes:
    sin_family = AF_INET refers to addresses from the internet
    sin_addr is the IP address in the socket server
    It is a union whose s_addr attribute is a 4 byte int.
    sin_port is the port number in the socket server
    htons is used to translate the PORT macro into network byte order that TCP/IP uses.
    */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portNumber);

    /*
    Binding the socket to the address.
    */
    int bind_val = bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (bind_val < 0)
    {
        printf("binding socket to the port failed");
        exit(EXIT_FAILURE);
    }

    printf("Server connected with port %d and IP address %s\n", portNumber, inet_ntoa(server_address.sin_addr));
    fflush(0); // use fflush to print the statement before listening for connections.

    /*
    Listening for connections:
    parameters: sockfd -> refers to the socket to listen
    backlog -> refers to the number of connections that can be queued
    */
    int listen_val = listen(server_fd, 3);
    if (listen_val < 0)
    {
        printf("listening for connections failed");
        exit(EXIT_FAILURE);
    }

    int flag = 1;

    while (1)
    {
        /*
        Accepting a connection:
        parameters: sockfd -> refers to the socket to listen
        addr -> refers to the address of the client
        addrlen -> refers to the length of the address
        */
       if (CURR_CLIENTS > MAX_CLIENTS)
        {
            if(flag)
                printf("Only 4 clients can be served at a time\n");
            flag = 0;
            continue;
        }
        new_socket = accept(server_fd, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);
        CURR_CLIENTS++;
        printf("NEW CLIENT ACCEPTED, current clients: %d\n", CURR_CLIENTS);
        pid = fork();

        if (pid == 0)
        {
            if (CURR_CLIENTS > MAX_CLIENTS)
            {
                // if(flag)
                // printf("Only 4 clients can be served at a time\n");
                valread = read(new_socket, buffer, sizeof(buffer));
                bzero(buffer, sizeof(buffer));
                char message[100] = "Client limit reached. Rejecting the connection request\n";
                send(new_socket, message, sizeof(message), 0);
                close(new_socket);
                flag = 0;
                exit(0);
            }

            char readable_IP[100] = {0};

            // get the IP address in readable format.
            inet_ntop(AF_INET, &(server_address.sin_addr), readable_IP, sizeof(readable_IP));

            if (new_socket < 0)
            {
                printf("Accepting connection failed");
                exit(EXIT_FAILURE);
            }

            // read the text file name sent by client.
            bzero(buffer, sizeof(buffer));

            while (1)
            {
                bzero(buffer, sizeof(buffer));
                valread = read(new_socket, buffer, sizeof(buffer));
                if(strlen(buffer) == 0){
                    break;
                }
                if(strcmp(buffer, "exit") == 0) break;
                printf("Client with port %d, IP address %s and message %s\n", portNumber, readable_IP, buffer);
                strcpy(copy, buffer);   
                revstr(buffer);

                send(new_socket, buffer, sizeof(buffer), 0);
            }
            printf("Closing the connection now..\n");

            // close client and file pointer.
            close(new_socket);
            CURR_CLIENTS--;
        }
        else
        {
            close(new_socket);
        }
    }
}