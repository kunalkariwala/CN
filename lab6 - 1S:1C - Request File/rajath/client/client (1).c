#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
    // initialising variables needed for the connection
    int client_sock = 0, valread;
    struct sockaddr_in client_address;

    char buffer[1024];
    char ipAddress[100];
    char fileName[100];
    int portNumber;


    printf("Enter a port number : ");
    fgets(buffer, sizeof(buffer), stdin);
    portNumber = atoi(buffer);
    bzero(buffer, sizeof(buffer));
    
    printf("Enter the IP address: ");
    fgets(ipAddress, sizeof(ipAddress), stdin);

    printf("Enter the file name: ");
    fgets(fileName, sizeof(fileName), stdin);

    
    /*
    Creating a socket file descriptor. This is the first step in creating a socket.
    args: AF_INET refers to addresses from the internet
    SOCK_STREAM refers to TCP protocol
    0 refers to default protocol
    */
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // check if there was an error in creating the server:
    if (client_sock == -1)
    {
        perror("Error creating socket\n");
        exit(0);
    }

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(portNumber);

    

    //connect to the server.
    int connect_val = connect(client_sock, (struct sockaddr *)&client_address, sizeof(client_address));
    if (connect_val == -1)
    {
        printf("Connection Failed, server unreachable\n");
        exit(0);
    }

    printf("Connected to server with port 4444 and IP\n");
    fflush(0);
    send(client_sock, fileName, strlen(fileName), 0);

    valread = read(client_sock, buffer, 1024);

    if(valread == -1){
	    printf("Error receiving data from server\n");
        exit(0);
    }

    FILE *fptr;

    fptr = fopen(fileName, "w");

    fprintf(fptr, "%s", buffer);
    printf("Received data from %s on server, writing it into a new file..\n", fileName);
    fclose(fptr);

    printf("Closing the connection...\n");
    close(client_sock);
    
    return 0;
}
