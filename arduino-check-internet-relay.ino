/*
  DHCP-based IP printer

  This sketch uses the DHCP extensions to the Ethernet library
  to get an IP address via DHCP and print the address obtained.
  using an Arduino Wiznet Ethernet shield.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  created 12 April 2011
  modified 9 Apr 2012
  by Tom Igoe
  modified 02 Sept 2015
  by Arturo Guadalupi

*/

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "www.google.com";    // name address for Google (using DNS)

EthernetClient client;

#define RESETINTERVAL 25200000UL

unsigned long resetTimer;

bool reseting = false;

unsigned long checkInternetPreviousMillis = 0;
unsigned long relayPreviousMillis = 0;

const long checkInternetInterval = 60000;
const long relayInterval = 300000;

const int relayPin =  3;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  setupDHCP();

  checkInternetPreviousMillis = -60000;
  resetTimer = millis();
}

void setupDHCP() {
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  int ethernetStatus = Ethernet.maintain();
  switch (ethernetStatus) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  }

  if (checkInternetTimer() && !reseting) {
    if (checkInternet()) {
      Serial.println("connected");
    } else {
      Serial.println("disconnected");
      resetRelay();
    }
  }
  if (releaseResetRelayTimer()) {
    Serial.println("reseting released.");
    reseting = false;
  }
  if (millis() - resetTimer > RESETINTERVAL)
  {
    Serial.println("reseting timer.");
    resetTimer = millis();
    resetRelay();
  }
}

void resetRelay() {
  if (!reseting) {
    Serial.println("reseting...");
    reseting = true;
    relayPreviousMillis = millis();
    digitalWrite(relayPin, LOW);
    delay(2000);
    digitalWrite(relayPin, HIGH);
  }
}

boolean releaseResetRelayTimer() {
  unsigned long currentMillis = millis();
  if (currentMillis - relayPreviousMillis >= relayInterval) {
    // save the last time you blinked the LED
    relayPreviousMillis = currentMillis;
    return true;
  }
  return false;
}

boolean checkInternetTimer() {
  unsigned long currentMillis = millis();
  if (currentMillis - checkInternetPreviousMillis >= checkInternetInterval) {
    // save the last time you blinked the LED
    checkInternetPreviousMillis = currentMillis;
    return true;
  }
  return false;
}

boolean checkInternet() {
  if (client.connect(server, 80)) {
    delay(1000);
    client.stop();
    //    Serial.print("connected");
    return true;
  } else {
    //   Serial.print("connected");
    return false;
  }
}
