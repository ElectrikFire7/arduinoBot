#include "WiFiS3.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "WiFiUdp.h"
#include "Servo.h"

#define SERVER_PORT 8008
#define WIFI_CHECK_INTERVAL 30000
#define STA_TIMEOUT 15000
#define STA_MAX_RETRIES 3
#define DISCOVERY_MAGIC "UNO_R4"

const int leftServoPin = 3;
const int rightServoPin = 5;
const int lf = 7;
const int lb = 8;
const int rf = 2;
const int rb = 4;
Servo servoLeft;
Servo servoRight;

//AP wifi creds
char ap_ssid[] = "Arduino_Control";
char ap_pass[] = "12345678";

WiFiServer server(SERVER_PORT);
ArduinoLEDMatrix matrix;
WiFiUDP Udp;

unsigned int localPort = 4210;      // Static Local port to listen on
unsigned int remotePort = 5000;     // Dynamic Port on the receiving PC/Device
char pcIpAddress[] = "192.168.1.5"; // Dynamic PC IP

enum WifiMode {
  MODE_AP,
  MODE_STA,
  SWITCH_TO_AP,
  SWITCH_TO_STA,
  CHANGE_STA
};

WifiMode wifiMode = MODE_AP;

//STP Wifi Creds
String savedSSID = "";
String savedPASS = "";

bool requestSTA = false;

void setup() {
  pinMode(lf, OUTPUT);
  pinMode(lb, OUTPUT);
  pinMode(rf, OUTPUT);
  pinMode(rb, OUTPUT);

  Serial.begin(9600);
  while (!Serial);
  startAP();
  matrix.begin();
  servoLeft.attach(leftServoPin);
  servoRight.attach(rightServoPin);
}

void loop() {
  if (wifiMode == MODE_AP || wifiMode == MODE_STA) {
    handleServer();
  }

  if(wifiMode == SWITCH_TO_AP){
    startAP();
  }
  else if(wifiMode == SWITCH_TO_STA || wifiMode == CHANGE_STA){
    startSTA();
  }

  // static unsigned long lastUpdate = 0;
  // if (millis() - lastUpdate > 100) {
  //   handleUdpStreaming();
  //   lastUpdate = millis();
  // }

  handleUDPInput();

  checkWiFi();
  yield();
}