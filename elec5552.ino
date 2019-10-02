// University of Western Australia
// ELEC5552 - Semester 2 2019 - Project 9
// Vihanga Silva - 21480143
// Vibhuthi Wickramage - 21498423
// Jacob Day - 21489759
// Peter-Jan Boom - 21529168
// Xia Wen - 22452416
// Congjian Sun - 22449928

//Libraries
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

//Initialisation
//--LCD
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//--LCD-END

//--EEPROM
struct storageStruct { //Structure for the stored data
    int debugBaud;
    int comBaud;
};

storageStruct storedData; //initialising a variable to hold data in memory
int dataAddress = 0; //Address where the data is stored

void eepromLoop(void);
//--EEPROM-END

//--Buttons
int btnInputPin = 0;
int lcd_key     = 0;
int adc_key_in  = 0;

//definitions for the diffent buttons
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int read_LCD_buttons(int pin);
//--Buttons-END

//--Controls
unsigned long conTimer ;
int conMenu = 0;

void displayControl(void);
//--Controls-END

//--Communications
int debugBaud;
int comBaud;

long baudRates[] = {4800, 9600, 14400, 19200, 38400, 57600, 115200};
//--Communications-END

//--General
int gpibErr = 0;
int serialErr = 0;
int genErr = 0;
//--General-END

void setup(void) {
    eepromLoop(); // to get the stored values
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    // initialize the serial communications
    Serial.begin(baudRates[debugBaud]);
    Serial1.begin(baudRates[comBaud]);
    //TODO - Need to change this over to flash memory based
}

void loop(void) {
  displayControl();
  eepromLoop();
    

}


//Other Functions
int read_LCD_buttons(int pin) {
  
 adc_key_in = analogRead(pin);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 250)  return btnUP;
 if (adc_key_in < 450)  return btnDOWN;
 if (adc_key_in < 650)  return btnLEFT;
 if (adc_key_in < 850)  return btnSELECT;

 return btnNONE;  // when all others fail, return this...
}

void displayControl(void) {
  lcd.setCursor(0, 0);                   // set the LCD cursor   position

  if (read_LCD_buttons(btnInputPin) != btnNONE) {
    conTimer = millis();

    //Debugging Code
    /*Serial.write("KEY PRESS ");
    switch (read_LCD_buttons(btnInputPin))
    {
    case 0:
      Serial.write("Right\n");
      break;
    case 1:
      Serial.write("Up\n");
      break;
    case 2:
      Serial.write("Down\n");
      break;
    case 3:
      Serial.write("Left\n");
      break;
    case 4:
      Serial.write("Select\n");
      break;
    default:
      Serial.write("None\n");
      break;
    }*/

    //int val = analogRead(0);
    //Serial.println(val);


  }
  
  if (millis() - conTimer < 5000 || conMenu == 3) {

    if (read_LCD_buttons(btnInputPin) == btnSELECT) { //Action for Select Button
      switch (conMenu) {
        case 3:
            conMenu = 0;
            Serial.println("Reset conMenu");
            break;
        default:
            conMenu++;
            lcd.clear();
            Serial.write("Select ");
            Serial.println(conMenu);
            break;
      }
    }

    if (read_LCD_buttons(btnInputPin) == btnUP) { //Action for Up Button
        switch (conMenu) { //FIXME - Sometimes doesn't change baud rate
            case 1:
                if (comBaud >= ((sizeof(baudRates)/sizeof(baudRates[0])) - 1)) {
                    comBaud = 0;
                } else {
                    comBaud++;
                }
                Serial.print("Changing Com Baud Rate to ");
                Serial.println(baudRates[comBaud]);
                eepromWrite();
                Serial1.end();
                Serial1.begin(baudRates[comBaud]);
                break;
            case 2:
                if (debugBaud >= ((sizeof(baudRates)/sizeof(baudRates[0])) - 1)) {
                    debugBaud = 0;
                } else {
                    debugBaud++;
                }
                Serial.print("Changing Debug Baud Rate to ");
                Serial.println(baudRates[debugBaud]);
                eepromWrite();
                Serial.end();
                Serial.begin(baudRates[debugBaud]);
                break;
            default:
                break;
        }
    }

    if (read_LCD_buttons(btnInputPin) == btnDOWN) { //Action for Down Button
        switch (conMenu) { //FIXME - Sometimes doesn't change baud rate
            case 1:
                if (comBaud <= 0) {
                    comBaud = ((sizeof(baudRates)/sizeof(baudRates[0])) - 1);
                } else {
                    comBaud--;
                }
                Serial.print("Changing Com Baud Rate to ");
                Serial.println(baudRates[comBaud]);
                eepromWrite();
                Serial1.end();
                Serial1.begin(baudRates[comBaud]);
                break;
            case 2:
                if (debugBaud <= 0) {
                    debugBaud = ((sizeof(baudRates)/sizeof(baudRates[0])) - 1);
                } else {
                    debugBaud--;
                }
                Serial.print("Changing Debug Baud Rate to ");
                Serial.println(baudRates[debugBaud]);
                eepromWrite();
                Serial.end();
                Serial.begin(baudRates[debugBaud]);
                break;
            default:
                break;
        }
    }

    delay(500);
    
  } else {
    conTimer = 0;
    conMenu = 0;
  }

  switch (conMenu)
  {
  case 0:
    lcd.print("Current Status");
    lcd.setCursor(0,1);
    if (gpibErr > 0) {
      lcd.print("ERR ");
      lcd.print("G:");
      lcd.print(gpibErr);
      lcd.print(" S:");
      lcd.print(serialErr);
      lcd.print(" B:");
      lcd.print(genErr);
    } else {
      lcd.print("Normal");
    }
    break;
  case 1:
    lcd.print("Coms Baud");
    lcd.setCursor(0,1);
    lcd.print(baudRates[comBaud]);
    lcd.print("    ");
    break;
  case 2:
    lcd.print("Debug Baud");
    lcd.setCursor(0,1);
    lcd.print(baudRates[debugBaud]);
    lcd.print("    ");
    break;
  case 3:
    lcd.print("Trans Data");
    lcd.setCursor(0,1);
    if (Serial.available()) {
        delay(100);
        while (Serial.available() > 0) {
            lcd.write(Serial.read());
        }
        lcd.print("          ");
    }
    break;
  default:
    conMenu = 0;
    break;
  }
}

void eepromLoop(void) {
    // TODO - EEPROM Loop Code
    EEPROM.get(dataAddress, storedData);

    //Debugging
    Serial.print("Stored Debug: ");
    Serial.println(storedData.debugBaud);
    Serial.print("Stored Debug: ");
    Serial.println(storedData.comBaud);

    //Safe Gauds for reading data that is out of range
    if ( storedData.debugBaud > ((sizeof(baudRates)/sizeof(baudRates[0])) - 1) || storedData.debugBaud < 0 ) {
        debugBaud = 6;
    } else {
        debugBaud = storedData.debugBaud;
    }

    if ( storedData.comBaud > ((sizeof(baudRates)/sizeof(baudRates[0])) - 1) || storedData.comBaud < 0 ) {
        comBaud = 6;
    } else {
        comBaud = storedData.comBaud;
    }
}

void eepromWrite(void) {
    storedData.comBaud = comBaud;
    storedData.debugBaud = debugBaud;
    EEPROM.put(dataAddress, storedData);
}