/*
 * HUST-IOT-Network-Labs Ping::main
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#include "ping.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./my_ping <domain>/<ip address>\n");
    exit(0);
  }
  my_ping new_ping(argv[1], 100);
  new_ping.ping(20);
  return 0;
}