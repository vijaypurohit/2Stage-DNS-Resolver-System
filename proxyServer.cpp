/****
 * Socket Programming: Server Multi Client C Program using select()
 * IITG
**/ 
#include <iostream>
#include <string>
#include <list>
#include <fstream>

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

using namespace std;

#define MAXPENDING 5 // Maximum outstanding connection requests
#define RCVBUFSIZE 1030 //Size of receive buffer
#define SERVIPADDR "127.0.0.1" // // SERVER IP ADDRESS
#define DNSSERVERPORT 8080
#define DNSSERVIPADDR "127.0.1.1"
#define MAXCLIENTS 30
#define CACHE_SIZE 3
#define MAX 255
#define FILENAME "proxy_cache.txt"

char recvBuffer[RCVBUFSIZE]; // buffer for echo string
char sendBuffer[RCVBUFSIZE]; // buffer for echo string

void proxy_server(int , int , int );
void proxy_to_dnsserver();

list<pair<string,string>> cache;

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
    if(servPort<0 || servPort >65535 || servPort==DNSSERVERPORT)
    {
        servPort = 4492;
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
                bzero(sendBuffer, RCVBUFSIZE);

                
                string ptr, domainname, ipaddress;
                int pos=0;
                int cacheFindFlag=0;
                ifstream cacheFile;
                cacheFile.open(FILENAME,ios::in);

                if(!cacheFile){
                    cout<<"Error in  accessing cache!\n";
                }
                
                cache.clear();

                while(getline(cacheFile,ptr)){
                    pos=ptr.find(" ");
                    cache.push_back(make_pair(ptr.substr(0,pos),ptr.substr(pos+1)));
                }
                
                list<pair<string,string>>::iterator it;
                for (it = cache.begin(); it != cache.end(); ++it){
                    cout<<it->first<<" "<<it->second<<"\n";
                }
                cacheFile.close();

                getpeername(sd , (struct sockaddr*)&clntAddr, (socklen_t*)&clntLen);  

                if( (recvMsgSize = recv(sd, recvBuffer, RCVBUFSIZE, 0)) < 0)
                {
                    printf("\nSERVER:RECV: Failed. Client: IP %s , PORT %d \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port));
                    break ;
                }
                else
                printf("\n----RECV FROM CLIENT IP %s, PORT %d:-----\n%s \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port), recvBuffer);
                printf("\n----RECV msg size %d, len %ld:----- \n",recvMsgSize, strlen(recvBuffer));
                
                string recvString(recvBuffer);   //char array to string 
               
                if(recvBuffer[0]=='1'){          //checking request type 

                    domainname=recvString.substr(1);
                    cout<<"::Domain Name Received: "<<domainname<<"\n";
                    for (it = cache.begin(); it != cache.end(); ++it){
                        string address=it->second;
                        if(domainname.compare(address)==0){
                            cacheFindFlag=1;
                            sprintf(sendBuffer,"3%s",it->first.c_str());
                        }
                    }
                    
                    if(cacheFindFlag==0){
                        sprintf(sendBuffer,"4entry not found in the database.");
                    }

                }
                else if(recvBuffer[0]=='2'){
                    ipaddress = recvString.substr(1);
                    cout<<"::IP Address Received: "<<ipaddress<<"\n";
                    list<pair<string,string>>::iterator it;
                    for (it = cache.begin(); it != cache.end(); ++it){
                        if(it->first.compare(ipaddress)==0){
                            cacheFindFlag=1;
                            sprintf(sendBuffer,"3%s",it->second.c_str());
                        }
                    }
                    if(cacheFindFlag==0){
                        sprintf(sendBuffer,"4entry not found in the database.");
                    }
                }

                if(cacheFindFlag==0 && recvBuffer[0]!='0')
                    proxy_to_dnsserver();


                
                 if (recvBuffer[0]=='0' || recvMsgSize<=0) 
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
                    if(send(sd, sendBuffer, strlen(sendBuffer), 0) < 0)
                    {
                        perror("\nSERVER:SEND To Client: Failed");
                        break;
                    }
                    else
                    printf("\n----SEND TO CLIENT IP %s, PORT %d:-----\n%s \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port), sendBuffer);
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


void proxy_to_dnsserver(){
    // int servSocketId; //Server Socket Descriptor, for return value of the socket function call
    int clntSktId; //Client Socket Descriptor

    struct sockaddr_in dnsServAddr; // Local Server Address
    // struct sockaddr_in clntAddr; // Client Address    
    bzero(&dnsServAddr, sizeof(dnsServAddr));
    // bzero(&clntAddr, sizeof(clntAddr));
    // unsigned short servPort; // Server Port

    // unsigned int clntLen; // Lenght of client address data structure

    // char shrtMsg[RCVBUFSIZE];

    // servPort=DNSSERVERPORT;
    printf("Miss in Proxy server cache, sending request to DNS server.\n");
    if((clntSktId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\nCLIENT SOCKET CREATION: Failed....\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n--->CLIENT SOCKET CREATION: Successfully Created, ID: %d ..\n", clntSktId);
    }


    dnsServAddr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY;   //Address port (16 bits) : chooses any incoming interface


    dnsServAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    dnsServAddr.sin_port = htons( DNSSERVERPORT );       //Internet address (32 bits)
    dnsServAddr.sin_addr.s_addr = inet_addr(DNSSERVIPADDR);

    if (connect(clntSktId, (struct sockaddr *)&dnsServAddr, sizeof(dnsServAddr)) < 0)
    {
        perror("\nCONNECT: Failed With Server");
        exit(EXIT_FAILURE);
    }
    else
    {   
    //converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
        // printf("\nCONNECT: Connection Established With Server: %s \n\n", inet_ntoa(servAddr.sin_addr));

        char wlcmMsg[RCVBUFSIZE];
    // see if more message to receive
        if( recv(clntSktId, wlcmMsg, RCVBUFSIZE, 0) < 0)
        {
            perror("\nCLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        printf("\n--->SERVER: %s\n", wlcmMsg);
    }

    //converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
        // printf("\nCONNECT: Connection Established With Server: %s \n\n", inet_ntoa(servAddr.sin_addr));

    if( (send(clntSktId, recvBuffer, strlen(recvBuffer), 0)) < 0)
        {
            perror("\nCLIENT:SEND: Failed");
            return;
        }

        cout<<"\n-----SEND TO SERVER:----\n"<<recvBuffer<<endl;

      bzero(sendBuffer, RCVBUFSIZE);
       
    // see if more message to receive
        if( recv(clntSktId, sendBuffer, RCVBUFSIZE, 0) < 0)
        {
            perror("\nCLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        printf("\n--->SERVER: %s\n", sendBuffer);
    
    if(sendBuffer[0]=='3'){
        if(cache.size()<3){
            string recvString(recvBuffer);
            string sendString(sendBuffer);
            if(recvBuffer[0]=='1'){
                cache.push_back(make_pair(sendString.substr(1),recvString.substr(1)));
            }
            else if(recvBuffer[0]=='2'){
                cache.push_back(make_pair(recvString.substr(1),sendString.substr(1)));
            }
        }
        else{
            cache.pop_front();
            string recvString(recvBuffer);
            string sendString(sendBuffer);
            if(recvBuffer[0]=='1'){
                cache.push_back(make_pair(sendString.substr(1),recvString.substr(1)));
            }
            else if(recvBuffer[0]=='2'){
                cache.push_back(make_pair(recvString.substr(1),sendString.substr(1)));
            }
        }
         list<pair<string,string>>::iterator it;
        for (it = cache.begin(); it != cache.end(); ++it){
                    cout<<it->first<<" "<<it->second<<"\n";
                }
        ofstream cacheFile;
                cacheFile.open(FILENAME,ofstream::out);
       
        for(it=cache.begin();it!=cache.end();it++){
            cacheFile << it->first <<" " <<it->second<<"\n";
        }
       // cacheFile << it->first <<" " <<it->second<<"\n";
        cacheFile.close();

    }



    if(close(clntSktId) == 0)
    {
        printf("\n--->CLIENT SOCKET: CONNECTION CLOSED.");
    }

    printf("\nCLIENT: BYE BYE.\n\n");


}