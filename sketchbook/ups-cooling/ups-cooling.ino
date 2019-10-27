#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define ONE_WIRE_BUS 10

int Relay = 8;
int t_limit = 32;
int hysterisis = 2;
float t_current = 0;

volatile byte seconds;
bool high_temp = false;
bool timer_on = false;

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

  OCR1A = 15624;  // установка регистра совпадения (1 sec)

  TCCR1B |= (1 << WGM12);  // включить CTC режим
  TCCR1B |= (1 << CS10);  // Установить биты на коэффициент деления 1024
  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A);  // включить прерывание по совпадению таймера
  sei(); // включить глобальные прерывания
}

// Main loop

void loop(void){ 
  readsensor();
  Serial.println(t_current);

  if (t_current > t_limit){
      if (timer_on == false){
          Serial.println("Hight temperature. Starting fan.");
          digitalWrite(Relay, LOW);
          high_temp = true;

          do {
              readsensor();
              Serial.println(t_current);
              delay(1000);
          } while (t_current > (t_limit - hysterisis));

          digitalWrite(Relay, HIGH);
          high_temp = false;
       }
  }

  delay(1000);
}

// Timer is here

ISR(TIMER1_COMPA_vect){
    seconds++;
    if(seconds == 180){
        seconds = 0;

        if (high_temp != true){
            if (timer_on != true){
                Serial.println("Timer has been enabled");
                digitalWrite(Relay, LOW);
                timer_on = true;
            } else if (timer_on == true){
                Serial.println("Timer has been disabled");
                digitalWrite(Relay, HIGH);
                timer_on = false;
            }
        } else {
            Serial.println("High temp fan is on. Skip timer.");
        }
    }
}
