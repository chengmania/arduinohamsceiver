/* Code by Chengmania developing a transceiver with Arduino Mega and Hamshield with
    peripherals such as a rotary encode push button, keypad matrix, 20x4 LCd display with
    adafruit spi daughterboard.  More to come.

    Hamshield code, examples, and git can be found here: https://github.com/EnhancedRadioDevices/HamShield

    and it was a Saturday.
*/
// included library for keypad matrix
#include <Keypad.h>
#include "Adafruit_LiquidCrystal.h"  //LCD Daughtboard Library
#include <SimpleRotary.h> //Rotary libray
#include <HamShield.h>//Hamsheild library

//keypad matrix////
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char hexaKeys[ROWS][COLS] = { //define the symbols on the buttons of the keypads
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'S', '0', 'E', 'D'}
};

byte rowPins[ROWS] = {53, 51, 49, 47}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {45, 43, 41, 39}; //connect to the column pinouts of the keypad
/// end Key Pad matrix/////////////

/*-----------------Rotary Knob-------------------------------------------------------------*/
#define CLK 15 // Connected to CLK on KY-040
#define DT  16 // Connected to DT on KY-040
#define SW  17 // Connected to SW

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


//initialize all instances of classes
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Connect via SPI. Data pin is #20, Clock is #21 and Latch is #19
Adafruit_LiquidCrystal lcd(20, 21, 19);

//intialize Rotary
// Pin A, Pin B, Button Pin
SimpleRotary rotary(CLK, DT, SW);


void setup() {
  // set up the LCD's number of rows and columns:
  lcd.begin(20, 4);
  lcd.setBacklight(1);
  // Print a message to the LCD.
  lcd.setCursor(0, 1);
  lcd.print("** ArduHamsceiver **");
  delay(5000);

  rotary.setDebounceDelay(5);
  rotary.setErrorDelay(5);

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
  displayMainMenu();
}
/*------------MAIN LOOP ------------------------*/
void loop() {

  char customKey = customKeypad.getKey();
  int x = 0;  //for use in testing menu items

  switch (customKey)
  {
    case '1':
      setFreq();
      displayMainMenu();
      break;

    case '2': //test code to for keypad matrix
      simplexRxTx();
      displayMainMenu();
      break;

    case '3': //test code to for keypad matrix
      weatherMenu();
      displayMainMenu();
      break;

    case '4'://test code to for keypad matrix
      repeaterTest();
      displayMainMenu();
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
  while (customKey != 'S')  //S is the Start button on keypad
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
  //freq = 162475; // default Local Weather Frequency

  //Nationwide Station Listing Using Broadcast Frequencies
  int32_t allFreqs[7] =
  {162400, 162425, 162450, 162475, 162500, 162525, 162550};

  int i = 3;
  freq = allFreqs[i];
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
  lcd.setCursor(1, 2);
  lcd.print("Press STOP to Exit");
  lcd.setCursor(7, 3);
  lcd.print("<     >");

  //Listen on the Key pad for the STOP button, which arduino knows as 'E'
  char customKey = 0;
  while (customKey != 'E')  //E is the stop button on keypad
  {
    customKey = customKeypad.getKey();

    switch (rotary.rotate()) {
      case 1:  //CW
        i++;
        i > 6 ? i = 0 : i = i; //short hand If statement to stay below 6
        freq = allFreqs[i];
        lcd.setCursor(5, 1);
        lcd.print("MHz ");
        lcd.print(float(freq) / 1000, 3);
        //delayMicroseconds(500);
        radio.frequency(freq);
        break;
      case 2:  //CCW
        i--;
        i < 0 ? i = 6 : i = i;  //short hand If statement to stay above 0
        freq = allFreqs[i];
        lcd.setCursor(5, 1);
        lcd.print("MHz ");
        lcd.print(float(freq) / 1000, 3);
        //delayMicroseconds(500);
        radio.frequency(freq);
        break;
      default:
        break;
    }
  }
  radio.setModeOff();  //Kill radio and return to main loop.
  return;
}
/*------------------------------------------------------*/

void simplexRxTx()
/* A simple simplex transmit and receive function.
    Creates a Walkie-Talkie type back and forth on a set
    frequency.  */
{
  char customKey;   //intialize keypad in function
  uint32_t newFreq = freq;

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
    customKey = customKeypad.getKey();

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

      while (rotary.push() != 1) {

        switch (rotary.rotate()) {
          case 1:
            newFreq += 25;
            break;
          case 2:
            newFreq -= 25;
            break;
          default:
            break;
        }

        lcd.setCursor(3, 2);
        lcd.print("Freq: ");
        lcd.print(float(newFreq) / 1000, 3);
        delay(1);
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
/*-------------SCAN MODE--------------------------------*/
void scanMode() {
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
  lcd.print("Scanning");


  freq = radio.scanMode(146000, 148000, 200, 250, 0);
  lcd.setCursor(3, 1);
  lcd.print("Freq: ");
  lcd.print(float(freq) / 1000, 3);

  while (customKey != 'E')
  {
    customKey = customKeypad.getKey();
  }
}

/*---------------Repeater Test --------------------------*/

////DFW!!!!/////
void repeaterTest()
{
  uint32_t txFreq = 147690;
  uint32_t rxFreq = 147090;
  float plTone = 131.80;

  radio.setCtcss(plTone);
  //radio.setCdcssSel(0);
  
  //radio.enableCtcssTx();

  char customKey;   //intialize keypad in function

  lcd.clear();
  delay(200);
  lcd.print("Press START to begin");
  while (customKey != 'S')  //Wait for START key on the keypad.
  {
    customKey = customKeypad.getKey();
  }
  initializeRadio();

  radio.frequency(rxFreq);

  radio.setSQOff();

  radio.setModeReceive();
  currently_tx = false;
  radio.setRfPower(1);
  do
  {
    customKey = customKeypad.getKey();

    if (!digitalRead(SWITCH_PIN))  //read the Hamsheild PTT button
    {
      //Currently transmitting
      if (!currently_tx)
      {
        currently_tx = true;
        radio.enableCtcss();
        radio.enableCtcssTx();

        radio.frequency(txFreq);
        // set to transmit
        radio.setModeTransmit();
        lcd.setCursor(9, 3);
        lcd.print("Tx");
        //radio.setTxSourceMic();
        //radio.setRfPower(1);
      }
      //Currently Receiving
    } else if (currently_tx) {
      
      radio.frequency(rxFreq);
      radio.setModeReceive();
      lcd.setCursor(9, 3);
      lcd.print("Rx");
      currently_tx = false;
    }
  } while (customKey != 'E');
  radio.setModeOff();
  return;
}

/*------------------------------------------------------*/
void initializeRadio()
/*method in handie talkie example sketch used to initialize the hamshield

*/
{
  // let the radio out of reset
  digitalWrite(RESET_PIN, HIGH);
  delay(5); // wait for device to come up

  lcd.setCursor(0, 0);
  lcd.print("beginning setup");
  delay(100);
  // verify connection
  lcd.setCursor(0, 1);
  lcd.print("Testing connections");
  lcd.setCursor(0, 2);
  lcd.print(radio.testConnection() ? "successful" : "failed");
  delay(2000);
  // initialize device
  lcd.clear();
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
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
void displayMainMenu()
{
  lcd.clear();
  delay(200);
  lcd.setCursor(7, 0);
  lcd.print("Menu");
  lcd.setCursor(0, 1);
  lcd.print("[1]Set Freq [2]Tx/Rx");
  lcd.setCursor(0, 2);
  lcd.print("[3]Weather  [4]Test");
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
