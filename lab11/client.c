#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>

// openSSL headers
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
 
#define MAX_SIZE 10000
fd_set s_rd, s_wr, s_ex;

char *privateKey = 0;
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
        printf( "Failed to create key BIO");
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
        printf( "Failed to create RSA");
    }
 
    return rsa;
}

int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
    RSA * rsa = createRSA(key,0);
    int  result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}
// =============================================================================

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
    for(int i=0; i<MAX_SIZE; i++) s[i] = '\0';                                       // Reset the string to null terminators.
}



int end = 0;
void *listenToServer(void *args){
    int client_fd = (int) args;
    char message[MAX_SIZE];
    while(1){
        resetString(message);
        int retVal = read(client_fd, message, sizeof(message)); readError(retVal);
        if(!strcmp(message, "EXIT")) break;
        if(strlen(message) == 0) continue;
        unsigned char decrypted[MAX_SIZE]={};
        int decrypted_length = private_decrypt(message,64,privateKey, decrypted);
        if(decrypted_length == -1)
        {
            printf("> Encrypted message size = %d\n", strlen(message));
            printf("> Encrypted message = %s\n", message);
            printf("> Private Decrypt failed\n");
            exit(0);
        }

        printf("\33[2K\r");
        printf("%s\n> Enter string : ", decrypted); fflush(0);
    }
    end = 1;
    pthread_exit(NULL);
}

void *tellToServer(void *args){
    int client_fd = (int) args;
    char message[MAX_SIZE];
    while(!end){
        printf("> Enter string : "); fflush(0); resetString(message);
        select(fileno(stdin)+1, &s_rd, &s_wr, &s_ex, NULL);
        fgets(message, sizeof(message), stdin);                                               // Put string in buffer.
        printf("\033[A");
        printf("\33[2K\r");
        for(int i=0; i<MAX_SIZE; i++){if(message[i]=='\n') message[i]='\0';}                  // Remove newline from string.
        int retVal = write(client_fd, message, strlen(message)); writeError(retVal);          // Send string to server.
        if(!strcmp(message, "EXIT")) break;
    }
    end = 1;
}

int main(int argc, char *argv[]){

    FD_ZERO(&s_rd);
    FD_ZERO(&s_wr);
    FD_ZERO(&s_ex);
    FD_SET(fileno(stdin), &s_rd);

    int client_fd;                                                              // Socket file descriptor for client
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented
    if(client_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking

    int PORT; 
    char message[MAX_SIZE]; resetString(message);     // Buffer used for communication.
    PORT = atoi(argv[1]);
    address.sin_family = AF_INET;                        // Need to specifiy address
    address.sin_addr.s_addr = inet_addr(argv[2]);        // and port number so that 
    address.sin_port = htons(PORT);                      // it can bind to it.

    resetString(message);                                                       // Reset the buffer.
    
    int retVal = connect(client_fd, (struct sockaddr*)&address, sizeof(address));    // need to bind the client_fd to an address 
    if(retVal<0){                                                                    // and a port as mentioned in
        printf("> Failed connecting to the server\n");                               // struct (sockaddr_in) (4444 in this case).
        return 0;                                                                    // Error checking.
    }

    retVal = read(client_fd, message, MAX_SIZE); readError(retVal);            // Read server response that says if client is accepted or rejected.
    printf("> Connected to server IP = %s and PORT = %d\n", inet_ntoa(address.sin_addr), PORT);


    // Loading the private key
    int id = atoi(message);                                                          // Convert string to integer.
    char privateKeyFile[MAX_SIZE];
    sprintf(privateKeyFile, "privateKey_client%d.txt", id+1);
    FILE *fp = fopen(privateKeyFile, "r");
    if(fp == NULL){
        printf("> Error in opening private key file\n");
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    privateKey = malloc(len);
    fread (privateKey, 1, len, fp);
    fclose(fp);
    privateKey[strlen(privateKey)-1] = '\0';

    // Deploying threads
    pthread_t responseThread;
    pthread_create(&responseThread, NULL, listenToServer, (void *) client_fd);

    pthread_t tellThread;
    pthread_create(&tellThread, NULL, tellToServer, (void *) client_fd);

    // Waiting for threads to finish
    while(!end);
    printf("\n> Closing client...\n");
    close(client_fd);           // Close client's file descriptor.
}
