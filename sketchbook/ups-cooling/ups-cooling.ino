#include <LiquidCrystal_I2C.h>
//#include <Servo.h>
#include <ServoTimer2.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define LCDVCC 2
#define MOTOR 3
#define RELAYGND 4
#define RELAY 6
#define RELAYVCC 5
#define DSIN 12
#define DSIN_GND 13
#define DSOUT 9
#define DSOUT_GND 10
#define DSOUT_VCC 11


LiquidCrystal_I2C lcd(0x27, 20, 4);

ServoTimer2 motor;

OneWire oneWireIn(DSIN);
  DallasTemperature sensorin(&oneWireIn);
OneWire oneWireOut(DSOUT);
  DallasTemperature sensorout(&oneWireOut);

// sensors vars
int temp_limit = 35;
int temp_hyst = 2;
float temp_current = 0;
float temp_error = -127.0;
bool high_temp = false;

// lcd lines length
char line0[20]; 
char line1[20];
char line2[20];
char line3[20];

// motor vars
int js_position = 800;
int max_position = 1600;
int min_position = 1000;
int cool_position = 1400;
int test_position = 1200;

void readDS() {
  sensorin.requestTemperatures();
  sensorout.requestTemperatures();

  if (sensorout.getTempCByIndex(0) != temp_error) {
    temp_current = sensorout.getTempCByIndex(0);
  } else {
    temp_current = 0;
  }
}

void updateDisplay() {
   char floatemp_str[7];
   dtostrf(temp_current,4,1,floatemp_str);

   sprintf(line0, "Current Temp: %-5s", floatemp_str);

   lcd.setCursor(0,0);
   lcd.print(line0);
   lcd.setCursor(0,1);
   lcd.print(line1);
   lcd.setCursor(0,2);
   lcd.print(line2);
   lcd.setCursor(0,3);
   lcd.print(line3);
}

void initMotor() {
  sprintf(line0, "ESC calibration: ");
  lcd.print(line0);
  lcd.blink();
  
  motor.attach(MOTOR);
  motor.write(min_position);
  delay(10000);
 
  motor.write(test_position);
  delay(5000);

  motor.write(min_position);
  delay(20);
  
  lcd.noBlink();
  lcd.clear();
  lcd.setCursor(4,1);
  lcd.print("Calibrated!");
  delay(3000);
  lcd.clear();
}

void maxFan() {
  motor.write(test_position);
  delay(5000);
  motor.write(cool_position);
  delay(30000);
  motor.write(min_position);
  delay(20);
}

void setup(void) {
  Serial.begin(9600);

  // VCC and GND pinouts
  pinMode(RELAYVCC, OUTPUT);
    digitalWrite(RELAYVCC, 1);
  pinMode(RELAYGND, OUTPUT);
    digitalWrite(RELAYGND, 0);
  pinMode(LCDVCC, OUTPUT);
    digitalWrite(LCDVCC, 1);
  pinMode(DSIN_GND, OUTPUT);
    digitalWrite(DSIN_GND, 0);
  pinMode(DSOUT_VCC, OUTPUT);
    digitalWrite(DSOUT_VCC, 1);
  pinMode(DSOUT_GND, OUTPUT);
    digitalWrite(DSOUT_GND, 0);  

  // Relay initial
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, 1);

  // Sensors initial
  sensorin.begin();
  sensorout.begin();

  // LCD initial
  lcd.begin();
//  lcd.backlight();

  // ESC initial
  initMotor();
}

// Main loop

void loop(void) {
  readDS();
    Serial.println(temp_current);

  updateDisplay();

  if (temp_current > temp_limit) {
    digitalWrite(RELAY, 0);
    do {
      readDS();
        Serial.println(temp_current);

      updateDisplay();
      delay(1000);
      
    } while (temp_current > (temp_limit - temp_hyst));
    digitalWrite(RELAY, 1);
  }
  delay(1000);
}
