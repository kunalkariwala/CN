#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int portNumber;
    printf("Enter the port number\n");
    scanf("%d", &portNumber);

    struct sockaddr_in server_address;

    int server_fd, new_socket;
    char buffer[1024] = {0};

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

    printf("Server connected with port %d and IP address %s", portNumber, inet_ntoa(server_address.sin_addr));
    fflush(0);

    /*
    Listening for connections:
    parameters: sockfd -> refers to the socket to listen
    backlog -> refers to the number of connections that can be queued
    */
    int listen_val = listen(server_fd, 0);
    if (listen_val < 0)
    {
        printf("listening for connections failed");
        exit(EXIT_FAILURE);
    }

    /*
    Accepting a connection:
    parameters: sockfd -> refers to the socket to listen
    addr -> refers to the address of the client
    addrlen -> refers to the length of the address
    */
    new_socket = accept(server_fd, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);
    char readable_IP[100] = {0};

    inet_ntop(AF_INET, &(server_address.sin_addr), readable_IP, sizeof(readable_IP));

    printf("Accepted a connection from a client with port %d and IP address %s\n", portNumber, readable_IP);
    if (new_socket < 0)
    {
        printf("Accepting connection failed");
        exit(EXIT_FAILURE);
    }

    // read the text file name sent by client.
    valread = read(new_socket, buffer, sizeof(buffer));

    FILE *fptr;
    fptr = fopen(buffer, "r");
    if (fptr == NULL)
    {
        fptr = fopen(buffer, "w");
    }

    char fileText[1024];

    fscanf(fptr, "%s", fileText);

    printf("%s", fileText);

    int size = (strlen(fileText) < 10 ? strlen(fileText) : 10);

    send(new_socket, fileText, size, 0);

    printf("Sent the contents of %s to the client\n", buffer);
    printf("Closing the connection now..\n");

    close(new_socket);
    fclose(fptr);
}