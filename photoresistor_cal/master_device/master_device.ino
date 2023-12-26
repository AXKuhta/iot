#include <Wire.h>

//
// Master device
//

void setup() {
  Serial.begin(115200);
  Serial.println("Master startup");

  pinMode(A0, INPUT_PULLUP);
  
  Wire.begin();
}

void loop() {
  int val = analogRead(A0);

  Serial.println(val);

  Wire.beginTransmission(1);
  Wire.write((byte*)&val, 2);
  Wire.endTransmission();

  delay(100);
}
