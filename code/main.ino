#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int servoPin = 9;
const int buzzerPin = 13;

const int hourUpPin = A0;
const int hourDownPin = A1;
const int minuteUpPin = A2;
const int minuteDownPin = A3;
const int setPin = 6;
const int modePin = 7;

const int maxModes = 3;
int currentMode = 0;
bool setMode = false;

struct Mode {
  int hour;
  int minute;
  int servoAngle;
};

Mode modes[maxModes] = {
  {8, 0, 45},
  {12, 0, 90},
  {18, 0, 135}
};

void setup() {
  servo.attach(servoPin);
  pinMode(buzzerPin, OUTPUT);
  pinMode(hourUpPin, INPUT_PULLUP);
  pinMode(hourDownPin, INPUT_PULLUP);
  pinMode(minuteUpPin, INPUT_PULLUP);
  pinMode(minuteDownPin, INPUT_PULLUP);
  pinMode(setPin, INPUT_PULLUP);
  pinMode(modePin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Pill Dispenser");

  if (!rtc.begin()) {
    lcd.setCursor(0, 1);
    lcd.print("RTC not found!");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  delay(2000);
  lcd.clear();
}

void loop() {
  handleSetMode();

  if (!setMode) {  // Normal Operation
    DateTime now = rtc.now();

    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute() < 10) lcd.print("0");
    lcd.print(now.minute());
    lcd.print(":");
    if (now.second() < 10) lcd.print("0");
    lcd.print(now.second());

    lcd.setCursor(0, 1);
    lcd.print("Mode ");
    lcd.print(currentMode + 1);
    lcd.print(": ");
    lcd.print(modes[currentMode].hour);
    lcd.print(":");
    if (modes[currentMode].minute < 10) lcd.print("0");
    lcd.print(modes[currentMode].minute);

    if (now.hour() == modes[currentMode].hour &&
        now.minute() == modes[currentMode].minute &&
        now.second() == 0) {
      dispensePill(modes[currentMode].servoAngle);
      delay(60000);
    }
  }
}

void handleSetMode() {
  if (digitalRead(setPin) == LOW) {
    delay(200);  // Debounce
    setMode = !setMode; // Toggle set mode
    lcd.clear();
  }

  if (setMode) {
    if (digitalRead(modePin) == LOW) {
      currentMode = (currentMode + 1) % maxModes;
      delay(200);
    }

    if (digitalRead(hourUpPin) == LOW) {
      modes[currentMode].hour = (modes[currentMode].hour + 1) % 24;
      delay(200);
    }
    if (digitalRead(hourDownPin) == LOW) {
      modes[currentMode].hour = (modes[currentMode].hour - 1 + 24) % 24;
      delay(200);
    }
    if (digitalRead(minuteUpPin) == LOW) {
      modes[currentMode].minute = (modes[currentMode].minute + 1) % 60;
      delay(200);
    }
    if (digitalRead(minuteDownPin) == LOW) {
      modes[currentMode].minute = (modes[currentMode].minute - 1 + 60) % 60;
      delay(200);
    }

    lcd.setCursor(0, 0);
    lcd.print("Mode ");
    lcd.print(currentMode + 1);
    lcd.print(": Set Time");
    lcd.setCursor(0, 1);
    lcd.print(modes[currentMode].hour);
    lcd.print(":");
    if (modes[currentMode].minute < 10) lcd.print("0");
    lcd.print(modes[currentMode].minute);
  }
}

void dispensePill(int angle) {
  servo.write(angle);
  delay(1000);
  servo.write(0);
  tone(buzzerPin, 1000, 1000);
}
