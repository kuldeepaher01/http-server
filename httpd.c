#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#define LISTENADDR "127.0.0.1"
/* structures*/
struct sHttpRequest
{
    char method[8];
    char url[128];
};
typedef struct sHttpRequest httpreq;

struct sFile
{
    char filename[64];
    char *fc;
    int size;
};
typedef struct sFile File;


/* global */
char *error;

/* Returns 0 if error else returns a socket fd*/
int srv_init(int portno)
{
    int s;
    struct sockaddr_in srv;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        error = "socket() error";
        return 0;
    }
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);
    srv.sin_port = htons(portno);

    if (bind(s, (struct sockaddr *)&srv, sizeof(srv)))
    {
        error = "bind() error";
        close(s);
        return 0;
    }
    // listen for connections on sfd with waiting queu of 5
    if (listen(s, 5))
    {
        close(s);
        error = "listen() error";
        return 0;
    }
    return s;
}

/* return 0 on error or returns the new client's socket*/
int cli_accept(int s)
{
    int c;
    socklen_t addrlen;
    struct sockaddr_in cli;
    addrlen = 0;
    memset(&cli, 0, sizeof(cli));
    c = accept(s, (struct sockaddr *)&cli, &addrlen);
    if (c < 0)
    {
        error = "accpet() error";
        return 0;
    }
    return c;
}

/* returns 0 on error or returns http request*/
httpreq *parse_http(char *str)
{
    httpreq *req;
    char *p;
    char *res;

    req = calloc(1, sizeof(httpreq));
    for (p = str; p && *p != ' '; p++)
        ;
    if (*p == ' ')
        *p = 0;
    else
    {
        error = "parse_http() NOSPACE error";
        free(req);
        return 0;
    }
    strncpy(req->method, str, 7);
    for (str = ++p; p && *p != ' '; p++)
        ;
    if (*p == ' ')
        *p = 0;
    else
    {
        error = "parse_http() 2ndSPACE error";
        free(req);
        return 0;
    }
    strncpy(req->url, str, 127);
    return req;
}
/* return 0 on erro or return the data*/
char *cli_read(int c)
{
    static char buff[512];
    memset(buff, 0, 511);
    if (read(c, buff, 511) < 0)
    {
        error = "read() error";
        return 0;
    }
    else
        return buff;
}
void http_response(int c,char *conttype, char *data)
{
    char buff[512];
    int n;
    memset(buff, 0, 511);
    snprintf(buff, 511, "Content-Type: %s\r\nContent-Length: %d\r\n\r\n%s", conttype, strlen(data), data);
    n = strlen(buff);
    write(c, buff, n);
    
    return;
}
/* return 0 on error or return the data*/
void http_headers(int c, int status)
{
    char buff[512];
    int n;
    memset(buff, 0, 511);
    snprintf(buff, 511, "HTTP/1.1 %d OK\r\nServer: httpd\r\nCache-Control: no-cache\r\nContent-Type: text/html\r\nContent-Language: en\r\nExpires: -1\r\nX-Frame-Options: SAMEORIGIN\r\n", status);
    n = strlen(buff);
    write(c, buff, n);
    return;
}
File *readfile(char *filename)
{
    char buff[512];
    char *p;
    int n, x, fd;
    fd = open(filename, O_RDONLY);
    if(fd < 0){
        return 0;
    }
    File *file = calloc(1, sizeof(File));
    if(!file){
        close(fd);
        return 0;
    }
    strncpy(file->filename, filename, 63);
    file->fc = calloc(1, 512);
    x = 0; // read bytes
    while (1)
    {
        memset(buff, 0, 511);
        n = read(fd, buff, 511);
        if (!n)
        {
            break;
        }
        else if(x==-1){
            close(fd);
            free(file->fc);
            free(file);
            return 0;
        }
        strncpy(buff, file->fc+x, n);
        x += n;
        file->fc = realloc(file->fc, (512+x));
    }
    file->size = x;
    close(fd);
    return file;
}
/* return 1 for ok and 0 for error*/

int sendfile(int c, char *contype, File *file)
{
    char buff[512];
    int n, x;
    char *p;
    if(!file){
        return 0;
    }
    else{
        printf("file size: %d\n", file->size);
        memset(buff, 0, 512);
        snprintf(buff, 511,
         "Content-Type: %s\n"
         "Content-Length: %d\n\n",
          contype, file->size);
          n = strlen(buff);
            write(c, buff, n);
        n = file->size;
        p = file->fc;
        while (1)
        {
            printf(".");
            x = write(c, p,(n<512)?n:512);
            if(x < 1){
                return 0;
            }
            n-=x;
            if(n<1){
                break;
            }
            else{
                p+=x;
            }
            return 1;
        }
        

    }
}


void cli_conn(int s, int c)
{
    httpreq *req;
    char *p;
    char *res;
    char str[96];

    p = cli_read(c);
    if (!p)
    {
        fprintf(stderr, "%s\n", error);
        close(c);
        return;
    }
    req = parse_http(p);
    if (!req)
    {
        fprintf(stderr, "%s\n", error);
        close(c);
        return;
    }
    printf("METHOD: '%s'\nURL: '%s'\n", req->method, req->url);
    if(!strcmp(req->method, "GET") && !strncmp(req->url, "/img/", 5)){
        memset(str, 0, 95);
        snprintf(str, 95, ".%s", req->url);
        printf("Opening file: %s\n", str);
        File *f = readfile(str);
        if(!f){
            printf("cannot open file\n");
            res = "file not found";
            http_headers(c, 404);
            http_response(c, "text/plain", res);
            free(req);
            close(c);
            return;
        }
        else{
            http_headers(c, 200);
            printf("Sending file\n");
            if(!sendfile(c, "image/png", f)){
                printf("Error sending file\n");
                res = "http server error";
                http_response(c, "text/plain", res);
                free(req);
            }
        }


    }
    if (!strcmp(req->method, "GET") && !strcmp(req->url, "/"))
    {
        res = "<html> <head> <title> Hello </title> </head> <body> <h1> Hello World </h1> <img src ='/img/test.png' alt = 'image'/> </body> </html>";
        http_headers(c, 200);
        http_response(c,  "text/html", res);
    }
    else
    {
        res = "file not found";
        http_headers(c, 404);
        http_response(c, "text/plain", res);
    }

    free(req);
    close(c);
    return;
}

int main(int argc, char const *argv[])
{
    int s, c;
    char *port;
    httpreq *req;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <listening port> \n", argv[0]);
        return -1;
    }
    else
    {
        port = argv[1];
        s = srv_init(atoi(port));
        if (!s)
        {
            fprintf(stderr, "%s\n", error);
            return -1;
        }
        printf("Listeneing on %s:%s\n", LISTENADDR, port);
        while (1)
        {
            c = cli_accept(s);
            if (!c)
            {
                fprintf(stderr, "%s\n", error);
                continue;
            }
            printf("Incoming connection \n");
            if (!fork())
            {
                cli_conn(s, c);
            }
        }
    }
    return -1;
}
