#include "ENCABULATOR_MPU6050.h"
#include <Arduino.h>

ENCABULATOR_MPU6050::ENCABULATOR_MPU6050()
{
}

void ENCABULATOR_MPU6050::Setup()
{
    Serial.println("GYRO SETUP");

    mpu.initialize();
    //pinMode(21, INPUT);
    devStatus = mpu.dmpInitialize();

    _isInitialized = true;

    // supply your own gyro offsets here, scaled for min sensitivity
    preferences.begin("yukon", true);
    mpu.setXAccelOffset(preferences.getInt("ax_offset", 0));
    mpu.setYAccelOffset(preferences.getInt("ay_offset", 0));
    mpu.setZAccelOffset(preferences.getInt("az_offset", 0));
    
    mpu.setXGyroOffset(preferences.getInt("gx_offset", 0));
    mpu.setYGyroOffset(preferences.getInt("gy_offset", 0));
    mpu.setZGyroOffset(preferences.getInt("gz_offset", 0));
    preferences.end();

    // mpu.setXGyroOffset(95);
    // mpu.setYGyroOffset(62);
    // mpu.setZGyroOffset(-3);
    // mpu.setZAccelOffset(1539); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0)
    {
        // turn on the DMP, now that it's ready

        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection

        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    }
    else
    {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
}

void ENCABULATOR_MPU6050::Loop()
{

    // wait for MPU interrupt or extra packet(s) available
    // while (!MpuInterrupt && fifoCount < packetSize)
    // {
    // }

    // reset interrupt flag and get INT_STATUS byte
    MpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024)
    {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

        // otherwise, check for DMP data ready interrupt (this should happen frequently)
    }
    else if (mpuIntStatus & 0x02)
    {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize)
            fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);

        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;

        // display Euler angles in degrees
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        //Serial.print("ypr\t");
        // Serial.print(ypr[0] * 180 / M_PI);
        // Serial.print("\t");
        //Serial.print(ypr[1] * 180 / M_PI);
        //Serial.print("\t");
        //Serial.println(ypr[2] * 180 / M_PI);
    }

    UpdateHeading(ypr[0] * -180 / M_PI);

    // display.clearDisplay();
    // display.setTextSize(1);
    // display.setCursor(0, 0);

    // display.print(Robot.State.LeftEnc);
    // display.print(" ");
    // display.println(Robot.State.RightEnc);

    // display.print(Robot.State.LiftEnc);
    // display.print(" ");
    // display.println(Robot.State.Heading());

    // display.print("USB: ");
    // display.println(Robot.State.USBState);

    // display.print("Lift: ");
    // display.println(Robot.State.LiftSpeed);

    // display.display();

    //Lift.SetMotorSpeed(Robot.State.LiftSpeed);

    // delay(1);
}

void ENCABULATOR_MPU6050::UpdateHeading(float newHeading)
{
    float prev = _gyroDegrees;
    float dif = prev - newHeading;
    if (abs(dif) > 180)
    {
        if (dif < 0)
        {
            _gyroRotations--;
        }
        else
        {
            _gyroRotations++;
        }
    }

    _gyroDegrees = newHeading;
}

float ENCABULATOR_MPU6050::Heading()
{
    return _gyroDegrees + (360 * _gyroRotations);
}

void ENCABULATOR_MPU6050::RunCalibration()
{
    if (_isInitialized)
    {
        mpu.setXAccelOffset(0);
        mpu.setYAccelOffset(0);
        mpu.setZAccelOffset(0);
        mpu.setXGyroOffset(0);
        mpu.setYGyroOffset(0);
        mpu.setZGyroOffset(0);

        bool isComplete = false;

        while (!isComplete)
        {
            if (state == 0)
            {
                Serial.println("\nReading sensors for first time...");
                meansensors();
                state++;
                delay(1000);
            }

            if (state == 1)
            {
                Serial.println("\nCalculating offsets...");
                calibration();
                state++;
                delay(1000);
            }

            if (state == 2)
            {
                meansensors();
                Serial.println("\nFINISHED!");
                Serial.print("\nSensor readings with offsets:\t");
                Serial.print(mean_ax);
                Serial.print("\t");
                Serial.print(mean_ay);
                Serial.print("\t");
                Serial.print(mean_az);
                Serial.print("\t");
                Serial.print(mean_gx);
                Serial.print("\t");
                Serial.print(mean_gy);
                Serial.print("\t");
                Serial.println(mean_gz);
                Serial.print("Your offsets:\t");
                Serial.print(ax_offset);
                Serial.print("\t");
                Serial.print(ay_offset);
                Serial.print("\t");
                Serial.print(az_offset);
                Serial.print("\t");
                Serial.print(gx_offset);
                Serial.print("\t");
                Serial.print(gy_offset);
                Serial.print("\t");
                Serial.println(gz_offset);
                Serial.println("\nData is printed as: acelX acelY acelZ giroX giroY giroZ");
                Serial.println("Check that your sensor readings are close to 0 0 16384 0 0 0");
                Serial.println("If calibration was succesful write down your offsets so you can set them in your projects using something similar to mpu.setXAccelOffset(youroffset)");

                Serial.println("Saving Updates to Preferences");

                preferences.begin("yukon", false);
                preferences.putInt("ax_offset", ax_offset);
                preferences.putInt("ay_offset", ay_offset);
                preferences.putInt("az_offset", az_offset);
                preferences.putInt("gx_offset", gx_offset);
                preferences.putInt("gy_offset", gy_offset);
                preferences.putInt("gz_offset", gz_offset);
                preferences.end();

                Serial.println("Save Complete");


                isComplete = true;

                ESP.restart();
            }
        }
    }
}

///////////////////////////////////   FUNCTIONS   ////////////////////////////////////
void ENCABULATOR_MPU6050::meansensors()
{
    long i = 0, buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

    while (i < (buffersize + 101))
    {
        // read raw accel/gyro measurements from device
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        if (i > 100 && i <= (buffersize + 100))
        { //First 100 measures are discarded
            buff_ax = buff_ax + ax;
            buff_ay = buff_ay + ay;
            buff_az = buff_az + az;
            buff_gx = buff_gx + gx;
            buff_gy = buff_gy + gy;
            buff_gz = buff_gz + gz;
        }
        if (i == (buffersize + 100))
        {
            mean_ax = buff_ax / buffersize;
            mean_ay = buff_ay / buffersize;
            mean_az = buff_az / buffersize;
            mean_gx = buff_gx / buffersize;
            mean_gy = buff_gy / buffersize;
            mean_gz = buff_gz / buffersize;
        }
        i++;
        delay(2); //Needed so we don't get repeated measures
    }
}

void ENCABULATOR_MPU6050::calibration()
{
    ax_offset = -mean_ax / 8;
    ay_offset = -mean_ay / 8;
    az_offset = (16384 - mean_az) / 8;

    gx_offset = -mean_gx / 4;
    gy_offset = -mean_gy / 4;
    gz_offset = -mean_gz / 4;
    while (1)
    {
        int ready = 0;
        mpu.setXAccelOffset(ax_offset);
        mpu.setYAccelOffset(ay_offset);
        mpu.setZAccelOffset(az_offset);

        mpu.setXGyroOffset(gx_offset);
        mpu.setYGyroOffset(gy_offset);
        mpu.setZGyroOffset(gz_offset);

        meansensors();
        Serial.print(mean_ax);
        Serial.print("\t");
        Serial.print(mean_ay);
        Serial.print("\t");
        Serial.print(mean_az);
        Serial.print("\t");
        Serial.print(mean_gx);
        Serial.print("\t");
        Serial.print(mean_gy);
        Serial.print("\t");
        Serial.print(mean_gz);
        Serial.print("\t");

        Serial.print(ax_offset);
        Serial.print("\t");
        Serial.print(ay_offset);
        Serial.print("\t");
        Serial.print(az_offset);
        Serial.print("\t");

        Serial.print(gx_offset);
        Serial.print("\t");
        Serial.print(gy_offset);
        Serial.print("\t");
        Serial.println(gz_offset);

        if (abs(mean_ax) <= acel_deadzone)
            ready++;
        else
            ax_offset = ax_offset - mean_ax / acel_deadzone;

        if (abs(mean_ay) <= acel_deadzone)
            ready++;
        else
            ay_offset = ay_offset - mean_ay / acel_deadzone;

        if (abs(16384 - mean_az) <= acel_deadzone)
            ready++;
        else
            az_offset = az_offset + (16384 - mean_az) / acel_deadzone;

        if (abs(mean_gx) <= giro_deadzone)
            ready++;
        else
            gx_offset = gx_offset - mean_gx / (giro_deadzone + 1);

        if (abs(mean_gy) <= giro_deadzone)
            ready++;
        else
            gy_offset = gy_offset - mean_gy / (giro_deadzone + 1);

        if (abs(mean_gz) <= giro_deadzone)
            ready++;
        else
            gz_offset = gz_offset - mean_gz / (giro_deadzone + 1);

        if (ready == 6)
            break;
    }
}