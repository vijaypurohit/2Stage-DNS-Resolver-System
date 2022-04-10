// /****
//  * Socket Programming: Server Multi Client C Program using select()
//  * IITG
// **/ 
// #include <iostream>
// #include <string>
// #include <list>
// #include <fstream>

// #include <stdio.h>  // printf() and fprintf()
// #include <stdlib.h> // exit(), atoi()
// #include <string.h> // strlen(), memset()

// #include <unistd.h> // read(), close()
// #include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
// #include <arpa/inet.h> // sockaddr_in and inet_ntoa()

// #include<sys/time.h> // for struct timeval {}, FD_SET, FD_ISSET, FD_ZERO macros

// #include <netdb.h> // for server ip address
// // #include <netinet/in.h> 

// #include <errno.h> 

// using namespace std;

// #define MAXPENDING 5 // Maximum outstanding connection requests
// #define RCVBUFSIZE 1030 //Size of receive buffer
// #define DefProxySERVIPADDR "127.0.1.5" // // SERVER IP ADDRESS
// #define DNSSERVERPORT 8080
// #define DNSSERVIPADDR "127.0.1.2"
// #define MAXCLIENTS 30 // max clients to handle
// #define CACHE_SIZE 3    // proxy cache size
// #define ProxyCacheFILENAME "proxy_cache.txt"

// char recvBuffer[RCVBUFSIZE]; // buffer for echo string
// char sendBuffer[RCVBUFSIZE]; // buffer for echo string

#include "allHeaders.h"

list<pair<string,string>> cache;
char recvBuffer[RCVBUFSIZE]; // buffer for echo string
char sendBuffer[RCVBUFSIZE]; // buffer for echo string
char ProxyShrtMsg[RCVBUFSIZE];

void proxy_to_dnsserver();


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

    servPort = atoi(argv[1]);
    if(servPort<0 || servPort >65535 || servPort==DNSSERVERPORT)
    {
        servPort = 8090;
    }

    cout<<"\n-----------------------------------------------------------------\n";
    cout<<"\n PROXY SERVER | 2Stage DNS Resolver \t<Proxy Server Port: "<<servPort<<"> \n";
    cout<<"\n-----------------------------------------------------------------";

    if((servSocketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\n--->PROXY SERVER SOCKET CREATION: Failed....\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n--->PROXY SERVER SOCKET CREATION: Successful, SocketID: %d ..\n", servSocketId);
    }


// Retrieve Server IP Address
    // char hostbuffer[256];
    // char *IPAddressBuffer;
    // struct hostent *hostObj;
    // // To retrieve host information
    // int hostname;
    // // To retrieve hostname
    // hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    
    // hostObj = gethostbyname(hostbuffer);

    // IPAddressBuffer = inet_ntoa(*((struct in_addr*)hostObj->h_addr_list[0]));
    // if (NULL == IPAddressBuffer)
    // {
    //     perror("PROXY SERVER IP Address Could not be Found. Using Default. inet_ntoa");
    //     // IPAddressBuffer = "127.0.0.1";
    //     strcpy(IPAddressBuffer, DefProxySERVIPADDR);
    //     // exit(1);
    // }

    
    servAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    servAddr.sin_port = htons( servPort );       //Address port (16 bits) : chooses any incoming interface 
    servAddr.sin_addr.s_addr = inet_addr(DefProxySERVIPADDR);    //Internet address (32 bits) //INADDR_ANY

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
    else
    {
        printf("\n--->LISTEN: Server listening on address: %s ....\n", inet_ntoa(servAddr.sin_addr));
         // wait for a client to connect
        printf("\n---------PROXY SERVER: Waiting For Connection.---------\n");
    }


    fd_set readSockSet;  //set of socket descriptors 
    int maxDescriptor = -1; //Maximum socket descriptor value
    int clntSocketIds[MAXCLIENTS] = {0};

    for (int c = 0; c < MAXCLIENTS; c++)      //Initialize all_connections
    {  
        clntSocketIds[c] = 0;  
    }  

    clntLen = sizeof(clntAddr); //Set the size of the in-out parameter

    while(1)
    {
        int recvMsgSize; // size of received message

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

        if (((select( maxDescriptor + 1 , &readSockSet , NULL , NULL , NULL)) < 0) && (errno!=EINTR))  
        {  
            printf("\nSELECT: Failed");
        }  

        //If something happened on the master socket, then its an incoming connection 
        if (FD_ISSET(servSocketId, &readSockSet))  //vector membership check
        {  

            if ((clntSocketId = accept(servSocketId,(struct sockaddr *)&clntAddr, (socklen_t*)&clntLen))<0)  
            {  
                perror("\nACCEPT: Failed");
                exit(EXIT_FAILURE); 
            }  

            //inform user of socket number - used in send and receive commands 
            printf("\n--> ACCEPT: Connection Established With Client. IP : %s , PORT: %d \n" , inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port)); 
            
            sprintf(ProxyShrtMsg, "Connection Established With PROXY SERVER: %s, PORT: %d ", inet_ntoa(servAddr.sin_addr) , ntohs(servAddr.sin_port) );

            //send new connection greeting message to client
            if( send(clntSocketId, ProxyShrtMsg, RCVBUFSIZE, 0) != RCVBUFSIZE )  
            {  
                perror("\nProxy SERVER:SEND: Failed");
                break;  
            }  
            
            for (int i = 0; i < MAXCLIENTS; i++)  //add new socket to array of sockets 
            {  
                if( clntSocketIds[i] == 0 )  //if position is empty 
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
                system(CLR);

                fflush(stdin);    
                bzero(recvBuffer, RCVBUFSIZE);
                bzero(sendBuffer, RCVBUFSIZE);

                string ptr, domainname, ipaddress;
                int pos=0;
                int cacheFindFlag=0;
                ifstream cacheFile;
                cacheFile.open(ProxyCacheFILENAME,ios::in);

                if(!cacheFile){
                    cout<<"Error in  accessing cache!\n";
                }
                
                cache.clear();

                while(getline(cacheFile,ptr)){
                    pos=ptr.find(" ");
                    cache.push_back(make_pair(ptr.substr(0,pos),ptr.substr(pos+1)));
                }
                
                list<pair<string,string>>::iterator it;
                if(debug)
                {
                    cout<<"\nCACHE:\n";
                    
                    for (it = cache.begin(); it != cache.end(); ++it){
                        cout<<it->first<<" "<<it->second<<"\n";
                    }
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
                // printf("\n----RECV msg size %d, len %ld:----- \n",recvMsgSize, strlen(recvBuffer));
                
                string recvString(recvBuffer);   //char array to string 
               
                if(recvBuffer[0]=='1'){          //checking request type 

                    domainname=recvString.substr(1);
                    cout<<"\n Domain Name Received: "<<domainname<<"\n";
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
                    cout<<"\n IP Address Received: "<<ipaddress<<"\n";
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

                if(cacheFindFlag==0 && recvBuffer[0]!='0' && recvMsgSize>0)
                    proxy_to_dnsserver();


                
                 if (recvBuffer[0]=='0' || recvMsgSize<=0) 
                {

                    sprintf(ProxyShrtMsg, "PROXY SERVER: CLIENT CLOSING CONNECTION: IP %s, PORT: %d ", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port) );

                    // IP %s, PORT %d: \n", inet_ntoa(clntAddr.sin_addr) , ntohs(clntAddr.sin_port));

                    printf("--\n%s\n--", ProxyShrtMsg);
                    send(sd, ProxyShrtMsg, strlen(ProxyShrtMsg), 0);

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

    if(close(servSocketId) == 0)
    {
        printf("\nSERVER SOCKET: CONNECTION CLOSED.");
    }
    
    printf("\nSERVER: BYE BYE\n\n");
    return 0;
}// end Main


void proxy_to_dnsserver(){
    
    int clntSktId; //Client Socket Descriptor

    struct sockaddr_in dnsServAddr; // Local Server Address
     
    bzero(&dnsServAddr, sizeof(dnsServAddr));
    
    printf("\n Miss in Proxy server cache, Sending request to DNS server.\n");

    if((clntSktId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\nProxy CLIENT SOCKET CREATION: Failed....\n");
        exit(EXIT_FAILURE);
    }
    // else
    // {
    //     printf("\n--->Proxy CLIENT SOCKET CREATION: Successfully Created, ID: %d ..\n", clntSktId);
    // }

    dnsServAddr.sin_family = AF_INET; //Internet protocol (AF_INET)
    dnsServAddr.sin_port = htons( DNSSERVERPORT );       //Internet address (32 bits)
    dnsServAddr.sin_addr.s_addr = inet_addr(DNSSERVIPADDR);

    if (connect(clntSktId, (struct sockaddr *)&dnsServAddr, sizeof(dnsServAddr)) < 0)
    {
        perror("\nProxy CONNECT: Failed With DNS Server");
        exit(EXIT_FAILURE);
    }
    else
    {   
   
        char wlcmMsg[RCVBUFSIZE]={0};
        // see if more message to receive
        if( recv(clntSktId, wlcmMsg, RCVBUFSIZE, 0) < 0)
        {
            perror("\nProxy CLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        printf("\n---> %s\n", wlcmMsg);
    }

    if( (send(clntSktId, recvBuffer, strlen(recvBuffer), 0)) < 0)
        {
            perror("\nProxy CLIENT:SEND: Failed");
            return;
        }

        cout<<"\n-----SEND TO DNS SERVER "<<inet_ntoa(dnsServAddr.sin_addr)<<", "<<ntohs(dnsServAddr.sin_port)<<":----\n"<<recvBuffer<<endl;

      bzero(sendBuffer, RCVBUFSIZE);
       
    // see if more message to receive
        if( recv(clntSktId, sendBuffer, RCVBUFSIZE, 0) < 0)
        {
            perror("\nProxy CLIENT:RECV: Failed");
            exit(EXIT_FAILURE);
        }

        printf("\n--->DNS SERVER: %s\n", sendBuffer);
    
    string recvString(recvBuffer);
    string sendString(sendBuffer);

    if(sendBuffer[0]=='3'){
        if(cache.size()<3){
            // string recvString(recvBuffer);
            // string sendString(sendBuffer);
            if(recvBuffer[0]=='1'){
                cache.push_back(make_pair(sendString.substr(1),recvString.substr(1)));
            }
            else if(recvBuffer[0]=='2'){
                cache.push_back(make_pair(recvString.substr(1),sendString.substr(1)));
            }
        }
        else{
            cache.pop_front();
            // string recvString(recvBuffer);
            // string sendString(sendBuffer);
            if(recvBuffer[0]=='1'){
                cache.push_back(make_pair(sendString.substr(1),recvString.substr(1)));
            }
            else if(recvBuffer[0]=='2'){
                cache.push_back(make_pair(recvString.substr(1),sendString.substr(1)));
            }
        }

        list<pair<string,string>>::iterator it;
        if(debug)
        {
            for (it = cache.begin(); it != cache.end(); ++it){
                    cout<<it->first<<" "<<it->second<<"\n";
                }
        }

        ofstream cacheFile;
        cacheFile.open(ProxyCacheFILENAME,ofstream::out);
        // writing into cache
        for(it=cache.begin(); it!=cache.end(); ++it){
            cacheFile << it->first <<" " <<it->second<<"\n";
            // if(++it==cache.end())
            //     break;
            // cout<<"\n";
        }
        cacheFile.close();

    }



    if(close(clntSktId) == 0)
    {
        printf("\n--->Proxy CLIENT SOCKET: CONNECTION CLOSED.");
    }

    printf("\nProxy CLIENT: EXIT.\n\n");


}