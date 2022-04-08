/****
 * Socket Programming: Server Single Client C Program using select()
 * IITG
**/

// #include <iostream>
// #include <string>
// #include <unordered_map>
// #include <fstream>
// #include <cstring> // strlen(), memset()


// #include <stdio.h>  // printf() and fprintf()
// #include <stdlib.h> // exit(), atoi()

// #include <unistd.h> // read(), close()
// #include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
// #include <arpa/inet.h> // sockaddr_in and inet_ntoa()
// 
// #include<sys/time.h> // for struct timeval {}
// #include <sys/types.h>
// #include <netdb.h> // for server ip address
// #include <netinet/in.h> 
// #include <ifaddrs.h>

// using namespace std;

// #define MAXPENDING 5 // Maximum outstanding connection requests
// #define RCVBUFSIZE 1030 //Size of receive buffer
// #define DefDNSIPADDR "127.0.1.2" // // SERVER IP ADDRESS | Use this in case of failure
// #define DBMapFILENAME "database_mappings.txt"

// #if defined(_WIN32)
//     #define PAUSE "pause"
//     #define CLR "cls"
// #elif defined(unix) || defined(__unix__) || defined(__unix)
//     #define PAUSE "read -p 'Press Enter to continue...' var"
//     #define CLR "clear"
// #endif

#include "allHeaders.h"

char DNSShrtMsg[RCVBUFSIZE];

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



    if(argc!=2) // test for correct number of argument
    {
        printf("\nArguments Invalid: %s <DNS Server Port>\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // printf("\n<Server Port: %s> \n", argv[1]);
    servPort = atoi(argv[1]);
    if(servPort<0 || servPort >65535)
    {
        servPort = 8080;
    }
    
    cout<<"\n-----------------------------------------------------------------\n";
    cout<<"\n DNS SERVER | 2Stage DNS Resolver \t<DNS Server Port: "<<servPort<<"> \n";
    cout<<"\n-----------------------------------------------------------------";


    if((servSocketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\nDNS SERVER SOCKET CREATION: Failed....\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n--->DNS SERVER SOCKET CREATION: Successful, SocketID: %d ..\n", servSocketId);
    }


// Retrieve Server IP Address
    // char hostbuffer[256];
    // char *IPAddressBuffer;
    // struct hostent *hostObj;// To retrieve host information
    // int hostname; // To retrieve hostname
    // hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    // hostObj = gethostbyname(hostbuffer);
    // IPAddressBuffer = inet_ntoa(*((struct in_addr*)hostObj->h_addr_list[0]));
    // if (NULL == IPAddressBuffer)
    // {
    //     perror("DNS SERVER IP Address Could not be found Using Default. inet_ntoa");
       
    //     strcpy(IPAddressBuffer, DefDNSIPADDR);
    // }

    servAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    servAddr.sin_port = htons( servPort );       //Address port (16 bits) : chooses any incoming interface 
    servAddr.sin_addr.s_addr = inet_addr(DefDNSIPADDR);    //Internet address (32 bits) //INADDR_ANY

    if ((bind(servSocketId, (struct sockaddr *)&servAddr, sizeof(servAddr))) < 0 )
    {
        perror("BIND: Failed");
        exit(EXIT_FAILURE);
    }
    else
    {
       printf("\n--->BIND Successful, port: %d.\n", ntohs(servAddr.sin_port));
    }

    if (listen(servSocketId, MAXPENDING) < 0)
    {
        perror("LISTEN: Failed");
        exit(EXIT_FAILURE);
    }

    

    while(1)
    {   

        printf("\n--->LISTEN: DNS Server listening on address: %s ....\n", inet_ntoa(servAddr.sin_addr));

         clntLen = sizeof(clntAddr); //Set the size of the in-out parameter
         // wait for a client to connect
        if ((clntSocketId = accept(servSocketId, (struct sockaddr *)&clntAddr, (socklen_t*)&clntLen)) < 0 )
        {
            perror("\nACCEPT: Failed");
            exit(EXIT_FAILURE);
        }  

            system(CLR);
            cout<<"\n-----------------------------------------------------------------\n";
            cout<<"\n DNS SERVER | 2Stage DNS Resolver \t<DNS Server Port: "<<servPort<<"> \n";
            cout<<"\n-----------------------------------------------------------------";


            printf("\n--->ACCEPT: Connection Established With Client. IP : %s , PORT: %d\n" , inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port)); 
            
            sprintf(DNSShrtMsg, "Connection Established With DNS SERVER: %s, PORT: %d", inet_ntoa(servAddr.sin_addr) , ntohs(servAddr.sin_port) );

            //send new connection greeting message 
            if( send(clntSocketId, DNSShrtMsg, strlen(DNSShrtMsg), 0) != strlen(DNSShrtMsg) )  
            {  
                perror("\nDNS SERVER:SEND: Failed");
                exit(1);  
            }  
        

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
        printf("\nDNS SERVER SOCKET: CONNECTION CLOSED.");
    }
    
    printf("\nDNS SERVER: BYE BYE\n\n");
    return 0;
}// end Main




void HandleTCPClient(int clntSktId)
{
    char DNSRecvBuffer[RCVBUFSIZE] = {0}; // buffer for receiveing string

    char DNSSendBuffer[RCVBUFSIZE] = {0}; // buffer for sending string

    int DNSRecvMsgSize; // size of received message

    bzero(&DNSShrtMsg, sizeof(DNSShrtMsg));

    struct sockaddr_in clntAddr; // Client Address 

    unsigned int clntLen; // Lenght of client address data structure

    clntLen = sizeof(clntAddr);

    int pos;
    string tp, domainname, ipaddress;
    unordered_map<string,string>DNtoIP,IPtoDN;

    ifstream dbFile;

    dbFile.open(DBMapFILENAME, ios::in);

    if (!dbFile) {
        cout << "\n**DNS Database File failed to open:"<<DBMapFILENAME<<"\n\n";
        dbFile.clear();
        return;
    }

    while(getline(dbFile, tp)){ //read data from file object and put it into string.
         pos=tp.find(" ");
        DNtoIP[tp.substr(0,pos)]=tp.substr(pos+1);
        IPtoDN[tp.substr(pos+1)]=tp.substr(0,pos);
    } 

    dbFile.close(); // file close

    getpeername(clntSktId , (struct sockaddr*)&clntAddr, (socklen_t*)&clntLen);  

    if( (DNSRecvMsgSize = recv(clntSktId, DNSRecvBuffer, RCVBUFSIZE, 0)) < 0)
    {
        printf("\nDNS SERVER:RECV: Failed. Client: %s \n", inet_ntoa(clntAddr.sin_addr) );
        return ;
    }

    printf("\n----RECV FROM CLIENT:-----\n%s \n", DNSRecvBuffer);

    string recvString(DNSRecvBuffer); // char arrray to string;

    if(DNSRecvBuffer[0]=='1') // Msg Request of Type 1
     {
        domainname = recvString.substr(1);

        cout<<"\n Domain Name Received: "<<domainname<<"\n";
        
        if(DNtoIP.count(domainname)==1)
        {
            sprintf(DNSSendBuffer, "3%s", DNtoIP[domainname].c_str());
        }
        else
        {
            sprintf(DNSSendBuffer, "4entry not found in the database.");
        }
     }
     else if(DNSRecvBuffer[0]=='2')
     {
        ipaddress = recvString.substr(1);

        cout<<"\n IP Address Received: "<<ipaddress<<"\n";

        if(IPtoDN.count(ipaddress)==1)
        {
            sprintf(DNSSendBuffer, "3%s", IPtoDN[ipaddress].c_str());
        }
        else
        {
            sprintf(DNSSendBuffer, "4entry not found in the database.");
        }
     }
    else if (DNSRecvBuffer[0]=='0' || DNSRecvMsgSize<=0) 
    {
        sprintf(DNSShrtMsg, "DNS SERVER: CLOSING CONNECTION: IP %s, PORT: %d ", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port) );

        send(clntSktId, DNSShrtMsg, sizeof(DNSShrtMsg), 0);
        printf("--\n%s\n--", DNSShrtMsg);
        close(clntSktId);
        return;
    }
    
    if(send(clntSktId, DNSSendBuffer, strlen(DNSSendBuffer), 0) < 0)
    {
        perror("\nDNS SERVER:SEND: Failed");
        return;
    }

    printf("\n----SEND TO CLIENT:-----\n%s \n", DNSSendBuffer);
    
    sprintf(DNSShrtMsg, "DNS SERVER: CLOSING CONNECTION: IP %s, PORT: %d ", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port) );
    printf("--\n%s\n--", DNSShrtMsg);

    close(clntSktId); 
}// end HandleTCPClient