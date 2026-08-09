#include <Arduino.h>

#include <slowflow/Terminology.h>

void setup() {
  Serial.begin(115200);
  Serial.print("SlowFlow ");
  Serial.print(slowflow::terminology::BoardType);
  Serial.print(" / ");
  Serial.println(slowflow::terminology::ShieldType);
}

void loop() {
  delay(1000);
}
