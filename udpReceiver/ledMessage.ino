void handleLedMessage(String packet){
  String msg = getParamValue(packet, "msg");
  msg.toUpperCase();

  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println("  " + msg);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}