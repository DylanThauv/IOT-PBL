// Communication libraries
#include <SPI.h>
#include <Wire.h>

// OLED display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BME280 Sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// OLED pins
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC    16
#define OLED_CS    5
#define OLED_RESET 19

// BME280
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

Adafruit_BME280 bme;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // initialize the OLED object

 if(!display.begin(SSD1306_SWITCHCAPVCC)) {

   Serial.println(F("SSD1306 allocation failed"));

   for(;;); // Don't proceed, loop forever

 }

  // Initialize sensor
  unsigned status;
    
  // default settings
  // status = bme.begin();  
  // You can also pass in a Wire library object like &Wire2
  status = bme.begin(0x76);
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }
 display.display();
 delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
 
  // Display Text
 
  display.setTextSize(1);
 
  display.setTextColor(WHITE);
 
  display.setCursor(0, 28);
 
  display.println("Hello world!");
 
  display.display();
  
  printValues();
  delay(2000);
}

void printValues() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}
