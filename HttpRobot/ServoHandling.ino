void handleServo(String req, WiFiClient &client){
  int q = req.indexOf('?');
  int e = req.indexOf(' ', q);
  String query = req.substring(q + 1, e);

  int leftDegrees = urlDecode(getParam(query, "ls")).toInt();
  int rightDegrees = urlDecode(getParam(query, "rs")).toInt();

  Serial.println(leftDegrees);
  Serial.println(rightDegrees);

  if(leftDegrees < 0 || leftDegrees > 180 || rightDegrees < 0 || rightDegrees > 180){
    String responseBody = "{\"message\":\"Servo angles must be between 0 and 180\"}";

    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(responseBody.length());
    client.println("Connection: close");
    client.println();
    client.println(responseBody);

    client.flush();
    client.stop();
  }else{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.flush();
    client.stop();

    servoLeft.write(leftDegrees);
    servoRight.write(rightDegrees);
  }
}