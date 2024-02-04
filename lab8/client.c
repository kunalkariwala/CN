#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <string.h>

# define MAX_SIZE 10000

int sock, bytes_received;
SSL *conn;
char hostname[MAX_SIZE], path[MAX_SIZE], filename[MAX_SIZE], message[4*MAX_SIZE];

void resetString(char *s){
    for(int i=0; i<sizeof(s); i++) s[i] = '\0';                                 // Reset the string to null terminators.
}

void processURL(char *url, int *isHTTPS){
    int hostname_start_index = 0;
    for(int i=0; i<strlen(url); i++){
        if(url[i] == 's') *isHTTPS = 1;
        if(url[i] == '/' && url[i+1] == '/'){
            hostname_start_index = i+2;
            break;
        }
    }

    int path_start_index = 0;
    for(int i=0; i<strlen(url); i++){
        if(url[i+hostname_start_index] == '/'){
            path_start_index = i+hostname_start_index+1;
            break;
        }
        hostname[i] = url[i+hostname_start_index];
    }

    int filename_start_index = strlen(url);
    for(int i=0; i+path_start_index<strlen(url); i++){
        path[i] = url[i+path_start_index];
        if(path[i] == '/') filename_start_index = i+1;
    }


    for(int i=0; path[i+filename_start_index]!='\0'; i++){
        filename[i] = path[i+filename_start_index];
    }

    printf("* Host Name = %s\n", hostname);
    printf("* Path = %s\n", path);
    printf("* File Name = %s\n", filename);
    printf("* HTTPS ? = %d\n", *isHTTPS);
}

int ReadHttpStatus(int isHTTPS){
    char c;
    char buff[1024]="",*ptr=buff+1;
    int bytes_received, status;
    printf("\n\n* Begin Header ..\n");
    while(1){
        if(isHTTPS){
            bytes_received = SSL_read(conn, ptr, 1);
        }else{
            bytes_received = read(sock, ptr, 1);
        }
        if(bytes_received == 0) break;
        if(bytes_received==-1){
            perror("ReadHttpStatus");
            exit(1);
        }

        if((ptr[-1]=='\r')  && (*ptr=='\n' )) break;
        ptr++;
    }
    *ptr=0;
    ptr=buff+1;

    sscanf(ptr,"%*s %d ", &status);

    printf("< %s\n",ptr);
    return (bytes_received>0)?status:0;
}

int ParseHeader(int isHTTPS){
    char c;
    char buff[2048],*ptr=buff+4;
    resetString(buff);
    while(1){
        if(isHTTPS){
            bytes_received = SSL_read(conn, ptr, 1);
        }else{
            bytes_received = read(sock, ptr, 1);
        }
        if(bytes_received == 0) break;
        if(bytes_received==-1){
            perror("Parse Header");
            exit(1);
        }

        if(
            (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
            (ptr[-1]=='\r')  && (*ptr=='\n' )
        ) break;
        ptr++;
    }
    printf("< ");
    for(int i=4; buff[i]!='\0'; i++){
        printf("%c", buff[i]);
        if(buff[i] == '\n'){
            printf("< ");
            if(buff[i-2] == '\n') break;
        }
    }
    printf("\n* End Header ..\n\n");

    if(bytes_received){
        ptr=strstr(buff+4,"Content-Length:");
        if(ptr){
            sscanf(ptr,"%s %d", message, &bytes_received);
            printf("Content-Length: %d\n",bytes_received);
        }else{
            bytes_received = -1; //unknown size
            printf("Content-Length is unknown: -1\n\n");
        }
    }
    return bytes_received;
}

void getResponse(int isHTTPS){
    resetString(message);
    if(ReadHttpStatus(isHTTPS)){
        int content_length = ParseHeader(isHTTPS);
        int bytes=0;
        FILE* fd=fopen(filename,"wb");
        if(fd == NULL){
        printf("\n* ERROR IN OPENING/CREATING FILE : %s\n", filename);
        printf("* CHANGING FILENAME TO \"%s\"\n", hostname);
        fd = fopen(hostname, "wb");
        }	

        while(1){
            if(isHTTPS){
                bytes_received = SSL_read(conn, message, sizeof(message));
            }else{
                bytes_received = read(sock, message, 1024);
            }
            if(bytes_received == 0) break;
            if(bytes_received==-1){
                perror("receive");
                exit(3);
            }
            fwrite(message,1,bytes_received,fd);
            // printf("%s", message);
            bytes+=bytes_received;
            printf("* Bytes received: %d/%d\n",bytes, content_length);
            if(content_length != -1 && bytes == content_length) break;
        }
        fclose(fd);
    }
}

void sendRequest(int isHTTPS){
    resetString(message);
    sprintf(message, "GET /%s HTTP/1.1\nHost: %s\nAccept: */*\n\n", path, hostname);

    if(isHTTPS){
        if(SSL_write(conn, message, strlen(message))==-1){
            perror("send");
            exit(2); 
        }
    }else{
        if(write(sock, message, strlen(message))==-1){
            perror("send");
            exit(2); 
        }
    }

    printf("* Sending request...\n");
    printf("> ");
    for(int i=0; message[i]!='\0'; i++){
        printf("%c", message[i]);
        if(message[i] == '\n') printf("> ");
    }  
}

void doHTTPS(){
    SSL_load_error_strings ();
    SSL_library_init ();
    SSL_CTX *ssl_ctx = SSL_CTX_new (SSLv23_client_method ());
    conn = SSL_new(ssl_ctx);
    SSL_set_fd(conn, sock);
    int err = SSL_connect(conn);
    if (err != 1){
        printf("* Error in SSL creation\n");
        exit(0);
    }

    sendRequest(1);
    getResponse(1);
}

void doHTTP(){
    sendRequest(0);
    getResponse(0);
}

int main(int argc, char *argv[]){

    struct sockaddr_in server_addr;
    struct hostent *he;

    char *url = argv[1];
    int  isHTTPS = 0;
    
    resetString(hostname); resetString(path); resetString(filename);
    processURL(url, &isHTTPS);

    he = gethostbyname(hostname);
    if (he == NULL){
       herror("* gethostbyname");
       exit(1);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1){
       perror("* Socket");
       exit(1);
    }
    
    int PORT;
    if(isHTTPS){
        PORT = 443;
    }else{
        PORT = 80;
    }

    server_addr.sin_family = AF_INET;     
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(server_addr.sin_zero),8); 

    printf("* Connecting ...\n");
    if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
       perror("Connect");
       exit(1); 
    }

    printf("* Connected to %s:%d\n", inet_ntoa(server_addr.sin_addr), PORT);

    if(isHTTPS){
        doHTTPS();
    }else{
        doHTTP();
    }
    printf("\n\n* Done.\n\n");
    
    close(sock);
    return 0;
}
