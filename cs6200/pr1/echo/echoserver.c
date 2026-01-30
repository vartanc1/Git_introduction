#include <unistd.h>
#include <sys/socket.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#define BUFSIZE 1024

#define USAGE                                                        \
    "usage:\n"                                                         \
    "  echoserver [options]\n"                                         \
    "options:\n"                                                       \
    "  -p                  Port (Default: 14757)\n"                    \
    "  -m                  Maximum pending connections (default: 5)\n" \
    "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"maxnpending",   required_argument,      NULL,           'm'},
    {"port",          required_argument,      NULL,           'p'},
    {"help",          no_argument,            NULL,           'h'},
    {NULL,            0,                      NULL,             0}
};


int main(int argc, char **argv) {
    int option_char;
    int portno = 8080; 
    int maxnpending = 5;
  
    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "p:m:hx", gLongOptions, NULL)) != -1) {
        switch (option_char) {
        case 'm': // server
            maxnpending = atoi(optarg);
            break; 
        case 'h': // help
            fprintf(stdout, "%s ", USAGE);
            exit(0);
            break;
        case 'p': // listen-port
            portno = atoi(optarg);
            break;                                        
        default:
            fprintf(stderr, "%s ", USAGE);
            exit(1);
        }
    }

    setbuf(stdout, NULL); // disable buffering

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }
    if (maxnpending < 1) {
        fprintf(stderr, "%s @ %d: invalid pending count (%d)\n", __FILE__, __LINE__, maxnpending);
        exit(1);
    }


  /* Socket Code Here */
  int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {exit(1); }
    int val = 1;

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(int));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));//clear data

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);             
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, maxnpending) < 0) {      
        perror("listen");
        exit(1);
    }


    /* client connection */

    for(;;){//infinite loop
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int new_socket = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
    if (new_socket < 0) {exit(1); }


    char buffer[16];
    int recv_msg_len = recv(new_socket, buffer, sizeof(buffer) - 1, 0); //recieve clients data

    if (recv_msg_len < 0) {close(new_socket); exit(1); }

    if (recv_msg_len == 0) {close(new_socket); close(sockfd); return 0; }//no message passed

    //send data back to client
    if (send(new_socket, buffer, recv_msg_len, 0) < 0) {
        exit(1);
    }    
    }
    return 0;
}