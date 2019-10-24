#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 10

int Relay = 8;
int t_limit = 30;
int hysterisis = 1;
float t_current = 0;

int fan_enabled = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void readsensor() {
  sensors.requestTemperatures();
  t_current = sensors.getTempCByIndex(0);
}

void setup(void){
  Serial.begin(9600);

  sensors.begin();
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, HIGH);

  readsensor();
}

void loop(void){ 
  readsensor();
  Serial.println(t_current);

  if (t_current > t_limit){
    digitalWrite(Relay, LOW);
    fan_enabled = 1;

    do {
       readsensor();
       Serial.println(t_current);
       delay(1000);
       } while (t_current > (t_limit - hysterisis));

    digitalWrite(Relay, HIGH);
    fan_enabled = 0;
  }

  delay(1000);
}
