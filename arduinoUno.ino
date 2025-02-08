#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MAX30105.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Czujnik temperatury bezdotykowy

#define ONE_WIRE_BUS 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); // Czujnik temperatury
MAX30105 pulsometr; // Czujnik Pulsometr
int pulsPin = A1;   // Czujnik tętna
int puls = 0;
int skora = A2;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mlx.begin();
  sensors.begin();
  pulsometr.begin(Wire, I2C_SPEED_FAST);
  pulsometr.setup();
}

void loop() {
  float temperaturaBezDotyku = mlx.readAmbientTempC();
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);
  long podczerwien = pulsometr.getIR();
  if (podczerwien > 50000) {
    float saturacjaKrwi = pulsometr.getSpO2();
    float tetno = pulsometr.getHeartRate();
    Serial.print("Saturacja Krwi: ");Serial.print(saturacjaKrwi);Serial.print(" % ");
    Serial.print("Tetno: ");Serial.print(tetno);Serial.println(" BPM");
  }
  puls = analogRead(pulsPin);
  int wynikSkory = analogRead(skora);
  float rezystancji = (1023.0 / wynikSkory - 1.0) * 10000;
  float przewodnoscSkory = 1000000.0 / rezystancji;

  Serial.print("MLX temperatura: ");Serial.print(temperaturaBezDotyku);Serial.print(" C ");
  Serial.print("DS18 temperatura: ");Serial.print(temperatura);Serial.print(" C ");
  Serial.print("Puls: ");Serial.print(puls);
  Serial.print(" Przewodnictwo skóry: ");Serial.print(przewodnoscSkory);Serial.println(" µS");
  delay(1000);
}
