/*
 * RGBLed.h
 *
 *  Created on: 04.08.2018
 *      Author: Dr. Martin Schaaf
 */

#ifndef RGBLED_H_
#define RGBLED_H_

#include <Arduino.h>

class RGBLed {
public:
	RGBLed(byte pinRed, byte pinGreen, byte pinBlue);
	virtual ~RGBLed();

	void setRGB (byte red, byte green, byte blue);
	void setHSV (int h, float s, float v);
	int getHue();
	float getSat();
	float getValue();
	String print ();
private:
	void initRGB_LED (byte pinRed, byte pinGreen, byte pinBlue);

	void convertPWM_2_HSV (int *h, float *s, float *v, byte r, byte g, byte b);
	void convertHSV_2_PWM (int h, float s, float v, byte *r, byte *g, byte *b);
	void convertRGB_2_HSV (int *h, float *s, float *v, float r, float g, float b);
	void convertHSV_2_RGB (int h, float s, float v, float *r, float *g, float *b);

#ifdef _ESP32_HAL_LEDC_H_
	void analogWrite(byte pin, byte value);
	int channelRed, channelGreen, channelBlue;
#endif
	int h;
	byte pwm_RED, pwm_GREEN, pwm_BLUE;
	byte pin_RED, pin_GREEN, pin_BLUE;
	float s, v;
	boolean cycle;
	unsigned long cycleDelay;
};

#endif /* RGBLED_H_ */
