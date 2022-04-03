/****
 * Socket Programming: Server Multi Client C Program using select()
 * Vijay Purohit
 * https://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf
 venky
**/ 

#include <stdio.h>  // printf() and fprintf()
#include <stdlib.h> // exit(), atoi()
#include <string.h> // strlen(), memset()

#include <unistd.h> // read(), close()
#include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
#include <arpa/inet.h> // sockaddr_in and inet_ntoa()

#include<sys/time.h> // for struct timeval {}, FD_SET, FD_ISSET, FD_ZERO macros

#include <netdb.h> // for server ip address
// #include <netinet/in.h> 

#include <errno.h> 


#define MAXPENDING 5 // Maximum outstanding connection requests
#define RCVBUFSIZE 1025 //Size of receive buffer
#define SERVIPADDR "127.0.0.1" // // SERVER IP ADDRESS
#define MAXCLIENTS 30

void HandleTCPClient(int, char *); //  TCP client handling function 

int main(int argc, char const *argv[])
{
    int servSocketId; //Server Socket Descriptor, for return value of the socket function call
    int clntSocketId; //Client Socket Descriptor

    struct sockaddr_in servAddr; // Local Server Address
    struct sockaddr_in clntAddr; // Client Address    
    bzero(&servAddr, sizeof(servAddr));
    bzero(&clntAddr, sizeof(clntAddr));
    unsigned short servPort; // Server Port

    unsigned int clntLen; // Lenght of client address data structure

    if(argc !=2) // test for correct number of argument
    {
        printf("\nArguments Invalid: %s <Server Port>\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    // printf("\n<Server Port: %s> \n", argv[1]);
    servPort = atoi(argv[1]);
    if(servPort<0 || servPort >65535)
    {
        servPort = 8080;
    }
    printf("\n---------------------------------------------------\n");
    printf("\n Welcome to Multi Client Server \t<Server Port: %d> \n", servPort);
    printf("\n---------------------------------------------------\n");

/***** 
 * Creation of Socket 
 * int sockid = socket(family, type, protocol); 
        sockid: socket descriptor, an integer (like a file-handle)
        family: integer, communication domain, 
                e.g., PF_INET, IPv4 protocols, Internet addresses (typically used)
                PF_UNIX, Local communication, File addresses
        type: communication type 
            SOCK_STREAM - reliable, 2-way, connection-based service
            SOCK_DGRAM - unreliable, connectionless, messages of maximum length
        protocol: specifies protocol 
            IPPROTO_TCP IPPROTO_UDP
            usually set to 0 (i.e., use default protocol)
        upon failure returns -1
*****/
    ;
    if((servSocketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\n--->SERVER SOCKET CREATION: Failed....\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n--->SERVER SOCKET CREATION: Successful, SocketID: %d ..\n", servSocketId);
    }

    
/*****    
 * Allows socket options values to be set.
    Forcefully attaching socket to the port 8080
    int setsockopt (sockid, level, optName, optVal, optLen);
     sockid: integer, socket descriptor
     level: integer, the layers of the protocol stack (socket, TCP, IP)
     optName: integer, option
     optVal: pointer to a buffer; upon return it contains the value of the specified option
     optLen: integer, in-out parameter
    it returns -1 if an error occured
*****/
    // int sockOptVal = 1;
    // if (setsockopt(servSocketId, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockOptVal, sizeof(sockOptVal)))
    // {
    //     perror("\nSETSOCKOPT: Failed");
    //     exit(EXIT_FAILURE);
    // }
    /*
/*****         
*****/

// Retrieve Server IP Address
    char hostbuffer[256];
    char *IPAddressBuffer;
    struct hostent *hostObj;
    // To retrieve host information
    int hostname;
    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    
    hostObj = gethostbyname(hostbuffer);

    IPAddressBuffer = inet_ntoa(*((struct in_addr*)hostObj->h_addr_list[0]));
    if (NULL == IPAddressBuffer)
    {
        perror("SERVER IP Address Could not be Found. Using Default. inet_ntoa");
        // IPAddressBuffer = "127.0.0.1";
        strcpy(IPAddressBuffer, SERVIPADDR);
        // exit(1);
    }


/***** 
 * Bind: associates and reserves a port for use by the socket
    int status = bind(sockid, &addrport, size); 
        sockid: integer, socket descriptor
        addrport: struct sockaddr, the (IP) address and port of the machine for TCP/IP server, internet address is usually set to INADDR_ANY, i.e., chooses any incoming interface
        size: the size (in bytes) of the addrport structure
        status: upon failure -1 is returned
*****/
    
    servAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    servAddr.sin_port = htons( servPort );       //Address port (16 bits) : chooses any incoming interface 
    servAddr.sin_addr.s_addr = inet_addr(IPAddressBuffer);    //Internet address (32 bits) //INADDR_ANY

    if ((bind(servSocketId, (struct sockaddr *)&servAddr, sizeof(servAddr))) < 0 )
    {
        perror("BIND: Failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n--->BIND Successful, port: %d.\n", ntohs(servAddr.sin_port));
    }


/*****
 * listen: Instructs TCP protocol implementation to listen for connections
 * int status = listen(sockid, queueLimit); 
        sockid: integer, socket descriptor
        queuelen: integer, # of active participants that can “wait” for a connection
        status: 0 if listening, -1 if error
    listen() is non-blocking: returns immediately         
*****/

    if (listen(servSocketId, MAXPENDING) < 0)
    {
        perror("LISTEN: Failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n--->LISTEN: Server listening on address: %s ....\n", inet_ntoa(servAddr.sin_addr));
         // wait for a client to connect
        printf("\n---------SERVER: Waiting For Connection.---------\n");
    }


    fd_set readSockSet;  //set of socket descriptors 
    int maxDescriptor = -1; //Maximum socket descriptor value
    int clntSocketIds[MAXCLIENTS] = {0};

    char recvBuffer[RCVBUFSIZE]; // buffer for echo string
    int recvMsgSize; // size of received message

    int wlcmMsgLength = 1024;
    char shrtMsg[wlcmMsgLength];

    //Initialize all_connections
    for (int c = 0; c < MAXCLIENTS; c++)  
    {  
        clntSocketIds[c] = 0;  
    }  

    clntLen = sizeof(clntAddr); //Set the size of the in-out parameter

    while(1)
    {
        FD_ZERO(&readSockSet); // removes all descriptors from vector. clear the socket set ;
        FD_SET(servSocketId, &readSockSet);  // add descriptor to vector. add servSocketId socket to set 
        maxDescriptor = servSocketId;  
        
        for ( int i = 0 ; i < MAXCLIENTS ; i++)   //Set the fd_set before passing it to the select call
        {     
            if(clntSocketIds[i] > 0)  //if valid socket descriptor then add to read list 
                FD_SET( clntSocketIds[i] , &readSockSet);  
                 
            if(clntSocketIds[i] > maxDescriptor)  
                maxDescriptor = clntSocketIds[i];  //highest file descriptor number, need it for the select function 
        } // for i , add child sockets to set

         //wait for an activity on one of the sockets , timeout is NULL , 
        if (((select( maxDescriptor + 1 , &readSockSet , NULL , NULL , NULL)) < 0) && (errno!=EINTR))  
        {  
            printf("\nACCEPT: Failed");
            // continue;
        }  

        

        //If something happened on the master socket, then its an incoming connection 
        if (FD_ISSET(servSocketId, &readSockSet))  //vector membership check
        {  

           
/****
* The server gets a socket for an incoming client connection by calling accept()
    int s = accept(sockid, &clientAddr, &addrLen);
        s: integer, the new socket (used for data-transfer)
        sockid: integer, the orig. socket (being listened on)
        clientAddr: struct sockaddr, address of the active participant. filled in upon return
        addrLen: sizeof(clientAddr): value/result parameter. must be set appropriately before call. adjusted upon return
* ***/
            if ((clntSocketId = accept(servSocketId,(struct sockaddr *)&clntAddr, (socklen_t*)&clntLen))<0)  
            {  
                perror("\nACCEPT: Failed");
                exit(EXIT_FAILURE); 
            }  

            //inform user of socket number - used in send and receive commands 
            printf("\n--> ACCEPT: Connection Established With Client. Socket FD: %d , IP : %s , PORT: %d \n" , clntSocketId , inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port)); 
            

            sprintf(shrtMsg, "Connection Established With SERVER: %s, PORT: %d ", inet_ntoa(servAddr.sin_addr) , ntohs(servAddr.sin_port) );

            //send new connection greeting message to client
            if( send(clntSocketId, shrtMsg, wlcmMsgLength, 0) != wlcmMsgLength )  
            {  
                perror("\nSERVER:SEND: Failed");
                break;  
            }  
            
            for (int i = 0; i < MAXCLIENTS; i++)  //add new socket to array of sockets 
            {  
                //if position is empty 
                if( clntSocketIds[i] == 0 )  
                {  
                    clntSocketIds[i] = clntSocketId;  
                    printf("Adding to list of sockets as %d\n" , i);
                    break;  
                }  
            }  
        }// if FD_set
        //else its some IO operation on some other socket
        for (int i = 0; i < MAXCLIENTS; i++)  
        {  
            int sd = clntSocketIds[i];  
                 
            if (FD_ISSET( sd , &readSockSet))  
            {  

                fflush(stdin);    
                bzero(recvBuffer, RCVBUFSIZE);

                getpeername(sd , (struct sockaddr*)&clntAddr, (socklen_t*)&clntLen);  

                if( (recvMsgSize = recv(sd, recvBuffer, RCVBUFSIZE, 0)) < 0)
                {
                    printf("\nSERVER:RECV: Failed. Client: IP %s , PORT %d \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port));
                    break ;
                }
                else
                printf("\n----RECV FROM CLIENT IP %s, PORT %d:-----\n%s \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port), recvBuffer);
                printf("\n----RECV msg size %d, len %ld:----- \n",recvMsgSize, strlen(recvBuffer));

                //Check if it was for closing , and also read the incoming message 
                 // if msg contains "Exit" then server exit and chat ended.
                if (strncmp("EXIT", recvBuffer, 4) == 0 || recvMsgSize<=0) 
                {

                    sprintf(shrtMsg, "SERVER: CLIENT CLOSING CONNECTION: IP %s, PORT: %d ", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port) );

                    // IP %s, PORT %d: \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port));

                    printf("--\n%s\n--", shrtMsg);
                    send(sd, shrtMsg, strlen(shrtMsg), 0);

                    close(sd);   // Close Client Socket
                    clntSocketIds[i] = 0;     
                }
                else
                {
                    // Send message back to Client
                    if(send(sd, recvBuffer, strlen(recvBuffer), 0) < 0)
                    {
                        perror("\nSERVER:SEND To Client: Failed");
                        break;
                    }
                    else
                    printf("\n----SEND TO CLIENT IP %s, PORT %d:-----\n%s \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port), recvBuffer);
                }
                
            }// if fd isset
        }

    }// while waiting for coonection



/*****         
 * close the socket. When finished using a socket, the socket should be closed
    status = close(sockid); 
        sockid: the file descriptor (socket being closed)
        status: 0 if successful, -1 if error
*****/

    if(close(servSocketId) == 0)
    {
        printf("\nSERVER SOCKET: CONNECTION CLOSED.");
    }
    
    printf("\nSERVER: BYE BYE\n\n");
    return 0;
}// end Main




// Function to Handle TCP Single Client
void HandleTCPClient(int clntSktId, char *clientIpAddress)
{
    char recvBuffer[RCVBUFSIZE]; // buffer for echo string
    int recvMsgSize; // size of received message


    // infinte loop for communication
    while(1)
    {
        bzero(recvBuffer, RCVBUFSIZE);

        if( (recvMsgSize = recv(clntSktId, recvBuffer, RCVBUFSIZE, 0)) < 0)
        {
            printf("\nSERVER:RECV: Failed. Client: %s \n", clientIpAddress);
            break ;
        }

        printf("\n----RECV FROM CLIENT:-----\n%s \n", recvBuffer);

         // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("EXIT", recvBuffer, 4) == 0) {
            char exitMsg[] = "SERVER: CLIENT CLOSING CONNECTION. EXITING";
                printf("\n%s.\n", exitMsg);
                send(clntSktId, exitMsg, sizeof(exitMsg), 0);
            break;
        }

        // Send message back to Client
        ;
        if(send(clntSktId, recvBuffer, recvMsgSize, 0) < 0)
        {
            perror("\nSERVER:SEND: Failed");
            return;
        }

        printf("\n----SEND TO CLIENT:-----\n%s \n", recvBuffer);

    } // while true
    
    
    close(clntSktId); // Close Client Socket

}// end HandleTCPClient