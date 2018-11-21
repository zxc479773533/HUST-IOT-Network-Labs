/*
 * HUST-IOT-Network-Labs HTTP Proxy::implementation
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#include "proxy.h"

// Get sockaddr, support for IPv4 and IPv6
void *HttpProxy::get_in_addr(struct sockaddr *sa) {
  // IPv4
  if (sa->sa_family == AF_INET)
    return &((struct sockaddr_in *)sa)->sin_addr;
  // IPv6
  return &((struct sockaddr_in6 *)sa)->sin6_addr;
}

// Start listen at "port"
int HttpProxy::Listen(const char *port) {
  int sockfd_listen;
  struct addrinfo hints, *servinfo, *addr;
  
  // Set addrinfo
  bzero(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE;      // IP
  hints.ai_protocol = 0;

  // Get addrinfo
  int ret = getaddrinfo(NULL, port, &hints, &servinfo);
  if (ret != 0) {
    printf("[ERROR] HttpProxy: getaddrinfo for client failed!\n");
    return -1;
  }

  // Loop, create and bind
  for (addr = servinfo; addr != NULL; addr = addr->ai_next) {
    sockfd_listen = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sockfd_listen == -1) {
      printf("[ERROR] HttpProxy: socket for client create error!\n");
      continue;
    }

    int sockopt = setsockopt(sockfd_listen, SOL_SOCKET, SO_REUSEADDR, &ret, sizeof(ret));
    if (sockopt == -1) {
      printf("[ERROR] HttpProxy: setsockopt failed!\n");
      exit(-1);
    }

    ret = bind(sockfd_listen, addr->ai_addr, addr->ai_addrlen);
    if (ret == -1) {
      printf("[ERROR] HttpProxy: bind error!\n");
      close(sockfd_listen);
      continue;
    }

    // Once bind succeed, over the loop
    break;
  }

  // All addr create socket or bind failed
  freeaddrinfo(servinfo);
  if (addr == NULL) {
    printf("[ERROR] HttpProxy: bind failed!\n");
    return -1;
  }

  // Listen 
  ret = listen(sockfd_listen, 20);
  if (ret == -1) {
    printf("[ERROR] HttpProxy: listen failed!\n");
    return -1;
  }

  cout << "[SUCCEED] HttpProxy: Listener successfully created!\n";
  return sockfd_listen;
}

// Accept connections
int HttpProxy::Accept(int sockfd_listen) {
  struct sockaddr_storage from_addr;
  socklen_t sin_size = sizeof(from_addr);
  char s[INET6_ADDRSTRLEN];
  int sockfd_client;

  // Wait for connections
  cout << "[SUCCEED] HttpProxy: Waitting for connections...\n";
  sockfd_client = accept(sockfd_listen, (struct sockaddr *)&from_addr, &sin_size);
  if (sockfd_client == -1) {
    printf("[ERROR] HttpProxy: accept failed!\n");
    return -1;
  }

  // Get address
  inet_ntop(from_addr.ss_family, get_in_addr((struct sockaddr *)&from_addr), s, sizeof(s));
  cout << "[SUCCEED] HttpProxy: Got connection from " << s << "\n";

  return sockfd_client;
}

// Make sure everything is sent
bool HttpProxy::Send(const int sockfd, const string &http_msg) {
  const char *buf = http_msg.c_str();
  int send_num;
  size_t i = 0;
  size_t size = http_msg.size();
  while (i < size) {
    send_num = send(sockfd, buf, size - i, 0);
    if (send_num == -1) {
      printf("[ERROR] HttpProxy: send failed!\n");
      return false;
    }
    i += send_num;
  }
  return true;
}

// Format HTTP request header
int HttpProxy::ReadHttpHeader(string &http_msg, string &address, string &dst_port) {
  // Find host
  string header = "Host:";
  size_t pos = http_msg.find(header);
  if (pos != string::npos) {
    // Find hostname
    char hostname[100];
    size_t end = http_msg.find_first_of('\r', pos);
    size_t len = http_msg.copy(hostname, end - pos - header.size() - 1, pos + header.size() + 1);
    hostname[len] = '\0';
    // Get address
    address = string(hostname, hostname + len);
    // Find port
    header = ":";
    pos = address.find(header);
    if (pos != string::npos) {
      char port_str[6];
      size_t end = address.find_first_of('\r', pos);
      size_t len = address.copy(port_str, end - pos - header.size() - 1, pos + header.size());
      port_str[len] = '\0';
      cout << "port: " << dst_port << endl;
      dst_port = string(port_str, port_str + len);
    }
    else {
      dst_port = HTTP_PORT;
    }
    // Find GET
    header = "GET";
    pos = http_msg.find(header);
    if (pos != string::npos) {
      // Find URI
      char uri_str[1024];
      size_t end = http_msg.find_first_of('\r', pos);
      size_t len = http_msg.copy(uri_str, end - pos - header.size() - 1, pos + header.size() + 1);
      uri_str[len] = '\0';
      string uri = string(uri_str, uri_str + len);
      size_t check = uri.find(address);
      if (check != string::npos)
        http_msg.replace(pos + header.size() + 1, check + address.size() - pos, "");
      else {
        cout << "[INFO] No " << address << " after GET.\n";
        return -1;
      }
    }
    else {
      cout << "[INFO] Hostname not found!\n";
      return -1;
    }
  }

  // Set connection to close
  header = "Connection:";
  pos = http_msg.find(header);
  if (pos != string::npos) {
    size_t end = http_msg.find_first_of("\r", pos);
    http_msg.replace(pos + header.size() + 1, end - pos - header.size() - 1, "close");
  }
  else {
    cout << "[INFO] Connection header not found!\n";
    return -1;
  }

  if (address.empty() || http_msg.size() < 8)
    return -1;
  
  return 0;
}

// Handle HTTP Requests
int HttpProxy::HandleRequest(const int sockfd_client) {
  int recv_num;
  char buf[BUFFER_SIZE];
  string request = "";

  recv_num = recv(sockfd_client, buf, sizeof(buf), 0);
  if (recv_num == -1) {
    printf("[ERROR] HttpProxy: recv from client failed!\n");
    return -1;
  }

  cout << "[SUCCEED] Bytes received from client: " << recv_num << "\n";

  request += string(buf, buf + recv_num);
  request += '\0';

  string hostname = "";
  string dst_port = "";
  if (ReadHttpHeader(request, hostname, dst_port) == -1) {
    printf("[ERROR] HttpProxy: handle request error!\n");
    return -1;
  }
  if (!Forward(hostname, dst_port, sockfd_client, request)) {
    printf("[ERROR] HttpProxy: forward data error!\n");
    return -1;    
  }

  return 0;
}

// Connect to server
int HttpProxy::Connect(const string host, const string dst_port) {
  int ret;
  struct addrinfo hints, *servinfo, *res;
  char s[INET6_ADDRSTRLEN];
  int sockfd_server;

  bzero(&hints, sizeof(hints));
  hints.ai_family = AF_INET;        // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE;      // IP
  hints.ai_protocol = 0;

  // Change string to cstr
  char *chost = (char*)host.c_str();
  char *cport = (char*)dst_port.c_str();

  // Get addrinfo
  ret = getaddrinfo(chost, cport, &hints, &servinfo);
  if (ret != 0) {
    printf("[ERROR] HttpProxy: getaddrinfo for server failed!\n");
    return -1;
  }

  // Loop, create and bind
  for (res = servinfo; res != NULL; res = res->ai_next) {
    sockfd_server = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd_server == -1) {
      printf("[ERROR] HttpProxy: socket for server create error!\n");
      continue;
    }
    int sockopt = setsockopt(sockfd_server, SOL_SOCKET, SO_REUSEADDR, &ret, sizeof(ret));
    if (sockopt == -1) {
      printf("[ERROR] HttpProxy: setsockopt failed!\n");
      exit(-1);
    }
    ret = connect(sockfd_server, res->ai_addr, res->ai_addrlen);
    if (ret == -1) {
      printf("[ERROR] HttpProxy: connect to servet error!\n");
      close(sockfd_server);
      continue;
    }
    // Once bind succeed, over the loop
    break;
  }

  // All addr create socket or bind failed
  freeaddrinfo(servinfo);
  if (res == NULL) {
    printf("[ERROR] HttpProxy: connect to server failed!\n");
    return -1;
  }

  // Get address
  inet_ntop(res->ai_family, get_in_addr((struct sockaddr *)res->ai_addr), s, sizeof(s));
  cout << "[SUCCEED] Connecting to server " << host << "\n";

  return sockfd_server;
}

// Forward HTTP data between client and server
bool HttpProxy::Forward(string &host, string &dst_port, const int sockfd_client, const string &http_msg) {
  string sever_response = "";

  // Connect to server
  int sockfd_server = Connect(host, dst_port);
  if (sockfd_server == -1) {
    printf("[ERROR] HttpProxy: connect to server error!\n");
    close(sockfd_server);
    return false;
  }

  // Send HTTP message to server
  if (Send(sockfd_server, http_msg) == false)
    return false;

  int total_byte_num = 0;
  while (true) {
    int recv_num;
    char buf[8192];

    recv_num = recv(sockfd_server, buf, sizeof(buf), 0);
    if (recv_num == -1) {
      printf("[ERROR] HttpProxy: recv from server failed!\n");
      return -1;
    }
    sever_response = string(buf, buf + recv_num);
    total_byte_num += recv_num;

  
    if (recv_num == 0)
      break;

    // Send HTTP message to client
    if (Send(sockfd_client, sever_response) == false)
      return false;
  }

  cout << "[SUCCEED] Forward data succeed! Bytes: " << total_byte_num <<"\n";

  close(sockfd_server);
  close(sockfd_client);
  return true;
}

// Start the proxy
int HttpProxy::Start() {
  cout << "[INFO] HTTP Proxy start at port: " << port << "\n";
  int sockfd_listen;
  sockfd_listen = Listen(port.c_str());
  if (sockfd_listen == -1)
    return -1;
  while (true) {
    int sockfd_client = Accept(sockfd_listen);
    if (sockfd_client >= 0) {
      if (!fork()) {
        close(sockfd_listen);
        HandleRequest(sockfd_client);
        exit(0);
      }
    }
    close(sockfd_client);
  }
  return 0;
}
