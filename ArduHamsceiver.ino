/* Code by Chengmania developing a transceiver with Arduino Mega and Hamshield with
    peripherals such as a rotary encode push button, keypad matrix, 20x4 LCd display with
    adafruit spi daughterboard.  More to come.

    Hamshield code can be found here: https://github.com/EnhancedRadioDevices/HamShield

    and it was a Saturday.
*/
// included library for keypad matrix
#include <Keypad.h>
// include the library code for LCD using spi connection
// #include "Wire.h"
#include "Adafruit_LiquidCrystal.h"

//TODO:  add hamsheild libraies and Tx/Rx code


//keypad matrix////
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

//define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'S', '0', 'E', 'D'}
};

byte rowPins[ROWS] = {53, 51, 49, 47}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {45, 43, 41, 39}; //connect to the column pinouts of the keypad
/// end Key Pad matrix

//Hamshield Stuff

unsigned int freq = 432100;

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Connect via SPI. Data pin is #20, Clock is #21 and Latch is #19
Adafruit_LiquidCrystal lcd(20, 21, 19);

void setup() {
  // set up the LCD's number of rows and columns:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  delay(5000);
  lcd.clear();
  delay(100);
}

void loop() {
  lcd.setCursor(7, 0);
  lcd.print("Menu");
  lcd.setCursor(0, 1);
  lcd.print("[1]Set Freq [2]Tx/Rx");
  lcd.setCursor(0, 2);
  lcd.print("[3]Menu 3  [4] Menu 4");

  char customKey = customKeypad.getKey();
  /*if (customKey) {
    lcd.clear();
    lcd.print(customKey);
    }*/
  switch (customKey)
  {
    case '1':
      setFreq();
      break;

    case '2': //test code to for keypad matrix
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("Menu 2");
      lcd.setCursor(2, 2);
      lcd.print("Press STOP to go back");
      customKey = customKeypad.getKey();
      int x = 0;
      while (customKey != 'E')
      {
        x++;
        lcd.setCursor(10, 3);
        lcd.print(x);
        customKey = customKeypad.getKey();
      }
      lcd.clear();
      delay(100);
      break;
    case '3': //test code to for keypad matrix
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("Menu 3");
      lcd.setCursor(2, 2);
      lcd.print("Press STOP to go back");
      customKey = customKeypad.getKey();
      while (customKey != 'E')
      {
        x++;
        lcd.setCursor(10, 3);
        lcd.print(x);
        customKey = customKeypad.getKey();
      }
      lcd.clear();
      break;
    case '4'://test code to for keypad matrix
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("Menu 4");
      lcd.setCursor(2, 2);
      lcd.print("Press STOP to go back");
      customKey = customKeypad.getKey();
      while (customKey != 'E')
      {
        x++;
        lcd.setCursor(10, 3);
        lcd.print(x);
        customKey = customKeypad.getKey();
      }
      lcd.clear();
      break;
    default:
      break;
  }
}

void setFreq()
{
  int x = 0;  //define the length og freqChar is 6 for Mhz
  char freqChar[x]; // collect keypad Characters as an array of 6 digits

  //New Page showing the old frequency and prompt user for new frequency.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Frequency in Hz");
  lcd.setCursor(2, 2);
  lcd.print("Cur Freq: ");
  lcd.print(String(freq));  // DFW display unsigned int freq as a string - doesn't display proper number DFW
  lcd.setCursor(2, 3);
  lcd.print("New Freq: "); // user prompt for new frequency in kHz

  //gather info from keypad matrix.
  char customKey = 0;
  while (customKey != 'E') { //E is the stop button on keypad
    {
      if (int(customKey) != 0) {
        lcd.print(customKey);
        freqChar[x] = customKey;
        customKey = 0;
        x++;
      }
      customKey = customKeypad.getKey();
    }
  }

  freq = atoi(freqChar);  // Arrary of Char collected above into the unsigned int freq.
  lcd.clear();
  delay(100); //wait to flush the display
  //new page to confirm new frequency
  lcd.setCursor(4, 1);
  lcd.print("New frequency: "); 
  lcd.setCursor(4, 2);
  lcd.print(String(freq)); //DFW displays new frequency - DFW doesn't display proper number
  delay(3000);
  return;
}
