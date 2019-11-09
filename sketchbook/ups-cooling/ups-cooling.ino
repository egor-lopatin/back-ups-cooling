#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // (RS, E, DB4, DB5, DB6, DB7)

#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define smokePin A0
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int Relay = 2;
int t_limit = 32;
int hysterisis = 2;
int analogSensor = 0;

float t_current = 0;
float t_error = -127.0;

volatile byte seconds;
bool high_temp = false;
bool timer_on = false;

void readMQ() {
  analogSensor = analogRead(smokePin);
}

void readDS() {
  sensors.requestTemperatures();

  if (sensors.getTempCByIndex(0) != t_error) {
    t_current = sensors.getTempCByIndex(0);
  } else {
    t_current = 0;
  }
}

void onDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("Gaz:");
  lcd.setCursor(5, 0);
  lcd.print(analogSensor);
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.setCursor(6, 1);
  lcd.print(t_current, 1);
}

void setup(void) {
  Serial.begin(9600);

  pinMode(3, OUTPUT); // Relay GND
  digitalWrite(3, 1);

  pinMode(5, OUTPUT); // LCD GND
  digitalWrite(5, 0);

  pinMode(6, OUTPUT); // LCD VCC
  digitalWrite(6, 1);

  pinMode(13, OUTPUT); // DS18B20 GND
  digitalWrite(13, 0);

  sensors.begin();
  lcd.begin(16, 2);

  readMQ();
  readDS();

  pinMode(Relay, OUTPUT);
  digitalWrite(Relay, 1);

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

void loop(void) {
  readMQ();
  readDS();
  onDisplay();

  Serial.println(t_current);

  if (t_current > t_limit) {
    if (timer_on == false) {
      Serial.println("Hight temperature. Starting fan.");
      digitalWrite(Relay, 0);
      high_temp = true;

      do {
        readDS();
        onDisplay();
        Serial.println(t_current);
        delay(1000);
      } while (t_current > (t_limit - hysterisis));

      digitalWrite(Relay, 1);
      high_temp = false;
    }
  }

  delay(1000);
}

// Timer is here

ISR(TIMER1_COMPA_vect) {
  seconds++;
  if (seconds == 180) {
    seconds = 0;

    if (high_temp != true) {
      if (timer_on != true) {
        Serial.println("Timer has been enabled");
        digitalWrite(Relay, 0);
        timer_on = true;
      } else if (timer_on == true) {
        Serial.println("Timer has been disabled");
        digitalWrite(Relay, 1);
        timer_on = false;
      }
    } else {
      Serial.println("High temp fan is on. Skip timer.");
    }
  }
}
