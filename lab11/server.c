#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

// openSSL headers
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#define MAX_SIZE 10000
#define MAX_CLIENTS 2

char publicKeys[2][MAX_SIZE] = {
    "-----BEGIN PUBLIC KEY-----\nMFswDQYJKoZIhvcNAQEBBQADSgAwRwJAcwvS9PhVDrtf0FyIBaD7AMTzVViDZrO3\nOyXSG2C1cTIncfOX5D5iRN5Pqilou3qGfqpR7c0YxnWSTShXHmxHIQIDAQAB\n-----END PUBLIC KEY-----",
    "-----BEGIN PUBLIC KEY-----\nMFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBANqpXvisI34jBOGFu1cr9yop+zsNOfuE\n+9E+om2riZi6gDdDGJ5TR5vYeT8BawJ4YLlA10AfOT5QsrHEByddxnECAwEAAQ==\n-----END PUBLIC KEY-----"
};

// =============================================================================
// Taken from http://hayageek.com/rsa-encryption-decryption-openssl-c/
int padding = RSA_PKCS1_PADDING;
 
RSA * createRSA(unsigned char * key,int public)
{
    RSA *rsa= NULL;
    BIO *keybio ;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio==NULL)
    {
        printf( "> Failed to create key BIO\n");
        return 0;
    }
    if(public)
    {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    }
    if(rsa == NULL)
    {
        printf( "> Failed to create RSA\n");
    }
 
    return rsa;
}
 
int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    RSA * rsa = createRSA(key,1);
    int result = RSA_public_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}
// =============================================================================

// Structure to pass to a thread. Has all the information of a client
typedef struct {
    int new_sock;
    int id;
    int port;
    struct in_addr addrs;
} client_info_struct;

client_info_struct client_sockets[MAX_CLIENTS];
pthread_t threads[MAX_CLIENTS];   // Threads for various clients.
int endAll = 0;

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

void *serviceClient(void *args){
    int id = (int) args;                          // id signifies the client's index in global arrays.
    client_info_struct temp = client_sockets[id];            // Temporary struct to store input arguments.
    int new_sock = temp.new_sock;              // Client socket id.
    int port = temp.port;                      // port is the PORT number of the client.
    struct in_addr addrs = temp.addrs;         // addrs is the IP address of the client.

    printf("> Service thread started for client %d.\n", id);
    char string_id[MAX_SIZE] = {'\0'};
    sprintf(string_id, "%d", id);
    int retVal = write(new_sock, string_id, MAX_SIZE); writeError(retVal);                 // Let the client know it has been accepted.
    char message[MAX_SIZE];                     // Buffer to pass messages.
    char filename[MAX_SIZE];                    // <client_id>_<timestamp>.txt
    while(!endAll){
        resetString(message);                   // Reset the buffer.
        int retVal = read(new_sock, message, sizeof(message)); readError(retVal);   // Read client's message.
        printf("> Client %d sent message: %s\n", id, message);
        if(!strcmp(message, "EXIT")){                                               // Check if client's message is "exit"
            retVal = write(client_sockets[(id+1)%2].new_sock, message, MAX_SIZE); writeError(retVal);
            endAll = 1;                                                              // Set endAll to 1 to end all threads.
            break;
        }else{
            resetString(filename);                                                   // Reset the filename buffer.
            sprintf(filename, "%d_%ld.txt", id, time(NULL));                          // Create the filename.
            FILE *fp = fopen(filename, "w");                                          // Open the file.
            fprintf(fp, "%s", message);
            fclose(fp);                                                               // Close the file.

            // Encrypt the message
            unsigned char encrypted[MAX_SIZE]={};
            int encrypted_length = public_encrypt((unsigned char*)message, strlen(message), (unsigned char*)publicKeys[(id+1)%2], encrypted);
            if(encrypted_length == -1){
                printf("> Public Encrypt failed\n");
                exit(0);
            }
            printf("> Encrypted length = %d\n",encrypted_length);
            // Send the encrypted message to the client.
            retVal = write(client_sockets[(id+1)%2].new_sock, encrypted, MAX_SIZE); writeError(retVal);
            printf("> MESSAGE SENT TO [%s:%d]\n", inet_ntoa(client_sockets[(id+1)%2].addrs), client_sockets[(id+1)%2].port);

            resetString(message);                                                               // Reset the buffer.
        }
    }
    pthread_exit(NULL);
}

void createNewClient(struct sockaddr_in address, int new_sock, int id){
    printf("> New connection [%s:%d]\n", inet_ntoa(address.sin_addr), address.sin_port);

    client_info_struct args;
    args.id = id;
    args.new_sock = new_sock;
    args.port = address.sin_port;
    args.addrs = address.sin_addr;

    client_sockets[id] = args;                                                   // Update client_socket's value.
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
    address.sin_addr.s_addr = inet_addr("0.0.0.0");     // and port number so that 
    address.sin_port = htons(PORT);                     // it can bind to it.

    int retVal = bind(server_fd, (struct sockaddr *) &address, sizeof(address));    // Need to bind the server_fd to an address 
    if(retVal<0){                                                                   // and a port as mentioned in
        printf("> Cannot bind %d, socket probably in TIME_WAIT state, please try again later\n", retVal);    //Error checking.
        return 0;
    }

    printf("> Server bound to IP : %s and PORT : %d\n", inet_ntoa(address.sin_addr), PORT);
    
    listen(server_fd, 0);               // This is needed so that the socket waits for client to approach. The number 1 is the maximum queue size.  

    int addrlen = sizeof(address);                                                          // Had to initialize this variable in order to pass it as a pointer in a later function.
    
    // First Client
    new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen);       // It accepts the first connection request from the listening socket queue 
    if(new_sock == -1){ printf("> Cannot accept socket\n"); return 0;}                      // Error checking.
    createNewClient(address, new_sock, 0);                                                    // Create a new client.

    // Second Client
    new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen);       // It accepts the first connection request from the listening socket queue 
    if(new_sock == -1){ printf("> Cannot accept socket\n"); return 0;}                      // Error checking.
    createNewClient(address, new_sock, 1);                                                    // Create a new client.
    
    // Create threads for each client.
    pthread_create(&threads[0], NULL, serviceClient, (void *) 0);                // Create the thread.   
    pthread_create(&threads[1], NULL, serviceClient, (void *) 1);                // Create the thread.

    // Wait for threads to finish.
    while(!endAll);                                                                            // Wait for all threads to finish.
    
    // Close all sockets.
    printf("\n> Closing connections for all clients...\n");
    close(client_sockets[0].new_sock);
    close(client_sockets[1].new_sock);

    printf("\n> Closing server...\n");
    shutdown(server_fd, 0);                           // Close server's file descriptor.
    return 0;
}
