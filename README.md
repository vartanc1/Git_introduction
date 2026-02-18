## Project 1 README

---

# WARMUP (ECHO)

## Overview and Description


This first warmup assignment focused on building a simple TCP client and server where the client sends data to the server and the server echoes the same data back. The purpose of this assignment was to understand the lifecycle of a TCP connection and how data flows between client and server using blocking sockets.

On the **client side**, the main system calls used were:

- `connect()` – establishes a TCP connection to the server  
- `send()` – sends data to the server  
- `recv()` – receives echoed data from the server  
- `close()` – closes the socket connection  

On the **server side**, the main system calls used were:

- `socket()` – creates a socket file descriptor  
- `setsockopt()` – enables port reuse (SO_REUSEADDR)  
- `bind()` – binds the socket to a specific port  
- `listen()` – sets the socket to accept incoming connections  
- `accept()` – accepts a new client connection and returns a new socket descriptor  
- `recv()` – receives data from the client  
- `send()` – sends the same data back to the client  

I used `char buffer[BUFSIZE]` to transfer and receive data across both client and server. `memset()` was used to clear structures and buffers before use to avoid undefined behavior caused by leftover memory.

The server was implemented using a simple iterative design. It handles one client at a time inside an infinite loop. This allowed me to focus on understanding connection establishment, data transmission, and proper cleanup of resources.

---

## Design and Flow of Control

### Server Side
1. `socket()`  
2. `setsockopt()`  
3. `bind()`  
4. `listen()`  
5. Infinite loop:
   - `accept()` new client  
   - `recv()` data from client  
   - `send()` same data back  
   - `close()` client socket  

### Client Side
1. `socket()`  
2. `connect()`  
3. `send()` message  
4. `recv()` echoed message  
5. `close()`  

---

## Testing

I ensured the program compiled without warnings. I tested the functionality using two separate terminals, one running the server and one running the client.

I verified that:
- The server correctly received the data sent by the client.
- The client received the exact same data back from the server.
- The connection closed properly after communication was complete.

I also used print statements during development to verify correct data flow and connection handling.



---

# WARMUP (TRANSFER FILE)

## Overview and Description


The transfer file assignment builds on the echo assignment but extends it to handle transferring a file of unknown size. Instead of simply echoing data back, the server reads a file and streams its contents to the client.

The key difference from the echo assignment is that the file size may not be known ahead of time, so data must be sent and received in chunks until the entire file has been transmitted.

### Server Side

The server maintains an infinite loop accepting clients. After accepting a connection, a `file_to_send` variable is used to open the file. The file descriptor is passed to a function that sends the file contents.

Inside the send loop:
- A `char buffer[BUFSIZE]` is used.
- `read()` reads chunks of data from the file.
- `send()` transmits those chunks to the client.
- The loop continues until the file has been fully transmitted.

This implementation approach was inspired by examples in Beej’s Guide to Network Programming.

### Client Side

The client setup is similar to the echo client but modified to handle file reception.

- `open()` is used to create or open a local file for writing.
- `recv()` is called in a loop to receive file data from the server.
- The received bytes are written to the file.
- The loop continues until the server closes the connection.

A fixed-size buffer allowed large files to be streamed without loading the entire file into memory.

---

## Design and Flow of Control

### Server Side
1. `socket()`  
2. `setsockopt()`  
3. `bind()`  
4. `listen()`  
5. Infinite loop:
   - `accept()` new client  
   - `open()` file  
   - Loop:
     - `read()` chunk from file  
     - `send()` chunk to client  
   - `close()` client socket  

### Client Side
1. `socket()`  
2. `connect()`  
3. `open()` output file  
4. Loop:
   - `recv()` data  
   - `write()` to file  
5. `close()`  

---

## Testing

I verified that:
- The file transferred completely.
- The received file matched the original file.
- Large files were handled properly.
- The connection closed cleanly after the transfer.

Testing was done using two terminals and by comparing file contents after transfer.

---



# Part 1 (gfclient)

## Overview and Description
Part 1 of gfclient deals with implementing a file transfer program according to the GetFile Protocol. We are expecting to send a request in the format `<scheme> <method> <path>\r\n\r\n` and receive a response in the format `<scheme> <status> <length>\r\n\r\n<content>`. All parameters used are stored in the struct `gfcrequest_t`, so a single structure carries all the necessary variables. The important function here is `gfc_perform()`, which builds the request string, sends the request data in bytes, and parses the response to ensure it is structured correctly before forwarding the data to my writefunc() callback.

---

## Design and Flow of Control
<p align="center">
  <img src="/Users/vartanc/Desktop/cs6200/gf_client.png" width="600"/>
</p>

---

## Testing and Implementation
---
```
struct gfcrequest_t{
  const char *path;
  const char *request;
  unsigned short portno; //port number
  void *arg;
  int sockfd;
  //call back functions
  void (*headerfunc)(void *, size_t, void *);
  void (*writefunc)(void *, size_t, void *);
  void *writearg, *headerarg;
  gfstatus_t status;
  size_t bytes_received;
  size_t file_len;
```

In `gfclient.c`, the following functions are used:

- `gfc_create()` – Allocates and initializes a `gfcrequest_t` request object on the heap.
- `gfc_set_server()` – Sets the server hostname/IP stored in `(*gfr)->request`.
- `gfc_set_port()` – Sets the server port stored in `(*gfr)->portno`.
- `gfc_set_path()` – Sets the file path to request from the server.
- `gfc_set_writefunc()` – Registers the callback function used to process received file bytes.
- `gfc_set_writearg()` – Sets the argument passed into the write callback.
- `gfc_set_headerfunc()` – Registers the callback function for header data (stored but not used in my current implementation).
- `gfc_set_headerarg()` – Sets the argument passed into the header callback.
- `gfc_get_status()` – Returns the parsed status code from the server response.
- `gfc_get_filelen()` – Returns the parsed file length from the server response
- `gfc_get_bytesreceived()` – Returns the total number of body bytes received so far.

## Testing

# Part 1 (gfserver)

## Overview and Description
GFSERVER implements the GetFile protocol by answer to request `<scheme> <method> <path>\r\n\r\n` and responding back with `<scheme> <method> <path>\r\n\r\n`. In my implementation, arguments like `port number`, `max pending connections`, `handler function`, and `handler arguments` are stored iside the struct `gfserver_t`. Each time a new client connects, a new `gfcontext_t` is allocated on the heap and contains a socket descriptor for that connection. The main server loop is built to handle accepting connections, receiving and validating headers, and finally passing that data to the handler function.


---

## Design and Flow of Control
The most important function in gfserver.c is gfs_serve which is shown below.gfs_serve() is the core of the server. It handles setting up new client connections, recieving and validating the GETfile request handler, and sending valid requests to the registered handler function.
<p align="center">
  <img src="/Users/vartanc/Desktop/cs6200/gf_server.png" width="600"/>
</p>



---
In `gfserver_serve.c`, the following functions and operations are used to handle client connections and validate requests:

- `accept()` – Accepts an incoming client connection.
  - If `accept()` returns a negative value:
    - Print error using `perror("accept")`.
    - Continue to the next loop iteration.

- `malloc()` (for `gfcontext_t`) – Allocates a new connection context on the heap.
  - If allocation fails:
    - Close the client socket.
    - Continue to the next loop iteration.

- `recv()` – Receives the request header in a loop.
  - Continue reading until the delimiter `\r\n\r\n` is detected.
  - Track total bytes received.
  - If `recv()` returns `<= 0`:
    - Call `gfs_abort(&ctx)`.
    - Free the context.
    - Continue to the next loop iteration.

- Header validation:
  - Ensure `\r\n\r\n` is present.
  - If not found:
    - Send `GF_INVALID` using `gfs_sendheader()`.
    - Abort the connection.
    - Free the context.

- Request parsing and validation:
  - Extract `scheme`, `method`, and `path`.
  - Ensure:
    - `scheme == "GETFILE"`
    - `method == "GET"`
    - `path` begins with `/`
  - If validation fails:
    - Send `GF_INVALID`.
    - Abort the connection.
    - Free the context.

- Handler dispatch:
  - If the request is valid:
    - Call `(*gfs)->handler_function(&ctx, path, (*gfs)->arg);`

- Cleanup:
  - After the handler returns:
    - If `ctx` is still non-NULL:
      - Close the socket.
      - Free the context.

---
## Testing and Implementation
## Implementation

```
struct gfserver_t {
    unsigned short portno;
    int max_npending;
    void *arg;
    gfh_error_t (*handler_function)(gfcontext_t **, const char *, void*);
};
```

```c
struct gfcontext_t {
    size_t file_len;
    int sockfd;
};
```
In `gfserver_main.c`, the following functions are used:

- `gfserver_create()` – Allocates and initializes the server structure on the heap.
- `gfserver_serve()` – Starts the main server loop, handles socket setup, and accepts incoming client connections.
- `gfs_sendheader()` – Formats and sends the protocol response header to the client.
- `gfs_send()` – Reliably sends data over the socket, handling partial sends.
- `gfs_abort()` – Safely closes a client connection in case of error.
- `gfserver_set_handlerarg()` – Sets the argument passed to the handler function.
- `gfserver_set_port()` – Sets the listening port for the server.
- `gfserver_set_maxpending()` – Sets the maximum number of pending connections.
- `gfserver_set_handler()` – Registers the handler function that processes valid client requests.
---

## Testing


---



# Part 2 (Multi-threaded gfserver)

## Overview and Description
This part of the project deals with writing gfserver_main.c in a multithreader fashion. A global variable steque_t is used. We also make use of the pthread libary and build our system to work with a boss/worker method. The boss/worker model is a concurrency design where the boss assigns tasks to a pool of worker threads.
`Queue_Context` represents a job that will be processed by a worker thread, where the `ctx` is the connection associated with the client and the path gives the file path needed. static `steque_t s_queue` holds pending jobs. `pthread_mutex_t mutex` is used to protect access to s_queue and static `pthread_cond_t worker` is used to signal when new jobs are avaliable. `Workers[i]` is used to enable the creation of a thread pool, allowing concucrency to happen in my software.

```
typedef struct queue_context {
    gfcontext_t *ctx;   
    char *path;     
} queue_context;
```
```
// globals
static steque_t s_queue;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t worker = PTHREAD_COND_INITIALIZER;

static pthread_t *workers;
static int nthreads;

static void *procedure(void *arg);
```
---

## Design and Flow of Control
<p align="center">
  <img src="/Users/vartanc/Desktop/cs6200/gfserver_multi.png" width="1000"/>
</p>

For the design, I will be going over the boss thread and worker (slave) thread functionality.

The boss thread is primarily responsible for accepting client connections and dispatching work by creating and enqueuing jobs for the worker threads. It initializes the server, creates a pool of worker threads by calling `handler_init(nthreads)`, and then calls `gfserver_serve()` to begin accepting new connections. When a valid request is received, `gfs_handler()` allocates a new `queue_context` job containing the connection and requested path. This job is added to the shared queue using `steque_enqueue(&s_queue, job)`. The workers are then notified using `pthread_cond_signal(&worker)`, and the boss immediately returns to accept the next incoming request.
The worker mainly runs through the `prodecure()` function which initially checks if the `steque_isempty()`. Once there is work to do, the worker will pop the job from the `s_queue` and grab the content.

---

## Testing



---

# Suggestions for Improvement (Extra Credit)

---


# Part 2 (Multi-threaded gfclient)

## Overview and Description

---

## Design and Flow of Control

---

## Testing







## References

- Beej’s Guide to Network Programming  
- https://www.youtube.com/watch?v=YecOnrNAWf0  
- https://github.com/flaviusone/ACS-APP/blob/master/Lab_1/boss-worker.c
- https://www.geeksforgeeks.org/cpp/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/
