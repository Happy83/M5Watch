#include <M5WatchLcd.h>
#include <M5WatchIcos.h>
#include <M5WatchFonts.h>
#include <EEPROM.h>
#include <Arduino_JSON.h>
#include <Arduino.h>
#include <Menu.h>

// Public

M5WatchLcd::M5WatchLcd() {
};

void M5WatchLcd::init(unsigned long powerUpTime, bool wakeup) {
    this->powerUpTime = powerUpTime;
    EEPROM.begin(this->EEPROMlength);
    if (wakeup) {               // wake by button or first init
        this->loadBrightness();
    }
    this->loadAlarms();
    M5.Lcd.setRotation(3);      // rotate to 270 degrees
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Rtc.GetBm8563Time();

    this->useBluetooth = false;  // for the first setBTstate

    this->redraw();    
};

void M5WatchLcd::redraw() {
    this->drawStatusLine();
    this->setTime(true);
    this->drawDate();
    this->drawAlarm();
};

void M5WatchLcd::terminal(char* text) {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(1, 20);
    M5.Lcd.printf(text);
};

void M5WatchLcd::setTime(bool redrawAll) {
    M5.Rtc.GetTime(&this->TimeStruct);
    int hour_high = (int)(this->TimeStruct.Hours / 10);
    int hour_low = (int)(this->TimeStruct.Hours % 10);
    int minute_high = (int)(this->TimeStruct.Minutes / 10);
    int minute_low = (int)(this->TimeStruct.Minutes % 10);
    bool change = false;

    if (redrawAll) {
        M5.Lcd.fillRect(0, 19, 160, 61, BLACK);
    }

    if (redrawAll || (hour_high != this->hour_high)) {
        this->hour_high = hour_high;
        if (!this->showMenu) {
            this->drawTime(10, 30, this->hour_high);
            this->drawDate();
        }
        change = true;
    }
    if (redrawAll || (hour_low != this->hour_low)) {
        this->hour_low = hour_low;
        if (!this->showMenu) {
            this->drawTime(40, 30, this->hour_low);
        }
        change = true;
    }
    if (redrawAll || (minute_high != this->minute_high)) {
        this->minute_high = minute_high;
        if (!this->showMenu) {
            this->drawTime(90, 30, this->minute_high);
        }
        change = true;
    }
    if (redrawAll || (minute_low != this->minute_low)) {
        this->minute_low = minute_low;
        if (!this->showMenu) {
            this->drawTime(120, 30, this->minute_low);
        }
        change = true;
    }

    if (change) {
        if (!this->showMenu) {
            M5.Lcd.fillRect(76, 41, 6, 17, BLACK);
            M5.Lcd.drawXBitmap(76, 41, colon_bits, 6, 17, WHITE);
        } else {
            M5.Lcd.setTextSize(1);
            M5.Lcd.setCursor(65, 6);
            M5.Lcd.printf("%02d:%02d", this->TimeStruct.Hours, this->TimeStruct.Minutes);
        }

        M5.Rtc.GetData(&this->DateStruct);
    }
}

void M5WatchLcd::setBattery(int bat) {
    int batPercent = 0;
    if (bat == this->BATTERY_FULL || bat == this->BATTERY_CHARGING) {
        batPercent = bat;
    } else {
        batPercent = map(bat, 3300, 4125, 0, 100);
    }
    if (batPercent != this->battery) {
        this->battery = batPercent;
        this->drawBattery();
    }
};

void M5WatchLcd::setBluetooth(bool state) {
    this->useBluetooth = state;

    if (state) {
        btStart();
    } else {
        btStop();
    }
    this->drawBT();
}

void M5WatchLcd::setBTstate(bool state) {
    this->BTstate = state;
    this->drawBT();
}

void M5WatchLcd::setDisplayOn(bool state) {
    this->displayOn = state;
    if (!state) {
        M5.Axp.ScreenBreath(7); // for the lowest power consumption but time visibility
        // pinMode(GPIO_NUM_37, INPUT);
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, LOW);
        long time_to_wake = 60000000 - (millis() - this->powerUpTime) * 1000;
        if (time_to_wake < 0) {
            time_to_wake = 60000000;
        }
        esp_sleep_enable_timer_wakeup(time_to_wake); // wake up every 60 seconds
        Serial.print("Sleep for ");
        Serial.println(time_to_wake);
        esp_deep_sleep_start();
    }
}

void M5WatchLcd::setBrightness(int state) {
    M5.Axp.ScreenBreath(state);
    this->memorySave(state, this->brightnessAddr);
}

void M5WatchLcd::menu(bool state) {
    this->showMenu = state;
    if (state) {
        M5.Lcd.fillRect(0, 19, 160, 61, BLACK);
        this->setTime(true); // change date on the top to time
    }
}

void M5WatchLcd::checkBTdata(char* chrs) {
  String text = chrs;
  text.trim();
  if (text.length() == 15) { // alarm settings
      this->addAlarm(0, (text.substring(4, 5) == "1" ? true : false), text.substring(0, 2).toInt(), text.substring(2, 4).toInt());
      this->addAlarm(1, (text.substring(9, 10) == "1" ? true : false), text.substring(5, 7).toInt(), text.substring(7, 9).toInt());
      this->addAlarm(2, (text.substring(14, 15) == "1" ? true : false), text.substring(10, 12).toInt(), text.substring(12, 14).toInt());
	  this->sendBTstate();
	  this->resetAlarmMenu();
  } else if (text.length() == 14) { // datetime settings
    RTC_TimeTypeDef TimeStruct;
    RTC_DateTypeDef DateStruct;
    DateStruct.Year = text.substring(0, 4).toInt();
    DateStruct.Month = text.substring(4, 6).toInt();
    DateStruct.Date = text.substring(6, 8).toInt();
    TimeStruct.Hours   = text.substring(8, 10).toInt();
    TimeStruct.Minutes = text.substring(10, 12).toInt();
    TimeStruct.Seconds = text.substring(12, 14).toInt();
    M5.Rtc.SetData(&DateStruct);
    M5.Rtc.SetTime(&TimeStruct);
  }
//   JSONVar json = JSON.parse(text);
//   if (JSON.typeof(json) == "undefined") {
//     Serial.println("Parsing input failed!");
//     return;
//   }

//   if (json.hasOwnProperty("type")) {
//     String type = String((const char*)json["type"]);

//     if (type == "terminal") {
//       String jsonString = JSON.stringify(json);
//       char *cstr = new char[jsonString.length() + 1];
//       strcpy(cstr, jsonString.c_str());
//       Serial.println(cstr);
//       this->terminal(cstr);
//     } else if (type == "clock") {
//       RTC_TimeTypeDef TimeStruct;
//       TimeStruct.Hours   = (int)json["h"];
//       TimeStruct.Minutes = (int)json["m"];
//       TimeStruct.Seconds = (int)json["s"];
//       Serial.println(TimeStruct.Hours);
//       M5.Rtc.SetTime(&TimeStruct);
//     } else if (type == "date") {
//       RTC_DateTypeDef DateStruct;
//       DateStruct.Month = (int)json["m"];
//       DateStruct.Date = (int)json["d"];
//       DateStruct.Year = (int)json["y"];
//       M5.Rtc.SetData(&DateStruct);
//     } else if (type == "alarmA") {
//       this->addAlarm((int)json["i"], (bool)json["s"], (int)json["h"], (int)json["m"]);
//     } else if (type == "alarmR") {
//       this->removeAlarm((int)json["i"]);
//     } else if (type == "alarmS") {
//       this->setAlarm((int)json["i"], (bool)json["s"]);
//     } 
//   }
}

void M5WatchLcd::setAlarm(bool state) {
    this->alarmNow = state;
    digitalWrite(M5_LED, !state);
}

void M5WatchLcd::setAlarmState(int index) {
    this->alarms[index].set = !this->alarms[index].set;
    this->saveAlarms(index);
    this->checkAlarmSet();
}

void M5WatchLcd::checkAlarms() {
    for (int i = 0; i < 3; i++) {
        M5WatchLcd::Alarm a = this->alarms[i];
        if (a.set && a.hour == this->TimeStruct.Hours && a.minute == this->TimeStruct.Minutes) {
            this->setAlarm(true);
        }
    }
}

void M5WatchLcd::setAlarmMenu (char* alarm_0, char* alarm_1, char* alarm_2) {
	this->alarm_0 = alarm_0;
	this->alarm_1 = alarm_1;
	this->alarm_2 = alarm_2;
	this->resetAlarmMenu();
}

void M5WatchLcd::resetAlarmMenu() {
    sprintf(this->alarm_0, "%02d:%02d", this->alarms[0].hour, this->alarms[0].minute); 
    sprintf(this->alarm_1, "%02d:%02d", this->alarms[1].hour, this->alarms[1].minute); 
    sprintf(this->alarm_2, "%02d:%02d", this->alarms[2].hour, this->alarms[2].minute);
    if (this->alarms[0].set) {
        strcat(this->alarm_0, " *");
    }
    if (this->alarms[1].set) {
        strcat(this->alarm_1, " *");
    }
    if (this->alarms[2].set) {
        strcat(this->alarm_2, " *");
    }
}

void M5WatchLcd::sendBTstate() {
    Serial.println("sendBTstate");
    char x0[5];
    sprintf(x0, "%02d%02d%1d", this->alarms[0].hour, this->alarms[0].minute, this->alarms[0].set ? 1 : 0);
    strcpy(this->BLEdata, x0);
    char x1[5];
    sprintf(x1, "%02d%02d%1d", this->alarms[1].hour, this->alarms[1].minute, this->alarms[1].set ? 1 : 0);
    strcat(this->BLEdata, x1);
    char x2[5];
    sprintf(x2, "%02d%02d%1d", this->alarms[2].hour, this->alarms[2].minute, this->alarms[2].set ? 1 : 0);
    strcat(this->BLEdata, x2);
}





// Private

void M5WatchLcd::drawStatusLine() {
    M5.Lcd.drawLine(0, 18, 160, 18, WHITE);
    M5.Lcd.setCursor(1, 1);
};

void M5WatchLcd::drawBattery() {
    M5.Lcd.drawRect(135, 3, 20, 12, WHITE);
    M5.Lcd.drawRect(155, 7, 2, 6, WHITE);
    uint16_t color = BLACK;
    if (this->battery == this->BATTERY_FULL) {
        color = GREEN;
    } else if (this->battery == this->BATTERY_CHARGING) {
        color = BLUE;
    }
    M5.Lcd.fillRect(136, 4, 18, 10, color);
    
    if (this->battery < 100) {
        M5.Lcd.setTextSize(1);  
        M5.Lcd.setCursor(140, 6);
        M5.Lcd.printf("%2d", this->battery);
    }
}

void M5WatchLcd::drawTime(int x, int y, int value) {
    M5.Lcd.fillRect(x, y, 30, 40, BLACK);
    M5.Lcd.drawXBitmap(x, y, this->getFontNumber(value), 30, 40, WHITE);
}

void M5WatchLcd::drawDate() {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(65, 6);
    M5.Lcd.printf("%02d/%02d", this->DateStruct.Month, this->DateStruct.Date);
}

void M5WatchLcd::drawBT() {
    if (this->useBluetooth) {
        if (this->BTstate) {
            M5.Lcd.drawBitmap(1, 1, ble_width, ble_height, ble, BLACK);
        } else {
            M5.Lcd.drawBitmap(1, 1, ble_off_width, ble_off_height, ble_off, BLACK);
        }
    } else {
        M5.Lcd.fillRect(1, 1, 10, 16, BLACK);
    }
}

void M5WatchLcd::drawAlarm() {
    if (this->alarmSet) {
        M5.Lcd.drawBitmap(16, 1, bell_width, bell_height, bell, BLACK);
    } else {
        M5.Lcd.fillRect(16, 1, 16, 16, BLACK);
    }
}

unsigned char* M5WatchLcd::getFontNumber(int x) {
    switch(x) {
        case 0: return num0_bits;
        case 1: return num1_bits;
        case 2: return num2_bits;
        case 3: return num3_bits;
        case 4: return num4_bits;
        case 5: return num5_bits;
        case 6: return num6_bits;
        case 7: return num7_bits;
        case 8: return num8_bits;
        case 9: return num9_bits;
        default: return num0_bits;
    }
}

void M5WatchLcd::loadBrightness() {
    Serial.println("load brightness");
    this->brightness = this->memoryLoad(this->brightnessAddr);
    if (this->brightness < 7 || this->brightness > 15) {
        this->brightness = 8; // default
    }
    M5.Axp.ScreenBreath(this->brightness);
}

void M5WatchLcd::memorySave(int num, int MemPos) {
    EEPROM.write(MemPos, num);
    EEPROM.commit();
}

int M5WatchLcd::memoryLoad(int MemPos) {
    return EEPROM.read(MemPos);
}

void M5WatchLcd::checkAlarmSet() {    
    bool anyAlarm = false;
    for (int i = 0; i < 3; i++) {
        if (this->alarms[i].set) {
            anyAlarm = true;
            break;
        }
    }
    this->alarmSet = anyAlarm;
    this->drawAlarm();
}

void M5WatchLcd::addAlarm(int index, bool state, int hour, int minute) {
    this->alarms[index] = (M5WatchLcd::Alarm) { hour, minute, state };
    this->saveAlarms(index);
    this->checkAlarmSet();
}

void M5WatchLcd::removeAlarm(int index) {
    Serial.println("removeAlarm");
    this->alarms[index] = (M5WatchLcd::Alarm) { 0, 0, false };
    this->saveAlarms(index);
    this->checkAlarmSet();
}

void M5WatchLcd::setAlarm(int index, bool state) {
    Serial.println("setAlarm");
    this->alarms[index].set = state;
    this->saveAlarms(index);
    this->checkAlarmSet();
}

void M5WatchLcd::saveAlarms(int index) {
    Serial.println("Save alarms");
    int h = this->alarms[index].hour;
    byte par = 100 - this->alarms[index].hour - this->alarms[index].minute;
    if (this->alarms[index].set) {
        h |= 0x80;
    }
    this->memorySave(h, this->alarmsAddr + index * 3);
    this->memorySave(this->alarms[index].minute, this->alarmsAddr + index * 3 + 1);
    this->memorySave(par, this->alarmsAddr + index * 3 + 2);
}

void M5WatchLcd::loadAlarms() {
    Serial.println("loadAlarms");
    for (int i = 0; i < 3; i++) {   
        bool set = false;
        int h = this->memoryLoad(this->alarmsAddr + i * 3);
        if ((h & 0x80) == 0x80) {
            set = true;
            h = h ^ 0x80;
        }
        int m = this->memoryLoad(this->alarmsAddr + i * 3 + 1);
        int par = this->memoryLoad(this->alarmsAddr + i * 3 + 2);
        if ((par + m + h != 100) || (h < 0 || h > 23) || (m < 0 || m > 59)) {
            h = 0;
            m = 0;
            set = false;
        }
        this->alarms[i] = (M5WatchLcd::Alarm) { h, m, set };
    }
    this->checkAlarmSet();
}
