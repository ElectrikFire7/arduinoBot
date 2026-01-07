void sendData(String message) {
  if (wifiMode == MODE_AP) {
    // BROADCAST: Send to everyone on the 192.168.4.x network
    IPAddress broadcastIp(192, 168, 4, 255); 
    Udp.beginPacket(broadcastIp, remotePort);
    Udp.print(message);
    Udp.endPacket();
  } 
  else if (wifiMode == MODE_STA) {
    // UNICAST: Send only to your specific ROS2 PC
    Udp.beginPacket(pcIpAddress, remotePort);
    Udp.print(message);
    Udp.endPacket();
  }
}

// In your loop or inside a timer:
void handleUdpStreaming() {
  if (WiFi.status() == WL_CONNECTED || wifiMode == MODE_AP) {
    String telemetry = "Hello PC";
    sendData(telemetry);
  }
}
