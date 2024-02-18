#include <ESP8266WiFi.h>
#include <WiFiServerSecure.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HTTPSRedirect.h>
#include <BlynkSimpleEsp8266.h>

#define DHTPIN 13       // D7
#define DHTTYPE DHT11
#define dspin 4         //D2
#define soilPin A0
#define numReadings 10
#define relay 2 //D4
#define led 5 //D1
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLixbBKp1X"
#define BLYNK_TEMPLATE_NAME "Skripsi NodeMCU Backprop"
#define BLYNK_AUTH_TOKEN "9gQknwYL0xeIi7dfAqAPDs7lqXXU51M4"
#define ssid "IoT"
#define pass "mhmdroji123"
#define alpha  0.01

OneWire pin_DS18B20(dspin);
DallasTemperature ds(&pin_DS18B20);
DHT dht(DHTPIN, DHTTYPE);
HTTPSRedirect* client = nullptr;
WidgetLCD lcd(V0);
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V7);}

int dataSoil[numReadings];
int totalData = 0;
int currentIndex = 0;
const char* GScriptId = "AKfycbyBX9eIOgX0xt0lh1il8kE81XvYdfNRHR109golFctQ1JdqQRKFt3aXMQqYlqdMHk4g";
String payload_base = "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
float varLama[3] = {0.0, 0.0, 0.0};

void setup(){
  Serial.begin(9600); dht.begin(); ds.begin();
  pinMode(soilPin, INPUT); pinMode(relay, OUTPUT); pinMode(led, OUTPUT);
  digitalWrite(relay, HIGH); digitalWrite(led, LOW);

  WiFi.disconnect(true);WiFi.mode(WIFI_OFF);
  WiFi.begin(ssid, pass); WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("");
  Serial.print("Terhubung ke WiFi. IP Address: ");
  Serial.println(WiFi.localIP());

  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str(),"blynk.cloud", 8080);
  Serial.println("Terhubung ke server Blynk!");
  digitalWrite(led, HIGH);

  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
}

void loop(){
  lcd.clear();
  float inputs[3];
  float sensors[3];
  float output; 
  
  sensors[0] = bacaSoil();
  sensors[1] = bacaDs();
  sensors[2] = bacaDht();

  Serial.print("Sensor: ");
  print(sensors);

  inputs[0] = minMaxNorm(sensors[0], 0, 100, 0, 1);
  inputs[1] = minMaxNorm(sensors[1], 0, 100, 0, 1);
  inputs[2] = minMaxNorm(sensors[2], 0, 100, 0, 1);

  Serial.print("Input: ");
  print(inputs);

  output = forwardprop(inputs);
  Serial.print("Output :");
  Serial.print(output); Serial.print("\t");

  float dataUpload[7] = {sensors[0], sensors[1], sensors[2], inputs[0], inputs[1], inputs[2], output};
  waterPump(output);
  kirimKeBlynk(dataUpload);
  if(checkBeda(sensors)) kirimKeGoogle(dataUpload);
  if(!checkBeda(sensors)) Serial.print("data sama\n");
  updateVarLama(sensors);
  delay(1000);
}

float bacaSoil() {
  dataSoil[currentIndex] = minMaxNorm(analogRead(soilPin), 1023, 0, 0, 100);
  currentIndex = (currentIndex + 1) % numReadings;
  if (totalData < numReadings) {
    totalData++;
  }
  float sum = 0;
  for (int i = 0; i < totalData; i++) {
    sum += dataSoil[i];
  }
  return totalData > 0 ? sum / totalData : 0;
}

float bacaDs() {
  ds.requestTemperatures();
  lcd.print(0, 0, ds.getTempCByIndex(0) == -127 ? "DS Off." : "DS On..");
  return ds.getTempCByIndex(0) == -127 ? 0 : ds.getTempCByIndex(0);
}

float bacaDht() {
  lcd.print(8, 0, isnan(dht.readHumidity()) ? "DHT Off" : "DHT On.");
  return isnan(dht.readHumidity()) ? 0 : dht.readHumidity();
}

float minMaxNorm(float x, float minVal, float maxVal, float newMin, float newMax) {
  return ((x - minVal) / (maxVal - minVal)) * (newMax - newMin) + newMin;
}

float leaky_relu(float x) {
  return x >= 0 ? x : alpha * x;
}

float forwardprop(float inputs[3]) {
  float weights_hidden[3][5] = {{-0.5781842,  -0.6468529,   2.2854223,  1.066174,     -0.5128325 },
                                { 0.7714282,  0.6958194,    0.2581082,  -0.00859765,  0.5971974 },
                                {0.20616204,  -0.40851936,  -0.2185891, 0.6193374,    -0.43563563}};
  float bias_hidden[5] = {-0.08098416, -0.04600018, -0.97094357, -0.22953583,  0.83695394};
  float weights_output[5] = {-0.67651397, -0.15414384, -2.479524, -0.8567315, 1.4811417};
  float bias_output = 0.81531256;
  float hidden_output[5] = {0};

  for (int i = 0; i < 5; i++) {
    float sum = bias_hidden[i];
    for (int j = 0; j < 3; j++) {
      sum += inputs[j] * weights_hidden[j][i];
    }
    hidden_output[i] = leaky_relu(sum);
  }

  float output = bias_output;
  for (int i = 0; i < 5; i++) {
    output += hidden_output[i] * weights_output[i];
  }
  return output;
}

void kirimKeGoogle(float values[]) {
  static bool flag = false;
  if (!flag) {
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr) {
    if (!client->connected()) {
      client->connect(host, httpsPort);
    }
  } else {
    Serial.println("gagal membuat objek klien!");
    return;
  }
  
  String payload = payload_base + "\"";
  for (int i = 0; i < 7; ++i) {
    payload += String(values[i]);
    if (i < 7 - 1) {
      payload += ",";
    }
  }
  payload += "\"}";

  Serial.print("\tupload data...");
  if (client->POST(url, host, payload)) {
  } else {
    Serial.print("\tkoneksi error");
  }
}

void kirimKeBlynk(float values[]) {
  for (int i = 0; i < 7; ++i) {
    Blynk.virtualWrite(V1 + i, values[i]);
  }
}
void waterPump(float x) {
  digitalWrite(relay, x >= 0.9 ? LOW : HIGH);
  lcd.print(1, 1, x >= 0.9 ? "WaterPump On" : "WaterPump Off");
}

void updateVarLama(float sensors[]) {
  for (int i = 0; i < 3; ++i) {
    varLama[i] = sensors[i];
  }
}

bool checkBeda(float sensors[]) {
  for (int i = 0; i < 3; ++i) {
    if (sensors[i] != varLama[i]) {
      return true;
    }
  }
  return false;
}

void print(float x[]){
  for (int i = 0; i < 3; i++) {
    Serial.print(x[i]);
    Serial.print("\t");
  }
}