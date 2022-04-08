/****
 * Socket Programming: Server Single Client C Program using select()
 * IITG
**/

#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstring> // strlen(), memset()


#include <stdio.h>  // printf() and fprintf()
#include <stdlib.h> // exit(), atoi()

#include <unistd.h> // read(), close()
#include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
#include <arpa/inet.h> // sockaddr_in and inet_ntoa()

#include<sys/time.h> // for struct timeval {}
// #include <sys/types.h>
#include <netdb.h> // for server ip address
// #include <netinet/in.h> 
#include <ifaddrs.h>

using namespace std;

#define MAXPENDING 5 // Maximum outstanding connection requests
#define RCVBUFSIZE 1030 //Size of receive buffer
#define SERVIPADDR "127.0.0.2" // // SERVER IP ADDRESS | Use this in case of failure
#define DBFILENAME "database_mappings.txt"

#if defined(_WIN32)
    #define PAUSE "pause"
    #define CLR "cls"
#elif defined(unix) || defined(__unix__) || defined(__unix)
    #define PAUSE "read -p 'Press Enter to continue...' var"
    #define CLR "clear"
#endif

void HandleTCPClient(int); //  TCP client handling function 

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

    char shrtMsg[RCVBUFSIZE];


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
    
    cout<<"\n-----------------------------------------------------------------\n";
    cout<<"\n DNS SERVER | 2Stage DNS Resolver \t<Server Port: "<<servPort<<"> \n";
    cout<<"\n-----------------------------------------------------------------";

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
        perror("\nSERVER SOCKET CREATION: Failed....\n");
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
        perror("SERVER IP Address Could not be found Using Default. inet_ntoa");
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

/*****
 * accept(): The server gets a socket for an incoming client connection by calling accept()
 * is blocking: waits for connection before returning
        int s = accept(sockid, &clientAddr, &addrLen); 
        s: integer, the new socket (used for data-transfer)
        sockid: integer, the orig. socket (being listened on)
        clientAddr: struct sockaddr, address of the active participant. filled in upon return
        addrLen: sizeof(clientAddr): value/result parameter. must be set appropriately before call. adjusted upon return       
*****/
    int running =1;

    while(1)
    {

         printf("\n--->LISTEN: Server listening on address: %s ....\n", inet_ntoa(servAddr.sin_addr));

         // printf("(You can press Enter to Close Server)\n\n");
         // if(getchar()=='\n')
         //    break;

         clntLen = sizeof(clntAddr); //Set the size of the in-out parameter
         // wait for a client to connect
        if ((clntSocketId = accept(servSocketId, (struct sockaddr *)&clntAddr, (socklen_t*)&clntLen)) < 0 )
        {
            perror("\nACCEPT: Failed");
            exit(EXIT_FAILURE);
        }
        else
        {   
            /****
             * char *inet_ntoa(struct in_addr in);
             * converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
             ***/
       
            printf("\n--->ACCEPT: Connection Established With Client. Socket FD: %d , IP : %s , PORT: %d\n" , clntSocketId , inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port)); 
            
            sprintf(shrtMsg, "Connection Established With DNS SERVER: %s, PORT: %d", inet_ntoa(servAddr.sin_addr) , ntohs(servAddr.sin_port) );

            //send new connection greeting message 
            if( send(clntSocketId, shrtMsg, strlen(shrtMsg), 0) != strlen(shrtMsg) )  
            {  
                perror("\nSERVER:SEND: Failed");
                exit(1);  
            }  
        }


    /*** HANDLE TCP CLIENT *****/
    /***
     * 
         int count = send(sockid, msg, msgLen, flags); 
             msg: const void[], message to be transmitted
            msgLen: integer, length of message (in bytes) to transmit
            flags: integer, special options, usually just 0
            count: # bytes transmitted (-1 if error)
    recv: Receive message from Client 
        int count = recv(sockid, recvBuf, bufLen, flags); 
            recvBuf: void[], stores received bytes
            bufLen: # bytes received
            flags: integer, special options, usually just 0
            count: # bytes received (-1 if error)
    */
        HandleTCPClient(clntSocketId);
       

        cout<<"\n\n\n\n";
    }
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




void HandleTCPClient(int clntSktId)
{
    char recvBuffer[RCVBUFSIZE] = {0}; // buffer for receiveing string
    // bzero(recvBuffer, RCVBUFSIZE);
    char sendBuffer[RCVBUFSIZE] = {0}; // buffer for sending string
    // string sendBuffer;

    int recvMsgSize; // size of received message
    char shrtMsg[RCVBUFSIZE];

    struct sockaddr_in clntAddr; // Client Address 

    unsigned int clntLen; // Lenght of client address data structure

    clntLen = sizeof(clntAddr);

    int pos;
    string tp, domainname, ipaddress;
    unordered_map<string,string>DNtoIP,IPtoDN;

    // FILE *newfile=fopen("database_mappings.txt",ios::in);
    // READING FROM DATABASE FILE
        ifstream dbFile;

        dbFile.open(DBFILENAME, ios::in);

        if (!dbFile) {
            cout << "\n**Database File failed to open:"<<DBFILENAME<<"\n\n";
            dbFile.clear();
            return;
        }

        while(getline(dbFile, tp)){ //read data from file object and put it into string.
             pos=tp.find(" ");
            DNtoIP[tp.substr(0,pos)]=tp.substr(pos+1);
            IPtoDN[tp.substr(pos+1)]=tp.substr(0,pos);
        } 

        dbFile.close(); // file close

        // cout<<"Mapping Database: "<<endl;
        // for (auto x : DNtoIP)
        //     cout << x.first << " " << x.second << endl;

        //  for (auto x : IPtoDN)
        //     cout << x.first << " " << x.second << endl;

// while(1)
// {
    getpeername(clntSktId , (struct sockaddr*)&clntAddr, (socklen_t*)&clntLen);  

    if( (recvMsgSize = recv(clntSktId, recvBuffer, RCVBUFSIZE, 0)) < 0)
    {
        printf("\nSERVER:RECV: Failed. Client: %s \n", inet_ntoa(clntAddr.sin_addr) );
        return ;
    }

    printf("\n----RECV FROM CLIENT:-----\n%s \n", recvBuffer);

    string recvString(recvBuffer); // char arrray to string;

    if(recvBuffer[0]=='1') // Msg Request of Type 1
     {
        domainname = recvString.substr(1);

        cout<<"::Domain Name Received: "<<domainname<<"\n";
        
        if(DNtoIP.count(domainname)==1)
        {
            sprintf(sendBuffer, "3%s", DNtoIP[domainname].c_str());
            // sendBuffer = "3"+DNtoIP[domainname];
            // send(sd,response,strlen(response),0);
        }
        else
        {
            // string response="4$entry not found in the database";.
            // sendBuffer = "4entry not found in the database.";
            sprintf(sendBuffer, "4entry not found in the database.");
            // send(sd,response,strlen(response),0);
        }
     }
     else if(recvBuffer[0]=='2')
     {
        ipaddress = recvString.substr(1);

        cout<<"::IP Address Received: "<<ipaddress<<"\n";

        if(IPtoDN.count(ipaddress)==1)
        {
            // sendBuffer = "3"+DNtoIP[ipaddress];
            sprintf(sendBuffer, "3%s", IPtoDN[ipaddress].c_str());
        }
        else
        {
            // sendBuffer = "4entry not found in the database.";
            sprintf(sendBuffer, "4entry not found in the database.");
        }
     }
     
     
     
     ////////////////////////////////////////////////////////////////////////////////////////////
    else if (recvBuffer[0]=='0' || recvMsgSize<=0) 
    {
        sprintf(shrtMsg, "SERVER: CLOSING CONNECTION: IP %s, PORT: %d ", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port) );

        printf("--\n%s\n--", shrtMsg);
        send(clntSktId, shrtMsg, sizeof(shrtMsg), 0);
        close(clntSktId);
        return;
    }


    // Send message back to Client
    
    if(send(clntSktId, sendBuffer, strlen(sendBuffer), 0) < 0)
    {
        perror("\nSERVER:SEND: Failed");
        return;
    }

    printf("\n----SEND TO CLIENT:-----\n%s \n", sendBuffer);

// }    
    
    
    sprintf(shrtMsg, "SERVER: CLOSING CONNECTION: IP %s, PORT: %d ", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port) );
    // send(clntSktId, shrtMsg, sizeof(shrtMsg), 0);
    printf("--\n%s\n--", shrtMsg);
    close(clntSktId); // Close Client Socket

}// end HandleTCPClient