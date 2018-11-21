/*
 * HUST-IOT-Network-Labs HTTP Proxy::main
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#include "proxy.h"

int main(int argc, char **argv) {
  int port;
  if (argc != 2) {
    printf("Usage: ./http_proxy <port>\n");
    exit(0);
  }
  port = atoi(argv[1]);
  if (port < 1024 || port > 65535) {
    printf("[ERROR] HttpProxy: Please set a port between 1024 to 65535");
    exit(0);
  }

  // Start Http Proxy
  HttpProxy proxy(argv[1]);
  proxy.Start();

  return 0;
}