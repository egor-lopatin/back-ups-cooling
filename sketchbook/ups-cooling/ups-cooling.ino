#include <LiquidCrystal_I2C.h>
#include <ServoTimer2.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Wire.h>

#define LCDVCC 2
#define MOTOR 3
#define RELAYGND 4
#define RELAY 6
#define RELAYVCC 5
#define DSOUT 12
#define DSOUT_GND 13
#define DSIN 9
#define DSIN_GND 10
#define DSIN_VCC 11

LiquidCrystal_I2C lcd(0x27, 20, 4);
ServoTimer2 motor;
OneWire oneWireIn(DSIN);
DallasTemperature sensorin(&oneWireIn);
OneWire oneWireOut(DSOUT);
DallasTemperature sensorout(&oneWireOut);

// sensors vars
float t_error = -127.0;

// ups sensor
int t_in_limit = 33;
int t_in_hyst = 2;
float t_in_current = 0;
bool t_in_high = false;

// out sensor
int t_out_limit = 28;
int t_out_hyst = 2;
float t_out_current = 0;
bool t_out_high = false;

// lcd lines length
char line0[21]; 
char line1[21];
char line2[21];
char line3[21];

// motor vars
int js_position = 800;
int max_position = 1600;
int min_position = 1000;
int cool_position = 1400;
int test_position = 1200;

// Functions

void readDS() {
  sensorin.requestTemperatures();
  sensorout.requestTemperatures();

  if (sensorin.getTempCByIndex(0) != t_error) {
     t_in_current = sensorin.getTempCByIndex(0);
  } else {
     t_in_current = 0;
  }

  if (sensorout.getTempCByIndex(0) != t_error) {
    t_out_current = sensorout.getTempCByIndex(0);
  } else {
    t_out_current = 0;
  }
}

void updateDisplay() {
   char float_in_str[7];
   dtostrf(t_in_current,4,1,float_in_str);

   char float_out_str[7];
   dtostrf(t_out_current,4,1,float_out_str);

   sprintf(line0, "UPS Temp: %-10s", float_in_str);
   sprintf(line1, "Room Temp: %-9s", float_out_str);

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

void printSensors() {
  char t_in[50];
  char t_out[50];
  sprintf(t_in, "%s ", t_in_current);
  sprintf(t_out, "%s", t_out_current);

//  Serial.print(t_in);
//  Serial.println(t_out);
  Serial.println(t_in_current);
//  Serial.println(t_out_current);
}

// Setup

void setup(void) {
  Serial.begin(9600);

  pinMode(RELAYVCC, OUTPUT);
    digitalWrite(RELAYVCC, 1);
  pinMode(RELAYGND, OUTPUT);
    digitalWrite(RELAYGND, 0);
  pinMode(RELAY, OUTPUT);
    digitalWrite(RELAY, 1);
  pinMode(LCDVCC, OUTPUT);
    digitalWrite(LCDVCC, 1);
  pinMode(DSOUT_GND, OUTPUT);
    digitalWrite(DSOUT_GND, 0);
  pinMode(DSIN_VCC, OUTPUT);
    digitalWrite(DSIN_VCC, 1);
  pinMode(DSIN_GND, OUTPUT);
    digitalWrite(DSIN_GND, 0);  

  sensorin.begin();
  sensorout.begin();

  lcd.begin();
  lcd.noBacklight();

  initMotor();
}

// Main loop

void loop(void) {
  readDS();
  printSensors();

  if (t_in_high = false) {
      if (t_in_current > t_in_limit ) {
          Serial.println("High temp. Enable relay.");
          digitalWrite(RELAY, 0);
          t_in_high = true;
      }
  } else if (t_in_high = true) {
      if (t_in_current < (t_in_limit - t_in_hyst)) {
          Serial.println("Low temp. Disable relay.");
          digitalWrite(RELAY, 1);
      }
  }

  updateDisplay();
  delay(1000);

}
