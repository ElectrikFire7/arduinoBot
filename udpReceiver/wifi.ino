void startAP() {
  Serial.println("Starting AP mode");

  udp.stop();
  WiFi.end();
  delay(500);

  WiFi.beginAP(ap_ssid, ap_pass);
  delay(1000);

  Serial.print("AP IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(arduinoPort);
}

void connectToSTA() {
  Serial.println("Connecting to STA");

  udp.stop();
  WiFi.end();
  delay(500);

  WiFi.begin(savedSSID.c_str(), savedPASS.c_str());

  unsigned long start = millis();
  while (millis() - start < 15000) {
    if (isSTAConnected()) {
      Serial.print("STA IP: ");
      Serial.println(WiFi.localIP());
      udp.begin(arduinoPort);
      return;
    }
    delay(300);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    delay(1000);
    Serial.print("STA IP: ");
    Serial.println(WiFi.localIP());
    udp.begin(arduinoPort);
  } else {
    Serial.println("\nFailed to connect. Restarting AP.");
    wifiMode = SWITCH_TO_AP;
  }
}

void wifiSwitchingHandler(){
  switch (wifiMode) {
    case SWITCH_TO_AP:
      startAP();
      wifiMode = MODE_AP;
      break;

    case SWITCH_TO_STA:
      connectToSTA();
      wifiMode = MODE_STA;
      break;

    case MODE_STA:
      if (!isSTAConnected()) {
        Serial.println("STA disconnected → switching to AP");
        wifiMode = SWITCH_TO_AP;
      }
      break;

    case MODE_AP:
      break;
  }
}

bool isSTAConnected() {
  if (WiFi.status() != WL_CONNECTED) return false;
  IPAddress ip = WiFi.localIP();
  return ip != IPAddress(0, 0, 0, 0);
}