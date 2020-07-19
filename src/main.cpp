#include <LiquidCrystal.h>
#include <Arduino.h>
#include <Keypad.h>
#include <Servo.h>
#include <arduino-timer.h>

// Various variables.
int iDeflection = 48;
int iDelay = 500;
int iSpeed = 5;
bool bRunning = false;

// LCD setup
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int displayCycle = 0;

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

  Serial.print("Speed (3/6): ");
  Serial.println(iSpeed);

  Serial.print("Total cycle time (2 x (Speed * Deflection)) + Delay): ");
  Serial.println((iSpeed * iDeflection) + 15 + (iSpeed * iDeflection) + iDelay);

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
  switch (displayCycle % 4)
  {
  case 0:
    lcd.setCursor(0, 0);
    lcd.print("Deflection (1/4)");
    lcd.setCursor(0, 1);
    lcd.print(iDeflection);
    break;

  case 1:
    lcd.setCursor(0, 0);
    lcd.print("Push Speed (2,5)");
    lcd.setCursor(0, 1);
    lcd.print(iSpeed);
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
    lcd.print((iSpeed * iDeflection) + 15 + (iSpeed * iDeflection) + iDelay);
    break;
  }
  displayCycle++;

  return (true);
}

/*
** Actually press the button.
** TODO: Make this non-blockig.
*/
void doPulse()
{
  for (int i = 0; i <= iDeflection; i++)
  {
    mainServo.write(i);
    delay(iSpeed);
  }
  delay(15);
  for (int i = iDeflection; i > servoPosition; i--)
  {
    mainServo.write(i);
    delay(iSpeed);
  }
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

  displayCycle = 0;

  timer.every(10000, printSerialSetting);
  timer.every(2500, printLCDSettings);
}

/*
** Handles keypresses from the keypad. Called from loop().
*/
void parseInput(char c)
{
  switch (c)
  {
  case '1':
    iDeflection++;
    displayCycle = 0;
    break;
  case '4':
    iDeflection--;
    displayCycle = 0;
    break;
  case '2':
    iSpeed++;
    displayCycle = 1;
    break;
  case '5':
    iSpeed--;
    displayCycle = 1;
    break;
  case '3':
    iDelay += 10;
    displayCycle = 2;
    break;
  case '6':
    iDelay -= 10;
    displayCycle = 3;
    break;
  }

  // Make sure the values we've set are within sane limtes.
  iDeflection = constrain(iDeflection, 0, 180);
  iSpeed = constrain(iSpeed, 1, 1000);
  iDelay = constrain(iDelay, 10, 10000);

  // Update the LCD now. This is cheesed a bit by setting the displaycycle in the switch statement above. Sometimes this is a bit weird if the timer event triggers immediately after pressing a number on the keypad.
  printLCDSettings(0);
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

/*
void loop()
{
  // put your main code here, to run repeatedly:
  //Serial.println("Hello.");
  //delay(1000);
  //printSetting();

  // print the string when a newline arrives:
  if (stringComplete)
  {

    switch (inputString[0])
    {
    case 's':
      if (bRunning)
      {
        bRunning = false;
      }
      else
      {
        bRunning = true;
      }
      break;

    case 't':
      // If we're running, don't do anything.
      if (bRunning)
      {
        break;
      }
      else
      {
        doPulse();
      }
      break;

    case 'a':
      iDeflection++;
      break;

    case 'z':
      iDeflection--;
      break;

    case 'd':
      iDelay += 100;
      break;

    case 'c':
      iDelay -= 100;
      break;

    case 'f':
      iSpeed++;
      break;

    case 'v':
      iSpeed--;
      break;
    }
    iDeflection = constrain(iDeflection, 0, 180);
    iDelay = constrain(iDelay, 10, 1000);
    iSpeed = constrain(iSpeed, 0, 100);

    printSetting();

    // Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  if (bRunning)
  {
    doPulse();
    delay(iDelay);
  }
}
  */

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.

void serialEvent()
{
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n')
    {
      stringComplete = true;
    }
  }
}
*/
