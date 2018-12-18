/*
 * HUST-IOT-Network-Labs TinyOS::lab2::BlinkToRadio
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#ifndef BLINKTORADIO_H
#define BLINKTORADIO_H

enum {
  AM_BLINKTORADIOMSG = 6,
  TIMER_PERIOD_MILLI = 1000
};

typedef nx_struct BlinkToRadioMsg {
  nx_uint16_t nodeid;
  nx_uint16_t counter;
} BlinkToRadioMsg;

#endif
