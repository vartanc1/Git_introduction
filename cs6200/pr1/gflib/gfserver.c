#include <stdlib.h>
#include "gfserver-student.h"

// Modify this file to implement the interface specified in
 // gfserver.h.
#define BUFSIZE 1024

struct gfserver_t{
    unsigned short portno; //portnumber
    int max_npending; //max number of pending requests
    void* arg; //used for handlerarg
    gfh_error_t (*handler_function)(gfcontext_t **, const char *, void*);
    char buf[BUFSIZE];
};
struct gfcontext_t{
    size_t file_len;
    int sockfd;
    char header[BUFSIZE];
    char path[BUFSIZE];
};

void gfs_abort(gfcontext_t **ctx){
    close((*ctx)->sockfd);
}

ssize_t gfs_send(gfcontext_t **ctx, const void *data, size_t len) {
    const char *buf = (const char *)data;
    size_t total = 0;

    while (total < len) {
        ssize_t n = send((*ctx)->sockfd, buf + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return total;
}

ssize_t gfs_sendheader(gfcontext_t **ctx, gfstatus_t status, size_t file_len){
    int n = 0;
    char header[BUFSIZE];

    if(status == GF_OK){
        n = snprintf(header, sizeof(header),"GETFILE OK %zu\r\n\r\n", file_len);
        
    }
    else if(status == GF_FILE_NOT_FOUND){
        n = snprintf(header, sizeof(header),"GETFILE FILE_NOT_FOUND\r\n\r\n");
    }
    else if(status == GF_INVALID){
        n = snprintf(header, sizeof(header),"GETFILE INVALID\r\n\r\n");
    }
    else{
        n = snprintf(header,sizeof(header),"GETFILE ERROR\r\n\r\n");
    }
    

    (*ctx)->file_len = file_len; //store file length in context
    return gfs_send(ctx,header,(size_t)n);
}

gfserver_t* gfserver_create(){
    //allocate server in memory
    gfserver_t *gfserver = malloc(sizeof(gfserver_t));
    if(gfserver == NULL){
        perror("Failed to allocate memory");
        return NULL;
    }
    bzero(gfserver,sizeof(gfserver_t)); //clear memory
    return gfserver;
}



void gfserver_serve(gfserver_t **gfs){
    int server_sock = -1, client_sock = -1;
    struct addrinfo hints, *servinfo = NULL, *p;
    int val = 1, status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    char portstr[6];
    snprintf(portstr, sizeof(portstr), "%hu", (*gfs)->portno);

    status = getaddrinfo(NULL, portstr, &hints, &servinfo);
    if (status != 0) return;

    for (p = servinfo; p != NULL; p = p->ai_next) {
        server_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_sock < 0) continue;

        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

        if (bind(server_sock, p->ai_addr, p->ai_addrlen) == 0) break;

        close(server_sock);
        server_sock = -1;
    }

    freeaddrinfo(servinfo);
    if (server_sock < 0) return;

    if (listen(server_sock, (*gfs)->max_npending) < 0) {
        close(server_sock);
        return;
    }

    while (1) {
        client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) continue;

        gfcontext_t *ctx = malloc(sizeof(*ctx));

        ctx->sockfd = client_sock;

        char buffer[BUFSIZE];
        ssize_t r = recv(client_sock, buffer, BUFSIZE - 1, 0);
        if (r <= 0) {
            gfs_abort(&ctx);
            free(ctx);
            continue;
        }
        buffer[r] = '\0';

        // parse "GETFILE GET /path"
        char *scheme = strtok(buffer, " ");
        char *method = strtok(NULL, " ");
        char *path   = strtok(NULL, "\r\n");

        if (!scheme || !method || !path ||
            strcmp(scheme, "GETFILE") != 0 ||
            strcmp(method, "GET") != 0 ||
            path[0] != '/') {
            gfs_sendheader(&ctx, GF_INVALID, 0);
            gfs_abort(&ctx);
            free(ctx);
            continue;
        }

        // call student handler (it should send header/body using gfs_* helpers)
        (*gfs)->handler_function(&ctx, path, (*gfs)->arg);

        gfs_abort(&ctx);
        free(ctx);
    }
}

void gfserver_set_port(gfserver_t **gfs, unsigned short port){
    (*gfs)->portno = port;
}

void gfserver_set_handlerarg(gfserver_t **gfs, void* arg){
    (*gfs)->arg = arg;
}

void gfserver_set_handler(gfserver_t **gfs, gfh_error_t (*handler)(gfcontext_t **, const char *, void*)){
    (*gfs)->handler_function = handler;
}

void gfserver_set_maxpending(gfserver_t **gfs, int max_npending){
    (*gfs)->max_npending = max_npending;
}

