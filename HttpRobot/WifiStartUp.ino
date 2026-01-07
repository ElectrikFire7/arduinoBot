void startAP() {
  Serial.println("\n[AP] Starting AP mode");

  server.end();
  Udp.stop();
  WiFi.end();
  delay(1000);

  WiFi.beginAP(ap_ssid, ap_pass);
  delay(1000);
  Udp.begin(localPort);

  server.begin();
  wifiMode = MODE_AP;

  Serial.println("[AP] Ready");
  Serial.println("[AP] Connect to http://192.168.4.1:8008");
}

void startSTA() {
  Serial.println("\n[STA] Connecting to " + savedSSID);

  //clear connections
  server.end();
  Udp.stop();
  if(wifiMode == SWITCH_TO_STA){
    WiFi.end();
    delay(1000);
  } else if(wifiMode == CHANGE_STA){
    WiFi.disconnect();
    WiFi.end();
    delay(1000);
  }

  WiFi.begin(savedSSID.c_str(), savedPASS.c_str());  
  Udp.begin(localPort);


  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < STA_TIMEOUT) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("CONNECTION ESTABLISHED"));
    Serial.print(F("SSID: "));      Serial.println(WiFi.SSID());
    Serial.print(F("IP Address: ")); Serial.println(WiFi.localIP()); // <--- This prints the IP
    Serial.print(F("mDNS Name: "));  Serial.print("esp32s3-07B3DC");
    Serial.print(F("Port: "));      Serial.println(SERVER_PORT);

    server.begin();
    wifiMode = MODE_STA;
    delay(500);
  } else {
    Serial.println("[STA] Failed → fallback to AP");
    wifiMode = SWITCH_TO_AP;
  }
}

void checkWiFi() {
  static unsigned long last = 0;
  if (millis() - last < WIFI_CHECK_INTERVAL) return;
  last = millis();

  if (wifiMode == MODE_STA &&
      WiFi.status() != WL_CONNECTED) {
    Serial.println("[STA] Lost connection");
    wifiMode = SWITCH_TO_AP;
  }
}