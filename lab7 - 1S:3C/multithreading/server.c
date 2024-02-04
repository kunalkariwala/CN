#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <strings.h>

#define MAX_SIZE 1024

#define MAX_CLIENTS 4
int client_sockets[MAX_CLIENTS]; // -1 = available
pthread_t threads[MAX_CLIENTS];

typedef struct {
    int new_sock;
    int id;
} client_info_struct;

void readError(int retVal){
    if(retVal < 0){
        printf("> Error in reading message, return value = %d\n", retVal);
        exit(0);
    }
}

void writeError(int retVal){
    if(retVal < 0){
        printf("> Error in writing message, return value = %d\n", retVal);
        exit(0);
    }
}

void resetString(char *s){
    for(int i=0; i<MAX_SIZE; i++) s[i] = '\0';
}

int SMAC_flag = 0;
void send_message_to_all_clients(char *message){
    for(int i=0; i<MAX_CLIENTS; i++){
        if(client_sockets[i] == -1) continue;
        int retVal = write(client_sockets[i], message, MAX_SIZE); writeError(retVal);
    }
}

void *serviceClient(void *args){
    client_info_struct *temp = args;
    int new_sock = temp->new_sock;
    int id = temp->id;
    free(temp);

    char message[MAX_SIZE];
    while(1){
        resetString(message);
        int retVal = read(new_sock, message, sizeof(message)); readError(retVal);

        if(!strcmp(message, "exit\n")){
            break;
        }else{
            while(SMAC_flag);
            printf("\n> Client %d message = %s\n", id, message);
            resetString(message);
            SMAC_flag = 1;
            printf("> Enter message to send to all clients : ");
            fgets(message, sizeof(message), stdin);
            send_message_to_all_clients(message);        
            SMAC_flag = 0;    
        }
    }

    printf("\n> Closing connection for client %d...\n", id);
    close(new_sock);
    client_sockets[id] = -1;

    pthread_exit(NULL);
}

int main(){

    int server_fd, new_sock;                                                    // Socket file descriptor for server.
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented.
    if(server_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking.

    int PORT;
    printf("> Enter port number : "); fflush(0);
    char message[MAX_SIZE];
    fgets(message, sizeof(message), stdin);
    PORT = atoi(message);

    address.sin_family = AF_INET;                       // Need to specifiy address
    address.sin_addr.s_addr = inet_addr("0.0.0.0");     // and port number so that 
    address.sin_port = htons(PORT);                     // it can bind to it.

    int retVal = bind(server_fd, (struct sockaddr *) &address, sizeof(address));    // Need to bind the server_fd to an address 
    if(retVal<0){                                                                   // and a port as mentioned in
        printf("> Cannot bind %d, socket probably in TIME_WAIT state, please try again later\n", retVal);    //Error checking.
        return 0;
    }

    printf("> Server bound to IP : %s and PORT : %d\n", inet_ntoa(address.sin_addr), PORT);
    
    listen(server_fd, 3);               // This is needed so that the socket waits for client to approach. The number 1 is the maximum queue size.  

    for(int i=0; i<MAX_CLIENTS; i++) client_sockets[i] = -1;
    while(1){
        
        while(1){
            int count=0;
            for(int i=0; i<MAX_CLIENTS; i++){
                if(client_sockets[i]!=-1) count++;
            }
            if(count<4) break;
        }

        int addrlen = sizeof(address);                                                    // Had to initialize this variable in order to pass it as a pointer in a later function.
        new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen); // It accepts the first connection request from the listening socket queue 
        printf("> New connection\n");                                                     // and creates a new connected socket and returns a file descriptor.
        if(new_sock == -1){ printf("> Cannot accept socket\n"); return 0;}                // Error checking.
    
        int acceptClient = 0;
        for(int i=0; i<MAX_CLIENTS; i++){
            if(client_sockets[i] == -1){
                client_info_struct *args = malloc(sizeof *args);
                args->id = i;
                args->new_sock = new_sock;
                client_sockets[i] = new_sock;
                pthread_create(&threads[i], NULL, serviceClient, (void *) args);
                acceptClient = 1;
                break;
            }
        }
    }

    printf("\n> Closing server...\n");
    shutdown(server_fd, 0);                           // Close server's file descriptor.
    return 0;
}
