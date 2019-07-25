/*
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <FastLED.h>
#include <fastpin.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <fastspi.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>
*/

//Some light chase hacks for the AM-2640 5V, Addressable LED strips http://www.andymark.com/product-p/am-2640.htm based on the WS2801 chipset
//We ran this demo off of our AM-2287 Arduino Ethernet http://www.andymark.com/product-p/am-2287.htm
//http://arduino.cc/en/Main/ArduinoBoardEthernet
//The FastLED library we use here supports multiple chipsets
//This code requires that the fastspi library be put in your arduino\libraries folder
//Arduino info on how to install software libraries http://arduino.cc/en/Guide/Libraries
//AndyMark, Inc.
//Craig Kessler 12/3/2013, 3/17/2014, 3/20/2014


//***NOTE: This strip runs off of 5V MAX!!!. Applying much more than 5V will damage/destroy you LED strip!***
//***Handling note: Don't mess with the wiring while the power is on. This can cause voltage spikes ***
//***or sneak ground paths that can damage the LED strip ***

//DO NOT try to power the whole strip (80 LEDs) off the arduino 5v regulator.
//At full bright white, the strip can draw 1.5Amps or so. This will overheat or burnout the regulator.
//Remember on the AM-2640 LED strip's yellow wire is ground! (don't blame us they come that way)
//Make sure you connect the Yellow ground from the LED strip to the Arduino ground.
//Communications to the LEDs requires a common ground to work.

//We recommend running these led strips off of the AM-0899 10-30Vin to 5V 5A out stepdown converter http://www.andymark.com/product-p/am-0899.htm

//If you are using the AndyMark AM-2297 Arduino Ethernet board then make sure you select Tools>Board>Arduino Ethernet from the Arduino IDE menu
//If you are new to working with Arduino a good place to start is here http://arduino.cc/en/Guide/HomePage
//Another new training resource provided by a 3rd party is here: http://www.arduinoclassroom.com/index.php/arduino-101



//CSK 3/17/2013 Libraries new location
//https://github.com/FastLED/FastLED
//https://github.com/FastLED/FastLED/wiki/Overview

//#include "FastSPI_LED2.h"
//CSK 3/17/2014 FastSPI library has been updated.  The new header file name is just FastLED.h. FastSPI_LED2.h is now just a shell with an include for FastLED.h
#include "FastLED.h"
#include "Wire.h"
#define LED_PIN 5
#define LED_PIN2 6
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define NUM_LEDS 36
#define BRIGHTNESS 200
#define FRAMES_PER_SECOND 60

//Tell it how many leds are in the strip. AndyMark's 2.5 meter strip has 80 leds

// This is an array of leds. One item for each led in your strip.
CRGB leds[NUM_LEDS];

#define MAX_BRIGHTNESS 200

const int redSwitch = 9;
const int blueSwitch = 8;
//variables:
int LEDRotationCurr = 0;
int redState;
int blueState;
int LEDmode;
int AllianceColor;
CRGB currColor;
uint32_t AllianceRGB;


//This function is used to setup things like pins, Serial ports etc.
//Here we specify which chipset our LEDs run off of by our choice of config function
void setup()
{
	delay(1000);
	pinMode(redSwitch, INPUT_PULLUP);
	pinMode(blueSwitch, INPUT_PULLUP);
	FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	FastLED.addLeds<CHIPSET, LED_PIN2, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	LEDRotationCurr = 0;
	Serial.begin(9600);
	Wire.begin(8);
}

void loop()
{
	// Serial.println("loop");
	Wire.onReceive(dataReceived);
	/*redState = digitalRead(redSwitch);
	blueState = digitalRead(blueSwitch);
	if (redState == LOW) {
		color_chase(CRGB::Red, 75);
	} else if (blueState == LOW) {
		color_chase(CRGB::DarkBlue, 75);
	} else {
		FastLED.setBrightness(0);
		FastLED.show();
	}*/

	switch (AllianceColor){
		case 0: // INVALID
			AllianceRGB = CRGB::Orange;
			break;
		case 1: // BLUE
			AllianceRGB = CRGB::Blue;
			break;
		case 2: // RED
			AllianceRGB = CRGB::Red;
			break;
		default: // SOMETHING BROKE
			AllianceRGB = CRGB::Green;
	}

	switch (LEDmode)
	{
	case 0: // BROKE
		color_chase(AllianceRGB, 100);
		break;
	case 1: // DISABLED
		color_chase(AllianceRGB, 250);	
		break;
	case 2: // AUTO
		color_chase(AllianceRGB, 75);	
		break;
	case 3: // TELEOP
		color_chase(AllianceRGB, 60);	
		break;
	default: // ANOTHER THING BROKE
		color_chase(AllianceRGB, 15);
		break;
	}
}

void dataReceived(int howMany){
	Serial.println("dataRecived");
	String LED = "";
	while (Wire.available()){
		char n = (char)Wire.read();
		if (((int)n) > ((int)(' '))){
			LED += n;
		}
	}

	int iDelimPos = LED.indexOf(",");
	String ParmStatus = "";
	String ParmAlliance = "";
	Serial.println(LED);
	if (iDelimPos == 0) {
		ParmStatus = LED.substring(0);
	} else {
		ParmStatus = LED.substring(0,iDelimPos);
		ParmAlliance = LED.substring(iDelimPos+1);
	}
	Serial.println(ParmStatus);
	Serial.println(ParmAlliance);

	if (ParmStatus == "DISABLED"){
		LEDmode = 1;
	}else if (ParmStatus== "AUTO"){
		LEDmode = 2;
	}else if (ParmStatus == "TELEOP"){
		LEDmode = 3; 
	}else{
		LEDmode = 0;
	}

	if (ParmAlliance == "BLUE"){
		AllianceColor = 1;
	}else if (ParmAlliance == "RED"){
		AllianceColor = 2;
	}else if (ParmAlliance == "INVALID"){
		AllianceColor = 3;
	}else{
		AllianceColor = 0;
	}

}

//These are the functions we have defined to do chase patterns.  They are actually called inside the  loop() above
//They are meant to demonstrate things such as setting LED colors, controlling brightness
void color_chase(uint32_t color, uint8_t wait)
{
	//clear() turns all LEDs off
//	FastLED.clear();
	//The brightness ranges from 0-255
	//Sets brightness for all LEDS at once
	FastLED.setBrightness(MAX_BRIGHTNESS);
	// Move a single led
	for(int led_number = 0; led_number < NUM_LEDS; led_number++)
	{
		// Turn our current led ON, then show the leds

		if (led_number % 4 != LEDRotationCurr) {
			leds[led_number] = color;
		} else {
			leds[led_number] = CRGB::DarkGreen;
		}

	}
	LEDRotationCurr = LEDRotationCurr + 1;
	LEDRotationCurr = LEDRotationCurr % 4;
	FastLED.show();
	delay(wait);
	return;
}

