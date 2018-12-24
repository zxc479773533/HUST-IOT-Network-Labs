/*
 * HUST-IOT-Network-Labs Wireless::lab3::BlinkToRadio
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */
 
#include <Timer.h>
#include "BlinkToRadio.h"

module BlinkToRadioC {
  uses interface Boot;
  uses interface Leds;
  uses interface Timer<TMilli> as Timer0;
  uses interface Packet;
  uses interface AMPacket;
  uses interface AMSend;
  uses interface Receive;
  uses interface SplitControl as AMControl;
}
implementation {

  uint16_t aim_node;
  uint16_t counter;
  message_t pkt;
  bool busy;

  void setLeds(uint16_t val) {
    if (val & 0x01)
      call Leds.led0On();
    else 
      call Leds.led0Off();
    if (val & 0x02)
      call Leds.led1On();
    else
      call Leds.led1Off();
    if (val & 0x04)
      call Leds.led2On();
    else
      call Leds.led2Off();
  }

  event void Boot.booted() {
    call AMControl.start();
  }

  event void AMControl.startDone(error_t err) {
    if (err == SUCCESS) {
      busy = FALSE;
    }
    else {
      call AMControl.start();
    }
  }

  event void AMControl.stopDone(error_t err) {
  }

  event void AMSend.sendDone(message_t* msg, error_t err) {
    if (&pkt == msg) {
      busy = FALSE;
    }
  }

  event void Timer0.fired() {
    if (call AMSend.send(aim_node, &pkt, sizeof(BlinkToRadioMsg)) == SUCCESS) {
      busy = TRUE;
    }
  }

  event message_t* Receive.receive(message_t* msg, void* payload, uint8_t len){
    if (len == sizeof(BlinkToRadioMsg)) {
      BlinkToRadioMsg* btrpkt = (BlinkToRadioMsg*)payload;
      BlinkToRadioMsg* new_pkt = (BlinkToRadioMsg*)(call Packet.getPayload(&pkt, sizeof(BlinkToRadioMsg)));
      if (btrpkt->nodeid == TOS_NODE_ID) {
        setLeds(btrpkt->counter);
        new_pkt->nodeid = 1;
        new_pkt->counter = btrpkt->counter;
        aim_node = TOS_NODE_ID - 1;
        call Timer0.startOneShot(3000);
      }
      else if (btrpkt->nodeid > TOS_NODE_ID) {
        setLeds(btrpkt->nodeid);
        new_pkt->nodeid = btrpkt->nodeid;
        new_pkt->counter = btrpkt->counter;
        aim_node = TOS_NODE_ID + 1;
        call Timer0.startOneShot(1000);
      }
      else {
        setLeds(btrpkt->nodeid);
        new_pkt->nodeid = btrpkt->nodeid;
        new_pkt->counter = btrpkt->counter;
        aim_node = TOS_NODE_ID - 1;
        call Timer0.startOneShot(1000);
      }
    }
    return msg;
  }
}
