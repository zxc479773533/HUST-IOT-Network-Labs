/*
 * HUST-IOT-Network-Labs Wireless::lab2::ReadAndSerial
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#ifndef RADIOANDSERIAL_H
#define RADIOANDSERIAL_H

enum {
  AM_BLINKTORADIOMSG = 6,
};

typedef nx_struct BlinkToRadioMsg {
  nx_uint16_t nodeid;
  nx_uint16_t counter;
} BlinkToRadioMsg;

#endif
