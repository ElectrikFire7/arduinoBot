#define UDP_TX_PACKET_MAX_SIZE 64
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 

void handleUDPInput(){
  int packetSize = Udp.parsePacket();
  
  if (packetSize) {
    int len = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE - 1);
    
    if (len > 0) {
      packetBuffer[len] = 0; // Ensure the string ends correctly
    }
    
    Serial.print("Received (");
    Serial.print(len);
    Serial.print(" bytes): ");
    Serial.println(packetBuffer);

    Udp.beginPacket(pcIpAddress, remotePort);
    Udp.print("message");
    Udp.endPacket();
  }
}