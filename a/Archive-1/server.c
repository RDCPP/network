/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> // O_RDONLY

int write_to_fd(int fd, char *str); // to HTTP response
void add_content_type(int fd, char * find); // specifying extensions

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd; //descriptors rturn from socket and accept system calls
	int portno; // port number
	socklen_t clilen;

	char buffer[256];

	/*sockaddr_in: Structure Containing an Internet Address*/
	struct sockaddr_in serv_addr, cli_addr;

	int n;
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	/*Create a new socket
	  AF_INET: Address Domain is Internet
	  SOCK_STREAM: Socket Type is STREAM Socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	portno = atoi(argv[1]); //atoi converts from String to Integer
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
	serv_addr.sin_port = htons(portno); //convert from host to network byte order

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
		error("ERROR on binding");

	listen(sockfd, 5); // Listen for socket connections. Backlog queue (connections to wait) is 5

	clilen = sizeof(cli_addr);
	/*accept function:
	  1) Block until a new connection is established
	  2) the new socket descriptor will be used for subsequent communication with the newly connected client.
	*/
	while(1) /* While server is not closed, client can connect this server by the socket. */
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");

		memset(buffer, 0, 256);
		/* equal to bzero function (init buffeer)
		   but memset is standard function. */
		n = read(newsockfd, buffer, 255); //Read is a block function. It will read at most 255 bytes
		if (n < 0) error("ERROR reading from socket");
		printf("%s\n", buffer); //Print HTTP request header.

		char *ptr1 = strchr(buffer,'/');
		char *ptr2 = strchr(ptr1,' ');
		int diff = ptr2 - ptr1;
		char *parse = (char *) malloc(sizeof(char) * 256);
		strncpy(parse,ptr1,diff);
		/* HTTP request header starts GET /*.* HTTP/1.1 .
		   We need *.*, so parse them.
		   ptr1 is / front of *.* pointer, ptr2 is ' ' rear to *.* pointer.
		   And we calculate length of *.* , then copy *.* by parse. */

		char *path = (char *) malloc(sizeof(char) * 256);

		if (*parse == '/' && strlen(parse) == 1)
			sprintf(path, "./src/index.html");
		else
			sprintf(path, "./src/%s", parse);
		/* And we need path to open file. So front of parsed data, attach source file path.*/
		int fd = open(path,O_RDONLY,0); 

		if(fd == -1)
			write_to_fd(newsockfd, "HTTP/1.1 404 Not Found\r\n");
		else
			write_to_fd(newsockfd, "HTTP/1.1 200 OK\r\n");

		write_to_fd(newsockfd, "Server: Linux version 5.0.1 (Ubuntu 7.3.0-27ubuntu1~18.04)\r\n");
		/* It is my test server profile */

		/* We need to open file. If file is not exist, HTTP should response 404 not found.
		   If not, HTTP should response 200 OK. */

		int point = -1;
		for (int i = 5; i < strlen(path); i++) {
			if (*(path+i) == '.') {
				point = i;
				break;
			}
		}

		// if (point == -1) {
		// 	error("No extension");
		// }

		/* Find point(.) to know extension.
		   If you find point, for-iteration will give integer value to use pointer calculation.
		   (It can have no point, but it is filtered by open function.) */

		add_content_type(newsockfd, path+point+1);

		/* Add Content-Type Header for client like text/html, image/jpeg and so on.*/

		if(fd > -1)
		{
			write_to_fd(newsockfd, "Content-Length: ");
			int size = (int) lseek(fd,0, SEEK_END);
			lseek(fd, 0, SEEK_SET);
			char sizestr[20];
			sprintf(sizestr, "%d", size);
			write_to_fd(newsockfd,  sizestr);
			/* Add Content-Length Header */
			write_to_fd(newsockfd, "\r\n\r\n");
			char *file_buffer = (char *) malloc(sizeof(char) * 1024);
			int n;
			int total_size = 0;
			while((n = read(fd, file_buffer, 1000)) > 0) {
				int wrote = write(newsockfd, file_buffer, n);
				printf("Wrote %d bytes\n", n);
				total_size += n;
			}
			printf("Total %d bytes\n", total_size);
			free(file_buffer);
			/* Finally, We read content file.
			   1. Create buffer for file.
			   2. Read file.
			   3. Send client buffer.
			   After reading all, free file_buffer to heap. */
		}
		
		if (n < 0) error("ERROR writing to socket");

		close(newsockfd);
		/* We finish what client wants, we should end 1 HTTP cycle. */
		free(parse);
	}
	close(sockfd);

	return 0;
}

int write_to_fd(int fd, char *str) {
	int len = strlen(str);
	return write(fd, str, len);
}


void add_content_type(int fd,char * find)
{	
	write_to_fd(fd, "Content-Type: ");
	if(!(strcmp(find,"htm") && strcmp(find,"html"))) write_to_fd(fd, "text/html\r\n");
	else if(!(strcmp(find,"jpeg") && strcmp(find,"jpg"))) write_to_fd(fd, "image/jpeg\r\n");
	else if(!strcmp(find,"gif")) write_to_fd(fd, "image/gif\r\n");
	else if(!strcmp(find,"png")) write_to_fd(fd, "image/png\r\n");
	else if(!strcmp(find,"mp3")) write_to_fd(fd, "audio/mpeg3\r\n");
	else if(!strcmp(find,"mp4")) write_to_fd(fd, "video/mpeg4\r\n");
	else if(!strcmp(find,"pdf")) write_to_fd(fd, "application/pdf\r\n");
	else write_to_fd(fd, "text/plain\r\n");
}
