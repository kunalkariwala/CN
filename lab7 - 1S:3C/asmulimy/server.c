#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define buffer_size 1024
pthread_t threads[4];
int msg_sent[4];
int clients[4];

typedef struct
{
    int new_sock;
    int id;
    int port;
    struct in_addr addrs;
} client;

// Taken from https://stackoverflow.com/questions/8534274/is-the-strrev-function-not-available-in-linux
char *strrev(char *str)
{
    char *p1, *p2;

    if (!str || !*str)
        return str;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
    {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

void *broadcast(void *args)
{
    char broadcastmsg[1024];
    int count = 0;
    while(1){
        while (1)
        {
            count = 0;
            for (int i = 0; i < 4; i++)
            {
                if (clients[i] != -1)
                {
                    count++;
                }
            }
            if (count == 0)
            {
                continue;
            }
            int sent = 0;
            for (int i = 0; i < 4; i++)
            {
                if (clients[i] == -1)
                    continue; // Consider only live connection's message sent status.
                if (msg_sent[i] == 1)
                    sent++; // Count how many messages were received.
            }               

            if (count == sent)  break;
        }

        memset(broadcastmsg, '\0', sizeof(broadcastmsg));
        printf("Enter thingamajig to send to clientz : "); fflush(0);
        fgets(broadcastmsg, sizeof(broadcastmsg), stdin);
        for(int i=0; i<1024; i++){if(broadcastmsg[i]=='\n') broadcastmsg[i]='\0';}
        for(int i=0; i<4; i++){
            msg_sent[i] = 0;                                                       
            if(clients[i] == -1) 
            {
                continue;     
            }                                    
            write(clients[i], broadcastmsg, 1024); 
        }
    }
}

void *clientfunction(void *args)
{
    client *tempstruct = malloc(sizeof(tempstruct));
    tempstruct = args;
    int new_sock = tempstruct->new_sock;
    int id = tempstruct->id;
    int port = tempstruct->port;
    struct in_addr addrs = tempstruct->addrs;
    char client_msg[1024];
    printf("Here!\n"); fflush(0);

    while (1)
    {   
        printf("here gain!\n");
        memset(client_msg, '\0', sizeof(client_msg));
        if (read(new_sock, client_msg, 1024) == -1)
        {
            perror("No message sent from client!\n");
            exit(0);
        }
        printf("C: %s \n", client_msg ); fflush(0);
        if (strcmp(client_msg, "exit") == 0)
        {
            printf("S:Closing connection for client %d \n", id);
            clients[id] = -1;
            close(new_sock);
            pthread_exit(NULL);
            break;
        }
        else
        {
            printf("S:Client %d prints %s \n", tempstruct->id, strrev(client_msg));
            msg_sent[tempstruct->id] = 1;
        }
    }
}

int main(int argc, char const *argv[])
{
    int port = atoi(argv[1]); //defining port
    //inputting port

    int server_fd, new_sock;
    struct sockaddr_in address;
    //this is address to bind socket to

    //creating socket
    //server_fd contains the file descriptor of the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Webserver not created successfully");
        return 1;
    }
    //socket created successfully

    //this variable stores the size of address
    int addrlen = sizeof(address);

    //Address struct gets info added to it
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //sin_family is always set to AF_INET, sin_port contains the port in network byte order.

    //Binding the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) != 0)
    {
        perror("No successful binding!"); //if it didnt bind successfully
        return 1;
    }

    //The listener for the socker listens for connections
    if (listen(server_fd, 1) != 0)
    {
        perror("Listener failure"); //in case of failure
        return 1;
    };

    for (int i = 0; i < 4; i++)
    {
        msg_sent[i] = 0;
        clients[i] = -1;
    }
    pthread_t listener;
    pthread_create(&listener, NULL, broadcast, (void *) NULL); // start broadcasting


    while (1)
    {   
        int addrlen = sizeof(address);
        new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen); //Incoming connections are accepted via this statement
        
        //In case of error
        if (new_sock < 0)
        {
            perror("Failed connections!");
            return 1;
        }
        //connection has been established at this stage
        printf("S:Connection has been made \n");
        int num_clients = 0;
        for (int i = 0; i < 4; i++)
        {
            if (clients[i] != -1)
                num_clients++;
        }
        printf("Here after num_clients!\n"); fflush(0); //THIS DOES NOT GET PRINTED IDK WHY
        if (num_clients >= 4)
        {   
            printf("S: rejected! \n");
            write(new_sock, "rejected!\0", 1024);
            close(new_sock);
            continue;
        }
        else
        {
            printf("S:We are accepting client: %d\n", num_clients + 1); fflush(0);
            write(new_sock, "Hi! You are accepted\0", 1024);
            for (int i = 0; i < 4; i++)
            {
                if (clients[i] == -1)
                {
                    client *tempstruct = malloc(sizeof(tempstruct));
                    tempstruct->id = i;
                    tempstruct->port = address.sin_port;
                    tempstruct->new_sock = new_sock;
                    tempstruct->addrs = address.sin_addr;
                    clients[i] = new_sock;
                    pthread_create(&threads[i], NULL, clientfunction, (void *)tempstruct);
                    break;
                }
            }
        }
    }

    return 0;
}
