#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <M5StickC.h>
#include <Menu.h>
#include <M5WatchLcd.h>

char * alarm_0;// = "00:00";
char * alarm_1;// = "00:00";
char * alarm_2;// = "00:00";

M5WatchLcd watch;
unsigned long powerUpTime = millis();
Menu m = Menu();

bool showSettings = false;
bool showMenuAfterWakeup = false;
bool BUTTON_HOME_CLICK = false;

bool shaking = false;
bool stopShaking = false;

bool alarmDisabled = false;

float accX = 0;
float accY = 0;
float accZ = 0;



void setup() {
  Serial.begin(115200);
  bool buttonWakeUp = false;
  alarm_0 = (char*)malloc(7);
  alarm_1 = (char*)malloc(7);
  alarm_2 = (char*)malloc(7);

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0      : showMenuAfterWakeup = true; buttonWakeUp = true;
    case ESP_SLEEP_WAKEUP_EXT1      :
    case ESP_SLEEP_WAKEUP_TIMER     :
    case ESP_SLEEP_WAKEUP_TOUCHPAD  :
    case ESP_SLEEP_WAKEUP_ULP       :
    case ESP_SLEEP_WAKEUP_GPIO      :
    case ESP_SLEEP_WAKEUP_UART      : M5.begin(true, false, true); break;
    default                         : buttonWakeUp = true; M5.begin(); break;
  }
  
  M5.Axp.EnableCoulombcounter();

  pinMode(M5_BUTTON_HOME,INPUT_PULLUP);
  pinMode(M5_BUTTON_RST, INPUT);
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH);
  M5.MPU6886.Init();  // for the 'third button'
  Serial.println("START");
  watch.init(powerUpTime, buttonWakeUp);
  watch.setAlarmMenu(alarm_0, alarm_1, alarm_2);
  M5BLEinit();
  M5MenuInit();
}

void loop() {
  if(digitalRead(M5_BUTTON_HOME) == LOW){
    while(digitalRead(M5_BUTTON_HOME) == LOW);
      BUTTON_HOME_CLICK = true;
  }
  if (showMenuAfterWakeup || BUTTON_HOME_CLICK) {
      watch.setAlarm(false);
      alarmDisabled = true;
      if (!showSettings) {
        watch.menu(true);
        showSettings = true;
        m.reset();
        m.show();
        showMenuAfterWakeup = false;
        watch.setBluetooth(true);
      } else {
        m.action(m.KEY_RIGHT_ENTER);
      }
  }
  if (showSettings) {
    M5.MPU6886.getAccelData(&accX,&accY,&accZ);
    if (accX > 1 ||  accY > 1 ) { // shaking
      shaking = true;
      stopShaking = false;
    } else {
      stopShaking = true;
    }

    if (shaking && stopShaking) {
      m.action(m.KEY_LEFT); // go back
      shaking = false;
    }
    
    if(digitalRead(M5_BUTTON_RST) == LOW){
      while(digitalRead(M5_BUTTON_RST) == LOW);
        m.action(m.KEY_DOWN);
    }
  }
  BUTTON_HOME_CLICK = false;

  M5BLEloop();

  if ((M5.Axp.GetIchargeData() == 0) && (M5.Axp.GetIdischargeData() == 0)) {
    watch.setBattery(watch.BATTERY_FULL);
  } else if ((M5.Axp.GetIchargeData() > 0) && (M5.Axp.GetIdischargeData() == 0)) {
    watch.setBattery(watch.BATTERY_CHARGING);
  } else {
    watch.setBattery(M5.Axp.GetVbatData() * 1.1);
  }

  watch.setTime(false); // check if is any change in time
  if (!alarmDisabled) {
    watch.checkAlarms();
  }

  if (watch.displayOn && !showSettings && !watch.alarmNow) {
    watch.setDisplayOn(false);
  }
}
