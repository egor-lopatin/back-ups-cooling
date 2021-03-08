#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

// Relay
#define RELAY 4
#define RELAYGND 5
#define RELAYVCC 6

// Inside temp sensor
#define DSIN_VCC 9
#define DSIN_GND 10
#define DSIN 11

// Outside temp sensor
#define DSOUT_VCC 8
#define DSOUT_GND 13
#define DSOUT 12

OneWire oneWireIn(DSIN);
DallasTemperature sensorin(&oneWireIn);
OneWire oneWireOut(DSOUT);
DallasTemperature sensorout(&oneWireOut);

float t_error = -127.0;
float t_min = 0.0;

int t_in_limit = 35;
int t_in_hyst = 3;
float t_in_current = 0;
bool t_in_enabled = false;

int t_out_limit = 40;
int t_out_hyst = 2;
float t_out_current = 0;
bool t_out_enabled = false;
bool t_out_high = false;

void initDS() {
  sensorin.requestTemperatures();
  if (sensorin.getTempCByIndex(0) != t_error) {
    t_in_enabled = true;
  }

  sensorout.requestTemperatures();
  if (sensorout.getTempCByIndex(0) != t_error) {
    t_out_enabled = true;
  }
}

void readDS() {
  if (t_in_enabled == true) {
    do {
      sensorin.requestTemperatures();
      t_in_current = sensorin.getTempCByIndex(0);
    } while (sensorin.getTempCByIndex(0) == t_error || sensorin.getTempCByIndex(0) < t_min);

  }

  if (t_out_enabled == true) {
    do {
      sensorout.requestTemperatures();
      t_out_current = sensorout.getTempCByIndex(0);
    } while (sensorout.getTempCByIndex(0) == t_error || sensorout.getTempCByIndex(0) < t_min);
  }
}

void printSensors() {
  char t_in[7];
  char t_out[7];
  char serial_out[20];

  dtostrf(t_in_current, 4, 1, t_in);
  dtostrf(t_out_current, 4, 1, t_out);
  sprintf(serial_out, "%s %s", t_in, t_out);

  Serial.println(serial_out);
}

void setup(void) {
  Serial.begin(9600);

// Relay init
  pinMode(RELAYVCC, OUTPUT);
  digitalWrite(RELAYVCC, 1);
  pinMode(RELAYGND, OUTPUT);
  digitalWrite(RELAYGND, 0);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, 1);

// Inside temp sensor init
  pinMode(DSIN_VCC, OUTPUT);
  digitalWrite(DSIN_VCC, 1);
  pinMode(DSIN_GND, OUTPUT);
  digitalWrite(DSIN_GND, 0);

// Outside temp sensor init
  pinMode(DSOUT_VCC, OUTPUT);
  digitalWrite(DSOUT_VCC, 1);
  pinMode(DSOUT_GND, OUTPUT);
  digitalWrite(DSOUT_GND, 0);

  sensorin.begin();
  sensorout.begin();
}

void loop(void) {
  readDS();
  printSensors();

  if (digitalRead(RELAY) == 1) {
    if (t_in_current > t_in_limit ) {
      digitalWrite(RELAY, 0);
    }
  } else if (digitalRead(RELAY) == 0) {
    if (t_in_current < (t_in_limit - t_in_hyst)) {
      digitalWrite(RELAY, 1);
    }
  }

  delay(1000);
}
