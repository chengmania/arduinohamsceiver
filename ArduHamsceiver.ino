/* Code by Chengmania developing a transceiver with Arduino Mega and Hamshield with
    peripherals such as a rotary encode push button, keypad matrix, 20x4 LCd display with
    adafruit spi daughterboard.  More to come.

    Hamshield code, examples, and git can be found here: https://github.com/EnhancedRadioDevices/HamShield

    and it was a Saturday.
*/
// included library for keypad matrix
#include <Keypad.h>
// include the library code for LCD using spi connection
// #include "Wire.h"
#include "Adafruit_LiquidCrystal.h"

//Hamsheild libraies
#include <HamShield.h>

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

/*-----------------Rotary Knob-------------------------------------------------------------*/
int clk = 15; // Connected to CLK on KY-040
int dt = 16; // Connected to DT on KY-040
int sw = 17; // Connected to SW
int encoderPosCount = 0;
int clkLast;
int aVal;
boolean bCW;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 25;    // the debounce time; increase if the output flickers


//-----------------------Hamshield Stuff---------------------------------------------------//
// create object for radio
HamShield radio;
// To use non-standard pins, use the following initialization
//HamShield radio(ncs_pin, clk_pin, dat_pin);

#define LED_PIN 13
#define RSSI_REPORT_RATE_MS 5000

#define MIC_PIN 3
#define RESET_PIN A3
#define SWITCH_PIN 2

bool blinkState = false;
bool currently_tx;

unsigned long rssi_timeout;
uint32_t freq = 432100;

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Connect via SPI. Data pin is #20, Clock is #21 and Latch is #19
Adafruit_LiquidCrystal lcd(20, 21, 19);

void setup() {
  // set up the LCD's number of rows and columns:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.setCursor(0, 1);
  lcd.print("** ArduHamsceiver **");
  delay(5000);
  //Rotary Knob Setup
  pinMode (clk, INPUT);
  pinMode (dt, INPUT);
  pinMode (sw, INPUT_PULLUP);
  /* Read Pin A
    Whatever state it's in will reflect the last position
  */
  clkLast = digitalRead(clk);
  /////////////////////////////////////////////////////////////////////

  //setup Hamsheild
  // NOTE: if not using PWM out, it should be held low to avoid tx noise
  pinMode(MIC_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP); // prep the switch
  pinMode(RESET_PIN, OUTPUT);        // set up the reset control pin
  // configure Arduino LED for
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(MIC_PIN, LOW);
  digitalWrite(RESET_PIN, LOW);
  rssi_timeout = 0;

  //initializeRadio();

  lcd.clear();
  delay(100);
  //display menu on Start
  mainMenu();
}
/*------------MAIN LOOP ------------------------*/
void loop() {

  char customKey = customKeypad.getKey();
  int x = 0;  //for use in testing menu items

  switch (customKey)
  {
    case '1':
      setFreq();
      mainMenu();
      break;

    case '2': //test code to for keypad matrix
      simplexRxTx();
      mainMenu();
      break;

    case '3': //test code to for keypad matrix
      lcd.setCursor(2, 2);
      lcd.print("X");
      delay(100);
      weatherMenu();
      mainMenu();
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
      mainMenu();
      break;

    default:
      break;
  }
}
/*----------------Functions------------------------*/
void setFreq()  //Set Frequency in Radio
{
  int x = 0;  //define the length og freqChar is 6 for Mhz
  uint32_t newFreq = 0;

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
  while (customKey != 'E')  //E is the stop button on keypad
  {
    if (int(customKey) != 0) {
      lcd.print(customKey);
      newFreq = (newFreq * 10) + charToNum(customKey); //change the char input to an int.
      customKey = 0;  //flush input
      x++; //TODO: add a 6 digit entry only
    }
    customKey = customKeypad.getKey();
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
/*------------------------------------------------------*/
void weatherMenu() {

  initializeRadio();  //Turn on hamshield and prepare to recieve
  radio.setSQOff();  //Setting Squelch level to off (on seems to kill all incoming tranmission, need to determine proper squelch level and implementation.  
  freq = 162475; // Local Weather Frequency
  radio.frequency(freq);  // set radio to the frequency
  radio.setModeReceive();  //Set in receive mode throughout the function.  Being a weather receiver you can only listen. 

  //Create new and display new screen
  lcd.clear();
  delay(100);
  lcd.setCursor(3, 0);
  lcd.print("NOAA   Weather");
  lcd.setCursor(5, 1);
  lcd.print("MHz ");
  lcd.print(float(freq) / 1000, 3);
  lcd.setCursor(1, 3);
  lcd.print("Press STOP to Exit");

  //Listen on the Key pad for the STOP button, which arduino knows as 'E'  
  char customKey = 0;
  while (customKey != 'E')  //E is the stop button on keypad
  {
    customKey = customKeypad.getKey();
  }
  radio.setModeOff();  //Kill radio and return to main loop.  
  return;
}
/*------------------------------------------------------*/

void simplexRxTx() 
/* A simple simplex transmit and receive function.  
 *  Creates a Walkie-Talkie type back and forth on a set 
 *  frequency.  */
{
  char customKey;   //intialize keypad in function
  lcd.clear();
  delay(200);
  lcd.print("Press START to begin");
  while (customKey != 'S')  //Wait for START key on the keypad.  
  {
    customKey = customKeypad.getKey();
  }
  initializeRadio(); //Initialize radio
  //Diplay current Screen
  lcd.setCursor(3, 0);
  lcd.print("Simplex Rx/TX");
  lcd.setCursor(3, 1);
  lcd.print("Freq: ");
  lcd.print(float(freq) / 1000, 3);
  lcd.setCursor(1, 3);
  lcd.print("[F1] [F2]"); //show useable keys.  F1 is setfreq F2 is use knob to set freq

  //Getting ready to Rx/Tx
  radio.setSQOff();
  radio.frequency(freq);
  radio.setModeReceive();
  currently_tx = false;
  radio.setRfPower(1);
  do
  {
    if (!digitalRead(SWITCH_PIN))  //read the Hamsheild PTT button
    {
      //Currently transmitting
      if (!currently_tx) 
      {
        currently_tx = true;

        // set to transmit
        radio.setModeTransmit();
        lcd.setCursor(9, 3);
        lcd.print("Tx");
        //radio.setTxSourceMic();
        //radio.setRfPower(1);
      }
      //Currently Receiving
    } else if (currently_tx) {
      radio.setModeReceive();
      lcd.setCursor(9, 3);
      lcd.print("Rx");
      currently_tx = false;
    }
    customKey = customKeypad.getKey();
    if (customKey == 'A') {  //Set Freq with keypad.  Using the F1 Key
      setFreq();  //Goto Set Frequency function and set with keypad
      //Update frequency on dislay
      lcd.clear();
      delay(100);
      lcd.setCursor(3, 0);
      lcd.print("Simplex Rx/TX");
      lcd.setCursor(3, 1);
      lcd.print("Freq: ");
      lcd.print(float(freq) / 1000, 3);
      lcd.setCursor(0, 3);
      lcd.print("[F1][F2]");
      radio.frequency(freq);
    }
    if (customKey == 'B') { //set freq with rotary knob, using the F2 key
      uint32_t newFreq = freq;
      lcd.setCursor(3, 2);
      lcd.print("Freq: ");
      lcd.print(float(newFreq) / 1000, 3);
      while (digitalRead(sw) != LOW) {
        if ((millis() - lastDebounceTime) > debounceDelay) {
          // whatever the reading is at, it's been there for longer than the debounce
          // delay, so take it as the actual current state:
          aVal = digitalRead(clk);
          if (aVal != clkLast) { // Means the knob is rotating
            // if the knob is rotating, we need to determine direction
            // We do that by reading pin B.
            if (digitalRead(dt) != aVal) { // Means pin A Changed first - We're Rotating Clockwise.
              encoderPosCount ++;
              newFreq += 25;
              bCW = true;
            } else {// Otherwise B changed first and we're moving CCW
              bCW = false;
              encoderPosCount--;
              newFreq -= 25;
            }
            lcd.setCursor(3, 2);
            lcd.print("Freq: ");
            lcd.print(float(newFreq) / 1000, 3);
          }
        }
      }

      freq = newFreq;
      lcd.clear();
      delay(100);
      lcd.setCursor(3, 0);
      lcd.print("Simplex Rx/TX");
      lcd.setCursor(3, 1);
      lcd.print("Freq: ");
      lcd.print(float(freq) / 1000, 3);
      lcd.setCursor(0, 3);
      lcd.print("[F1][F2]");
      radio.frequency(freq);
    }


  } while (customKey != 'E');

  //shut down Radio and return to main menu
  radio.setModeOff();

  return;

}

/*------------------------------------------------------*/
void initializeRadio() 
/*method in handie talkie example sketch used to initialize the hamshield
 * 
 */
{
  // let the radio out of reset
  digitalWrite(RESET_PIN, HIGH);
  delay(5); // wait for device to come up

  lcd.setCursor(0, 0);
  lcd.print("beginning radio setup");
  delay(100);
  // verify connection
  lcd.setCursor(0, 1);
  lcd.print("Testing connections");
  lcd.setCursor(0, 2);
  lcd.print(radio.testConnection() ? "radio connection successful" : "radio connection failed");
  delay(2000);
  // initialize device
  lcd.clear();
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Initializing radio device...");
  radio.initialize(); // initializes automatically for UHF 12.5kHz channel
  delay(3000);

  radio.setVolume1(0xFF);
  radio.setVolume2(0xFF);
  radio.setRfPower(0xF);
  lcd.clear();
  delay(200);
  return;
}
/*------------------------------------------------------*/
void mainMenu()
{
  lcd.clear();
  delay(200);
  lcd.setCursor(7, 0);
  lcd.print("Menu");
  lcd.setCursor(0, 1);
  lcd.print("[1]Set Freq [2]Tx/Rx");
  lcd.setCursor(0, 2);
  lcd.print("[3]Weather  [4] Menu");
}

/*------------------------------------------------------*/
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
