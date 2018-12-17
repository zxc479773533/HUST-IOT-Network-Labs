/*
 * HUST-IOT-Network-Labs TinyOS::lab1::BlinkToRadio1
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

#ifndef BLINKTORADIO_H
#define BLINKTORADIO_H

enum {
  AM_BLINKTORADIO = 6,
  TIMER_PERIOD_MILLI = 1000,
  NOID_ID_1 = 1,
  NOID_ID_2 = 2
};

typedef nx_struct BlinkToRadioMsg {
  nx_uint16_t nodeid;
  nx_uint16_t counter;
} BlinkToRadioMsg;

#endif
