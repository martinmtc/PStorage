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
	RGBLed(byte pinRed, byte pinGreen, byte pinBlue, boolean inverted);
	virtual ~RGBLed();

	void setRGB (int red, int green, int blue);
	void setHSV (int h, float s, float v);
	int getHue();
	float getSat();
	float getValue();
	int getRed();
	int getGreen();
	int getBlue();
	String print ();
private:
	void _initRGB_LED (byte pinRed, byte pinGreen, byte pinBlue, boolean inverted);

	void _convertPWM_2_HSV (int *h, float *s, float *v, int r, int g, int b);
	void _convertHSV_2_PWM (int h, float s, float v, int *r, int *g, int *b);

	void _convertRGB_2_HSV (int *h, float *s, float *v, float r, float g, float b);
	void _convertHSV_2_RGB (int h, float s, float v, float *r, float *g, float *b);

	void _enableRGB();

#ifdef _ESP32_HAL_LEDC_H_
	void analogWrite(byte pin, byte value);
	int channelRed, channelGreen, channelBlue;
#endif
	int pwm_RED, pwm_GREEN, pwm_BLUE;
	byte pin_RED, pin_GREEN, pin_BLUE;
	int h;
	float s, v;
	boolean inverted;
};

#endif /* RGBLED_H_ */
