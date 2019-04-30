#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "server.h"

void myerror(char *str)
{
  myerror(str);
  exit(1);
}

/*
 * Main Function
 */

int main(int argc, const char *argv[])
{
  int sock_fd, new_sock_fd; /* File descriptor for server request/response socket device */
  int port_no;              /* Port Number passed by user, via command line argument */
  socklen_t clilen;         /* Variable to store address length of client device */

  struct sockaddr_in serv_addr, cli_addr; /* struct variable to store server/client address */

  /* Integer variable to store return values from system calls. 
   * This variable is evaluated by if statements after each functions are called. 
   * Most system calls returns negative integers, so we'll check whether the return value is negative or not.
   */
  int n;

  if (argc < 2)
  { /* If statement to check if user has provided valid port number by checking argument count */
    /* Entering this statement means user hasn't provided valid port number. 
     * So we'll print error description to stderr pipe using error function. 
     */
    myerror("ERROR, no port provided");
    exit(1); /* terminates program with positive exit code. */
  }

  port_no = atoi(argv[1]); /* All arguments are passed to program as char array, so we should convert it into Integer to use it. */

  sock_fd = socket(AF_INET, SOCK_STREAM, 0); /* Creates socket to accept input from user. */
  if (sock_fd < 0)
    myerror("ERROR opening socket");            /* When socket creation process is failed, system will return -1 instead of usual file descriptors. */
  bzero((char *)&serv_addr, sizeof(serv_addr)); /* Fills serv_addr struct with zero. */

  /* Declares address type of server socket. */
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  serv_addr.sin_port = htons(port_no); /* Set port number to value which we have collected earlier. */

  /* Tries to bind socket to system with given value. 
   * if it fails, print error and terminate program. 
   */
  if (bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    myerror("ERROR while binding");

  listen(sock_fd, BACKLOG); /* starts listening from client. */

  clilen = sizeof(cli_addr); /* Assigns clilen variable with the size of client address structure. */
  while (true)
  { /* HTTP server should run forever. */
    /* Wait until connection from client is made. 
     * If returned file descriptor contains negative interger(which means accept() function haven't worked successful), 
     * this program prints error and terminates itself.
     */
    char *buffer = (char *)malloc(sizeof(char) * HEADER_BUFFER_SIZE);  /* char array which stores buffer for header */
    char *header = (char *)malloc(sizeof(char) * MAX_HEADER_SIZE);     /* char array to store header */
    char *path = (char *)malloc(sizeof(char) * MAX_REQUEST_PATH_SIZE); /* char array to store request path from client */

    new_sock_fd = accept(sock_fd, (struct sockaddr *)&cli_addr, &clilen);

    if (new_sock_fd < 0)
      myerror("ERROR while accepting");

    while (true)
    {
      n = read(new_sock_fd, buffer, MAX_SOCKET_READ_SIZE - 1);
      if (n < 0)
        myerror("ERROR while reading from socket");

      printf("%s", buffer);
      strcat(header, buffer);
      if (n < 255)
        break;
    }

    /* Get request path from header. */
    char *p = header;
    char *pp = path;
    while (*(p++) != '/')
      ;
    while (*(p) != ' ' && *(p) != '\n')
      *(pp++) = *(p++);

    printf("Opening file [%s]\n", path);

    *(pp) = '\0';

    if (*path == '/' && strlen(path) == 1)
    {
      sprintf(path, "index.html");
    }

    /* Retrieves file descriptior via open() function with given file path. 
     * If open() returns negative value, this means system couldn't find file from server folder, 
     * so we should return 404 error to client. 
     * Otherwise, just return 200 status code.
     */
    int status = open(path, O_RDONLY);
    if (status >= 0)
    {
      n = write_status(new_sock_fd, HTTP_OK_CODE);
      if (n < 0)
        myerror("ERROR writing to socket");
    }
    else
    {
      n = write_status(new_sock_fd, HTTP_NOT_FOUND_ERR_CODE);
      if (n < 0)
        myerror("ERROR writing to socket");
    }

    /* Since valid connection is made, server returns Connection and Server header to user. 
     * Writing header uses write_header() function. 
     * This function is also described in func.c file.
    */
    n = write_header(new_sock_fd, "Connection", "Close");
    if (n < 0)
      myerror("ERROR writing to socket");
    n = write_header(new_sock_fd, "Server", "MyCServer/macOS 10.13.4");
    if (n < 0)
      myerror("ERROR writing to socket");

    /* If file requested by user is found, server writes file to response stream. 
     * This step uses write_file() function, so check func.c file.
     */
    if (status >= 0)
    {
      n = write_file(new_sock_fd, status, path);
      if (n < 0)
        myerror("ERROR writing to socket");
    }

    close(new_sock_fd); /* Ends connection with user. */
  }
}