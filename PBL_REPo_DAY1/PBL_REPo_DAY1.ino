#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// OLED pins
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC    16
#define OLED_CS    5
#define OLED_RESET 19



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // initialize the OLED object

 if(!display.begin(SSD1306_SWITCHCAPVCC)) {

   Serial.println(F("SSD1306 allocation failed"));

   for(;;); // Don't proceed, loop forever

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
 
  delay(2000);
}
