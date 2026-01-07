void handleMovement(String packet){
  String dir = getParamValue(packet, "dir");

  if(dir == "fwd"){
    digitalWrite(lf, HIGH);
    digitalWrite(rf, HIGH);
    digitalWrite(lb, LOW);
    digitalWrite(rb, LOW);
  }
  else if(dir == "back"){
    digitalWrite(lf, LOW);
    digitalWrite(rf, LOW);
    digitalWrite(lb, HIGH);
    digitalWrite(rb, HIGH);
  }
  else if(dir == "rigth"){
    digitalWrite(lf, HIGH);
    digitalWrite(rf, LOW);
    digitalWrite(lb, LOW);
    digitalWrite(rb, HIGH);
  }
  else if(dir == "left"){
    digitalWrite(lf, LOW);
    digitalWrite(rf, HIGH);
    digitalWrite(lb, HIGH);
    digitalWrite(rb, LOW);
  }
}

void resetMotors(){
  digitalWrite(lf, LOW);
  digitalWrite(rf, LOW);
  digitalWrite(lb, LOW);
  digitalWrite(rb, LOW);
  Serial.println("Motors reset");
  motorsReset = true;
}