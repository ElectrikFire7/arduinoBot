void udpController(String packet){
  if(packet.startsWith("wifi")){
    handleSwitchWifi(packet);
  }
  else if(packet.startsWith("ipswap")){
    handleIpSwap(packet);
  }
  else if(packet.startsWith("ledmsg")){
    handleLedMessage(packet);
  }else if(packet.startsWith("move")){
    motorsReset = false;
    handleMovement(packet);
  }
}

void handleSwitchWifi(String packet){
  String ssid = getParamValue(packet, "ssid");
  String pass = getParamValue(packet, "pass");

  Serial.println(ssid);
  Serial.println(pass);

  if (ssid.length() > 0) {
    savedSSID = ssid;
    savedPASS = pass;
    wifiMode = SWITCH_TO_STA;
  }
}

void handleIpSwap(String packet){
  // 1. Get sender (PC) IP & port from UDP
  IPAddress senderIP = udp.remoteIP();
  unsigned int senderPort = udp.remotePort();

  // 2. Save PC info
  pcPort = senderPort;
  senderIP.toString().toCharArray(pcIpAddress, sizeof(pcIpAddress));

  // 3. Reply with Arduino IP + port
  String response = "ipswap";
  response += "$ip=" + WiFi.localIP().toString() + "/";
  response += "$port=" + String(arduinoPort) + "/";

  udp.beginPacket(senderIP, senderPort);
  udp.write(response.c_str());
  udp.endPacket();

  Serial.println("Sent Arduino IP info to PC");
}