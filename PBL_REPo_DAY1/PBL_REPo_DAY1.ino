//  Includes & Libraries 
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//  Pin Definitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     17
#define OLED_CS     5
#define OLED_RESET  16

#define SEALEVELPRESSURE_HPA (1013.25)

// RGB LED pins
#define LED_R 25
#define LED_G 26
#define LED_B 27

//  Global Objects & Variables
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

Adafruit_BME280 bme;

// Wi-Fi settings
const String ssid = "ESP32_RoomSensor_Boris_Dylan";
const String password = "12345678";

WebServer server(80);

float temperatureC = 0;
float humidity = 0;
float pressureHPA = 0;

unsigned long lastLogTime = 0;
unsigned long logInterval = 5000; // log to CSV every 5 sec

//  Function Declaration        
void initOLED();
void initBME();
void initLED();
void initWiFi();
void initSPIFFS();

void readSensorValues();
void updateDisplay();
void printToSerial();
void updateLEDAlerts();
void logToCSV();

void enterDeepSleep();

void handleWebRoot();
String generateHTML();

//  SETUP
void setup() {
  Serial.begin(115200);
  delay(500);

  initOLED();
  initBME();
  initLED();
  initSPIFFS();
  initWiFi();

  // Web handlers
  server.on("/", handleWebRoot);
  server.begin();
}

//  LOOP
void loop() {
  server.handleClient();        // handle web server

  readSensorValues();           // get sensor values
  updateDisplay();              // update OLED
  updateLEDAlerts();            // update RGB LED alerts
  printToSerial();              // serial monitor

  // CSV logging
  if (millis() - lastLogTime >  logInterval) {
    logToCSV();
    lastLogTime = millis();
  }

  delay(2000);
}
//  MODULE: OLED Initialization
void initOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED FAILED");
  } 

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("OLED Ready");
  display.display();
  delay(1000);
}

//  MODULE: BME280 Initialization
void initBME() {
  if (!bme.begin(0x76)) {
    Serial.println("BME280 ERROR");
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("BME280 Ready");
  display.display();
  delay(1000);
}

//  MODULE: RGB LED Initialization
void initLED() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

//  MODULE: Wi-Fi Access Point
void initWiFi() {
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}
                
//  MODULE: SPIFFS Initialization                   
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }
  Serial.println("SPIFFS Ready");
}
              
//  MODULE: Sensor Reading                   
void readSensorValues() {
  temperatureC = bme.readTemperature();
  humidity = bme.readHumidity();
  pressureHPA  = bme.readPressure() / 100.0F;
}

//  MODULE: OLED Update
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

  display.display();
}

//  MODULE: RGB LED Alerts

void updateLEDAlerts() {
  // normal: green
  int r = 0, g = 255, b = 0;

  if (temperatureC > 28) {         // too hot
    r = 255; g = 0; b = 0;
  }
  else if (humidity > 70) {        // too humid
    r = 128; g = 0; b = 128;
  }
  else if (pressureHPA < 1000) {   // low pressure
    r = 255; g = 255; b = 0;
  }

  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}
//  MODULE: Serial Debug Output
void printToSerial() {
  Serial.print("Temp: "); Serial.println(temperatureC);
  Serial.print("Hum : "); Serial.println(humidity);
  Serial.print("Pres: "); Serial.println(pressureHPA);
  Serial.println("-----------------------");
}

//  MODULE: CSV Logging
void logToCSV() {
  File file = SPIFFS.open("/data.csv", FILE_APPEND);
  if (!file) return;

  file.printf("%lu,%.2f,%.2f,%.2f\n", millis(), temperatureC, humidity, pressureHPA);
  file.close();

  Serial.println("Logged to CSV");
}
//  MODULE: Web Dashboard
void handleWebRoot() {
  server.send(200, "text/html", generateHTML());
}

String generateHTML() {
  String page = "<html><head><meta http-equiv='refresh' content='3'/>";
  page += "<style>body{font-family:Arial;background:#111;color:white;text-align:center;}";
  page += "h1{color:#00FFAA;} table{margin:auto; font-size:22px;}</style></head>";
  page += "<body><h1>ESP32 Room Sensor</h1>";
  page += "<table>";
  page += "<tr><td>Temperature:</td><td>" + String(temperatureC) + " C</td></tr>";
  page += "<tr><td>Humidity:</td><td>" + String(humidity) + " %</td></tr>";
  page += "<tr><td>Pressure:</td><td>" + String(pressureHPA) + " hPa</td></tr>";
  page += "</table><br><p>Updated automatically every 3 seconds</p></body></html>";
  return page;
}
//  MODULE: Deep Sleep (Optional Call)
void enterDeepSleep() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sleeping...");
  display.display();

  delay(1000);

  esp_sleep_enable_timer_wakeup(30 * 1000000); // 30 sec
  esp_deep_sleep_start();
}