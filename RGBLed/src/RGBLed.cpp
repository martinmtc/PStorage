/*
 * RGBLed.cpp
 *
 *  Created on: 04.08.2018
 *      Author: Dr. Martin Schaaf
 */

#include "RGBLed.h"

#ifdef _ESP32_HAL_LEDC_H_
const int freq = 10000;
const int resolution = 8;
int ledChannel = 0;

void RGBLed::analogWrite(byte pin, byte dutyCycle) {
	byte channel = 0;
	if (pin == pin_BLUE ) channel = channelBlue;
	if (pin == pin_GREEN) channel = channelGreen;
	if (pin == pin_RED) channel = channelRed;
	ledcWrite(channel, dutyCycle);
}
#endif

RGBLed::RGBLed(byte pinRed, byte pinGreen, byte pinBlue) {
	initRGB_LED(pinRed, pinGreen, pinBlue);
	pinMode(pin_RED, OUTPUT);
	pinMode(pin_GREEN, OUTPUT);
	pinMode(pin_BLUE, OUTPUT);
#ifdef _ESP32_HAL_LEDC_H_
	channelRed = ledChannel++;
	channelGreen = ledChannel++;
	channelBlue = ledChannel++;
	ledcSetup(channelRed, freq, resolution);
	ledcSetup(channelGreen, freq, resolution);
	ledcSetup(channelBlue, freq, resolution);
	ledcAttachPin(pinRed, channelRed);
	ledcAttachPin(pinGreen, channelGreen);
	ledcAttachPin(pinBlue, channelBlue);
#endif
}

RGBLed::~RGBLed() {
}

void RGBLed::setRGB(byte red, byte green, byte blue) {
	pwm_RED = red; pwm_GREEN = green; pwm_BLUE = blue;
	convertPWM_2_HSV(&h, &s, &v, pwm_RED, pwm_GREEN, pwm_BLUE);
	analogWrite(pin_RED, pwm_RED);
	analogWrite(pin_GREEN, pwm_GREEN);
	analogWrite(pin_BLUE, pwm_BLUE);
}

void RGBLed::setHSV(int h, float s, float v) {
	this->h = h; this->s = s; this->v = v;
	convertHSV_2_PWM (this->h, this->s, this->v, &(this->pwm_RED), &(this->pwm_GREEN), &(this->pwm_BLUE));
	analogWrite(pin_RED, pwm_RED);
	analogWrite(pin_GREEN, pwm_GREEN);
	analogWrite(pin_BLUE, pwm_BLUE);
}

void RGBLed::initRGB_LED(byte pinRed, byte pinGreen, byte pinBlue) {
	this->pin_RED = pinRed;
	this->pin_GREEN = pinGreen;
	this->pin_BLUE = pinBlue;
	h = 0; s = 1.0; v = 1.0;
	convertHSV_2_PWM(h, s, v, &pwm_RED, &pwm_GREEN, &pwm_BLUE);
	cycle = true;
	cycleDelay = 30;
}

String RGBLed::print () {
	String ret = "";
	ret += "<Hue, Sat, Val> = ";
	ret += "<";
	ret += (String(h) + ", " + String(s, 3) + ", " + String(v, 3) + ">\n");
	ret += "<R, G, B> = ";
	ret += "<";
	ret += (String(pwm_RED) + ", " + String(pwm_GREEN) + ", " + String(pwm_BLUE) + ">\n");
	ret += "Channel <R, G, B> = ";
	ret += "<";
	ret += (String(channelRed) + ", " + String(channelGreen) + ", " + String(channelBlue) + ">\n");
	return ret;
}

void RGBLed::convertPWM_2_HSV (int *h, float *s, float *v, byte r, byte g, byte b) {
	convertRGB_2_HSV(h, s, v, float(r) / 255, float(g) / 255, float(b) / 255);
}


void RGBLed::convertHSV_2_PWM (int h, float s, float v, byte *r, byte *g, byte *b) {
	float fr, fg, fb;
	convertHSV_2_RGB(h, s, v, &fr, &fg, &fb);
	*r = byte(fr * 255); *g = byte(fg * 255); *b = byte(fb * 255);
}

void RGBLed::convertRGB_2_HSV (int *h, float *s, float *v, float r, float g, float b) {
	float max, min;
	if (r <= g) {
		min = r; max = g;
	} else {
		min = g; max = r;
	}
	if (b < min) {
		min = b;
	}
	if (b > max) {
		max = b;
	}
	// calculation of H
	if (max == min) {
		*h = 0;
	} else if (max == r) {
		*h = 60 * (0 + (g - b) / (max - min));
	} else if (max == g) {
		*h = 60 * (2 + (b - r) / (max - min));
	} else if (max == b) {
		*h = 60 * (4 + (r - g) / (max - min));
	}
	if (*h < 0) {
		*h = *h + 360;
	}
	if (max == 0.0) {
		*s = 0;
	} else {
		*s = (max - min) / max;
	}
	*v = max;
}

/*
   HSV -> RGB
   H: 0 ... 360 Grad
   S: 0 ... 1
   V: 0 ... 1
 */
void RGBLed::convertHSV_2_RGB (int h, float s, float v, float *r, float *g, float *b) {
	int hi;
	float f, p, q, t;

	// norming h to [0...360[ modula
	h = h % 360;
	hi = h / 60;  // hi = 0...5
	f = float(h % 60) / 60;
	p = v * (1 - s); q = v * (1 - s * f); t = v * (1 - s * (1 - f));
	switch (hi) {
	case 0: *r = v; *g = t; *b = p; break;
	case 1: *r = q; *g = v; *b = p; break;
	case 2: *r = p; *g = v; *b = t; break;
	case 3: *r = p; *g = q; *b = v; break;
	case 4: *r = t; *g = p; *b = v; break;
	case 5: *r = v; *g = p; *b = q; break;
	}
}

int RGBLed::getHue() {
	return h;
}

float RGBLed::getSat() {
	return s;
}

float RGBLed::getValue() {
	return v;
}
