#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     17
#define OLED_CS     5
#define OLED_RESET  16

#define LED_R 25
#define LED_G 26
#define LED_B 27

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

Adafruit_BME280 bme;

const String ssid = "ESP32_RoomSensor_Boris_Dylan";
const String password = "12345678";

WebServer server(80);

float temperatureC = 0;
float humidity = 0;
float pressureHPA = 0;

unsigned long currentTimestamp = 0;

void initOLED();
void initBME();
void initLED();
void initWiFi();

void readSensorValues();
void updateDisplay();
void printToSerial();
void updateLEDAlerts();

void handleWebRoot();
void handleJSON();
String generateHTML();
String generateJSON();

void setup() {
  Serial.begin(115200);
  delay(300);

  initOLED();
  initBME();
  initLED();
  initWiFi();

  server.on("/", handleWebRoot);
  server.on("/data.json", handleJSON);
  server.on("/sleep", enterDeepSleep);
  server.begin();
}

void loop() {
  server.handleClient();

  readSensorValues();
  updateDisplay();
  updateLEDAlerts();
  printToSerial();

  delay(2000);
}

void initOLED() {
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("OLED Ready");
  display.display();
  delay(500);
}

void initBME() {
  bme.begin(0x76);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("BME Ready");
  display.display();
  delay(500);
}

void initLED() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void initWiFi() {
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void readSensorValues() {
  currentTimestamp = millis();
  temperatureC = bme.readTemperature();
  humidity = bme.readHumidity();
  pressureHPA = bme.readPressure() / 100.0F;
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println(" Indoor Climate ");
  display.println("--------------------");

  display.print("Temp: ");
  display.print(temperatureC);
  display.println(" C");

  display.print("Hum : ");
  display.print(humidity);
  display.println(" %");

  display.print("Pres: ");
  display.print(pressureHPA);
  display.println(" hPa");

  display.print("Time: ");
  display.println(currentTimestamp);

  display.display();
}

void updateLEDAlerts() {
  int r = 0, g = 255, b = 0;

  if (temperatureC > 28) { r = 255; g = 0; b = 0; }
  else if (humidity > 70) { r = 128; g = 0; b = 128; }
  else if (pressureHPA < 1000) { r = 255; g = 255; b = 0; }

  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void printToSerial() {
  Serial.println(generateJSON());
}

void handleWebRoot() {
  server.send(200, "text/html", generateHTML());
}

void handleJSON() {
  server.send(200, "application/json", generateJSON());
}

String generateHTML() {
  String page = "<html><head><meta http-equiv='refresh' content='3'/>";
  page += "<style>body{font-family:Arial;background:#111;color:white;text-align:center;}table{margin:auto;font-size:22px;}</style></head>";
  page += "<body><h1>ESP32 Room Sensor</h1>";
  page += "<table>";
  page += "<tr><td>Temperature:</td><td>" + String(temperatureC) + " C</td></tr>";
  page += "<tr><td>Humidity:</td><td>" + String(humidity) + " %</td></tr>";
  page += "<tr><td>Pressure:</td><td>" + String(pressureHPA) + " hPa</td></tr>";
  page += "<tr><td>Timestamp:</td><td>" + String(currentTimestamp) + "</td></tr>";
  page += "</table>";
  page += "</body>";

  page += "<form action=\"/sleep\" method=\"POST\">";
  page += "<button style='padding:12px; font-size:18px';'>Enter Deep Sleep</button>";
  page += "</form></html>";

  return page;
}

String generateJSON() {
  String json = "{";
  json += "\"timestamp\":" + String(currentTimestamp) + ",";
  json += "\"temperature\":" + String(temperatureC) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"pressure\":" + String(pressureHPA);
  json += "}";
  return json;
}

void enterDeepSleep() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Sleeping...");
  display.display();

  delay(1000);

  esp_sleep_enable_timer_wakeup(30 * 1000000ULL);
  esp_deep_sleep_start();
}

