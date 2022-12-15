//  The BME280 Temp & Humidity is a network based tempature, humidity,
//  and barametric pressor sensor perpheriphal designed to provide the
//  Q-SYS architecture with realtime local weather data. Built around
//  readily available open-source hardware the BME280 Temp & Humidity
//  can be assembled for around $50 per unit. An Arduino Nano microcontroller
//  in companion with a W5500 ethernet shield and a BME280 sensor enables
//  weather data to be sent over the network via UDP communication with
//  the Q-SYS core. This plugin allows for ease of setup and integration
//  into an existing design appearing as any other design component.
//
//  HARDWARE:
//    Arduino Nano, W5500 ethernet shield, BME280 I2C sensor, LED, Push button
//
//  CONNECTIONS:
//    W5500
//      CS-> D10
//      MOS1-> D11
//      MISO-> D12
//      SCK-> D13
//
//    BME280
//      SDA-> A4
//      SCL-> A5
//
//    ID LED
//      anode-> D1
//      cathode-> GND
//
//    ID Button
//      NO-> D0
//      COM-> GND
//
//    _-===-_  _-===-_
//   //     \\//     \\
//  ((   #   XX       ))
//   \\     //\\    QSC
//     "==="    "===Communities
//


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>        // Ethernet library for offical w5500 shields
//#include <EthernetENC.h>        // Ethernet library for cheap enc28j60 shields
#include <EthernetUdp.h>        // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EEPROM.h>
#include "Seeed_BME280.h"
#include <Wire.h>




BME280 bme280;

// Enter a MAC address for your Remote GPIO Nano.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xA6
};


// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
#define W5200_CS  10

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// The broadcast IP address:
IPAddress Bip(255, 255, 255, 255);     // Idealy this is the IP of the controller (Q-SYS Core), but the NIC is being dificult so reasons :-)

int period = 1000;
unsigned long time_now = 0;

int IDpin = 1;    // the pin for the ID LED
int ipA, ipB, ipC, ipD; // Vars for storing IP address
int ipAget, ipBget, ipCget, ipDget; // Vars for buffering stored IP address
int Port;    // Var for storing port address


bool sel = false;   //Var for storing static or DHCP state

String N;     // Var for storing / parsing webpage entry

char firmware[7] = "TH-0.0";    // Var for storing firmware version
char StaticMode[7];    // Var for storing Static or DHCP mode
char HostName[21];   // Var for storing device hostname
char numBox[26] = " <input type=number name=";
char brk[9] = "<br><br>";
char ipRange[22] = " min=0 max=255 value=";
char sendPacket = "";

// Parsing vars
char * QsysData;
char EndDelimiter = '*';
char Delimiter2 = ':';




//========== Setup input voltage reading ===============================
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}





void setup() {
  pinMode(1, OUTPUT);      // sets the digital pin as output
  pinMode(0, INPUT_PULLUP);      // sets the digital pin as an input with pullup resistor enabled

  EEPROM.get(10, ipAget);   // Get the saved IP[0] value from the eeprom
  EEPROM.get(20, ipBget);   // Get the saved IP[1] value from the eeprom
  EEPROM.get(30, ipCget);   // Get the saved IP[2] value from the eeprom
  EEPROM.get(40, ipDget);   // Get the saved IP[3] value from the eeprom
  EEPROM.get(50, StaticMode);   // Get the saved Static or DHCP state from the eeprom
  EEPROM.get(60, Port);   // Get the saved port value from the eeprom
  EEPROM.get(100, HostName);   // Get the saved Hostname from the eeprom

  IPAddress ip(ipAget, ipBget, ipCget, ipDget);
  unsigned int localPort = Port;      // local port for the Remote GPIO Nano to listen on

  // start the Ethernet and UDP:
  if (strncmp(StaticMode, "Static", 6) == 0) {
    Ethernet.begin(mac, ip);
    sel = false;
  }
  else {
    Ethernet.begin(mac);
    sel = true;
  }


  ////********** Uncomment below for Debugging ******************************
  //    Serial.begin(115200);
  //    Serial.println(Ethernet.localIP());
  ////********** Uncomment above for Debugging ******************************
  

  // start BME280 I2C
  if (!bme280.init()) {}
  // start UDP
  Udp.begin(localPort);
  // start the server
  server.begin();
}



void(* resetFunc) (void) = 0; //declare reset function @ address 0





void loop() {
  //========== QDP packet sent every second ===============================

  if (millis() >= time_now + period) {
    time_now += period;
    Udp.beginPacket(Bip, 2467);
    Udp.write("<QDP><device>\r  <name>");
    Udp.write(HostName);
    Udp.write("</name>\r  <type>ioframe8s</type>\r  <lan_a_ip>");
    Udp.print(Ethernet.localIP());
    Udp.write("</lan_a_ip>\r <periph_cfg_url>/#network</periph_cfg_url>\r</device>\r</QDP>");
    Udp.endPacket();
  }

  //========== Read incoming UDP packets ===============================

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {

    ////********** Uncomment below for Debugging ******************************
    //    Serial.print("Received packet of size ");
    //    Serial.println(packetSize);
    //    Serial.print("From ");
    //    IPAddress remote = Udp.remoteIP();
    //    for (int i =0; i < 4; i++)
    //    {
    //      Serial.print(remote[i], DEC);
    //      if (i < 3)
    //      {
    //        Serial.print(".");
    //      }
    //    }
    //    Serial.print(", port ");
    //    Serial.println(Udp.remotePort());
    ////********** Uncomment above for Debugging ******************************

    float vccValue = readVcc() / 1000.0; //Read Arduino refence voltage & convert mV to V
    char packetBuffer[packetSize];  // buffer to hold incoming packet

    // read the packet into packetBufffer
    Udp.read(packetBuffer, packetSize);
    //        Serial.println("Contents:");
    //        Serial.println(packetBuffer);


    //========== Parse the things ===============================

    QsysData = strtok(packetBuffer, &EndDelimiter);
    QsysData = strtok(packetBuffer, &Delimiter2);
    //    Serial.println("QsysData:");
    //    Serial.println(QsysData);

    while (QsysData != NULL)  {
      //========== Handle ID Light ===============================
      // if packets are ID request, respond and enable or disable ID light
      if (strcmp(QsysData, "IDON") == 0) {
        digitalWrite(IDpin, HIGH);
      }
      if (strcmp(QsysData, "IDOFF") == 0) {
        digitalWrite(IDpin, LOW);
      }
      QsysData = strtok(NULL, ":");
    }

    //========== Send Device & Sensor Data ===============================
    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.print(millis());
    Udp.write(":");
    Udp.print(vccValue);  //Arduino refence voltage
    Udp.write(":");
    Udp.print(firmware);  //Arduino firmware version
    Udp.write(":");
    Udp.print(bme280.getTemperature());  //Temperature
    Udp.write(":");
    Udp.print(bme280.getHumidity());  //Humidity
    Udp.write(":");
    Udp.print(bme280.getPressure());  //Pressure
    Udp.endPacket();


  }
  //========== End of UDP response ===============================




  //========== Handle Web UI ===============================
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean currentLineIsGet = true;
    int tCount = 0;
    char tBuf[64];
    char *pch;



    //Serial.print("Client request: ");

    while (client.connected()) {
      while (client.available()) {
        char c = client.read();

        if (currentLineIsGet && tCount < 63)
        {
          tBuf[tCount] = c;
          tCount++;
          tBuf[tCount] = 0;
        }

        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response
          while (client.available()) client.read();
          //Serial.println(tBuf);
          pch = strtok(tBuf, "?");

          while (pch != NULL)
          {
            if (strncmp(pch, "H=", 2) == 0)
            {
              N = pch + 2;
              N.toCharArray(HostName, 21);
              //              Serial.println(HostName);
              EEPROM.put(100, HostName);   // save value to the eeprom
              //              Serial.print("H=");
              //              Serial.println(H);
            }

            if (strncmp(pch, "Mode=", 5) == 0)
            {
              N = pch + 5;
              N.toCharArray(StaticMode, 7);
              EEPROM.put(50, StaticMode);   // save value to the eeprom
              //              Serial.print("S=");
              //              Serial.println(S);
            }

            if (strncmp(pch, "A=", 2) == 0)
            {
              ipA = atoi(pch + 2);
              EEPROM.put(10, ipA);   // save value to the eeprom
              //              Serial.print("A=");
              //              Serial.println(A,DEC);
            }

            if (strncmp(pch, "B=", 2) == 0)
            {
              ipB = atoi(pch + 2);
              EEPROM.put(20, ipB);   // save value to the eeprom
              //              Serial.print("B=");
              //              Serial.println(B,DEC);
            }

            if (strncmp(pch, "C=", 2) == 0)
            {
              ipC = atoi(pch + 2);
              EEPROM.put(30, ipC);   // save value to the eeprom
              //              Serial.print("C=");
              //              Serial.println(C,DEC);
            }

            if (strncmp(pch, "D=", 2) == 0)
            {
              ipD = atoi(pch + 2);
              EEPROM.put(40, ipD);   // save value to the eeprom
              //              Serial.print("D=");
              //              Serial.println(D,DEC);
            }

            if (strncmp(pch, "P=", 2) == 0)
            {
              Port = atoi(pch + 2);
              EEPROM.put(60, Port);   // save value to the eeprom
              //              Serial.print("D=");
              //              Serial.println(D,DEC);
              delay(100);
              resetFunc();  //call reset
            }



            pch = strtok(NULL, "& ");
          }
          //          Serial.println("Sending response");
          client.write("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><H1>BME280 TEMP & HUMIDITY</H1>");

          client.write("This is a temperature and humidity network peripheral designed for the Q-SYS architecture.");
          client.write(brk);

          client.write("<form method=GET>Hostname: <input type=text name=H maxlength=20 value=");
          client.print(HostName);
          client.write(">");
          client.write(brk);
          client.write("<select name=Mode >");
          client.write("<option value=Auto");
          if (sel == true) {
            client.write(" selected");
          }
          client.write(">Auto</option>");
          client.write("<option value=Static");
          if (sel == false) {
            client.write(" selected");
          }
          client.write(">Static</option>");
          client.write("</select> ");


          client.write("IP Address:");
          client.write(numBox);
          client.write("A");
          client.write(ipRange);
          client.print(ipAget);
          client.write(">.");
          client.write(numBox);
          client.write("B");
          client.write(ipRange);
          client.print(ipBget);
          client.write(">.");
          client.write(numBox);
          client.write("C");
          client.write(ipRange);
          client.print(ipCget);
          client.write(">.");
          client.write(numBox);
          client.write("D");
          client.write(ipRange);
          client.print(ipDget);
          client.write(">");
          client.print(brk);


          client.write("UDP port:");
          client.write(numBox);
          client.write("P min=1 max=65535 value=");
          client.print(Port);
          client.write(">");
          client.write(brk);
          client.write("MAC Address: ");
          for (uint8_t i = 0; i < 6; i++) {
            if (i) client.write(":");
            if (mac[i] < 16) client.write('0');
            client.print(mac[i], HEX);
          }
          client.write(brk);
          client.write("<input type=submit></form>");
          client.write(brk);

          client.write("Designed by: Victor R Ellis");

          client.write("</body></html>\r\n\r\n");
          client.stop();
        }
        else if (c == '\n') {
          currentLineIsBlank = true;
          currentLineIsGet = false;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    //    Serial.println("done");
  }
  //========== End of Web UI ===============================


  //  delay(1);
}
