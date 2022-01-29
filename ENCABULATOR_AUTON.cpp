#include "ENCABULATOR_AUTON.h"

ENCABULATOR_AUTON::ENCABULATOR_AUTON()
{

}

void ENCABULATOR_AUTON::Setup()
{
    
}

void ENCABULATOR_AUTON::StartAuton(String AutonName)
{
    _RunningAuton = AutonName;
}

void ENCABULATOR_AUTON::LaunchQueued()
{
    if (_QueuedAuton != "")
    {
        _RunningAuton = _QueuedAuton;
        _QueuedAuton = "";
        _AutonARMED = false;
    }
}
bool ENCABULATOR_AUTON::IsArmed()
{
    return _AutonARMED;
}
bool ENCABULATOR_AUTON::IsArmLocked()
{
    return _AutonARMLOCKED;
}
String ENCABULATOR_AUTON::QueuedProgramName()
{
    return _QueuedAuton;
}

void ENCABULATOR_AUTON::QueueNext()
{
    if (_QueuedProgramNumber == _MaxProgramNumber)
    {
        _QueuedProgramNumber = 0;
        _QueuedAuton = "";
    }
    else
    {
        _QueuedProgramNumber++;
        _QueuedAuton = "Auton" + String(_QueuedProgramNumber);
    }
}
void ENCABULATOR_AUTON::QueuePrev()
{
    if (_QueuedProgramNumber == 0)
    {
        _QueuedProgramNumber = _MaxProgramNumber;
        _QueuedAuton = "Auton" + String(_MaxProgramNumber);
    }
    else
    {
        _QueuedProgramNumber--;
        if (_QueuedProgramNumber > 0)
            _QueuedAuton = "Auton" + String(_QueuedProgramNumber);
        else
            _QueuedAuton = "";
    }
}
void ENCABULATOR_AUTON::ToggleArmed()
{
    _AutonARMED = !_AutonARMED;
}
