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

uint32_t freq = 432100;

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Connect via SPI. Data pin is #20, Clock is #21 and Latch is #19
Adafruit_LiquidCrystal lcd(20, 21, 19);

void setup() {
  // set up the LCD's number of rows and columns:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.setCursor(2, 1);
  lcd.print("ArduHamsceiver");
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
    //switch case seems to fail after 2!!!!!
          
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
  uint32_t newFreq =0;

  //New Page showing the old frequency and prompt user for new frequency.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Frequency in Hz");
  lcd.setCursor(2, 2);
  lcd.print("Cur Freq: ");
  lcd.print(float(freq) / 1000, 3); // DFW display unsigned int freq as a string DFW
  lcd.setCursor(2, 3);
  lcd.print("New Freq: "); // user prompt for new frequency in kHz

  //gather info from keypad matrix.
  char customKey = 0;
  while (customKey != 'E') { //E is the stop button on keypad
    {
      if (int(customKey) != 0) {
        lcd.print(customKey);
        newFreq = (newFreq * 10) + charToNum(customKey); //change the char input to an int and udpate the newFreq variable appropriately.  
        customKey = 0;  //flush input
        x++; //TODO: add a 6 digit entry only
      }
      customKey = customKeypad.getKey();
    }
  }

  freq = newFreq;  //update to new frequency.
  lcd.clear();
  delay(100); //wait to flush the display
  
  //new page to confirm new frequency
  lcd.setCursor(4, 1);
  lcd.print("New frequency: ");
  lcd.setCursor(4, 2);
  lcd.print(float(freq) / 1000, 3); 
  delay(3000);
  return;
}

int charToNum(char customKey)
//To Be used in conjunction with the keypad to return a int value number
{
  int value;
  switch (customKey) {
    case '1':
      value = 1;
      break;
    case '2':
      value = 2;
      break;
    case '3':
      value = 3;
      break;
    case '4':
      value = 4;
      break;
    case '5':
      value = 5;
      break;
    case '6':
      value = 6;
      break;
    case '7':
      value = 7;
      break;
    case '8':
      value = 8;
      break;
    case '9':
      value = 9;
      break;
    case '0':
      value = 0;
      break;
  }
  return value;
}

