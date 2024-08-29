#include <OneWire.h>
#include <DallasTemperature.h>
#define PORT_CZUJNIKA 2

OneWire oneWire(PORT_CZUJNIKA);
DallasTemperature sensors(&oneWire);
DeviceAddress tablicaAdrsow;

void setup()
{
  Serial.begin(9600);
  sensors.begin();
  sensors.getAddress(tablicaAdrsow, 0);
}

void loop()
{ 
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(tablicaAdrsow);

  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.println();
}
