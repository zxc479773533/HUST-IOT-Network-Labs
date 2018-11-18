/*
 * HUST-IOT-Network-Labs Ping::implementation
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#include "ping.h"

my_ping::my_ping(const char *target, int timeout) {
  domain = target;
  ip_address = "";
  my_ping_id = 0;
  timeout = false;
  timeout_num = timeout;
  send_num = 0;
  recv_num = 0;
  min_time = 0;
  avg_time = 0;
  max_time = 0;
  mdev_time = 0;
  total_time = 0;
  sock_fd = 0;
}

unsigned short my_ping::get_checksum(unsigned short *packet, int len) {
  int nleft = len;
  int sum = 0;
  unsigned short answer = 0;
  while (nleft > 1) {
    sum += *packet++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(unsigned char *)(&answer) = *(unsigned char *)packet;
    sum += answer;
  }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  answer = ~sum;
  return answer;
}

bool my_ping::create_socket() {
  close_socket();

  unsigned long in_addr = 0l;
  struct hostent *hostinfo;
  struct protoent *protocol;

  if ((protocol = getprotobyname("icmp")) == NULL) {
    printf("[ERROR] my_ping: Getprotobyname failed!\n");
    return false;
  }

  // Using row socket (NEED ROOT!)
  if ((sock_fd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0) {
    printf("[ERROR] my_ping: Create socket failed!\n");
    return false;
  }

  // Set sockaddr
  bzero(&dst_addr, sizeof(dst_addr));
  dst_addr.sin_family = AF_INET;

  // If use domain
  if ((in_addr = inet_addr(domain.c_str())) == INADDR_NONE) {
    bool ret;
    hostinfo = gethostbyname(domain.c_str());
    if (ret) {
      printf("[ERROR] my_ping: Gethostbyname failed!\n");
      return false;
    }
    memcpy((char *)&dst_addr.sin_addr.s_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
  }
  else
    dst_addr.sin_addr.s_addr = in_addr;

  ip_address = inet_ntoa(dst_addr.sin_addr);
  return true;
}

bool my_ping::close_socket() {
  if (sock_fd != 0)
    close(sock_fd);
  sock_fd = 0;
  return true;
}

void my_ping::send_icmp() {
  struct icmp *icmp_packet;
  struct timeval *tval;
  while (send_num < PACKET_TEST_NUM) {
    send_num++;
    icmp_packet = (struct icmp *)send_packet;
    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_cksum = 0;
    icmp_packet->icmp_seq = send_num;
    icmp_packet->icmp_id = my_ping_id;
    // Set time as data
    tval = (struct timeval *)icmp_packet->icmp_data;
    gettimeofday(tval, NULL);
    // Set checksum
    icmp_packet->icmp_cksum =
        get_checksum((unsigned short *)icmp_packet, ICMP_PACKET_LEN);
    // Send data
    if (sendto(sock_fd, send_packet, ICMP_PACKET_LEN, 0,
               (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0) {
      printf("[ERROR] my_ping: Send failed!\n");
      continue;
    }
    usleep(2);
  }
}

void my_ping::recv_icmp() {
  int read_num;
  int addrlen = sizeof(struct sockaddr_in);
  struct timeval tval;
  while (recv_num < send_num) {
    read_num = recvfrom(sock_fd, recv_packet, sizeof(recv_packet), 0, (struct sockaddr *) &from_addr, (socklen_t *)&addrlen);
    if (read_num <= 0) {
      printf("[ERROR] my_ping: Receive failed!\n");
      return;
    }
    gettimeofday(&recv_tval, NULL);
    if (unpack(recv_packet, read_num) == -1)
      continue;
    recv_num++;
  }
}

int my_ping::unpack(char *buf, int len) {
  int ip_header_len;
  struct ip *ip_packet;
  struct icmp *icmp_packet;
  struct timeval *send_tval;
  double rtt;

  ip_packet = (struct ip *)buf;
  ip_header_len = ip_packet->ip_hl * 4;
  icmp_packet = (struct icmp *)(buf + ip_header_len);
  len -= ip_header_len;
  if (len < 8) {
    printf("[ERROR] my_ping: ICMP packet length error!\n");
    return -1;
  }
  if ((icmp_packet->icmp_type == ICMP_ECHOREPLY) &&
      (icmp_packet->icmp_id == my_ping_id) &&
      (inet_ntoa(from_addr.sin_addr) == ip_address)) {
    send_tval = (struct timeval *)icmp_packet->icmp_data;
    // Get time
    if( (recv_tval.tv_usec-=send_tval->tv_usec)<  0) {       
        --recv_tval.tv_sec;
        recv_tval.tv_usec+=1000000;
    }
    recv_tval.tv_sec-=send_tval->tv_sec;
    // Get rtt
    rtt = recv_tval.tv_sec * 1000.0 + recv_tval.tv_usec / 1000.0;
    total_time += rtt;
    if (max_time == 0 || max_time < rtt)
      max_time = rtt;
    if (min_time == 0 || min_time > rtt)
      min_time = rtt;
    printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n", len,
           inet_ntoa(from_addr.sin_addr), icmp_packet->icmp_seq,
           ip_packet->ip_ttl, rtt);
  }
  return 0;
}

void my_ping::statistics() {
  printf("\n--------------- %s ping statistics ---------------\n", ip_address.c_str());
  printf("%d packets transmitted, %d received , %%%d packet loss, time %.3fms\n", send_num,
           recv_num, (send_num - recv_num) / send_num * 100, total_time);
  close(sock_fd);
  timeout_num = send_num - recv_num;
  avg_time = total_time / recv_num;
}

bool my_ping::ping(int times) {
  bool ret;
  int i = 0;
  int pid;
  while (i < times) {
    ret = create_socket();
    if (!ret) {
      printf("[ERROR] my_ping: Create socket failed!\n");
      return false;
    }
    int pid = getpid();
    pid = pid << 8;
    time_t t;
    time(&t);
    my_ping_id = (pid & 0xff00) | (t & 0xff);
    printf("PING %s(%s): %d bytes data in ICMP packets.\n", domain.c_str(),
           ip_address.c_str(), ICMP_DATA_LEN);
    i++;
    send_num = 0;
    recv_num = 0;
    send_icmp();
    recv_icmp();
    statistics();
    if (recv_num > 0)
      break;
  }
  timeout = false;
  if (recv_num > 0) {
    timeout_num = send_num - recv_num;
    avg_time = total_time / recv_num;
  }
  else {
    timeout_num = send_num;
    avg_time = -1;
    return false;
  }
  return true;
}

my_ping::~my_ping() {
  close_socket();
}
