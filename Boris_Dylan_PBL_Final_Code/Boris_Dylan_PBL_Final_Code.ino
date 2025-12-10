// Serial communication protocols
#include <SPI.h> // SPI
#include <Wire.h> //I2C

// Web Server
#include <WiFi.h>
#include <WebServer.h>

// OLED Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BME280 Sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ArduinoJson.h> // JSON

// OLED Display settings and pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     17
#define OLED_CS     5
#define OLED_RESET  16

// LED pins
#define LED_R 25
#define LED_G 26
#define LED_B 27

// Alert thresholds. These can be changed.
#define HIGH_TEMP 28
#define MIN_PRESSURE 1000
#define HIGH_HUM 70

// Initializing variables for data that will be collected from the BME280 Sensor
float temperatureC = 0;
float humidity = 0;
float pressureHPA = 0;

// WiFi configuration information
const String ssid = "ESP32_RoomSensor_Boris_Dylan";
const String password = "12345678";

unsigned long currentTimestamp = 0; // Variable used for timestamping

// Initializing library objects (display, sensor and server)
// Reference: https://www.electronicshub.org/esp32-oled-display/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS); 

// References: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/examples/bme280test/bme280test.ino
// https://zhillan-arf.medium.com/using-bme280-to-display-temperature-humidity-and-pressure-with-esp32-91333957c9e
Adafruit_BME280 bme; 

// Reference: https://randomnerdtutorials.com/esp32-web-server-arduino-ide/
WebServer server(80);

// Function declaration
void initOLED();
void initBME();
void initLED();
void initWiFi();

void readSensorValues();
void updateDisplay();
void printToSerial();
void updateLEDAlerts();

void handleWebRoot();
void handleSensorData();
String generateHTML();

void setup() {
  // Beggining serial monitor at 115200 baud for debug and to view the data before it is logged (in this code, the data is not saved to anywhere)
  Serial.begin(115200);
  delay(300);

  // Initialization (more detailed comments above each function definition)
  initOLED();
  initBME();
  initLED();
  initWiFi();

  // Creating server routes to ensure the servers functionality and beginning the server
  server.on("/", handleWebRoot); // Overall website code
  server.on("/sleep", enterDeepSleep); // Deep sleep mode for the esp32
  server.begin();
  // Reference: Lab 8 - ESP32 WiFi Access Point & Web-Controlled RGB LED
}

// The loop updates the system every 2 seconds. What it does is intuitive to understand, because the function name are straight forward
void loop() {
  server.handleClient(); // Reference: Lab 8 - ESP32 WiFi Access Point & Web-Controlled RGB LED

  readSensorValues();
  updateDisplay();
  updateLEDAlerts();
  handleSensorData(); // Creating a JSON document with the collected data

  delay(2000);
}

// Initialization of the OLED display with '.begin'. Displays a message to visually confirm that the display works 
void initOLED() {
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("OLED Ready");
  display.display();
  delay(500);
} // Reference: https://www.electronicshub.org/esp32-oled-display/

// Initialization of the BME280 Sensor. Displays a message on the display to confirm that it works
void initBME() {
  bool status = bme.begin(0x76); // 0x76 address is used to identify the hardware sensor with the I2C protocol. This parameter has to be set for the sensor to work
  if (status){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("BME Ready");
    display.display();
  }
  
  delay(500);
} // Reference: https://zhillan-arf.medium.com/using-bme280-to-display-temperature-humidity-and-pressure-with-esp32-91333957c9e


// Set the pin mode for the LED pins to OUTPUT
void initLED() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

// Initializing the WiFi server on the esp32 using the configuration information set above
void initWiFi() {
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP()); // This prints the IP address of the web server. This is later used to access the web server in a web browser
} // Reference: https://randomnerdtutorials.com/esp32-web-server-arduino-ide/

// Reads the values of the BME280 Sensor
void readSensorValues() {
  currentTimestamp = millis(); // Creates a timestamp
  temperatureC = bme.readTemperature();
  humidity = bme.readHumidity();
  pressureHPA = bme.readPressure() / 100.0F;
} // Reference: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/examples/bme280test/bme280test.ino

// Updates the display to show current sensor readings
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
} // Reference: https://www.electronicshub.org/esp32-oled-display/

// ALERTS. Update the LED color and the message printed (or not) on the display based on the collected data from the sensor
void updateLEDAlerts() {
  int r = 0, g = 255, b = 0; // green, everything is working normally

  if (temperatureC > HIGH_TEMP) { r = 255; g = 0; b = 0; display.println("TEMPERATURE TOO HIGH");} // red
  if (humidity > HIGH_HUM) { r = 0; g = 0; b = 255; display.println("HUMIDITY TOO HIGH");} // blue
  if (pressureHPA < MIN_PRESSURE) { r = 0; g = 255; b = 255; display.println("PRESSURE TOO LOW");} // cyan
  if (temperatureC > HIGH_TEMP && humidity > HIGH_HUM) {r = 255; g = 0; b = 255; display.println("TEMP AND HUM TOO HIGH");} // purple
  if (temperatureC > HIGH_TEMP && pressureHPA < MIN_PRESSURE) {r = 255; g = 255; b = 0; display.println("TEMPERATURE HIGH, PRESSURE LOW");} // yellow
  if (humidity > HIGH_HUM && pressureHPA < MIN_PRESSURE) {r = 255; g = 255; b = 255; display.println("HUM HIGH, PRES HIGH");} // white

  if (temperatureC > HIGH_TEMP && humidity > 70 && pressureHPA < 1000) {
    r = 0; g = 0; b = 0; display.println("OUT OF CONTROL"); // black, very special case
  } 

  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
  display.display();
}

// Sends the html code created in generateHTML() to the web server
void handleWebRoot() {
  server.send(200, "text/html", generateHTML());
} // Reference: Lab 8 - ESP32 WiFi Access Point & Web-Controlled RGB LED

// Creates HTML code for the website hosted on the web server 
String generateHTML() {
  String page = "<html><head><meta http-equiv='refresh' content='3'/>";
  page += "<style>body{font-family:Arial;background:#111;color:white;text-align:center;}table{margin:auto;font-size:22px;}</style></head>"; // style
  page += "<body><h1>ESP32 Room Sensor</h1>";

  // Displaying the sensor readings on the web server
  page += "<table>";
  page += "<tr><td>Temperature:</td><td>" + String(temperatureC) + " C</td></tr>";
  page += "<tr><td>Humidity:</td><td>" + String(humidity) + " %</td></tr>";
  page += "<tr><td>Pressure:</td><td>" + String(pressureHPA) + " hPa</td></tr>";
  page += "<tr><td>Timestamp:</td><td>" + String(currentTimestamp) + "</td></tr>";
  page += "</table>";
  page += "</body>";

  // Creates a button that redirect to /sleep. When the button is pressed, enterDeepSleep() runs which puts the esp32 in sleep mode
  page += "<form action=\"/sleep\" method=\"POST\">";
  page += "<button style='padding:12px; font-size:18px';'>Enter Deep Sleep</button>";
  page += "</form></html>";

  return page;
} // References: https://randomnerdtutorials.com/esp32-web-server-arduino-ide/
// Lab 8 - ESP32 WiFi Access Point & Web-Controlled RGB LED 
//

// Create a JSON document containing the timestamp and the sensor readings
void handleSensorData(){
  DynamicJsonDocument doc(128);
  doc["Timestamp"] = currentTimestamp;
  doc["Temperature"] = temperatureC;
  doc["Humidity"] = humidity;
  doc["Pressure"] = pressureHPA;

  String outputString;
  serializeJsonPretty(doc, outputString); // Make the JSON document printable on the serial monitor
  Serial.println(outputString);
} // References: Lab 10 - ESP32 Smart Plant Monitoring System, 
// https://arduinojson.org, https://randomnerdtutorials.com/decoding-and-encoding-json-with-arduino-or-esp8266/
// https://www.programiz.com/html/form-action


// Makes the ESP32 sleep for 30 seconds (the entire system stops working, including the web server)
void enterDeepSleep() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Sleeping...");
  display.display();

  delay(1000);

  // Built-in functions
  esp_sleep_enable_timer_wakeup(30 * 1000000ULL);
  esp_deep_sleep_start();
} // Reference: https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
