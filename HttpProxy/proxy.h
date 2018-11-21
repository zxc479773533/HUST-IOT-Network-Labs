/*
 * HTTP Proxy::headfile
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#ifndef ZXCPYP_PROXY
#define ZXCPYP_PROXY

#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>

#define HTTP_PORT "80"
#define BUFFER_SIZE 8192

using namespace std;

class HttpProxy {
  const string port;
public:
  HttpProxy(char *listen_port) : port(listen_port){};
  void *get_in_addr(struct sockaddr *sa);  // Get sockaddr, support for IPv4 and IPv6
  int Listen(const char *port);  // Start listen at "port"
  int Accept(int sockfd_listen);  // Accept connections
  bool Send(const int sockfd, const string &http_msg);  // Make sure everything is sent
  int ReadHttpHeader(string &http_msg, string &address, string &dst_port);  // Read HTTP header and get some info
  int HandleRequest(const int sockfd_client);  // Handle HTTP Requests
  int Connect(const string host, const string dst_port);  // Connect to server
  bool Forward(string &host, string &dst_port, const int sockfd_client,
               const string &http_msg);  // Forward HTTP data between client and server
  int Start();  // Start the proxy
};

#endif // !ZXCPYP_PROXY
