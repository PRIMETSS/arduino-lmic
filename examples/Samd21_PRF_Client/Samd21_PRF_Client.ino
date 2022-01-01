// Does RAW Serial, seems to lock on TX (doesnt cycle through TX then RX) if LoraWAN jumpers set on SAMD21 RF board!

/*
  Both the TX and RX ProRF boards will need a wire antenna. We recommend a 3" piece of wire.
  This example is a modified version of the example provided by the Radio Head
  Library which can be found here:
  www.github.com/PaulStoffregen/RadioHeadd
*/

#include <SPI.h>

//Radio Head Library:
#include <RH_RF95.h> 

// We need to provide the RFM95 module's chip select and interrupt pins to the
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

int LED = 13; //Status LED is on pin 13

int packetCounter = 0; //Counts the number of packets sent
long lastPacketMills = 0; //Tracks the time stamp of last packet received
long lastStatusLEDOfftMills = millis(); //Tracks the time stamp of the status LED off time

// The broadcast frequency is set to 921.2, but the SADM21 ProRf operates
// anywhere in the range of 902-928MHz in the Americas.
// Europe operates in the frequencies 863-870, center frequency at 868MHz.
// This works but it is unknown how well the radio configures to this frequency:
// https://www.semtech.com/products/wireless-rf/lora-core/sx1276

float frequency = 915.00;// Gateway repeater, centre frequency!

void setup()
{
  pinMode(LED, OUTPUT);

  SerialUSB.begin(9600);
  // It may be difficult to read serial messages on startup. The following line
  // will wait for serial to be ready before continuing. Comment out if not needed.
  //while(!SerialUSB); 
  SerialUSB.println("RFM Client!"); 

  //Initialize the Radio.
  if (rf95.init() == false){
    SerialUSB.println("Radio Init Failed - Freezing");
    while (1);
  }
  else{
    //An LED inidicator to let us know radio initialization has completed. 
    SerialUSB.println("Client up!"); 
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }

  // Set frequency
  rf95.setFrequency(frequency);
  rf95.setSpreadingFactor(7);
  rf95.setSignalBandwidth(125000);
  
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm: Value 2-20
  // Transmitter power can range from 14-20dbm.
  rf95.setTxPower(20, false);
}

void loop()
{
 // Turn off status LED if it is on, do it here as some functions block
  if(millis() - lastStatusLEDOfftMills >= 450){
    digitalWrite(LED, LOW); //Turn off status LED   
    lastStatusLEDOfftMills = millis(); 
  }     

  SerialUSB.println("Sending message");

  //Send a message to the other radio
  uint8_t toSend[] = "Hi there!!!!!";

  // frequency = 915.0; //TX frequency to Server!
  rf95.send(toSend, sizeof(toSend));  
  rf95.waitPacketSent();
  //  frequency = 920.0; //RX frequency to GateWay!
 
  if (rf95.waitAvailableTimeout(3000)) { // Blocking

    byte buf[RH_RF95_MAX_MESSAGE_LEN];
    byte len = sizeof(buf);

   // Status LED
   digitalWrite(LED, HIGH); //Turn on status LED
   lastPacketMills = millis();
 
   // Read RX to buffer
   if (rf95.recv(buf, &len)) {
    SerialUSB.print("Got reply from server: ");
    SerialUSB.println((char*)buf);
    SerialUSB.print(" RSSI: ");
    SerialUSB.println(-137 + rf95.lastRssi(), DEC);
   }
   else {
    SerialUSB.println("Receive failed");
   }
  }
  else {
    SerialUSB.println("No reply, is the server running?");
  }  
 
  // Pause so doesnt TX Packets to quickly
  delay(800);
 
}
