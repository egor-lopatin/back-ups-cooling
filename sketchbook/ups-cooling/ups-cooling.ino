#include <OneWire.h> // Инициализация библиотеки шины OneWire.
#include <DallasTemperature.h> // Инициализация библиотеки термодатчиков.

#define ONE_WIRE_BUS 10// Подключение цифрового вывода датчика к 10-му пину Ардуино.

int Relay = 8;
float t_current = 0;
int t_limit = 30;
int hysterisis = 1;

OneWire oneWire(ONE_WIRE_BUS); // Запуск интерфейса OneWire для подключения OneWire устройств.
DallasTemperature sensors(&oneWire); // Указание, что устройством oneWire является термодатчик от  Dallas Temperature.

void readsensor() {
  sensors.requestTemperatures();
  t_current = sensors.getTempCByIndex(0);
}

void setup(void){
  Serial.begin(9600); // Запуск СОМ порта.
  sensors.begin(); // Запуск сенсора.
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, HIGH);  // реле выключено
  readsensor(); // read sensor function
}

void loop(void){ 
  readsensor();
  Serial.println(t_current);

  if (t_current < t_limit){
      digitalWrite(Relay, HIGH);  // реле выключено
  }
  else {
      digitalWrite(Relay, LOW);   // реле включено

      do {
        readsensor();
        Serial.println(t_current);
        delay(1000);
      } while (t_current > (t_limit - hysterisis));

      digitalWrite(Relay, HIGH);
  }
  delay(1000);
}
