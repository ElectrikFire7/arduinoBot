void handleMovement(String packet){
  String dir = getParamValue(packet, "dir");

  if(dir == "fwd"){
    if(moveDir == fwd){
      return;
    }
    digitalWrite(lf, HIGH);
    digitalWrite(rf, HIGH);
    digitalWrite(lb, LOW);
    digitalWrite(rb, LOW);
    moveDir = fwd;
  }
  else if(dir == "back"){
    if(moveDir == bck){
      return;
    }
    digitalWrite(lf, LOW);
    digitalWrite(rf, LOW);
    digitalWrite(lb, HIGH);
    digitalWrite(rb, HIGH);
    moveDir = bck;
  }
  else if(dir == "right"){
    if(moveDir == rgt){
      return;
    }
    digitalWrite(lf, HIGH);
    digitalWrite(rf, LOW);
    digitalWrite(lb, LOW);
    digitalWrite(rb, HIGH);
    moveDir = rgt;
  }
  else if(dir == "left"){
    if(moveDir == lft){
      return;
    }
    digitalWrite(lf, LOW);
    digitalWrite(rf, HIGH);
    digitalWrite(lb, HIGH);
    digitalWrite(rb, LOW);
    moveDir = lft;
  }
}

void resetMotors(){
  digitalWrite(lf, LOW);
  digitalWrite(rf, LOW);
  digitalWrite(lb, LOW);
  digitalWrite(rb, LOW);
  Serial.println("Motors reset");
  motorsReset = true;
  moveDir = sta;
}