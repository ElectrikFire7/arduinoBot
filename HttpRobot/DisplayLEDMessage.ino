void handleLedMessage(String req, WiFiClient &client) {
  int start = req.indexOf('?');
  int end = req.indexOf(' ', start);
  String query = req.substring(start + 1, end);

  String msg = getParam(query, "msg");
  msg = urlDecode(msg);
  msg.toUpperCase();


  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println("{\"status\":\"displaying\", \"text\":\"" + msg + "\"}");
  client.flush();
  client.stop();

  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println("  " + msg);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}