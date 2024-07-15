#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define LISTENADDR "127.0.0.1"
/* structures*/
struct sHttpRequest{
    char method[8];
    char url[128];
};
typedef struct sHttpRequest httpreq;

/* global */
char *error;

/* Returns 0 if error else returns a socket fd*/
int srv_init(int portno){
    int s;
    struct sockaddr_in srv;
    s = socket(AF_INET, SOCK_STREAM, 0 );
    if (s < 0){
        error = "socket() error";
        return 0;
    }
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);
    srv.sin_port = htons(portno);

    if(bind(s, (struct sockaddr *)&srv, sizeof(srv))){
        error = "bind() error";
        close(s);
        return 0;
    }
    // listen for connections on sfd with waiting queu of 5
    if(listen(s, 5)){
        close(s);
        error = "listen() error";
        return 0;
    }
    return s;
}

/* return 0 on error or returns the new client's socket*/
int cli_accept(int s){
    int c;
    socklen_t addrlen;
    struct sockaddr_in cli;
    addrlen = 0;
    memset(&cli, 0, sizeof(cli));
    c = accept(s, (struct sockaddr *)&cli, &addrlen);
    if(c < 0){
        error = "accpet() error";
        return 0;
    }
    return c;
}

void cli_conn(int s, int c){
    return;
}

/* returns 0 on error or returns http request*/
httpreq *parse_http(char *str){
    httpreq *req;
    char *p;

    req = calloc(1, sizeof(httpreq));
    for(p=str; p && *p != ' '; p++);
    if(*p == ' ')
         *p = 0;
    else{
        error = "parse_http() NOSPACE error";
        free(req);
        return 0;
    }
    strncpy(req->method, str, 7);
    for(str = ++p; p && *p != ' '; p++);
    if(*p == ' ')
         *p = 0;
    else{
        error = "parse_http() 2ndSPACE error";
        free(req);
        return 0;
    }
    strncpy(req->url, str, 127);
    return req;
}


int main(int argc, char const *argv[])
{
    int s, c;
    char *port;
    char *template;
    httpreq *req;
    char buff[512];
    template = "GET /sdfsdfd HTTP/1.1\n"
    "host: fahfjkh.net:8184\n"
    "jafjafh";
    memset(buff, 0, 512);
    strncpy(buff, template, 511)
    req = parse_http(template);
    printf("Method: '%s' \n URL: '%s'\n", req->method, req->url);
    free(req);
    return 0;

    if(argc  < 2){
        fprintf(stderr, "Usage: %s <listening port> \n", argv[0]);
        return -1;
    }
    else{
        port = argv[1];
        s = srv_init(atoi(port));
        if(!s){
            fprintf(stderr, "%s\n", error);
            return -1;
        }
        printf("Listeneing on %s:%s\n", LISTENADDR, port);
        while(1){
            c = cli_accept(s);
            if(!c){
                fprintf(stderr, "%s\n", error);
                continue;
            }
            printf("Incoming connection \n");
            if(!fork()){
                cli_conn(s, c);
            }
        }
    }
    return -1;
}
