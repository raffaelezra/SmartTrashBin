#define MIC_PIN 4

void setup() {
  Serial.begin(921600);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  uint16_t sample = analogRead(MIC_PIN);
  Serial.write((uint8_t*)&sample, 2);
  delayMicroseconds(50);
}