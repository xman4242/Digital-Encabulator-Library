#ifndef _ENCABULATOR_MPU6050_H_
#define _ENCABULATOR_MPU6050_H_

#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <Preferences.h>

class ENCABULATOR_MPU6050
{

public:
  ENCABULATOR_MPU6050();

  MPU6050 mpu;
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  void Setup();
  void Loop();
  float Heading();

  void RunCalibration();

  static const int GyroInt = 39;
  volatile bool MpuInterrupt = false;

private:
  void UpdateHeading(float newHeading);
  float _gyroDegrees;
  int _gyroRotations = 0;
  bool _isInitialized = false;

  // MPU control/status vars
  bool dmpReady = false;  // set true if DMP init was successful
  uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
  uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
  uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
  uint16_t fifoCount;     // count of all bytes currently in FIFO
  uint8_t fifoBuffer[64]; // FIFO storage buffer

  // orientation/motion vars
  Quaternion q;        // [w, x, y, z]         quaternion container
  VectorInt16 aa;      // [x, y, z]            accel sensor measurements
  VectorInt16 aaReal;  // [x, y, z]            gravity-free accel sensor measurements
  VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
  VectorFloat gravity; // [x, y, z]            gravity vector
  float euler[3];      // [psi, theta, phi]    Euler angle container
  float ypr[3];        // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  //CALIBRATION ITEMS

  ///////////////////////////////////   CONFIGURATION   /////////////////////////////
  //Change this 3 variables if you want to fine tune the skecth to your needs.
  int buffersize = 1000; //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
  int acel_deadzone = 8; //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
  int giro_deadzone = 1; //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)


  int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz, state = 0;
  int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;

  void calibration();
  void meansensors();
  
	Preferences preferences;
};

#endif /* _ENCABULATOR_MPU6050_H_ */