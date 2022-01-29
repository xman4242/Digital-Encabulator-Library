#pragma once

#include <Arduino.h>
#include <Preferences.h>

class ENCABULATOR_AUTON
{

public:
	ENCABULATOR_AUTON();
	
	void Setup();

	void StartAuton(String AutonName);
	void LaunchQueued();
	void QueueNext();
	void QueuePrev();
	void ToggleArmed();
	void ToggleLockArmed();
	bool IsArmed();
	bool IsArmLocked();
	String QueuedProgramName();

	Preferences preferences;
	
	String _RunningAuton = "";
	String _QueuedAuton = "";
	uint8_t _QueuedProgramNumber = 0;
	bool _AutonARMED = false;
	bool _AutonARMLOCKED = false;
  uint8_t _MaxProgramNumber = 0;

private:
};
