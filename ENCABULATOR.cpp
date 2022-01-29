#include "ENCABULATOR.h"

DIGITAL_ENCABULATOR::DIGITAL_ENCABULATOR(const char *RobotName, const char *Password) : OLED(-1), Server(80),PWM1(0x40), PWM2(0x41)
{
    robotName = RobotName;
    password = Password;
}

void DIGITAL_ENCABULATOR::Setup()
{
    GYRO.Setup();
    PWM1.begin();
    PWM2.begin();
    PWM1.setPWMFreq(500);
    PWM2.setPWMFreq(500);

    //Initialize the Display
    OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    OLED.clearDisplay();
    OLED.setCursor(0, 0);
    OLED.setTextSize(1);
    OLED.setTextColor(WHITE);
    OLED.drawBitmap(0, 0, Boot_Screen, 128, 64, WHITE);
    OLED.display();
    DisableWatchdog();
    delay(3000);
    EnableWatchdog();

    //Pat the dog
    _lastWatchdogPat = millis();

}

void DIGITAL_ENCABULATOR::ToggleWIFI()
{
    preferences.begin("yukon", false);
    bool setupWifi = preferences.getBool("setupwifi", false);
    preferences.putBool("setupwifi", !setupWifi);
    preferences.end();
    
    if(setupWifi)
        ESP.restart();
    else
        SetupWIFI();
}

void DIGITAL_ENCABULATOR::SetupWIFI()
{
    digitalWrite(39, HIGH);

    _watchdogPaused = true;
    _lastWatchdogPat = millis();

    AsyncWiFiManager wifiManager(&Server, &dns);
    //wifiManager.resetSettings();
    wifiManager.autoConnect();
    SetupOTA();

    OLED.clearDisplay();
    OLED.setCursor(0, 0);
    OLED.println(WiFi.localIP());
    OLED.println(robotName);
    OLED.display();

    delay(1000);

    Server.on("/auton", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<html><body><form method=\"post\" action=\"/post\">  Auton:<br><textarea name=\"message\" value=\"Mickey\" rows=\"20\"></textarea><br><br><input type=\"submit\" value=\"Submit\"></form></body></html>");
    });

    

    Server.begin();

    _lastWatchdogPat = millis();
    _watchdogPaused = false;
}

void DIGITAL_ENCABULATOR::Loop()
{
    GYRO.Loop();
    ArduinoOTA.handle();
    _lastWatchdogPat = millis();
}

void DIGITAL_ENCABULATOR::EnableWatchdog()
{
    _lastWatchdogPat = millis();
    _watchdogEnabled = true;
}
void DIGITAL_ENCABULATOR::DisableWatchdog()
{
    _watchdogEnabled = false;
}

void DIGITAL_ENCABULATOR::WatchdogLoop()
{
    if (_watchdogEnabled && !_watchdogPaused && millis() - _lastWatchdogPat > _watchdogBite)
    {
        Serial.println("Watchdog Bite! Restarting.");
        Serial.print(millis() - _lastWatchdogPat);
        Serial.print(" > ");
        Serial.println(_watchdogBite);
        ESP.restart();
    }
}

int PrevPercent = 0;
unsigned long PrevMillis = 0;
unsigned long LastUpdate = 0;
void DIGITAL_ENCABULATOR::SetupOTA()
{
    ArduinoOTA.setHostname(robotName);
    ArduinoOTA.setPassword(password);
    ArduinoOTA
        .onStart([this]() {
            DisableWatchdog();

            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            PrevMillis = millis();
            LastUpdate = millis();

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            //KillAllTasks();
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([this](unsigned int progress, unsigned int total) {
            if (millis() - LastUpdate > 1000)
            {
                LastUpdate = millis();
                if (PrevPercent < (progress / (total / 100)))
                {
                    PrevPercent = (progress / (total / 100));
                    //disp.WritePercentLarge(PrevPercent, 0, 0);
                    OLED.clearDisplay();
                    OLED.setTextSize(2);
                    OLED.setCursor(0, 0);
                    OLED.print(PrevPercent);
                    OLED.print("%    ");
                    OLED.print((millis() - PrevMillis) / 1000);
                    OLED.println("s");

                    for (int i = 0; i < PrevPercent; i += 11)
                    {
                        OLED.print("=");
                    }

                    OLED.display();
                }
            }
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });

    ArduinoOTA.begin();
}

int16_t DIGITAL_ENCABULATOR::ScrubInputWithParameters(int16_t JoystickValue, int16_t Deadzone, int16_t InputMin, int16_t InputMax, bool reverseInput)
{
    int16_t _inZero = abs(InputMax + InputMin) / 2;

    if (JoystickValue > _inZero + Deadzone)
    {
        JoystickValue = map(JoystickValue, _inZero + Deadzone, InputMax, 0, 255);
    }
    else if (JoystickValue < _inZero - Deadzone)
    {
        JoystickValue = map(JoystickValue, _inZero - Deadzone, InputMin, 0, -255);
    }
    else
    {
        JoystickValue = 0;
    }
    if (reverseInput)
        return JoystickValue * -1;

    return JoystickValue;
}

//Value Mappers

int DIGITAL_ENCABULATOR::PS4Joystick(StickEnum Joystick, int16_t Deadzone)
{   
    int retval = 0;

    if(ParseStick(Joystick) > Deadzone || ParseStick(Joystick) < -Deadzone)
    {
        retval = (ParseStick(Joystick))*2;
    }
    else retval = 0;
    if(retval > 255 || retval == 254) retval = 255;
    if(retval < -255) retval = -255;

    return retval;
}

bool DIGITAL_ENCABULATOR::ButtonPress(ButtonEnum b)
{
    if(ParseButton(b)) return true;
    else return false;
}

bool DIGITAL_ENCABULATOR::ButtonClick(ButtonEnum b)
{   
     
    bool retval = false;
    if(ParseButton(b) && !LastVal[b])
    {     
        retval = true;
        LastVal[b] = true;
    }

    else if(!ParseButton(b) && LastVal[b]) 
    {
        LastVal[b] = false;
        retval = false;

    }

    else if(!ParseButton(b))
    {
        retval = false;
    }

    else if(ParseButton(b) && LastVal[b])
    {
        retval = false;
    }
    
    return retval;

}

int DIGITAL_ENCABULATOR::ParseButton(ButtonEnum b)
{   
    int retval = 0;
    if(b == L1) retval = PS4.data.button.l1;
    if(b == R1) retval = PS4.data.button.r1;
    if(b == 3) retval = 0;
    if(b == 4) retval = 0;
    if(b == R3) retval = PS4.data.button.r3;
    if(b == L3) retval = PS4.data.button.l3;
    if(b == Up) retval = PS4.data.button.up;
    if(b == Left) retval = PS4.data.button.left;
    if(b == Down) retval = PS4.data.button.down;
    if(b == Right) retval = PS4.data.button.right;
    if(b == Share) retval = PS4.data.button.share;
    if(b == Touchpad) retval = PS4.data.button.touchpad;
    if(b == Options) retval = PS4.data.button.options;
    if(b == Triangle) retval = PS4.data.button.triangle;
    if(b == Square) retval = PS4.data.button.square;
    if(b == Cross) retval = PS4.data.button.cross;
    if(b == Circle) retval = PS4.data.button.circle;
    if(b == PSButton) retval = PS4.data.button.ps;
    return retval;
}

int DIGITAL_ENCABULATOR::ParseStick(StickEnum s)
{   
    int retval = 0;
    if(s == LeftHatX) retval = PS4.data.analog.stick.lx;
    if(s == LeftHatY) retval = PS4.data.analog.stick.ly;
    if(s == RightHatX) retval = PS4.data.analog.stick.rx;
    if(s == RightHatY) retval = PS4.data.analog.stick.ry;
    return retval;
}

int DIGITAL_ENCABULATOR::AnalogButton(AnalogEnum a)
{   
    int retval = 0;
    if(a == L2) retval = PS4.data.analog.button.l2;
    if(a == R2) retval = PS4.data.analog.button.r2;
    return retval;
}