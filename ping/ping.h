/*
 * HUST-IOT-Network-Labs Ping::headfile
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#ifndef ZXCPYP_PING
#define ZXCPYP_PING

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <signal.h>

#define ICMP_HEAD_LEN 8
#define ICMP_DATA_LEN 56
#define ICMP_PACKET_LEN (ICMP_HEAD_LEN + ICMP_DATA_LEN)
#define PACKET_SIZE 4096
#define PACKET_TEST_NUM 10

using namespace std;

class my_ping {
  string domain;
  string ip_address;
  int my_ping_id;
  bool timeout;
  // Count packet
  int timeout_num;
  int send_num;
  int recv_num;
  // Time statistics
  int min_time;
  int avg_time;
  int max_time;
  int mdev_time;
  double total_time;
  // Socket
  int sock_fd;
  struct sockaddr_in from_addr;
  struct sockaddr_in dst_addr;
  // Packet
  char send_packet[PACKET_SIZE];
  char recv_packet[PACKET_SIZE];
  // Receive time
  struct timeval recv_tval;
public:
  my_ping(const char *ip, int timeout);
  unsigned short get_checksum(unsigned short *packet, int len);
  bool create_socket();
  bool close_socket();
  void send_icmp();
  void recv_icmp();
  int unpack(char *buf, int len);
  bool ping(int times);
  void statistics();
  ~my_ping();
};



#endif // !ZXCPYP_PING
