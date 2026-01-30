#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>

/* Be prepared accept a response of this length */
#define BUFSIZE 16

#define USAGE                                                                       \
    "usage:\n"                                                                      \
    "  echoclient [options]\n"                                                      \
    "options:\n"                                                                    \
    "  -p                  Port (Default: 14757)\n"                                  \
    "  -s                  Server (Default: localhost)\n"                           \
    "  -m                  Message to send to server (Default: \"Hello Spring!!\")\n" \
    "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"message", required_argument, NULL, 'm'},
    {"port", required_argument, NULL, 'p'},
    {"server", required_argument, NULL, 's'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* Main ========================================================= */
int main(int argc, char **argv)
{
    int option_char = 0;
    unsigned short portno = 14757;
    char *hostname = "localhost";
    char *message = "Hello Spring!!";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:m:hx", gLongOptions, NULL)) != -1) {
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
        case 'm': // message
            message = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        }
    }

    setbuf(stdout, NULL); // disable buffering

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (NULL == message) {
        fprintf(stderr, "%s @ %d: invalid message\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == hostname) {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Socket Code Here */
    //client sends message to server, server sends that back to client
    //client prints exact message
    int client_connect = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));//struct is empty

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);      // Port 8080

    if (connect(client_connect, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(client_connect);
        exit(1);
    }


    if (send(client_connect, message, strlen(message), 0) < 0) {
        close(client_connect);
        exit(1);
    }

    //receive echo value
    char buffer[BUFSIZE];
    ssize_t n = recv(client_connect, buffer, BUFSIZE-1, 0);
    if (n < 0) {
        close(client_connect);
        exit(1);
    }

    buffer[n] = '\0';
    printf("%s",buffer);


    close(client_connect); //end connection once message received
    return 0;




}
