#include <LiquidCrystal.h>
#include <Arduino.h>
#include <Keypad.h>
#include <Servo.h>
#include <arduino-timer.h>

const int LCD_TIMER=2500;

// Various variables.
int iDeflection = 48;
int iDelay = 500;
int iHoldDelay = 500;
bool bRunning = false;

// LCD setup
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
int iDisplayCycle = 0;

// Keypad setup.
const byte KEYPAD_ROWS = 2;
const byte KEYPAD_COLS = 4;
char hexaKeys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'}};
byte rowPins[KEYPAD_ROWS] = {A5, A4};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

// Servo setup
Servo mainServo;
int servoPosition = 0;

// Timer setup
auto timer = timer_create_default(); // create a timer with default settings
uintptr_t timerClickTask = 0;
uintptr_t lcdTimerTask = 0;

/*
** Shows the state of affairs to the serial interface.
*/
bool printSerialSetting(void *)
{
  Serial.println("ACNH A button clicker, the debug interface.");

  Serial.print("Run State: ");
  if (bRunning)
  {
    Serial.println("Is running. A to stop.");
  }
  else
  {
    Serial.println("Is not running. B to test, A to start.");
  }
  Serial.print("Deflection (1/4): ");
  Serial.println(iDeflection);

  Serial.print("Delay (2/5): ");
  Serial.println(iDelay);

  Serial.print("HoldDelay (3/6):");
  Serial.println(iHoldDelay);

  Serial.print("Total cycle time iHoldDelay + Delay): ");
  Serial.println(iHoldDelay + iDelay);

  Serial.println();

  return (true);
}

/*
** Shows the state of things to the LCD display.
** Top line is the setting (and buttons to adjust)
** Second line is the value.
*/
bool printLCDSettings(void *)
{
  lcd.clear();
  switch (iDisplayCycle % 5)
  {
  case 0:
    lcd.setCursor(0, 0);
    lcd.print("Deflection (1/4)");
    lcd.setCursor(0, 1);
    lcd.print(iDeflection);
    break;

  case 1:
    lcd.setCursor(0, 0);
    lcd.print("Hold Delay (2,5)");
    lcd.setCursor(0, 1);
    lcd.print(iHoldDelay);
    break;

  case 2:
    lcd.setCursor(0, 0);
    lcd.print("Push Delay (3,6)");
    lcd.setCursor(0, 1);
    lcd.print(iDelay);
    break;

  case 3:
    lcd.setCursor(0, 0);
    lcd.print("Total Cycle:    ");
    lcd.setCursor(0, 1);
    lcd.print(iHoldDelay + iDelay);
    break;

  default:
    lcd.setCursor(0, 0);
    lcd.print("A start/stop.");
    if (!bRunning)
    {
      lcd.setCursor(0, 1);
      lcd.print("B to test");
    }
  }
  iDisplayCycle++;

  return (true);
}

/*
** Actually press the button.
*/
bool doPulse(void *)
{
  mainServo.write(iDeflection);
  delay(iHoldDelay);
  mainServo.write(servoPosition);

  return (true);
}

/*
** Setup activities..
*/
void setup()
{
  //set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // put your setup code here, to run once:
  Serial.begin(9600);

  // Setup the main server
  mainServo.attach(6);
  mainServo.write(0);

  Serial.println("Setup complete");
  lcd.setCursor(0, 0);
  lcd.print("ACNH AutoClicker");

  iDisplayCycle = 0;

  timer.every(10000, printSerialSetting);
  lcdTimerTask = timer.every(LCD_TIMER, printLCDSettings);
}

/*
** Handles keypresses from the keypad. Called from loop().
*/
void parseInput(char c)
{
  if (!bRunning)
  {
    switch (c)
    {
    case '1':
      iDeflection++;
      iDisplayCycle = 0;
      break;
    case '4':
      iDeflection--;
      iDisplayCycle = 0;
      break;
    case '2':
      iHoldDelay += 50;
      iDisplayCycle = 1;
      break;
    case '5':
      iHoldDelay -= 50;
      iDisplayCycle = 1;
      break;
    case '3':
      iDelay += 50;
      iDisplayCycle = 2;
      break;
    case '6':
      iDelay -= 50;
      iDisplayCycle = 2;
      break;
    case 'B':
      doPulse(0);
      break;
    }
  }

  switch (c)
  {
  case 'A':
    if (bRunning)
    {
      bRunning = false;
      if (timerClickTask != 0)
      {
        timer.cancel(timerClickTask);
        timerClickTask = 0;
      }
    }
    else
    {
      bRunning = true;
      timerClickTask = timer.every(iDelay + iHoldDelay, doPulse);
    }
    break;
  }

  // Make sure the values we've set are within sane limtes.
  iDeflection = constrain(iDeflection, 0, 180);
  iHoldDelay = constrain(iHoldDelay, 200, 10000);
  iDelay = constrain(iDelay, iHoldDelay, 10000);

  // Update the LCD now. This is cheesed a bit by setting the displaycycle in the switch statement above. Sometimes this is a bit weird if the timer event triggers immediately after pressing a number on the keypad.
  if (!bRunning)
  {
    timer.cancel(lcdTimerTask);
    printLCDSettings(0);
    lcdTimerTask = timer.every(LCD_TIMER, printLCDSettings);
  }
}

void loop()
{
  // Tick the timer.
  timer.tick();

  // Do keypad stuff.
  char customKey = customKeypad.getKey();

  if (customKey)
  {
    Serial.print("Console received: ");
    Serial.println(customKey);
    parseInput(customKey);
  }
}
