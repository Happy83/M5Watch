#ifndef M5WatchLcd_h
#define M5WatchLcd_h

#include <M5StickC.h>
#include <Menu.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class M5WatchLcd {
    public:
        const int BATTERY_FULL = 5000;
        const int BATTERY_CHARGING = 4999;
        bool displayOn = true; // is display on?
        bool useBluetooth = true; // use bluetooth?
        bool alarmNow = false; // alarm is going right now
        char BLEdata[20];

        unsigned long powerUpTime = 0;

        typedef struct Alarm {
            int hour;
            int minute;
            bool set;
        };
        
        Alarm alarms[3]; // setted alarms, max 3

        M5WatchLcd();
        void init(unsigned long powerUpTime, bool wakeup);
        void redraw();
        void terminal(char* text);
        void setTime(bool redrawAll);
        void setBattery(int bat);
        void setBluetooth(bool state);
        void setBTstate(bool state);
        void setBrightness(int state);
        void setDisplayOn(bool state);
        void menu(bool state);
        void checkBTdata(char* text);
        void setAlarm(bool state);
        void setAlarmState(int index);
        void checkAlarms();
        void setAlarmMenu(char* alarm_0, char* alarm_1, char* alarm_2);
        void resetAlarmMenu();
        void sendBTstate();
    private:
        RTC_TimeTypeDef TimeStruct;
        RTC_DateTypeDef DateStruct;
        bool change; // Has to be redrawn? 
        int battery; // Battery in percent
        bool BTstate = false;// Bluetooth state (connected / disconnected)

        int EEPROMlength = 10; // brightness, 3 alarms(3 bytes each of them)
        int brightnessAddr = 0;
        int brightness = 8;
        int alarmsAddr = 1;

        int hour_low = -1;
        int hour_high = -1;
        int minute_low = -1;
        int minute_high = -1;

        bool showMenu = false;
        bool alarmSet = false;

        char* alarm_0;
        char* alarm_1;
        char* alarm_2;

        void drawStatusLine();
        void drawTime(int x, int y, int value);
        void drawDate();
        void drawBT();
        void drawAlarm();
        void drawBattery();
        unsigned char* getFontNumber(int x);

        void loadBrightness();
        void memorySave(int num, int memPos);
        int memoryLoad(int memPos);

        void checkAlarmSet();
        void addAlarm(int index, bool state, int hour, int minute);
        void removeAlarm(int index);
        void setAlarm(int index, bool state);
        void saveAlarms(int index);
        void loadAlarms();
};

#endif