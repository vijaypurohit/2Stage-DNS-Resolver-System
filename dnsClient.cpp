/****
 * Socket Programming: Client C Program using select()
 * Vijay Purohit
 * https://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf
 https://github.com/rahuliitg/Multi-stage-DNS-Resolving-System
**/

#include <iostream>
#include <limits>
#include <string> // strlen(), memset(), length
#include <cstring> // strlen(), memset(), length, bzero
// #include <vector>


#include <stdio.h>  // printf() and fprintf()
#include <stdlib.h> // exit(), atoi()

#include <unistd.h> // read(), close()
#include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
#include <arpa/inet.h>
// #include <netinet/in.h> // sockaddr_in and inet_ntoa()

using namespace std;

#define msgBUFSIZE 1030 //Size of receive buffer

#if defined(_WIN32)
    #define PAUSE "pause"
    #define CLR "cls"
#elif defined(unix) || defined(__unix__) || defined(__unix)
    #define PAUSE "read -p 'Press Enter to continue...' var"
    #define CLR "clear"
#endif

void HandleTCPServer(int); //  TCP Server handling function 

int main(int argc, char const *argv[])
{

	// int servSocketId; //Server Socket Descriptor, for return value of the socket function call
    int clntSocketId; //Client Socket Descriptor

    struct sockaddr_in servAddr; // Local Server Address
    struct sockaddr_in clntAddr; // Client Address    
    unsigned short servPort; // Server Port

    unsigned int clntLen; // Lenght of client address data structure

    // char inpServAddr[16] = {0}; 

    if(argc !=3) // test for correct number of argument
    {
        printf("\nArguments Invalid: <Server Addr: %s> <Server Port: %s>\n", argv[1], argv[2]);
        exit(EXIT_FAILURE);
    }
    // strcpy(inpServAddr,argv[1]);
    servPort = atoi(argv[2]);

    // strcpy(argv[1],"127.0.0.1");
    // servPort = 8080;
    printf("\n-----------------------------------------------------------------\n");
    printf("\n 2 STAGE DNS RESOLVER | Welcome to Client Program");
    printf("\n Connecting to <Server Addr: %s> <Server Port: %d> \n", argv[1], servPort);
    printf("\n-----------------------------------------------------------------\n");

	if((clntSocketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("\nCLINET SOCKET CREATION: Failed....\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("\n--->CLINET SOCKET CREATION: Successfully Created, ID: %d ..\n", clntSocketId);
	}


	
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY;   //Address port (16 bits) : chooses any incoming interface


    servAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    servAddr.sin_port = htons( servPort );       //Internet address (32 bits)
    servAddr.sin_addr.s_addr = inet_addr(argv[1]);
/******
 * int inet_aton(const char *cp, struct in_addr *inp);
 * converts the Internet host address cp from the IPv4 numbers-and-dots notation into binary form (in network byte order)
 * stores it in the structure that inp points to.
 * it returns nonzero if the address is valid, and 0 if not
****/
    // if(inet_pton(AF_INET, argv[1], &servAddr.sin_addr)<=0) 
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

        char wlcmMsg[msgBUFSIZE];
    // see if more message to receive
        if( recv(clntSocketId, wlcmMsg, msgBUFSIZE, 0) < 0)
        {
            perror("\nCLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        printf("\n--->SERVER: %s\n", wlcmMsg);
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
    
    char choice;


    int msgType;
    char msgIp[msgBUFSIZE-3]; // buffer for echo string
    char msgBuff[msgBUFSIZE];
    int sendMsgSize; // size of received message

    string reqTypeString[] = {
        "CLIENT: Closing Connection.",
        "Enter the Message String (Domain Name: www.sitename.com): ",
        "Enter the Message String (IP Address: xyz.abc.def.mno): ",
        "Invalid Request Type."
    };

    do
    {
        char ch ;
        // string msgIp = "";
        // string msgBuff = "";
        // Send message to Server
        msgType = -1;
        bzero(msgIp, msgBUFSIZE-3);
        bzero(msgBuff, msgBUFSIZE);

        cout<<"\n\n\n";
        system(PAUSE);
        system(CLR);

        cout<<"\n-----------------------------------------------------------------\n";
        cout<<"\n CLIENT | 2Stage DNS Resolver \n";
        cout<<"\n Type 1: Domain Name --> Request for corresponding IP Address.";
        cout<<"\n Type 2: IP Address --> Request for corresponding Domain Name.";
        cout<<"\n Type 0: Exit";
        cout<<"\n-----------------------------------------------------------------";
        cout <<"\n\n--Choice : ";

        cout<<"Enter the Message Request Type: ";
            cin>>msgType;

            cout<< " " <<msgType;

        if(cin.fail() || msgType < 0 || msgType > 3 ) // EXIT
        {
            cin.clear();
            cin.ignore(256,'\n');
            cout<<"\n: "<<reqTypeString[3];
            continue;
        }

        cout<<reqTypeString[msgType];

        if(msgType == 0)
        {
             sprintf(msgBuff, "EXIT");
        }
        else if(msgType == 1 || msgType == 2)
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.getline(msgIp, msgBUFSIZE); 
            // int n = 0;
            // while ((msgIp[n++] = getchar()) != '\n');
            sprintf(msgBuff, "%d$%s$", msgType, msgIp);
        }
    

    	if( (sendMsgSize = send(clntSktId, msgBuff, strlen(msgBuff), 0)) < 0)
	    {
	        perror("\nCLIENT:SEND: Failed");
	        return;
	    }

		cout<<"\n-----SEND TO SERVER:----\n"<<msgBuff<<endl;

        // see if more message to receive
        if( (sendMsgSize = recv(clntSktId, msgBuff, msgBUFSIZE, 0)) < 0)
        {
            perror("\nCLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        cout<<"\n-----RECV FROM SERVER:-----\n"<< msgBuff << endl;

        // fflush(stdout);  

        if(msgType == 0) break;

        cout<<"\nDo You Want To Continue: (y/n): ";
            cin>>ch;
        choice = ch;
    }while(choice!='n');// while count


}// end HandleTCPClient