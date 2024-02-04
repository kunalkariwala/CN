#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#define MAX_SIZE 1024
#define MAX_CLIENTS 10
int client_sockets[MAX_CLIENTS];  // -1 = available
int client_messages[MAX_CLIENTS]; // 1 if sent, 0 if not sent
pthread_t threads[MAX_CLIENTS];   // Threads for various clients.

int screen_sock;

// Structure to pass to a thread. Has all the information of a client
typedef struct {
    int new_sock;
    int id;
    int port;
    struct in_addr addrs;
} client_info_struct;

void readError(int retVal){
    if(retVal < 0){
        printf("> Error in reading message, return value = %d\n", retVal);     // Helper function to check errors.
        exit(0);
    }
}

void writeError(int retVal){
    if(retVal < 0){
        printf("> Error in writing message, return value = %d\n", retVal);     // Helper function to check errors.
        exit(0);
    }
}

void resetString(char *s){
    for(int i=0; i<MAX_SIZE; i++) s[i] = '\0';                                 // Reset the string to null terminators.
}

void sendToAll(char *message){
    for(int i=0; i<MAX_CLIENTS; i++){
        if(client_sockets[i] == -1) continue;
        int retVal = write(client_sockets[i], message, MAX_SIZE); readError(retVal); // Write to client screen.
    }
}

void *serviceClient(void *args){
    client_info_struct *temp = args;            // Temporary struct to store input arguments.
    int new_sock = temp->new_sock;              // Client socket id.
    int id = temp->id;                          // id signifies the client's index in global arrays.
    int port = temp->port;                      // port is the PORT number of the client.
    struct in_addr addrs = temp->addrs;         // addrs is the IP address of the client.
    free(temp);

    char message[MAX_SIZE];                     // Buffer to pass messages.

    char user[MAX_SIZE]; resetString(user);
    int retVal = read(new_sock, user, sizeof(user)); readError(retVal);   // Read client's message.
    printf("[%s:%d] = %s\n", inet_ntoa(addrs), port, user);
    while(1){
        resetString(message);                   // Reset the buffer.
        int retVal = read(new_sock, message, sizeof(message)); readError(retVal);   // Read client's message.

        if(!strcmp(message, "exit")){                                               // Check if client's message is "exit"
            break;
        }else{
            char print_this[3*MAX_SIZE];
            sprintf(print_this, "> [%s:%d] %s : %s\n", inet_ntoa(addrs), port, user, message); // Print client's detials and message in reverse.
            printf("%s", print_this);
            sendToAll(print_this);
            client_messages[id] = 1;                                                            // Set variable to 1, as the client sent their message.
            resetString(message);                                                               // Reset the buffer.
        }
    }

    printf("\n> Closing connection for client [%s:%d]...\n", inet_ntoa(addrs), port);
    close(new_sock);
    client_sockets[id] = -1;

    pthread_exit(NULL);
}

int main(int argc, char *argv[]){

    int server_fd, new_sock;                                                    // Socket file descriptor for server.
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented.
    if(server_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking.

    int PORT;
    char message[MAX_SIZE];
    PORT = atoi(argv[1]);

    address.sin_family = AF_INET;                       // Need to specifiy address
    address.sin_addr.s_addr = inet_addr("10.20.9.99");  // and port number so that 
    address.sin_port = htons(PORT);                     // it can bind to it.

    int retVal = bind(server_fd, (struct sockaddr *) &address, sizeof(address));    // Need to bind the server_fd to an address 
    if(retVal<0){                                                                   // and a port as mentioned in
        printf("> Cannot bind %d, socket probably in TIME_WAIT state, please try again later\n", retVal);    //Error checking.
        return 0;
    }

    printf("> Server bound to IP : %s and PORT : %d\n", inet_ntoa(address.sin_addr), PORT);
    
    listen(server_fd, 0);               // This is needed so that the socket waits for client to approach. The number 1 is the maximum queue size.  

    for(int i=0; i<MAX_CLIENTS; i++){
        client_sockets[i] = -1;         // Initialize all sockets to available.
        client_messages[i] = 0;         // None of the clients if any, sent messages initially.
    }

    
    int addrlen = sizeof(address);                                                          // Had to initialize this variable in order to pass it as a pointer in a later function.
    screen_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen);
    if(screen_sock == -1){ printf("> Cannot accept screen socket\n"); return 0;}                // Error checking.

    while(1){
        new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen);       // It accepts the first connection request from the listening socket queue 
        if(new_sock == -1){ printf("> Cannot accept socket\n"); return 0;}                      // Error checking.

        int count=0;                                    // Count variabe to keep track of live clients.
        for(int i=0; i<MAX_CLIENTS; i++){
            if(client_sockets[i]!=-1) count++;          // If client_socket has a valid number, then it is live.
        }

        if(count==MAX_CLIENTS){                                                                 // If count is 4, then maximum clients reached
            int retVal = write(new_sock, "REJECT\0", MAX_SIZE); writeError(retVal);             // Return to client saying it was rejected.
            close(new_sock);                                                                    // Close the connection.
            continue;                                                                           // Continue listening.
        }
        
        printf("> New connection [%s:%d]\n", inet_ntoa(address.sin_addr), address.sin_port);    // If count is less than 4, then show that there is a new connectoin.
        int retVal = write(new_sock, "ACCEPT\0", MAX_SIZE); writeError(retVal);                 // Let the client know it has been accepted.
  
        for(int i=0; i<MAX_CLIENTS; i++){
            if(client_sockets[i] == -1){                                                        // Find an empty slot for the new client.
                client_info_struct *args = malloc(sizeof *args);                                // Create struct to pass on parameters to thread.
                args->id = i;
                args->new_sock = new_sock;
                args->port = address.sin_port;
                args->addrs = address.sin_addr;

                client_sockets[i] = new_sock;                                                   // Update client_socket's value.
                pthread_create(&threads[i], NULL, serviceClient, (void *) args);                // Create the thread.
                break;
            }
        }
    }

    printf("\n> Closing server...\n");
    shutdown(server_fd, 0);                           // Close server's file descriptor.
    return 0;
}
