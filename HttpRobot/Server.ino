void handleServer() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = "";
  unsigned long start = millis();

  while (client.connected() && millis() - start < 1000) {
    if (client.available()) {
      request += char(client.read());
      start = millis();
      if (request.endsWith("\r\n\r\n")) break;
    }
  }

  if (request.startsWith("GET /wifi?")) {
    handleWiFiRequest(request, client);
  } else if (request.startsWith("GET /ledmessage?")) {
    handleLedMessage(request, client);
  } else if (request.startsWith("GET /ipswap?")) {
    handleIpSwap(request, client);
  } else if (request.startsWith("GET /move?")) {
    handleDriverequest(request, client);
  } else if (request.startsWith("GET /servo?")) {
    handleServo(request, client);
  } else {
    sendHelpPage(client);
  }

  delay(10);

  client.stop();
}


void sendHelpPage(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  
  String jsonResponse = "{";
  jsonResponse += (String)"\"mode\":\"" + wifiMode + "\",";
  if(wifiMode == MODE_AP){
    jsonResponse += "\"ssid\":\"Arduino_Control\"";
  }else{
    jsonResponse += "\"ssid\":\"" + savedSSID + "\"";
  }
  jsonResponse += "}";

  client.println(jsonResponse);

  client.flush(); 
  delay(500);
}

void handleWiFiRequest(String req, WiFiClient &client) {

  int q = req.indexOf('?');
  int e = req.indexOf(' ', q);
  String query = req.substring(q + 1, e);

  String ssid = getParam(query, "ssid");
  String pass = getParam(query, "pass");

  if (ssid.length() == 0 || pass.length() == 0) {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Connection: close");
    client.println();
    return;
  }

  savedSSID = urlDecode(ssid);
  savedPASS = urlDecode(pass);

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json ");
  client.println("Connection: close");
  client.println();
  String jsonResponse = "{";
  jsonResponse += "\"status\":\"connecting\",";
  jsonResponse += "\"newSsid\":\"" + savedSSID + "\"";
  jsonResponse += "}";
  client.println(jsonResponse);


  client.flush();
  delay(500);
  client.stop();

  if(wifiMode == MODE_AP){
    wifiMode = SWITCH_TO_STA;
  } else {
    wifiMode = CHANGE_STA;
  }
}

void handleIpSwap(String req, WiFiClient &client) {

  int q = req.indexOf('?');
  int e = req.indexOf(' ', q);
  if (q == -1 || e == -1) return;

  String query = req.substring(q + 1, e);
  String pcIpStr = urlDecode(getParam(query, "pcip"));
  String pc_udp_port = urlDecode(getParam(query, "udpport"));

  if (pc_udp_port.length() > 0) {
    remotePort = pc_udp_port.toInt();
    Serial.println(("set PC PORT"));
    Serial.println(remotePort);
  }
  if (pcIpStr.length() > 0) {
    pcIpStr.toCharArray(pcIpAddress, sizeof(pcIpAddress));
    Serial.println(("set PC IP"));
    Serial.println(pcIpAddress);
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Connection: close");
  client.println();
}
