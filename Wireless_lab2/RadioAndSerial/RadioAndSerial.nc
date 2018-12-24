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

  void getRadioPkt() {
    call Leds.led2Toggle();
  }

  void getSerialPkt() {
    call Leds.led0Toggle();
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

  event message_t* RadioReceive.receive(message_t* msg, void* payload, uint8_t len) {
    if (!busy_r && len == sizeof(BlinkToRadioMsg)) {
      BlinkToRadioMsg* ppkt = (BlinkToRadioMsg*)payload;
      BlinkToRadioMsg* new_pkt = (BlinkToRadioMsg*)(call SerialPacket.getPayload(&packet, sizeof(BlinkToRadioMsg)));
      if (new_pkt == NULL) {
        return msg;
      }
      getRadioPkt();
      new_pkt->nodeid = ppkt->nodeid;
      new_pkt->counter = ppkt->counter;
      if (call SerialAMSend.send(new_pkt->nodeid, &packet, sizeof(BlinkToRadioMsg)) == SUCCESS) {
        busy_s = TRUE;
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
      getSerialPkt();
      new_pkt->nodeid = ppkt->nodeid;
      new_pkt->counter = ppkt->counter;
      if (call RadioAMSend.send(new_pkt->nodeid, &packet, sizeof(BlinkToRadioMsg)) == SUCCESS) {
        busy_r = TRUE;
      }
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