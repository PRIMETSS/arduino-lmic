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

int LED = 13; //Status LED on pin 13

int packetSendCounter = 0; //Counts the number of packets sent
long lastPacketMills = 0; //Tracks the time stamp of last packet received
long lastStatusLEDOfftMills = 0; //Tracks the time stamp of the status LED off time

// The broadcast frequency is set to 921.2, but the SADM21 ProRf operates
// anywhere in the range of 902-928MHz in the Americas.
// Europe operates in the frequencies 863-870, center frequency at 
// 868MHz.This works but it is unknown how well the radio configures to this frequency:
// https://www.semtech.com/products/wireless-rf/lora-core/sx1276
// https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R0000001Rbr/6EfVZUorrpoKFfvaF_Fkpgp5kzjiNyiAbqcpqh9qSjE
// https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R0000001OJk/yDEcfAkD9qEz6oG3PJryoHKas3UMsMDa3TFqz1UQOkM
// The main idea of this code is to change the normally fixed RX/TX frequency to match the dual (one RX one TX) radios in the LG02 Gateway running in repeater mode so as to make sure the local radios 'see' 
// only the signals form the Gateway and not direct from the other radio
// Dragino LG02 (In Repeater mode, p26)
// https://www.dragino.com/downloads/downloads/LoRa_Gateway/LG02-OLG02/LG02_LoRa_Gateway_User_Manual_v1.7.0.pdf

// The HoperF RFM95 Modem Module uses a SF chip that can do FSK, OOK (FSK) & Lora modes (Radio head only supports Lora modulation scheme)[why is modem labled with RF96?? as thats 433Mhz not RF95 868/915MHz ??)
// Register 0x01 is used to select Lora/FSK mode (Registers can change their function after that)
// Closing both 'LoraWAN' jumbers on SMAD21 connects DI01 & DIO2 from HopeRF9x module to D10 D11 on ATSAMD21G uP (Pins D10, D11) (?? maybe https://github.com/matthijskooijman/arduino-lmic DI01=RxTimeOut (Lora Mode), DI02=TimeOut (FSK Mode) ?)
// TS_HOP Hop time 200kHz step,1M,5M,7M,12M,20M,25Mhz set from set frequence
// https://github.com/arduino/ArduinoCore-samd
// https://www.airspayce.com/mikem/arduino/RadioHead/index.html   RadioHead Main Page

float frequency = 860.00;// Gateway repeater, centre frequency!

void setup()
{
  pinMode(LED, OUTPUT); // PA_17, Pin13 HIGH=On LOW=Off

  SerialUSB.begin(9600);
  // It may be difficult to read serial messages on startup. The following
  // line will wait for serial to be ready before continuing. Comment out if not needed.
  //  while(!SerialUSB);
  SerialUSB.println("RFM Server!");

  //Initialize the Radio. 
  if (rf95.init() == false){
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    SerialUSB.println("Radio Init Failed - Freezing");
    while (1);
  }
  else{
  // An LED indicator to let us know radio initialization has completed.
    SerialUSB.println("Server up!");
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }

   rf95.setFrequency(frequency); 
   rf95.setSpreadingFactor(7);
   rf95.setSignalBandwidth(125000);

   // The default transmitter power is 13dBm, using PA_BOOST.
   // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
   // you can set transmitter powers from 5 to 23 dBm: Value 2-20
   // Transmitter power can range from 14-20dbm.
   rf95.setTxPower(20, false); // power, RFO transmitter pin;
}

void loop()
{
   // Status LED      
  //Turn off status LED if we haven't received a packet after 1s
  if(millis() - lastStatusLEDOfftMills >= 450){
    digitalWrite(LED, LOW); //Turn off status LED
    lastStatusLEDOfftMills = millis();
  }
  
  SerialUSB.println("Waiting...");

  if (rf95.waitAvailableTimeout(3000)){
    
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

   // Status LED
    digitalWrite(LED, HIGH); //Turn on status LED
    lastPacketMills = millis(); //Timestamp this packet

  // Read RX to buffer
  if (rf95.recv(buf, &len)){
  
    SerialUSB.print("Got message from client: ");
    SerialUSB.println((char*)buf);
    SerialUSB.print(" RSSI: dBm"); // Ref RFHope manual dBm = -137 + RSSI
    SerialUSB.println(-137 + rf95.lastRssi(), DEC);
 
    // Send a reply
    SerialUSB.println("Sent a reply");
  
    uint8_t toSend[] = "Hello Back!"; 
 //     frequency = 915.0; //TX frequency to GateWay!
    rf95.send(toSend, sizeof(toSend));
    rf95.waitPacketSent();
 //     frequency = 920.0; //RX frequency to GateWay!*/
  }
  else
    SerialUSB.println("Recieve failed");
  }    

 
}
