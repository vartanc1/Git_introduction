#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
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
#include <fcntl.h> // file control options 

#define BUFSIZE 512


void send_file(int sockfd,int file){
    int f = file;
    if(f < 0){
        return;
    }
    char buffer[BUFSIZE];
    int n;

    while((n = read(f,buffer, BUFSIZE)) >0){
        int sent = 0;
        while(sent < n){
            int s = send(sockfd, buffer+sent, (size_t)(n-sent),0);
            if(s <0){
                perror("send");
                close(f);
                return;
            }
            sent +=s;
            }

    }
    if(n < 0){perror("read");}
    close(f);
 
}
#define USAGE                                                \
    "usage:\n"                                               \
    "  transferserver [options]\n"                           \
    "options:\n"                                             \
    "  -f                  Filename (Default: 6200.txt)\n"   \
    "  -p                  Port (Default: 61321)\n"          \
    "  -h                  Show this help message\n"         \

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"port", required_argument, NULL, 'p'},
    {"filename", required_argument, NULL, 'f'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

int main(int argc, char **argv)
{
    int option_char;
    int portno = 61321;             /* port to listen on */
    char *filename = "6200.txt"; /* file to transfer */

    setbuf(stdout, NULL); // disable buffering

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "p:hf:x", gLongOptions, NULL)) != -1) {
        switch (option_char) {
        case 'p': // listen-port
            portno = atoi(optarg);
            break;
        case 'f': // file to transfer
            filename = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        }
    }


    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }
    
    if (NULL == filename) {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Socket Code Here */
    int sockfd;
    int file_to_send;
    struct sockaddr_in server_addr, client_address; //server and client sockets
    socklen_t addr_size; //size of socket address
    addr_size = sizeof(client_address);
    int server_size = sizeof(server_addr);

    int val =1;

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd <0){
        printf("error1");
        exit(1); //socket error occured
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

    int e = bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));


    getsockname(sockfd, (struct sockaddr *) &server_addr,(socklen_t *)&server_size);

    if(e <0){
        printf("error");
        exit(1); //binding error
    }
    e = listen(sockfd,10);
    if(e <0){
        printf("error5");
        exit(1); //error in listening
    }

    for(;;){//infinite loop to always accept clients


        
        int new_sock = accept(sockfd, (struct sockaddr*)&client_address,&addr_size);
        if(new_sock < 0){
            exit(1);
            perror("New sock");
        }

       
        file_to_send = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if(file_to_send < 0){
            exit(1);//failed to open file
            perror("New open");
        }
        send_file(new_sock,file_to_send);//write back file to client
        sleep(1);
        
        printf("Data written in the file successfully.\n");
        close(new_sock);
    }
    return 0;
}
