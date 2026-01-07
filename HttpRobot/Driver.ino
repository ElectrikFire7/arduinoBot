void handleDriverequest(String req, WiFiClient &client){
  int q = req.indexOf('?');
  int e = req.indexOf(' ', q);
  String query = req.substring(q + 1, e);

  String s_dirl = getParam(query, "dirl");
  String s_dirr = getParam(query, "dirr");
  String s_tt    = getParam(query, "t");

  int dirl = s_dirl.toInt();
  int dirr = s_dirr.toInt();
  int t    = s_tt.toInt();

  Serial.println(dirl);
  Serial.println(dirr);
  Serial.println(t);

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println("{\"status\":\"moving\", \"dirl\":" + String(dirl) + ", \"dirr\":" + String(dirr) + "}");
  client.flush();

  client.stop();

  drive(dirl, dirr, t);
}

void drive(int dirl, int dirr, int t){
  Serial.println(dirl);
  Serial.println(dirr);
  Serial.println(t);
  
  if(dirl == 2){
    digitalWrite(lf, HIGH);
    digitalWrite(lb, LOW);
  }else if(dirl == 1){
    digitalWrite(lf, LOW);
    digitalWrite(lb, HIGH);
  }else {
    digitalWrite(lf, LOW);
    digitalWrite(lb, LOW);
  }

  if(dirr == 2){
    digitalWrite(rf, HIGH);
    digitalWrite(rb, LOW);
  }else if (dirr == 1){
    digitalWrite(rf, LOW);
    digitalWrite(rb, HIGH);
  }else {
    digitalWrite(rf, LOW);
    digitalWrite(rb, LOW);
  }

  delay(t);

  digitalWrite(lf, LOW);
  digitalWrite(lb, LOW);
  digitalWrite(rf, LOW);
  digitalWrite(rb, LOW);
}