#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

char ap_ssid[] = "Arduino_Control";
char ap_pass[] = "12345678";
unsigned int arduinoPort = 5006;

unsigned int pcPort = 5005;     // Dynamic Port on the receiving PC/Device
char pcIpAddress[] = "192.168.1.5"; // Dynamic PC IP

bool credentialsReceived = false;
String savedSSID = "";
String savedPASS = "";

WiFiUDP udp;
ArduinoLEDMatrix matrix;

enum WifiMode {
  MODE_AP,
  MODE_STA,
  SWITCH_TO_AP,
  SWITCH_TO_STA,
  CHANGE_STA
};

enum MoveDir {
  fwd,
  bck,
  lft,
  rgt,
  sta
};

MoveDir moveDir = sta;

const int lf = 7;
const int lb = 8;
const int rf = 2;
const int rb = 4;
bool motorsReset = true;

WifiMode wifiMode = MODE_AP;

char incomingPacket[255];

void setup() {
  Serial.begin(9600);
  while (!Serial);
  matrix.begin();
  pinMode(lf, OUTPUT);
  pinMode(lb, OUTPUT);
  pinMode(rf, OUTPUT);
  pinMode(rb, OUTPUT);

  wifiMode = SWITCH_TO_AP;
}

void loop() {
  wifiSwitchingHandler();

  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 254);
    if (len > 0) incomingPacket[len] = 0;

    Serial.print("Received: ");
    Serial.println(incomingPacket);

    String data = String(incomingPacket);

    udpController(data);
  }
  else if(!motorsReset){
    resetMotors();
  }

  delay(200);
}
