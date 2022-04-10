/****
 * Socket Programming: 
 * IITG
**/ 
#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <cstring> // strlen(), memset()
#include <unordered_map>
#include <limits>

#include <stdio.h>  // printf() and fprintf()
#include <stdlib.h> // exit(), atoi()
// #include <string.h> // strlen(), memset()

#include <unistd.h> // read(), close()
#include <sys/socket.h> // socket(), bind(), connect(), recv() and send()
#include <arpa/inet.h> // sockaddr_in and inet_ntoa()

#include<sys/time.h> // for struct timeval {}, FD_SET, FD_ISSET, FD_ZERO macros

#include <netdb.h> // for server ip address
// #include <netinet/in.h> 

#include <ifaddrs.h>
#include <errno.h> 

using namespace std;

#define MAXPENDING 5 // Maximum outstanding connection requests
#define RCVBUFSIZE 1030 //Size of receive buffer
//DNS
#define DefDNSIPADDR "127.0.1.4" // 127.0.1.4 DNS SERVER IP ADDRESS | Use this in case of failure
#define DBMapFILENAME "database_mappings.txt"
// proxy
#define DefProxySERVIPADDR "127.0.1.5" // // SERVER IP ADDRESS 127.0.1.5 172.16.115.196
#define DNSSERVERPORT 8080
#define DNSSERVIPADDR "127.0.1.4"  //127.0.1.4 DNS
#define MAXCLIENTS 30 // max clients to handle
#define CACHE_SIZE 3    // proxy cache size
#define ProxyCacheFILENAME "proxy_cache.txt"

#if defined(_WIN32)
    #define PAUSE "pause"
    #define CLR "cls"
#elif defined(unix) || defined(__unix__) || defined(__unix)
    #define PAUSE "read -p 'Press Enter to continue...' var"
    #define CLR "clear"
#endif

#define debug 0
