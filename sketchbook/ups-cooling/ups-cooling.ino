#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define GAZ 14
#define RELAY 2
#define RELAYGND 3
#define LCDGND 5
#define LCDVCC 6
#define DS 4
#define DSGND 13

LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // (RS, E, DB4, DB5, DB6, DB7)

OneWire oneWire(DS);
DallasTemperature sensors(&oneWire);

int temp_limit = 32;
int temp_hyst = 2;
float temp_current = 0;
float temp_error = -127.0;
int gaz_current = 0;

volatile byte seconds;
bool high_temp = false;
bool timer_on = false;

char line0[17]; 
char line1[17];

void readMQ() {
  gaz_current = analogRead(GAZ);
}

void readDS() {
  sensors.requestTemperatures();

  if (sensors.getTempCByIndex(0) != temp_error) {
    temp_current = sensors.getTempCByIndex(0);
  } else {
    temp_current = 0;
  }
}

void updateDisplay() {
   char floatemp_str[7];
   dtostrf(temp_current,4,1,floatemp_str);

   sprintf(line0, "Gaz: %-11d", gaz_current);
   sprintf(line1, "Temp: %-10s", floatemp_str);

   lcd.setCursor(0,0);
   lcd.print(line0);
   lcd.setCursor(0,1);
   lcd.print(line1);
}

void setup(void) {
  Serial.begin(9600);

  pinMode(RELAYGND, OUTPUT);
    digitalWrite(RELAYGND, 1);
  pinMode(LCDGND, OUTPUT);
    digitalWrite(LCDGND, 0);
  pinMode(LCDVCC, OUTPUT);
    digitalWrite(LCDVCC, 1);
  pinMode(DSGND, OUTPUT);
    digitalWrite(DSGND, 0);

  sensors.begin();
  lcd.begin(16, 2);

  readMQ();
  readDS();

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, 1);

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

  updateDisplay();
  Serial.println(temp_current);

  if (temp_current > temp_limit) {
    if (timer_on == false) {
      Serial.println("Hight temperature. Starting fan.");
      digitalWrite(RELAY, 0);
      high_temp = true;

      do {
        readMQ();
        readDS();

        updateDisplay();
        Serial.println(temp_current);

        delay(1000);
      } while (temp_current > (temp_limit - temp_hyst));

      digitalWrite(RELAY, 1);
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
        digitalWrite(RELAY, 0);
        timer_on = true;
      } else if (timer_on == true) {
        Serial.println("Timer has been disabled");
        digitalWrite(RELAY, 1);
        timer_on = false;
      }
    } else {
      Serial.println("High temp fan is on. Skip timer.");
    }
  }
}
