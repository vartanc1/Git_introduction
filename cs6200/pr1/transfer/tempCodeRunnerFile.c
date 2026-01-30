#include <stdio.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <getopt.h>
#include <string.h>

#define BUFSIZE 512

#define USAGE                                                \
  "usage:\n"                                                 \
  "  transferclient [options]\n"                             \
  "options:\n"                                               \
  "  -p                  Port (Default: 61321)\n"            \
  "  -s                  Server (Default: localhost)\n"      \
  "  -h                  Show this help message\n"           \
  "  -o                  Output file (Default cs6200.txt)\n" 

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server", required_argument, NULL, 's'},
    {"output", required_argument, NULL, 'o'},
    {"help", no_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}};

/* Main ========================================================= */
int main(int argc, char **argv)
{
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 61321;
    char *filename = "cs6200.txt";

    setbuf(stdout, NULL);

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:o:hx", gLongOptions, NULL)) != -1) {
        switch (option_char) {
        case 's': // server
            hostname = optarg;
            break;
        case 'p': // listen-port
            portno = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        case 'o': // filename
            filename = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        }
    }

    if (NULL == hostname) {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == filename) {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    /* Socket Code Here */
    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd <0){
        exit(1); //error occured
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int e = connect(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e == -1){
        exit(1); //error while connecting
    }

    fp = fopen(filename,"r"); //read file
    if(fp == NULL){
        exit(1); //no file to be opened exit
    }

    char data[BUFSIZE]={0};

    while(fgets(data,sizeof(data),fp)!= NULL){
        if (send(sockfd, data, sizeof(data), 0) == -1) {
            fclose(fp);
            close(sockfd);
            return 1;
        }
    }

    fclose(fp);
    close(sockfd);


}
