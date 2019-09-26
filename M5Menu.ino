Menu::ITEM menu_main_0; // brightness
Menu::ITEM menu_main_1; // alarms
Menu::ITEM menu_br_0; // brightness +
Menu::ITEM menu_br_1; // brightness -
Menu::ITEM menu_al_0; // alarm 1
Menu::ITEM menu_al_1; // alarm 2
Menu::ITEM menu_al_2; // alarm 3

int menuRow = 0;
int maxMenuRow = 3;
int brightness = 8;


bool changeBrightnessDown() {
  if (brightness > 7) {
    brightness--;
  }
  watch.setBrightness(brightness);
}

bool changeBrightnessUp() {
  if (brightness < 16) {
    brightness++;
  }
  watch.setBrightness(brightness);
}

bool setAlarm_0() {
  watch.setAlarmState(0);
  watch.setAlarmMenu(alarm_0, alarm_1, alarm_2);
}
bool setAlarm_1() {
  watch.setAlarmState(1);
  watch.setAlarmMenu(alarm_0, alarm_1, alarm_2);
}
bool setAlarm_2() {
  watch.setAlarmState(2);
  watch.setAlarmMenu(alarm_0, alarm_1, alarm_2);
}

void menuPreRender() {
  M5.Lcd.fillRect(0, 20, 160, 60, BLACK);
}

void renderMenuTitle(Menu::ITEM * item) {
    menuRow = 0; // first row
}

void renderMenuItem(Menu::ITEM * item, int activeUid, bool markActive, bool inlineValue) {
  int yMenu = 14 * menuRow + 22; // 14 pixels on 1 row, 22 pixels for the top menu
  String radek = "";
  if (markActive) {
    if (item->uid == activeUid) {
      radek += "> ";
    } else {
      radek += "  ";
    }
  }
  
  // show item title
  radek += item->text;

  if (item->disabled == true) {
    radek += " (disabled) ";
  }

  // show item expand symbol
  if (item->child != NULL ) {
    // radek += " + ";
  }

  if (inlineValue && item->value_cb != NULL) {
    (item->value_cb)(true);
  }
  
  M5.Lcd.setTextSize(1);  
  M5.Lcd.setCursor(2, yMenu);
  M5.Lcd.print(radek);
  menuRow++;
}

void menuExit() {
  showSettings = false;
  watch.menu(false);
  watch.setBluetooth(false);
  watch.redraw();
}


void M5MenuInit() {
  m.renderTitle = &renderMenuTitle;
  m.renderItem = &renderMenuItem;
  m.menuExit = &menuExit;
  m.preRender = &menuPreRender;

  m.addItem(&menu_main_0, NULL, "Brightness", NULL, NULL, NULL);
  m.addItem(&menu_main_1, NULL, "Alarms", NULL, NULL, NULL);
  m.addItem(&menu_br_0, NULL, "+", &menu_main_0, &changeBrightnessUp, NULL);
  m.addItem(&menu_br_1, NULL, "-", &menu_main_0, &changeBrightnessDown, NULL);
  m.addItem(&menu_al_0, NULL, alarm_0, &menu_main_1, &setAlarm_0, NULL);
  m.addItem(&menu_al_1, NULL, alarm_1, &menu_main_1, &setAlarm_1, NULL);
  m.addItem(&menu_al_2, NULL, alarm_2, &menu_main_1, &setAlarm_2, NULL);
  
  m.init(&menu_main_0);
}
