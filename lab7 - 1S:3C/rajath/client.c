#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


int compare(char a[],char b[])  
{  
    int flag=0,i=0;  // integer variables declaration  
    while(a[i]!='\0' &&b[i]!='\0')  // while loop  
    {  
       if(a[i]!=b[i])  
       {  
           flag=1;  
           break;  
       }  
       i++;  
    }  
    if(flag==0)  
        return 0;  
    else  
        return 1;  
}  
int main(int argc, char *argv[])
{
    // initialising variables needed for the connection
    int client_sock = 0, valread;
    struct sockaddr_in client_address;

    char buffer[1024] = {0};
    char *ipAddress;

    int portNumber;

    portNumber = atoi(argv[1]);
    bzero(buffer, sizeof(buffer));

    ipAddress = argv[2];

    // printf("Received %s %d\n", ipAddress, portNumber);

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

    // adding attributes to the client_address struct, giving the ipAddress received and the portNumber.
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(portNumber);
    client_address.sin_addr.s_addr = inet_addr(ipAddress);

    // connect to the server.
    int connect_val = connect(client_sock, (struct sockaddr *)&client_address, sizeof(client_address));

    // connection error, if port is wrong, IP is wrong, or if server is unavailable.
    if (connect_val == -1)
    {
        printf("Connection Failed, server unreachable\n");
        exit(0);
    }
    
    printf("Connected to server with port %d and IP %s\n", portNumber, ipAddress);
    fflush(0);

    // send the sentence to reverse in server.
    printf("Enter a sentence\n");
    fgets(buffer, sizeof(buffer), stdin);

    buffer[strlen(buffer) - 1] = '\0';
    if (compare(buffer, "exit") == 0)
    {
        exit(0);
    }
    send(client_sock, buffer, strlen(buffer), 0);

    bzero(buffer, sizeof(buffer));

    valread = read(client_sock, buffer, sizeof(buffer));
    char error[100] = "Client limit reached. Rejecting the connection request\n";
    if(compare(buffer, error) == 0){
        printf("Client rejected by server. Exiting\n");
        exit(0);
    }

    // printf("Received from server: %s\n", buffer);
    int flag = 1;

    while (compare(buffer, "exit") != 0)
    {
        if (!flag)
        {
            send(client_sock, buffer, strlen(buffer), 0);

            bzero(buffer, sizeof(buffer));

            valread = read(client_sock, buffer, sizeof(buffer));

            printf("Received from server: %s\n", buffer);
        }else{
            flag = 0;
        }
        printf("Enter a sentence\n");
        fgets(buffer, sizeof(buffer), stdin);
    }

    // close the client
    printf("Closing the connection...\n");
    close(client_sock);

    return 0;
}
