// Server Side 
// https://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf
// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
// https://www.geeksforgeeks.org/socket-programming-cc/
// https://tutorialspoint.dev/language/cpp/socket-programming-cc
// https://www.educative.io/edpresso/how-to-implement-tcp-sockets-in-c
// https://aticleworld.com/socket-programming-in-c-using-tcpip/
// https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html

#include <stdio.h>
#include <stdlib.h> // exit();
#include <string.h> // strlen
#include <sys/socket.h>

#define PORT 8080

int main(int argc, char const *argv[])
{

	/***** Creation of Socket *****/
	// int sockid = socket(family, type, protocol);

	int socketId = socket(AF_INET, SOCK_STREAM, 0);
	if(socketId < 0)
	{
		perror("\nSocket Creation Failed....\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("\nSocket Successfully Created..");
	}

	struct sockaddr_in address;
	int server_fd; 
	int opt = 1;
	// Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

     // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    char buffer[1024] = {0};
    char *hello = "Hello from server";
    int valread;

    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    send(new_socket , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");

	return 0;
}// end