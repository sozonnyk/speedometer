#include "speedometer.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FlorenceBlackNum21pt.h"

#include <BH1750.h>
#include <NMEAGPS.h>
#include <NeoSWSerial.h>
#include "GPSport.h"
#include "Streamers.h"

#define DEBUG

#ifdef DEBUG
#define DEBUG_SPEED 115200
#define D(input) {Serial.print(input); Serial.flush();}
#define Dln(input) {Serial.println(input); Serial.flush();}
#else
#define D(input)
#define Dln(input)
#endif

#define MAX_PRECHARGE 0xF1
#define MAX_VCOM  0x40
#define MIN_LUX  10
#define MAX_LUX 500

#define GPS_SPEED 19200
#define DISP_ADDR  0x3C // initialize with the I2C addr 0x3C (for the 128x32)


BH1750 lightMeter;
Adafruit_SSD1306 display(0);
static NMEAGPS gps;


static void GPSisr(uint8_t c) {
	gps.handle(c);
} // GPSisr

void adjust_brightness() {
	uint16_t lux = lightMeter.readLightLevel();

//	D("Light: ");
//	Dln(lux);

	lux = constrain(lux, MIN_LUX, MAX_LUX);
	byte precharge = map(lux, MIN_LUX, MAX_LUX, 0, MAX_PRECHARGE);
	byte vcom = map(lux, MIN_LUX, MAX_LUX, 0, MAX_VCOM);
	byte contrast = map(lux, MIN_LUX, MAX_LUX, 0, 255);

//	D("Precharge: ");
//	D(precharge);
//	D(" Vcom: ");
//	D(vcom);
//	D(" Contrast: ");
//	Dln(contrast);

	display.ssd1306_command(SSD1306_SETPRECHARGE);
	display.ssd1306_command(precharge);
	display.ssd1306_command(SSD1306_SETVCOMDETECT);
	display.ssd1306_command(vcom);
	display.ssd1306_command(SSD1306_SETCONTRAST);
	display.ssd1306_command(contrast);

}

void display_speed(float speed) {
	char buf[6];
	dtostrf(speed, 1, 0, buf);
	display.clearDisplay();
	display.setCursor(0, 29);
	D("Print speed: ")
	Dln(buf);
	display.println(buf);
	display.display();
}

void display_blank() {
	display.clearDisplay();
	display.display();
}

void display_init() {
		display.begin(SSD1306_SWITCHCAPVCC, DISP_ADDR);
		display.setFont(&FlorenceBlack21pt7b);
		display.setTextColor(WHITE);
		display_blank();

//		display.setCursor(0, 29);
//		display.println("000.0");
//		display.display();
//
//		delay(5000);
}

void setup() {
	Serial.begin(DEBUG_SPEED);
	Dln("Start");

	gps_port.attachInterrupt(GPSisr);
	gps_port.begin(GPS_SPEED);

	lightMeter.begin(BH1750_CONTINUOUS_HIGH_RES_MODE);
	display_init();

}

void loop() {
	if (gps.available()) {

		//Dln("GPS available: ");
		gps_fix fix = gps.read();
		if (fix.valid.speed) {
			float speed = fix.speed_kph();
			D(" hdop: ");
			D(fix.hdop);

			D(" Speed: ");
			Dln(speed);
			display_speed(speed);
		} else {

			Dln("Speed invalid.")
			display_blank();
		}
	}

	if (gps.overrun()) {
		gps.overrun(false);
		Dln("GPS DATA OVERRUN: took too long to use gps.read() data!");
	}

	adjust_brightness();
}
