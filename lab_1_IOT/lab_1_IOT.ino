void setup() {
  Serial.begin(115200);
  pinMode(D4, OUTPUT);
}

void loop() {
  char input = Serial.read();
  if (input == '0') {
    Serial.println("0");
    digitalWrite(D4, HIGH);
  }
  else if (input == '1') {
    Serial.println("1");
    digitalWrite(D4, LOW);
  }
}
