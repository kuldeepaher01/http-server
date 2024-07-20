
This is a simple HTTP server written in C that listens on a specified port and responds to GET requests. It can serve HTML pages and images.

## **Function List**

Here is a list of functions in the server, along with a brief description, return type, and how to call them:

### 1. `int srv_init(int portno)`

-   Initializes the server and binds it to the specified port.
-   Returns: 0 on error, socket fd on success
-   Call: `int s = srv_init(8080);`

### 2. `int cli_accept(int s)`

-   Accepts an incoming connection and returns a new socket fd for the client.
-   Returns: 0 on error, new socket fd on success
-   Call: `int c = cli_accept(s);`

### 3. `httpreq *parse_http(char *str)`

-   Parses an HTTP request string and returns a struct containing the method and URL.
-   Returns: 0 on error, parsed HTTP request struct on success
-   Call: `httpreq *req = parse_http(buffer);`

### 4. `char *cli_read(int c)`

-   Reads data from a client socket and returns the received data.
-   Returns: 0 on error, received data on success
-   Call: `char *p = cli_read(c);`

### 5. `void http_response(int c, char *conttype, char *data)`

-   Sends an HTTP response to a client with the specified content type and data.
-   Returns: void
-   Call: `http_response(c, "text/html", "<html>...</html>");`

### 6. `void http_headers(int c, int status)`

-   Sends HTTP headers to a client with the specified status code.
-   Returns: void
-   Call: `http_headers(c, 200);`

### 7. `File *readfile(char *filename)`

-   Reads a file from disk and returns a struct containing the file data.
-   Returns: 0 on error, file struct on success
-   Call: `File *f = readfile("example.html");`

### 8. `int sendfile(int c, char *contype, File *file)`

-   Sends a file to a client with the specified content type.
-   Returns: 1 on success, 0 on error
-   Call: `sendfile(c, "image/png", f);`

### 9. `void cli_conn(int s, int c)`

-   Handles a client connection, parsing the request and sending a response.
-   Returns: void
-   Call: `cli_conn(s, c);`

## **Compiling and Running the Server**

To compile the server, use the following command:

`gcc -o httpd httpd.c`

To run the server, use the following command:

`./httpd <port>`

Replace `<port>` with the desired port number, such as 8080.

**Note**

This is a very basic HTTP server and does not support many features, such as POST requests, SSL/TLS, or error handling. It is intended for educational purposes only.
