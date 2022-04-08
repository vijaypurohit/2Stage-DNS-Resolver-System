/****
 * Socket Programming: DNS Client C Program using select()
 * IITG
**/

// #include <iostream>
// #include <limits>
// #include <string> // strlen(), memset(), length
// #include <cstring> // strlen(), memset(), length, bzero
// #include <vector>


// #include <stdio.h>  // printf() and fprintf()
// #include <stdlib.h> // exit(), atoi()

// #include <unistd.h> // read(), close()
// #include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
// #include <arpa/inet.h>
// #include <netinet/in.h> // sockaddr_in and inet_ntoa()

// using namespace std;

// #define RCVBUFSIZE 1030 //Size of receive buffer

// #if defined(_WIN32)
//     #define PAUSE "pause"
//     #define CLR "cls"
// #elif defined(unix) || defined(__unix__) || defined(__unix)
//     #define PAUSE "read -p 'Press Enter to continue...' var"
//     #define CLR "clear"
// #endif

#include "allHeaders.h"

void HandleTCPServer(int); //  TCP Server handling function 

int main(int argc, char const *argv[])
{

	 int clntSocketId; //DNS Client Socket Descriptor

    struct sockaddr_in servAddr; // Local Server Address
    struct sockaddr_in clntAddr; // DNS Client Address    
    unsigned short servPort; // Server Port

    unsigned int clntLen; // Lenght of DNS client address data structure


    if(argc !=3) // test for correct number of argument
    {
        printf("\nArguments Invalid: <Server Addr: %s> <Server Port: %s>\n", argv[1], argv[2]);
        exit(EXIT_FAILURE);
    }
    // strcpy(inpServAddr,argv[1]);
    servPort = atoi(argv[2]);


    printf("\n-----------------------------------------------------------------\n");
    printf("\n 2 STAGE DNS RESOLVER | Welcome to DNS Client Program");
    printf("\n Connecting to <Server Addr: %s> <Server Port: %d> \n", argv[1], servPort);
    printf("\n-----------------------------------------------------------------\n");

	if((clntSocketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("\nDNS CLIENT SOCKET CREATION: Failed....\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("\n--->DNS CLIENT SOCKET CREATION: Successfully Created, ID: %d ..\n", clntSocketId);
	}

    servAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    servAddr.sin_port = htons( servPort );       //Internet address (32 bits)
    // servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY;   //Address port (16 bits) : chooses any incoming interface
    servAddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(clntSocketId, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("\nCONNECT: Failed With Server");
        exit(EXIT_FAILURE);
    }
    else
    {   
    //converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
        char wlcmMsg[RCVBUFSIZE];
    // see if more message to receive
        if( recv(clntSocketId, wlcmMsg, RCVBUFSIZE, 0) < 0)
        {
            perror("\nDNS CLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }
        printf("\n--->%s\n", wlcmMsg);
    }

    HandleTCPServer(clntSocketId);

	if(close(clntSocketId) == 0)
	{
		printf("\n--->DNS CLIENT SOCKET: CONNECTION CLOSED.");
	}

	printf("\nDNS CLIENT: EXITED.\n\n");
	return 0;
}// end Main



void HandleTCPServer(int clntSktId)
{
    
    char choice;


    int msgType;
    char msgIp[RCVBUFSIZE-1]; // buffer for echo string
    char ClientMSGBuff[RCVBUFSIZE];
    char ClientRECVBuff[RCVBUFSIZE];
    int sendMsgSize; // size of received message

    string reqTypeString[] = {
        "DNS CLIENT: Closing Connection.",
        "Enter the Message String (Domain Name: www.sitename.com): ",
        "Enter the Message String (IP Address: xyz.abc.def.mno): ",
        "Invalid Request Type."
    };

    do
    {
        char ch ;
        msgType = -1;
        bzero(msgIp, RCVBUFSIZE-1);
        bzero(ClientMSGBuff, RCVBUFSIZE);
        bzero(ClientRECVBuff,RCVBUFSIZE);

        cout<<"\n\n\n";
        system(PAUSE);
        system(CLR);

        // cout<<"\n-----------------------------------------------------------------";
        // cout<<"\n-----------------------------------------------------------------\n";
        cout<<"\n DNS CLIENT | 2Stage DNS Resolver \n";
        cout<<"\n Type 1: Domain Name --> IP Address.";
        cout<<"\n Type 2: IP Address --> Domain Name.";
        cout<<"\n 0: Close Connection.";
        cout<<"\n-----------------------------------------------------------------";
        cout <<"\n Choice : ";
        
        cout<<" Enter the Message Request Type: ";
            cin>>msgType;
        cout<<"\n";

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
             sprintf(msgIp, "EXIT");
        }
        else if(msgType == 1 || msgType == 2)
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.getline(msgIp, RCVBUFSIZE); 
        }
        sprintf(ClientMSGBuff, "%d%s", msgType, msgIp);


    	if( (sendMsgSize = send(clntSktId, ClientMSGBuff, strlen(ClientMSGBuff), 0)) < 0)
	    {
	        perror("\nDNS CLIENT:SEND: Failed");
	        return;
	    }

        if(debug)
		  cout<<"\n-----SEND TO SERVER:----\n"<<ClientMSGBuff<<endl;


        // see if more message to receive
        if( (sendMsgSize = recv(clntSktId, ClientRECVBuff, RCVBUFSIZE, 0)) < 0)
        {
            perror("\nDNS CLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        if(debug)
            cout<<"\n-----RECV FROM SERVER:-----\n"<< ClientRECVBuff << endl;

        if(msgType!=0)
        cout<<"\n "<<msgType<<": "<<msgIp<<" ---> "<<ClientRECVBuff+1<<"\n";

        cout<<"\n-----------------------------------------------------------------";
             
        if(msgType == 0) break;

        cout<<"\nDo You Want To Continue: (y/n): ";
            cin>>ch;
        choice = ch;
    }while(choice!='n');// while count


}// end HandleTCPDNS Client