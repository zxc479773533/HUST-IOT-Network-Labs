/*
 * HUST-IOT-Network-Labs Wireless::lab2::BlinkToRadio
 * 
 * Created by zxcpyp
 * 
 * Github: zxc479773533
 */

import java.io.IOException;
import java.util.Scanner; 

import net.tinyos.message.*;
import net.tinyos.packet.*;
import net.tinyos.util.*;

public class BlinkToRadio implements MessageListener {

  private MoteIF moteIF;

  public BlinkToRadio(MoteIF moteIF) {
    this.moteIF = moteIF;
    this.moteIF.registerListener(new BlinkToRadioMsg(), this);
  }

  public void sendPackets(int nodeid, int counter) {
    BlinkToRadioMsg payload = new BlinkToRadioMsg();
    try {
      payload.set_nodeid(nodeid);
      payload.set_counter(counter);
      moteIF.send(nodeid, payload);
    } catch (IOException exception) {
      System.err.println("Exception thrown when sending packets. Exiting.");
      System.err.println(exception);
    }
  }

  public void messageReceived(int to, Message message) {
    BlinkToRadioMsg msg = (BlinkToRadioMsg)message;
    System.out.println("\nReceived packet to: " + to + " nodeid: " + msg.get_nodeid() + " counter: " + msg.get_counter());
    System.out.println("Input the nodeid and counter(like 1  2): ");
    Scanner sc = new Scanner(System.in);
    int nodeid = sc.nextInt();
    int counter = sc.nextInt();
    System.out.println("Sending to " + nodeid + " number is: " + counter);
    sendPackets(nodeid, counter);
  }

  private static void usage() {
    System.err.println("usage: BlinkToRadio [-comm <source>]");
  }

  public static void main(String[] args) throws Exception {
    String source = null;
    if (args.length == 2) {
      if (!args[0].equals("-comm")) {
        usage();
        System.exit(1);
      }
      source = args[1];
    } else if (args.length != 0) {
      usage();
      System.exit(1);
    }

    PhoenixSource phoenix;

    if (source == null) {
      phoenix = BuildSource.makePhoenix(PrintStreamMessenger.err);
    } else {
      phoenix = BuildSource.makePhoenix(source, PrintStreamMessenger.err);
    }

    MoteIF mif = new MoteIF(phoenix);
    BlinkToRadio radio = new BlinkToRadio(mif);
  }
}