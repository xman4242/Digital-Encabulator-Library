#pragma once

class Adafruit_PWMServoDriver;
class Adafruit_MCP23017;

class CytronMDD10
{

  public:
	// CytronMDD10(int channel, int pinpwm, int pindir, bool reversed);
	CytronMDD10(int channel, Adafruit_PWMServoDriver *Refpwm, int pindir, bool reversed);

	void Init();

	void SetMotorSpeed(float speed);

  private:
	Adafruit_PWMServoDriver *pwm;
	int dir = 0;
	int _pinPWM;
	int _pinDIR;
	bool _reverse;
	int _pinpwm = 0;

	float _prevSpeed = 1;
};
