/*
 * HUST-IOT-Network-Labs Wireless::lab2::ReadAndSerial
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */
#include "RadioAndSerial.h"

module RadioAndSerial {
  uses interface Boot;
  uses interface Leds;
  uses interface Timer<TMilli> as Timer0;
  uses interface Timer<TMilli> as Timer1;
  // Radio
  uses interface Packet as RadioPacket;
  uses interface AMSend as RadioAMSend;
  uses interface Receive as RadioReceive;
  uses interface SplitControl as RadioAMControl;
  // Serial
  uses interface Packet as SerialPacket;
  uses interface AMSend as SerialAMSend;
  uses interface Receive as SerialReceive;
  uses interface SplitControl as SerialControl;
}
implementation {

  // Radio packet and Serial packet
  message_t packet;
  // Busy mark for Radio and Serial
  bool busy_r;
  bool busy_s;
  // Send aim node
  uint16_t aim_node;

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
  // Start control
  event void Boot.booted() {
    call RadioAMControl.start();
    call SerialControl.start();
  }

  event void RadioAMControl.startDone(error_t err) {
    if (err == SUCCESS) {
      busy_r = FALSE;
    }
    else {
      call RadioAMControl.start();
    }
  }

  event void SerialControl.startDone(error_t err) {
    if (err == SUCCESS) {
      busy_s = FALSE;
    }
    else {
      call SerialControl.start();
    }
  }

  event void RadioAMControl.stopDone(error_t err) {
  }

  event void SerialControl.stopDone(error_t err) {
  }

  event void Timer0.fired() {
    if (call SerialAMSend.send(AM_BROADCAST_ADDR, &packet, sizeof(BlinkToRadioMsg)) == SUCCESS) {
      busy_s = TRUE;
    }
  }

  event void Timer1.fired() {
    if (call RadioAMSend.send(aim_node, &packet, sizeof(BlinkToRadioMsg)) == SUCCESS) {
      busy_r = TRUE;
    }
  }

  event message_t* RadioReceive.receive(message_t* msg, void* payload, uint8_t len) {
    if (!busy_r && len == sizeof(BlinkToRadioMsg)) {
      BlinkToRadioMsg* ppkt = (BlinkToRadioMsg*)payload;
      BlinkToRadioMsg* new_pkt = (BlinkToRadioMsg*)(call SerialPacket.getPayload(&packet, sizeof(BlinkToRadioMsg)));
      if (new_pkt == NULL) {
        return msg;
      }
      new_pkt->nodeid = ppkt->nodeid;
      new_pkt->counter = ppkt->counter;
      if (new_pkt->nodeid == TOS_NODE_ID) {
        setLeds(new_pkt->nodeid);
        call Timer0.startOneShot(3000);
      }
      else {
        if (ppkt->nodeid > TOS_NODE_ID) {
          setLeds(ppkt->nodeid);
          aim_node = TOS_NODE_ID + 1;
          call Timer1.startOneShot(1000);
        }
        else {
          setLeds(ppkt->nodeid);
          aim_node = TOS_NODE_ID - 1;
          call Timer1.startOneShot(1000);
        }
      }
    }
    return msg;
  }

  event message_t* SerialReceive.receive(message_t* msg, void* payload, uint8_t len) {
    if (!busy_s && len == sizeof(BlinkToRadioMsg)) {
      BlinkToRadioMsg* ppkt = (BlinkToRadioMsg*)payload;
      BlinkToRadioMsg* new_pkt = (BlinkToRadioMsg*)(call RadioPacket.getPayload(&packet, sizeof(BlinkToRadioMsg)));
      if (new_pkt == NULL) {
        return msg;
      }
      setLeds(ppkt->nodeid);
      new_pkt->nodeid = ppkt->nodeid;
      new_pkt->counter = ppkt->counter;
      aim_node = 2;
      call Timer1.startOneShot(1000);
    }
    return msg;
  }

  event void RadioAMSend.sendDone(message_t* msg, error_t err) {
    if (&packet == msg) {
      busy_r = FALSE;
    }
  }

  event void SerialAMSend.sendDone(message_t* msg, error_t err) {
    if (&packet == msg) {
      busy_s = FALSE;
    }
  }
}