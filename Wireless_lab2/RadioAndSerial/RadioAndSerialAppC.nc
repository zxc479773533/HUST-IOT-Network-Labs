/*
 * HUST-IOT-Network-Labs Wireless::lab2::ReadAndSerial
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */
 
#include "RadioAndSerial.h"

configuration RadioAndSerialAppC {
}
implementation {
  components MainC;
  components LedsC;
  components RadioAndSerial as App;
  components ActiveMessageC as Radio;
  components SerialActiveMessageC as Serial;
  components new AMSenderC(AM_BLINKTORADIOMSG);
  components new AMReceiverC(AM_BLINKTORADIOMSG);
  components new SerialAMSenderC(AM_BLINKTORADIOMSG);
  components new SerialAMReceiverC(AM_BLINKTORADIOMSG);

  App.Boot -> MainC;
  App.Leds -> LedsC;

  App.RadioAMControl -> Radio;
  App.SerialControl -> Serial;

  App.RadioPacket -> Radio;
  App.RadioAMSend -> AMSenderC;
  App.RadioReceive -> AMReceiverC;

  App.SerialPacket -> Serial;
  App.SerialAMSend -> SerialAMSenderC;
  App.SerialReceive -> SerialAMReceiverC;
}
