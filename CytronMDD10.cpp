
#include "CytronMDD10.h"
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

CytronMDD10::CytronMDD10(int pinpwm, Adafruit_PWMServoDriver *Refpwm, int pindir, bool reversed)
{
	pwm = Refpwm;
	_reverse = reversed;

	//Pinout Mapping
	//Kind of a mess, but I don't know a better way
	//Do phone jacks in different class constructor
	if(pinpwm==1) _pinpwm = 3;
	if(pinpwm==2) _pinpwm = 1;
	if(pinpwm==3) _pinpwm = 2;
	if(pinpwm==4) _pinpwm = 0;
	if(pinpwm==5) _pinpwm = 7;
	if(pinpwm==6) _pinpwm = 5;
	if(pinpwm==7) _pinpwm = 6;
	if(pinpwm==8) _pinpwm = 4;
	if(pinpwm==9) _pinpwm = 11;
	if(pinpwm==10) _pinpwm = 9;
	if(pinpwm==11) _pinpwm = 10;
	if(pinpwm==12) _pinpwm = 8;
	if(pinpwm==13) _pinpwm = 15;
	if(pinpwm==14) _pinpwm = 13;
	if(pinpwm==15) _pinpwm = 14;
	if(pinpwm==16) _pinpwm = 12;
	if(pinpwm==17) _pinpwm = 3;
	if(pinpwm==18) _pinpwm = 1;
	if(pinpwm==19) _pinpwm = 2;
	if(pinpwm==20) _pinpwm = 0;
	if(pinpwm==21) _pinpwm = 7;
	if(pinpwm==22) _pinpwm = 5;
	if(pinpwm==23) _pinpwm = 6;
	if(pinpwm==24) _pinpwm = 4;
	//More Pins
	if(pindir==1) _pinDIR = 3;
	if(pindir==2) _pinDIR = 1;
	if(pindir==3) _pinDIR = 2;
	if(pindir==4) _pinDIR = 0;
	if(pindir==5) _pinDIR = 7;
	if(pindir==6) _pinDIR = 5;
	if(pindir==7) _pinDIR = 6;
	if(pindir==8) _pinDIR = 4;
	if(pindir==9) _pinDIR = 11;
	if(pindir==10) _pinDIR = 9;
	if(pindir==11) _pinDIR = 10;
	if(pindir==12) _pinDIR = 8;
	if(pindir==13) _pinDIR = 15;
	if(pindir==14) _pinDIR = 13;
	if(pindir==15) _pinDIR = 14;
	if(pindir==16) _pinDIR = 12;
	if(pindir==17) _pinDIR = 3;
	if(pindir==18) _pinDIR = 1;
	if(pindir==19) _pinDIR = 2;
	if(pindir==20) _pinDIR = 0;
	if(pindir==21) _pinDIR = 7;
	if(pindir==22) _pinDIR = 5;
	if(pindir==23) _pinDIR = 6;
	if(pindir==24) _pinDIR = 4;

	if(pindir < 0 || pindir > 24)
	{
		Serial.println("Direction Pinout Error!");
	}

	if(pinpwm < 0 || pinpwm > 24)
	{
		Serial.println("PWM Pinout Error!");
	}
}

void CytronMDD10::Init()
{
	pwm->setPWM(_pinpwm, 0, 0);
	pwm->setPWM(_pinDIR,0,0);
	
}

void CytronMDD10::SetMotorSpeed(float speed)
{
	if (_reverse)
		speed = speed * -1;

	if (_prevSpeed != speed)
	{
		_prevSpeed = speed;
		
		if (speed > 0)
		{
			dir = 1;
			pwm->setPWM(_pinDIR,4096,0);
		}
		else
		{
			dir = 0;
			pwm->setPWM(_pinDIR,0,4096);
		}
		
		pwm->setPWM(_pinpwm, 0, abs(map(speed, 0, 255, 0, 4095)));
	
	}
}
