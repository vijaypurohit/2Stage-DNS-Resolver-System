/****
 * Socket Programming: Client C Program using select()
 * Vijay Purohit
 * https://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf
**/

#include <stdio.h>  // printf() and fprintf()
#include <stdlib.h> // exit(), atoi()
#include <string.h> // strlen(), memset()

#include <unistd.h> // read(), close()
#include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
#include <arpa/inet.h>
// #include <netinet/in.h> // sockaddr_in and inet_ntoa()

#define msgBUFSIZE 1025 //Size of receive buffer

void HandleTCPServer(int); //  TCP Server handling function 

int main(int argc, char const *argv[])
{

	// int servSocketId; //Server Socket Descriptor, for return value of the socket function call
    int clntSocketId; //Client Socket Descriptor

    struct sockaddr_in servAddr; // Local Server Address
    struct sockaddr_in clntAddr; // Client Address    
    unsigned short servPort; // Server Port

    unsigned int clntLen; // Lenght of client address data structure

    char inpServAddr[16] = {0}; 

    if(argc !=3) // test for correct number of argument
    {
        printf("\nArguments Invalid: <Server Addr: %s> <Server Port: %s>\n", argv[1], argv[2]);
        exit(EXIT_FAILURE);
    }
    strcpy(inpServAddr,argv[1]);
    servPort = atoi(argv[2]);

    // strcpy(inpServAddr,"127.0.0.1");
    // servPort = 8080;
    printf("\n---------------------------------------------------\n");
    printf("\n Welcome to Client ");
    printf("\n   Connecting to <Server Addr: %s> <Server Port: %d> \n", inpServAddr, servPort);
    printf("\n---------------------------------------------------\n");

	if((clntSocketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("\nCLINET SOCKET CREATION: Failed....\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("\n--->CLINET SOCKET CREATION: Successfully Created, ID: %d ..\n", clntSocketId);
	}


	
    // servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY;   //Address port (16 bits) : chooses any incoming interface


    servAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    servAddr.sin_port = htons( servPort );       //Internet address (32 bits)
    servAddr.sin_addr.s_addr = inet_addr(inpServAddr);
/******
 * int inet_aton(const char *cp, struct in_addr *inp);
 * converts the Internet host address cp from the IPv4 numbers-and-dots notation into binary form (in network byte order)
 * stores it in the structure that inp points to.
 * it returns nonzero if the address is valid, and 0 if not
****/
    // if(inet_pton(AF_INET, inpServAddr, &servAddr.sin_addr)<=0) 
    // {
    //     printf("\nINPUT: Invalid Input Server Address.\n");
    //     exit(EXIT_FAILURE);
    // }

 /*****
  * The client establishes a connection with the server by calling connect()
	int status = connect(sockid, &foreignAddr, addrlen);
		 sockid: integer, socket to be used in connection
		 foreignAddr: struct sockaddr: address of the passive participant
		 addrlen: integer, sizeof(name)
		 status: 0 if successful connect, -1 otherwise
		 connect() is blocking
 ****/

    
    if (connect(clntSocketId, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("\nCONNECT: Failed With Server");
        exit(EXIT_FAILURE);
    }
    else
    {   
    //converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
        // printf("\nCONNECT: Connection Established With Server: %s \n\n", inet_ntoa(servAddr.sin_addr));

        char recvBuffer[msgBUFSIZE];
    // see if more message to receive
        if( recv(clntSocketId, recvBuffer, msgBUFSIZE, 0) < 0)
        {
            perror("\nCLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        printf("\n-----RECV FROM SERVER:-----\n%s \n", recvBuffer);
    }

    /***
 * recv: Receive message from server 
     int count = send(sockid, msg, msgLen, flags); 
         msg: const void[], message to be transmitted
        msgLen: integer, length of message (in bytes) to transmit
        flags: integer, special options, usually just 0
        count: # bytes transmitted (-1 if error)
    int count = recv(sockid, recvBuf, bufLen, flags); 
        recvBuf: void[], stores received bytes
        bufLen: # bytes received
        flags: integer, special options, usually just 0
        count: # bytes received (-1 if error)
*/

    HandleTCPServer(clntSocketId);
   
    // send(clntSocketId, msgBuffer, msgBUFSIZE, 0);

	if(close(clntSocketId) == 0)
	{
		printf("\n--->CLIENT SOCKET: CONNECTION CLOSED.");
	}

	printf("\nCLIENT: BYE BYE.\n\n");
	return 0;
}// end Main



void HandleTCPServer(int clntSktId)
{
    // Send message to Server
    char msgBuffer[msgBUFSIZE]; // buffer for echo string
    char recvBuffer[msgBUFSIZE];
    int sendMsgSize; // size of received message

    // int count = 1;
     int n ;
    while(1)
    {
    	bzero(msgBuffer, msgBUFSIZE);

		printf("Enter the String to Send (EXIT for close):\n");
         n = 0;
        while ((msgBuffer[n++] = getchar()) != '\n');

    	if( (sendMsgSize = send(clntSktId, msgBuffer, msgBUFSIZE, 0)) < 0)
	    {
	        perror("\nCLIENT:SEND: Failed");
	        return;
	    }

		printf("\n-----SEND TO SERVER:----\n%s \n", msgBuffer);

        // see if more message to receive
        if( (sendMsgSize = recv(clntSktId, recvBuffer, msgBUFSIZE, 0)) < 0)
        {
            perror("\nCLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        printf("\n-----RECV FROM SERVER:-----\n%s \n", recvBuffer);

         // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("EXIT", msgBuffer, 4) == 0) {
            char exitMsg[] = "CLIENT: Closing Connection.";
                printf("\n%s.\n", exitMsg);
            break;
        }
        // count++;
    }// while count


}// end HandleTCPClient