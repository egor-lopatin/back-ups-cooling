#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define ONE_WIRE_BUS 10

int Relay = 8;
int t_limit = 30;
int hysterisis = 1;
float t_current = 0;

int seconds = 0;
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

  // инициализация Timer1
  cli();  // отключить глобальные прерывания
  TCCR1A = 0;   // установить регистры в 0
  TCCR1B = 0;

  OCR1A = 19530;  // установка регистра совпадения (5 min)

  TCCR1B |= (1 << WGM12);  // включить CTC режим
  TCCR1B |= (1 << CS10);  // Установить биты на коэффициент деления 1024
  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A);  // включить прерывание по совпадению таймера
  sei(); // включить глобальные прерывания
}

void loop(void){ 
  readsensor();
  Serial.println(t_current);

  if (t_current > t_limit){
    Serial.println("Hight temperature. Starting fan.");
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

ISR(TIMER1_COMPA_vect)
{
    seconds++;
    if(seconds == 300)
    {
        seconds = 0;

        if (fan_enabled != 1){
            Serial.println("Timer has been enabled");
            digitalWrite(Relay, LOW);

//            while(true) {
//                Serial.println(t_current);
//            }
        }
    }
}
