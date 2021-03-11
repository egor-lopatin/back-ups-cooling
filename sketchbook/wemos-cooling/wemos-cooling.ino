#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "WIFISPOT2G"
#define STAPSK  "Zr$NR49s4%BcUyRPCXe6"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Relay
#define RELAY 4
#define RELAYGND 5

// Inside temp sensor
#define DSIN 2
//#define DSIN_VCC 9
//#define DSIN_GND 10

// Outside temp sensor
#define DSOUT 16
#define DSOUT_VCC 14
#define DSOUT_GND 12

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

String getJson() {
    DynamicJsonDocument response(256);
    response["ups_temp"] = t_in_current;
    response["room_temp"] = t_out_current;

    String jsonAnswer;
    serializeJson(response, jsonAnswer);
    return jsonAnswer;
}

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");

// Relay init
  pinMode(RELAYGND, OUTPUT);
  digitalWrite(RELAYGND, 0);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, 1);

// Inside temp sensor init
//  pinMode(DSIN_VCC, OUTPUT);
//  digitalWrite(DSIN_VCC, 1);
//  pinMode(DSIN_GND, OUTPUT);
//  digitalWrite(DSIN_GND, 0);

// Outside temp sensor init
//  pinMode(DSOUT_VCC, OUTPUT);
//  digitalWrite(DSOUT_VCC, 1);
//  pinMode(DSOUT_GND, OUTPUT);
//  digitalWrite(DSOUT_GND, 0);

  sensorin.begin();
  sensorout.begin();

  initDS();

  delay(3000);
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();

  readDS();
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

  if (client) {
    Serial.println("New client");
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && blank_line) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Server: Wemos_D1_mini");
            client.println("Connection: close");
            client.println();
            client.println(getJson());
            break;
        }
        if (c == '\n') {
          blank_line = true;
        }
        else if (c != '\r') {
          blank_line = false;
        }
      }
    }  
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
